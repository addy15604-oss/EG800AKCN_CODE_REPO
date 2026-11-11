#ifndef FILE_S_H
#define FILE_S_H

#include <stdint.h>      /* uint8_t is used below */

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_SIZE   200
#define MAX_RECORDS   3600

extern char READ_UFS_DATA[100];
extern char PHONE_NO[20];

void SDMMC_app_init(void);

void WriteFileUFS(char *Data, int FileNo);
void ReadFileUFS(char *Data, int FileNo);      /* Data must hold >= 100 bytes */
void RemoveFileUFS(int FileNo);

void WriteFileSD(char *Data, int FileNo);
void ReadFileSD(char *Data, int FileNo);
void RemoveFileUFS_Name(char *FileName);       /* full path, e.g. "U:/watechlog_3.bin" */

void Log_Init(void);                           /* call once before other tasks start */
int  Log_Write(uint8_t *data);
int  Log_Read(int index, uint8_t *out);
int  Log_Read_Count(void);
void Clear_Storage(void);

void Write_Logs_Data(void);                    /* not in FileSystem.c: must exist elsewhere if called */
void Read_Log_File(void);                      /* not in FileSystem.c: must exist elsewhere if called */

void Read_Init_Data(void);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* FILE_S_H */