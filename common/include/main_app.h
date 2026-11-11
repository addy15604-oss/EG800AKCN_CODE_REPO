#ifndef MAIN_APP_H
#define MAIN_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "ql_log.h"

/* ---- globals shared between modules ---- */
extern int      Inverter_Disconnect_Count;
extern int      FOAT_FLAG;
extern int      INVERTER_OTA_FLAG;
extern int write_index;
extern int Max_S_Count;



extern uint16_t REMOTE_INTERVEL;
extern uint32_t LAST_REMOTE_CALL;
extern uint16_t LOGGER_INTERVAL;
extern uint32_t LAST_LOGGER_CALL;

extern uint32_t  Systick;

extern char RSSI[10];
extern char SIM_NO[20];

extern char INV_DATE[12];
extern char INV_TIME[12];

extern char APP_Date[12];
extern char APP_Time[12];
extern char RTC_Date[10];
extern char RTC_Time[10];
extern char LAT_DATA[10];
extern char LON_DATA[10];
extern char SET_APN_2[20];       
extern char MOD_SOFTWARE_VERSION[10];
extern int  Final_InverterData_Interval;

/* runtime temperature string (from dev_info) */
extern char TEMPRATURE[10];

typedef struct
{
    char IMEI[16];
    char ICCID[25];
    char Operator[30];
    char APN_Name[30];
    char SET_APN[30];
    char CELL_ID[10];
    unsigned char CSQ;
    char Mobile_No[16];
    char IMSI_NO[20];
    char Module_FW[50];
} GPRS_InitTypeDef;

extern GPRS_InitTypeDef GPRSData;

/* ---- main module API ---- */
void dev_info(void);
int  main_app_init(void);                  /* kept for compatibility             */

void get_apn(char *buffer);
void VIRTUAL_AT_CMD(void);
void Seprate_Time_Date(unsigned char *data);
void Seprate_Signal(unsigned char *data);
char QSDK_Get_Str(char *src_string, char *dest_string, unsigned char index);

/* Called from main.c supervisor to kick off virtual-AT polling. */
void main_app_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */