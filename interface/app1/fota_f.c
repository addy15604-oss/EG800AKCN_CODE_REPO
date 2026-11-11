/*================================================================
  fota_f.c - FTP FOTA for Quectel EG800AK-CN (EG800AKCN91LC)

  APIs   : ql_ftp_client_* / ql_fota_image_* / ql_data_call (EG800AK SDK,
           the FTP and FOTA examples)
  Layout : your existing fota_f.c (config block, network step, server
           connect, size check, download with retries, SMS on connect
           failure, one worker task started by ql_fota_app_init()).

  The package is streamed straight into the FOTA partition, so no UFS
  temp file is needed. Resume from an offset is therefore not possible:
  every retry reconnects and restarts from byte 0.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_rtos.h"
#include "ql_power.h"
#include "sockets.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "ql_fota.h"
#include "ql_ftp_client.h"

#include "fota_f.h"
#include "FileSystem.h"      /* PHONE_NO, WriteFileUFS */
#include "ql_sms.h"        /* SendSms (set FOTA_F_SMS_EN 0 if not ported yet) */

#define FOTA_LOG(fmt, ...)   printf("[FOTA_F][%s,%d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* ---------------- configuration (was global in the old file) ---------------- */
#define FOTA_F_PROFILE_IDX      1
#define FOTA_F_APN              "airtelgprs.com"
#define FOTA_F_HOSTNAME         "184.168.124.9"          /* "ip" or "ip:port" */
#define FOTA_F_USERNAME         "EastmenOne"
#define FOTA_F_PASSWORD         "Eastmen@123"
#define FOTA_F_DEFAULT_FILE     "newout2.pack"

#define FOTA_F_START_DELAY_S    30
#define FOTA_F_TRY_TIMES        5
#define FOTA_F_RETRY_DELAY_S    10
#define FOTA_F_NET_WAIT_S       120

#define FOTA_F_TASK_STACK       (1024 * 16)
#define FOTA_F_TASK_PRIO        100                      /* same scale as ql_rtos_task_create in the SDK examples */

#define FOTA_F_SMS_EN           1                        /* SMS to PHONE_NO if FTP connect keeps failing */
#define FOTA_F_CLEAR_FLAG       1                        /* write "0" to UFS key 1 after the attempt */

/* Adjust to your SDK: some headers declare ql_power_reset(void),
 * others ql_power_reset(int type) -> then use ql_power_reset(RESET_NORMAL). */
#define FOTA_SYSTEM_RESET()     ql_power_reset()

/* exported (see fota_f.h) - FileSystem.c strcpy()s into it at boot */
char file_full_path[128] = FOTA_F_DEFAULT_FILE;

/* ---------------- state ---------------- */
typedef enum
{
    FOTAF_OK = 0,
    FOTAF_ERR_NET,
    FOTAF_ERR_CONNECT,      /* FTP login failed */
    FOTAF_ERR_FILE,         /* file missing / size unknown / bad path */
    FOTAF_ERR_DOWNLOAD,
    FOTAF_ERR_VERIFY
} fotaf_err_e;

static qlFotaImgProcCtxPtr s_fota_ctx = NULL;
static ql_task_t           s_task     = NULL;
static volatile int        s_busy     = 0;

static int s_total_len   = 0;
static int s_written_len = 0;
static int s_last_logged = 0;
static int s_failed      = 0;

int fota_f_is_busy(void)
{
    return s_busy;
}

/* ---------------- helpers ---------------- */
/* "fw/new.pack" -> dir "/fw", name "new.pack";  "new.pack" -> dir "/", name "new.pack" */
static int fotaf_split_path(const char *full, char *dir, int dir_size, char *name, int name_size)
{
    const char *p;
    int len;

    if (!full || !full[0])
        return -1;

    p = strrchr(full, '/');
    if (p == NULL)
    {
        snprintf(dir, dir_size, "/");
        snprintf(name, name_size, "%s", full);
    }
    else
    {
        len = (int)(p - full);
        if (len == 0)
            snprintf(dir, dir_size, "/");
        else if (full[0] == '/')
            snprintf(dir, dir_size, "%.*s", len, full);
        else
            snprintf(dir, dir_size, "/%.*s", len, full);
        snprintf(name, name_size, "%s", p + 1);
    }
    return (name[0] != '\0') ? 0 : -1;
}

/* ---------------- network ---------------- */
static int fotaf_data_call_up(void)
{
    struct ql_data_call_info info = {0};

    if (ql_get_data_call_info(FOTA_F_PROFILE_IDX, 0, &info) == 0 && info.v4.state == 1)
        return 1;
    return 0;
}

/* mqtt_app.c normally already holds the data call, so reuse it and only dial
 * if it is still down after a grace period.
 * NOTE: ql_wan_start() is deliberately NOT called so the network callback
 * registered by mqtt_app.c (network LED) is not replaced. */
static int fotaf_net_ready(void)
{
    int i;

    if (fotaf_data_call_up())
        return 0;

    FOTA_LOG("wait for network register");
    for (i = 0; i < 10; i++)
    {
        if (ql_network_register_wait(FOTA_F_NET_WAIT_S) == 0)
            break;
        ql_rtos_task_sleep_s(1);
    }
    if (i >= 10)
    {
        FOTA_LOG("*** network register fail ***");
        return -1;
    }

    for (i = 0; i < 15; i++)            /* give mqtt_app up to ~30 s */
    {
        if (fotaf_data_call_up())
            return 0;
        ql_rtos_task_sleep_s(2);
    }

    FOTA_LOG("starting data call");
    if (ql_start_data_call(FOTA_F_PROFILE_IDX, 0, FOTA_F_APN, NULL, NULL, 0) != 0)
    {
        FOTA_LOG("*** data call fail ***");
        return -1;
    }
    return 0;
}

/* ---------------- FTP data callback ---------------- */
/* return 1 = continue, -1 = abort */
static int fotaf_write_cb(unsigned char *data, int data_len, void *user_data)
{
    int pct;

    (void)user_data;

    if (s_failed || s_fota_ctx == NULL)
        return -1;

    if (ql_fota_image_write(s_fota_ctx, (void *)data, data_len))
    {
        FOTA_LOG("*** fota image write fail ***");
        s_failed = 1;
        return -1;
    }

    s_written_len += data_len;
    if (s_total_len > 0)
    {
        pct = (int)(((long long)s_written_len * 100) / s_total_len);
        if (pct >= s_last_logged + 10)
        {
            s_last_logged = pct - (pct % 10);
            FOTA_LOG("download %d%% (%d/%d)", pct, s_written_len, s_total_len);
        }
    }
    return 1;
}

/* ---------------- one full attempt ---------------- */
static fotaf_err_e fotaf_download_once(void)
{
    void *client = NULL;
    QL_FTP_CLIENT_FLIE_INFO_T file_list[1] = {0};
    char dir[96]  = {0};
    char name[64] = {0};
    int ret;
    fotaf_err_e err = FOTAF_ERR_DOWNLOAD;

    s_total_len = 0;
    s_written_len = 0;
    s_last_logged = 0;
    s_failed = 0;

    if (fotaf_split_path(file_full_path, dir, sizeof(dir), name, sizeof(name)) != 0)
    {
        FOTA_LOG("bad file path: [%s]", file_full_path);
        return FOTAF_ERR_FILE;
    }

    s_fota_ctx = ql_fota_init();
    if (!s_fota_ctx)
    {
        FOTA_LOG("*** fota init fail ***");
        return FOTAF_ERR_DOWNLOAD;
    }

    client = ql_ftp_client_new();
    if (!client)
    {
        FOTA_LOG("*** ftp client new fail ***");
        goto done;
    }
    ql_ftp_client_setopt(client, QL_FTP_CLIENT_OPT_PDP_CID, FOTA_F_PROFILE_IDX);

    ret = ql_ftp_client_open(client, FOTA_F_HOSTNAME, FOTA_F_USERNAME, FOTA_F_PASSWORD);
    FOTA_LOG("ftp open ret=%d", ret);
    if (ret < 0)
    {
        FOTA_LOG("ftp open error_code=%d", ql_ftp_client_get_error_code(client));
        err = FOTAF_ERR_CONNECT;
        goto done;
    }

    /* file size from the server */
    ret = ql_ftp_client_list(client, dir, name, file_list, 1);
    FOTA_LOG("ftp list [%s][%s] ret=%d size=%d", dir, name, ret, file_list[0].size);
    if (ret <= 0 || file_list[0].size <= 0)
    {
        FOTA_LOG("file not found on server or size unknown");
        err = FOTAF_ERR_FILE;
        goto done;
    }
    s_total_len = file_list[0].size;

    ret = ql_ftp_client_cwd(client, dir);
    FOTA_LOG("ftp cwd [%s] ret=%d", dir, ret);
    if (ret < 0)
    {
        err = FOTAF_ERR_FILE;
        goto done;
    }

    ret = ql_ftp_client_get(client, name, NULL, fotaf_write_cb, NULL);
    FOTA_LOG("ftp get ret=%d written=%d/%d", ret, s_written_len, s_total_len);

    ql_rtos_task_sleep_s(2);            /* let the last callback finish, as in the SDK example */
    ql_ftp_client_close(client);
    ql_ftp_client_release(client);
    client = NULL;

    if (s_failed || s_written_len != s_total_len)
    {
        FOTA_LOG("download incomplete");
        err = FOTAF_ERR_DOWNLOAD;
        goto done;
    }

    if (ql_fota_image_flush(s_fota_ctx))
    {
        FOTA_LOG("*** fota image flush fail ***");
        err = FOTAF_ERR_VERIFY;
        goto done;
    }
    if (ql_fota_image_verify(s_fota_ctx))
    {
        FOTA_LOG("*** fota image verify fail ***");
        err = FOTAF_ERR_VERIFY;
        goto done;
    }

    FOTA_LOG("package verified, update flag set");
    err = FOTAF_OK;

done:
    if (client)
    {
        ql_ftp_client_close(client);
        ql_ftp_client_release(client);
    }
    ql_fota_deinit(s_fota_ctx);
    s_fota_ctx = NULL;
    return err;
}

/* ---------------- worker task ---------------- */
static void fotaf_task(void *argv)
{
    int tries;
    fotaf_err_e err = FOTAF_ERR_NET;
    ql_task_t self;

    (void)argv;

    ql_rtos_task_sleep_s(FOTA_F_START_DELAY_S);
    FOTA_LOG("ftp fota start, file [%s]", file_full_path);

    for (tries = 1; tries <= FOTA_F_TRY_TIMES; tries++)
    {
        FOTA_LOG("attempt %d/%d", tries, FOTA_F_TRY_TIMES);

        if (fotaf_net_ready() != 0)
            err = FOTAF_ERR_NET;
        else
            err = fotaf_download_once();

        if (err == FOTAF_OK || err == FOTAF_ERR_FILE)   /* missing file: retrying will not help */
            break;

        ql_rtos_task_sleep_s(FOTA_F_RETRY_DELAY_S);
    }

#if FOTA_F_CLEAR_FLAG
    WriteFileUFS("0", 1);               /* key 1 = FOTA flag: no boot loop, no re-download after update */
#endif

    if (err == FOTAF_OK)
    {
        FOTA_LOG("FOTA ok, rebooting to apply update");
        ql_rtos_task_sleep_s(5);
        FOTA_SYSTEM_RESET();
        /* not reached */
    }

    FOTA_LOG("*** FTP FOTA failed (err=%d), staying on current firmware ***", err);
#if FOTA_F_SMS_EN
    if (err == FOTAF_ERR_CONNECT)
    {
        SendSms(PHONE_NO, "FTP Process Faild\nUnsuccessful connection to FTP server");
    }
#endif

    self = s_task;
    s_task = NULL;
    s_busy = 0;
    ql_rtos_task_delete(self);
}

/* ---------------- entry ---------------- */
void ql_fota_app_init(void)
{
    int err;

    if (s_busy)
    {
        FOTA_LOG("FTP FOTA already running");
        return;
    }

    s_busy = 1;
    err = ql_rtos_task_create(&s_task, FOTA_F_TASK_STACK, FOTA_F_TASK_PRIO,
                              "QfotaFtp", fotaf_task, NULL);
    if (err != 0)
    {
        FOTA_LOG("created task failed (%d)", err);
        s_task = NULL;
        s_busy = 0;
    }
}