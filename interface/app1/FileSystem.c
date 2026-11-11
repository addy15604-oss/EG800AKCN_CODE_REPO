/*================================================================
  FileSystem.c - UFS storage layer for Quectel EG800AK-CN

  Layout : your existing FileSystem.c (Log_*, Write/ReadFileUFS,
           Read_Init_Data, same key numbers, same file format).
  APIs   : ql_fs.h style from the SDK examples
           (QFILE *, "U:/..." paths, ql_fwrite(buf,size,1,fp), NULL on
           open failure instead of fd < 0).

  Public function names and signatures are unchanged, so FileSystem.h
  and every caller (inverter, mqtt, gps, ota) stay as they are.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "fs/ql_fs.h"
#include "ql_application.h"
#include "ql_rtos.h"

#include "FileSystem.h"
#include "main_app.h"
#include "inverter_app.h"
// #include "gps.h"
#include "mqtt_app.h"
#include "fota_f.h"
#include "fota_http.h"      /* HTTP_DLOAD_URL, HTTP_FILE_NAME */
#include "inverterOta.h"

#define FS_LOG(fmt, ...)    printf("[FS][%s,%d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* ---------------- disks / paths ---------------- */
#define FS_DISK_UFS         "U:"                 /* internal flash */
#define FS_DISK_SD          "SD:"                /* only if FS_USE_SD */
#define FS_USE_SD           0

#define LOG_FILE            FS_DISK_UFS "/watechlog_3.bin"
#define FS_PATH_MAX         32
#define FS_VAL_MAX          100                  /* size of READ_UFS_DATA */

/* ---------------- globals kept from the old module ---------------- */
char PHONE_NO[20] = "";
char READ_UFS_DATA[FS_VAL_MAX] = "";
int  CheckVal = 0;

static ql_mutex_t s_fs_mutex = NULL;

#define FS_LOCK()    do { if (s_fs_mutex) ql_rtos_mutex_lock(s_fs_mutex, QL_WAIT_FOREVER); } while (0)
#define FS_UNLOCK()  do { if (s_fs_mutex) ql_rtos_mutex_unlock(s_fs_mutex); } while (0)

/*================================================================
  internal helpers (no locking)
 ================================================================*/
static void fs_make_path(char *out, int size, const char *disk, int file_no)
{
    snprintf(out, size, "%s/FILE_%d.txt", disk, file_no);
}

static int fs_write_text(const char *path, const char *data)
{
    QFILE *fp;
    int len, ret;

    if (!path || !data)
        return -1;

    fp = ql_fopen(path, "w+");               /* truncate + write */
    if (fp == NULL)
    {
        FS_LOG("open failed: %s", path);
        return -1;
    }

    len = (int)strlen(data);                 /* '\0' not stored */
    if (len > 0)
    {
        ret = ql_fwrite((void *)data, len, 1, fp);
        if (ret < 0)
        {
            FS_LOG("write failed: %s", path);
            ql_fclose(fp);
            return -2;
        }
    }
    ql_fclose(fp);
    FS_LOG("wrote [%s] -> %s", data, path);
    return 0;
}

/* out is always '\0' terminated; at most out_size-1 bytes are read */
static int fs_read_text(const char *path, char *out, int out_size)
{
    QFILE *fp;
    int size, ret;

    if (!path || !out || out_size < 2)
        return -1;
    out[0] = '\0';

    fp = ql_fopen(path, "r");
    if (fp == NULL)
    {
        FS_LOG("open failed (file missing?): %s", path);
        return -1;
    }

    size = (int)ql_fsize(fp);
    if (size <= 0)
    {
        ql_fclose(fp);
        return 0;
    }
    if (size > out_size - 1)
        size = out_size - 1;

    ql_fseek(fp, 0, SEEK_SET);
    ret = ql_fread((void *)out, size, 1, fp);
    if (ret < 0)
    {
        FS_LOG("read failed: %s", path);
        out[0] = '\0';
        ql_fclose(fp);
        return -2;
    }
    out[size] = '\0';
    ql_fclose(fp);
    FS_LOG("read [%s] <- %s", out, path);
    return 0;
}

static void fs_read_key(int file_no, char *out, int out_size)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_UFS, file_no);
    fs_read_text(path, out, out_size);
}

/*================================================================
  LOG STORAGE  (fixed-size circular records)
 ================================================================*/
void Log_Init()
{
    QFILE *fp;

    if (s_fs_mutex == NULL)
        ql_rtos_mutex_create(&s_fs_mutex);   /* call Log_Init() before other tasks start */

    FS_LOCK();
    fp = ql_fopen(LOG_FILE, "r");
    if (fp == NULL)
    {
        FS_LOG("%s not found, creating", LOG_FILE);
        fp = ql_fopen(LOG_FILE, "w+");
        if (fp == NULL)
        {
            FS_LOG("create failed: %s", LOG_FILE);
            FS_UNLOCK();
            return;
        }
    }
    ql_fclose(fp);
    FS_UNLOCK();
    FS_LOG("init done");
}

int Log_Write(uint8_t *data)
{
    QFILE *fp;
    char  idx[10] = {0};
    char  path[FS_PATH_MAX];
    long  offset;
    int   written, ret = 0;

    if (data == NULL)
    {
        FS_LOG("NULL data");
        return -10;
    }

    FS_LOCK();

    fp = ql_fopen(LOG_FILE, "r+");
    if (fp == NULL)
    {
        FS_LOG("log file missing, creating");
        fp = ql_fopen(LOG_FILE, "w+");
        if (fp == NULL) { ret = -1; goto out; }
        ql_fclose(fp);
        fp = ql_fopen(LOG_FILE, "r+");
        if (fp == NULL) { ret = -2; goto out; }
    }

    offset = (long)write_index * RECORD_SIZE;
    FS_LOG("index=%d offset=%ld", write_index, offset);

    if (ql_fseek(fp, offset, SEEK_SET) < 0)
    {
        ql_fclose(fp);
        ret = -2;
        goto out;
    }

    written = ql_fwrite((void *)data, RECORD_SIZE, 1, fp);
    ql_fclose(fp);
    if (written <= 0)
    {
        FS_LOG("record write failed (%d)", written);
        ret = -3;
        goto out;
    }

    write_index++;
    if (write_index >= MAX_RECORDS)
    {
        write_index = 0;
        FS_LOG("restarted from beginning");
    }

    snprintf(idx, sizeof(idx), "%d", write_index);
    fs_make_path(path, sizeof(path), FS_DISK_UFS, 10);   /* 10 = write index */
    fs_write_text(path, idx);

out:
    FS_UNLOCK();
    return ret;
}

int Log_Read(int index, uint8_t *out)
{
    QFILE *fp;
    int size, total, r, ret = 0;
    long offset;

    if (!out)
    {
        FS_LOG("null buffer");
        return -10;
    }

    FS_LOCK();

    fp = ql_fopen(LOG_FILE, "r");
    if (fp == NULL)
    {
        FS_LOG("open failed");
        ret = -1;
        goto out;
    }

    size = (int)ql_fsize(fp);
    if (size <= 0)
    {
        FS_LOG("empty file");
        ql_fclose(fp);
        ret = -2;
        goto out;
    }
    if (size % RECORD_SIZE != 0)
        FS_LOG("WARNING misaligned size=%d rec=%d", size, RECORD_SIZE);

    total = size / RECORD_SIZE;
    if (index < 0 || index >= total)
    {
        FS_LOG("invalid index=%d max=%d", index, total - 1);
        ql_fclose(fp);
        ret = -3;
        goto out;
    }

    offset = (long)index * RECORD_SIZE;
    if (ql_fseek(fp, offset, SEEK_SET) < 0)
    {
        FS_LOG("seek failed");
        ql_fclose(fp);
        ret = -4;
        goto out;
    }

    r = ql_fread((void *)out, RECORD_SIZE, 1, fp);
    ql_fclose(fp);
    if (r <= 0)
    {
        FS_LOG("read failed (%d)", r);
        ret = -5;
        goto out;
    }
    FS_LOG("OK index=%d first=0x%02X last=0x%02X", index, out[0], out[RECORD_SIZE - 1]);

out:
    FS_UNLOCK();
    return ret;
}

int Log_Read_Count()
{
    QFILE *fp;
    int size, total = 0;

    FS_LOCK();
    fp = ql_fopen(LOG_FILE, "r");
    if (fp == NULL)
    {
        FS_LOG("open failed");
        FS_UNLOCK();
        return -1;
    }

    size = (int)ql_fsize(fp);
    ql_fclose(fp);
    FS_UNLOCK();

    if (size <= 0)
        return 0;
    if (size % RECORD_SIZE != 0)
        FS_LOG("WARNING misaligned size=%d rec=%d", size, RECORD_SIZE);

    total = size / RECORD_SIZE;
    FS_LOG("total records=%d", total);
    return total;
}

void Clear_Storage()
{
    char path[FS_PATH_MAX];

    FS_LOCK();
    ql_remove(LOG_FILE);
    write_index = 0;
    fs_make_path(path, sizeof(path), FS_DISK_UFS, 10);
    fs_write_text(path, "0");
    FS_UNLOCK();
    FS_LOG("LOG CLEARED");
}

/*================================================================
  SIMPLE KEY FILES  (U:/FILE_<n>.txt, plain text)
 ================================================================*/
void WriteFileUFS(char *Data, int FileNo)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_UFS, FileNo);
    FS_LOCK();
    fs_write_text(path, Data);
    FS_UNLOCK();
}

/* Data must hold at least FS_VAL_MAX (100) bytes, as before */
void ReadFileUFS(char *Data, int FileNo)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_UFS, FileNo);
    FS_LOCK();
    fs_read_text(path, Data, FS_VAL_MAX);    /* empty string if missing */
    FS_UNLOCK();
}

void RemoveFileUFS(int FileNo)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_UFS, FileNo);
    FS_LOCK();
    if (ql_remove(path) < 0)
        FS_LOG("remove failed: %s", path);
    FS_UNLOCK();
}

/* FileName is a full path, e.g. "U:/watechlog_3.bin" */
void RemoveFileUFS_Name(char *FileName)
{
    FS_LOCK();
    if (ql_remove(FileName) < 0)
        FS_LOG("remove failed: %s", FileName);
    FS_UNLOCK();
}

#if FS_USE_SD
void WriteFileSD(char *Data, int FileNo)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_SD, FileNo);
    FS_LOCK();
    fs_write_text(path, Data);
    FS_UNLOCK();
}

void ReadFileSD(char *Data, int FileNo)
{
    char path[FS_PATH_MAX];

    fs_make_path(path, sizeof(path), FS_DISK_SD, FileNo);
    FS_LOCK();
    fs_read_text(path, Data, FS_VAL_MAX);
    FS_UNLOCK();
}
#else
/* SD card not used on this board: keep the symbols so callers still link */
void WriteFileSD(char *Data, int FileNo)
{
    (void)Data;
    FS_LOG("SD disabled, file %d not written", FileNo);
}

void ReadFileSD(char *Data, int FileNo)
{
    FS_LOG("SD disabled, file %d not read", FileNo);
    if (Data)
        Data[0] = '\0';
}
#endif

/* Kept from the old module (it was an empty stub there too) */
void SDMMC_app_init(void)
{
}

/*================================================================
  BOOT-TIME SETTINGS  (same key numbers as before)
    1 FOTA flag        2 phone no.        3 last lat      4 last lon
    5 data interval    6 inverter OTA     7 inverter OTA file path
    8 module OTA path  9 store interval  10 write index   11 HTTP url
 ================================================================*/
void Read_Init_Data()
{
    int val;

    /* 1: FOTA flag */
    fs_read_key(1, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("FOTA VAL_UFS:%s", READ_UFS_DATA);
    CheckVal = atoi(READ_UFS_DATA);
    if (CheckVal == 1)
{
    FOAT_FLAG = 1;
    FS_LOG("FOTA FLAG SET TO 1");
    RemoveFileUFS(1);          /* consume the flag so it cannot loop */
}

    /* 2: phone number */
    // fs_read_key(2, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    // strncpy(PHONE_NO, READ_UFS_DATA, sizeof(PHONE_NO) - 1);
    // PHONE_NO[sizeof(PHONE_NO) - 1] = '\0';
    // FS_LOG("PHONE_NO_UFS:%s", PHONE_NO);

    // /* 3, 4: last latitude / longitude */
    // fs_read_key(3, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    // FS_LOG("LAT_DATA_UFS:%s", READ_UFS_DATA);
    // last_lat = atof(READ_UFS_DATA);

    // fs_read_key(4, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    // FS_LOG("LON_DATA_UFS:%s", READ_UFS_DATA);
    // last_lon = atof(READ_UFS_DATA);

    // if (last_lat > 10.00 && last_lon > 10.00)
    // {
    //     FIRST_CHECK = FALSE;
    // }
    // else
    // {
    //     sprintf(LAT_DATA, "%0.6f", last_lat);
    //     sprintf(LON_DATA, "%0.6f", last_lon);
    //     FIRST_CHECK = TRUE;
    // }

    /* 5: inverter data interval (s) */
    fs_read_key(5, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("INTERVAL VAL_UFS:%s", READ_UFS_DATA);
    val = atoi(READ_UFS_DATA);
    if (val > 10 && val < 3600)
    {
        Final_InverterData_Interval = val;
        FS_LOG("INTERVAL SET TO %d s", Final_InverterData_Interval);
    }
    else
    {
        FS_LOG("INTERVAL DEFAULT %d s", Final_InverterData_Interval);
    }

    /* 6: inverter OTA flag (1 = target 1, 2 = target 2) */
       /* 6: inverter OTA flag (1 = target 1, 2 = target 2) */
    fs_read_key(6, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("INVERTER OTA VAL_UFS:%s", READ_UFS_DATA);
    CheckVal = atoi(READ_UFS_DATA);
    if (CheckVal == 1)
    {
        INVERTER_OTA_FLAG = 1;
        OTA_MODBUS_TARGET = 0;
        FS_LOG("INVERTER OTA FLAG SET TO 1");
        RemoveFileUFS(6);      /* consume the flag so it cannot loop */
    }
    else if (CheckVal == 2)
    {
        INVERTER_OTA_FLAG = 2;
        OTA_MODBUS_TARGET = 1;
        FS_LOG("INVERTER OTA FLAG SET TO 2");
        RemoveFileUFS(6);      /* consume the flag so it cannot loop */
    }
    /* 7: inverter OTA file path */
    fs_read_key(7, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("STORED_OTA_FILE_PATH_UFS:%s", READ_UFS_DATA);
    if (strlen(READ_UFS_DATA) > 1)
    {
        strcpy(INV_OTA_URL_SUFFIX, READ_UFS_DATA);
        FS_LOG("UPDATED OTA FILE:%s", INV_OTA_URL_SUFFIX);
    }

    /* 8: module OTA file path */
    fs_read_key(8, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("STORED_MODULE_OTA_FILE_PATH_UFS:%s", READ_UFS_DATA);
    if (strlen(READ_UFS_DATA) > 1)
    {
        strcpy(file_full_path, READ_UFS_DATA);
        strcpy(HTTP_FILE_NAME, READ_UFS_DATA);
        FS_LOG("UPDATED FILE PATH:%s", file_full_path);
    }

    /* 9: storing interval */
    fs_read_key(9, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("ST INTERVAL VAL_UFS:%s", READ_UFS_DATA);
    val = atoi(READ_UFS_DATA);
    if (val > 1 && val < 10000)
    {
        Max_S_Count = val;
        FS_LOG("ST INTERVAL SET TO %d", Max_S_Count);
    }
    else
    {
        FS_LOG("ST INTERVAL DEFAULT %d x10 s", Max_S_Count);
    }

    /* 10: write index */
    fs_read_key(10, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("WRITE INDEX VAL_UFS:%s", READ_UFS_DATA);
    write_index = (strlen(READ_UFS_DATA) > 0) ? atoi(READ_UFS_DATA) : 0;
    if (write_index < 0 || write_index >= MAX_RECORDS)
        write_index = 0;

    /* 11: HTTP base url */
    fs_read_key(11, READ_UFS_DATA, sizeof(READ_UFS_DATA));
    FS_LOG("STORED_HTTP_URL_UFS:%s", READ_UFS_DATA);
    if (strlen(READ_UFS_DATA) > 1)
    {
        strcpy(HTTP_DLOAD_URL, READ_UFS_DATA);
        FS_LOG("UPDATED HTTP_URL:%s", HTTP_DLOAD_URL);
    }
}