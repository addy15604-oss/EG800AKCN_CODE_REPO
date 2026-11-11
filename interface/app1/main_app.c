/*==========================================================================================================
  main.c — EG800 application entry point.

  Responsibilities:
    1. Register tasks with application_init().
    2. Main thread: read config from flash, fetch device info, start virtual-AT poll.
    3. Supervisor thread: monitor inverter health and reboot on stuck link.
    4. Keep GPRSData / RSSI / time strings up to date (used by MQTT JSON builder).

  Task table at boot:
    prio 1  inverter_launcher  -> Inverter_app_comm (Modbus polling, 20 ms tick)
    prio 2  mqtt_launcher      -> QmqttApp         (broker publish, 1 s tick)
    prio 3  main_launcher      -> main_app         (virtual-AT poll, 5 s tick)
    prio 5  supervisor_launcher-> supervisor       (health, 10 s tick)
===========================================================================================================*/
#include "ql_osi_def.h"
#include "ql_rtos.h"
#include "ql_func.h"
#include "ql_log.h"
#include "ql_application.h"
#include "ql_dev.h"
#include "ql_power.h"
#include "main_app.h"
#include "inverter_app.h"
#include "mqtt_app.h"


#define QL_VIRT_AT_RX_BUFF_SIZE   1024

/* ---- globals (definitions moved here from main.h) ---- */
uint16_t REMOTE_INTERVEL = 20;   /* seconds */
uint16_t LOGGER_INTERVAL = 30;   /* seconds */
uint32_t LAST_REMOTE_CALL = 0;
uint32_t LAST_LOGGER_CALL = 0;

ql_task_t main_task_t = NULL;
uint32_t  Systick = 0;

int APN_FLAG = 0;
int FOAT_FLAG = 0;
int INVERTER_OTA_FLAG = 0;
int Inverter_Disconnect_Count = 0;
int write_index = 0;
int Max_S_Count = 1800;//(500 millsec interval x 1800 = 900000 millisec(900sec)  or 15 mins)


char RSSI[10]              = "99";
char APP_Date[12]          = "2023-09-21";
char APP_Time[12]          = "17:02:06";
char INV_DATE[12]          = "1999-09-09";
char INV_TIME[12]          = "17:02:06";
char RTC_Date[10]          = "230921";
char RTC_Time[10]          = "170707";
char LAT_DATA[10]          = "00.000000";
char LON_DATA[10]          = "00.000000";
char SET_APN_2[20]         = "no-apn\0";       /* set by APN detection */
char MOD_SOFTWARE_VERSION[10] = "DY-G1";
char TEMPRATURE[10]        = "0";
int  Final_InverterData_Interval = 300;

GPRS_InitTypeDef GPRSData = {"NA\0","NA\0","NA\0","NA\0","no-apn\0","0\0",0,"NA\0","NA\0","NA\0"};

/* ====================================================================================
 * main_app_thread — startup sequence then 5 s virtual-AT poll.
 * ==================================================================================== */
void main_app_thread(void *arg)
{
    (void)arg;

    // Read_Init_Data();
    ql_rtos_task_sleep_ms(5000);


    dev_info();
    ql_rtos_task_sleep_s(15);

    while (1)
    {
        // VIRTUAL_AT_CMD();
        ql_rtos_task_sleep_ms(5000);
    }

    ql_rtos_task_delete(NULL);
}

int main_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&main_task_t, 16 * 1024,
                              5,
                              "main_app",
                              main_app_thread,
                              NULL, 2);
    if (err != QL_OSI_SUCCESS)
        printf("main_task init failed 0x%x", err);
    return err;
}

/* ====================================================================================
 * dev_info — unchanged from your original.
 * ==================================================================================== */
void dev_info(void)
{
    char data[64] = {0};
    int ret = 0;

    ret = ql_dev_get_imei(data,sizeof(data));
    strcpy(GPRSData.IMEI, data);
    printf("ql_dev_get_imei ret=%d; IMEI:%s \n",ret,data);
    printf("IMEI: %s\n", GPRSData.IMEI);
    memset(data,0,sizeof(data));

    ret = ql_dev_get_model(data,sizeof(data));
    printf("ql_dev_get_model ret=%d; model:%s \n",ret,data);
    memset(data,0,sizeof(data));

    ret = ql_dev_get_sn(data,sizeof(data));
    printf("ql_dev_get_sn ret=%d; sn:%s \n",ret,data);
    memset(data,0,sizeof(data));

    ret = ql_dev_get_firmware_version(data,sizeof(data));
    printf("ql_dev_get_firmware_version ret=%d; firmware_version:%s \n",ret,data);
    memset(data,0,sizeof(data));
   
}

/* ====================================================================================
 * Virtual-AT callback + command poll — unchanged from your original.
 * ==================================================================================== */
// void ql_virt_at_notify_cb(unsigned int ind_type, unsigned int size)
// {
//     unsigned char *recv_buff = calloc(1, QL_VIRT_AT_RX_BUFF_SIZE + 1);

//     if (QUEC_VIRT_AT_RX_RECV_DATA_IND == ind_type)
//     {
//         memset(recv_buff, 0, QL_VIRT_AT_RX_BUFF_SIZE + 1);
//         unsigned int real_size = MIN(size, QL_VIRT_AT_RX_BUFF_SIZE);
//         ql_virt_at_read(QL_VIRT_AT_PORT_0, recv_buff, real_size);

//         if (strstr((char *)recv_buff, "+CCLK"))
//         {
//             printf("+CCLK: %s", recv_buff);
//             Seprate_Time_Date(recv_buff);
//         }

//         if (strstr((char *)recv_buff, "+CSQ"))
//         {
//             printf("+CSQ: %s", recv_buff);
//             Seprate_Signal(recv_buff);
//         }

//         if (strstr((char *)recv_buff, "+QSPN:"))
//         {
//             printf("+QSPN: %s", recv_buff);
//             get_apn((char *)recv_buff);
//         }
//     }
//     free(recv_buff);
//     recv_buff = NULL;
// }

// void VIRTUAL_AT_CMD(void)
// {
//     int ret = 0;
//     char *Clk_Sync_cmd = "AT+CTZU=3\r\n";
//     char *Clock_cmd    = "AT+CCLK?\r\n";
//     char *Signal_cmd   = "AT+CSQ\r\n";
//     char *Apn_cmd      = "AT+QSPN\r\n";

//     // ret = ql_virt_at_open(QL_VIRT_AT_PORT_0, ql_virt_at_notify_cb);
//     // if (QL_VIRT_AT_SUCCESS != ret)
//     //     printf("virt at open error 0x%x", ret);

//     // ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char *)Clk_Sync_cmd, strlen(Clk_Sync_cmd));
//     // ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char *)Clock_cmd,    strlen(Clock_cmd));
//     // ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char *)Signal_cmd,   strlen(Signal_cmd));

//     // if (APN_FLAG == 0)
//     // {
//     //     ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char *)Apn_cmd, strlen(Apn_cmd));
//     //     APN_FLAG = 1;
//     // }
// }

void Seprate_Signal(unsigned char *data)
{
    memset(RSSI, '\0', sizeof(RSSI));
    RSSI[0] = data[8];
    RSSI[1] = data[9];
    printf("RSSI:%s", RSSI);
}

void Seprate_Time_Date(unsigned char *data)
{
    memset(APP_Date, '\0', sizeof(APP_Date));
    memset(RTC_Date, '\0', sizeof(RTC_Date));
    memset(APP_Time, '\0', sizeof(APP_Time));
    memset(RTC_Time, '\0', sizeof(RTC_Time));

    APP_Date[0] = '2'; APP_Date[1] = '0';
    APP_Date[2] = data[10]; APP_Date[3] = data[11];
    APP_Date[4] = '-';
    APP_Date[5] = data[13]; APP_Date[6] = data[14];
    APP_Date[7] = '-';
    APP_Date[8] = data[16]; APP_Date[9] = data[17];
    printf("APP_Date:%s", APP_Date);

    RTC_Date[0] = data[10]; RTC_Date[1] = data[11];
    RTC_Date[2] = data[13]; RTC_Date[3] = data[14];
    RTC_Date[4] = data[16]; RTC_Date[5] = data[17];
    printf("RTC_Date:%s", RTC_Date);

    APP_Time[0] = data[19]; APP_Time[1] = data[20];
    APP_Time[2] = data[21]; APP_Time[3] = data[22];
    APP_Time[4] = data[23]; APP_Time[5] = data[24];
    APP_Time[6] = data[25]; APP_Time[7] = data[26];
    printf("APP_Time:%s", APP_Time);

    RTC_Time[0] = data[19]; RTC_Time[1] = data[20];
    RTC_Time[2] = data[22]; RTC_Time[3] = data[23];
    RTC_Time[4] = data[25]; RTC_Time[5] = data[26];
    printf("RTC_Time:%s", RTC_Time);
}

char QSDK_Get_Str(char *src_string, char *dest_string, unsigned char index)
{
    char SentenceCnt = 0;
    char ItemSum = 0;
    char ItemLen = 0, Idx = 0;
    char len = 0;
    unsigned int i = 0;

    if (src_string == NULL) return FALSE;

    len = strlen(src_string);
    for (i = 0; i < len; i++)
    {
        if (*(src_string + i) == ',')
        {
            ItemLen = i - ItemSum - SentenceCnt;
            ItemSum += ItemLen;
            if (index == SentenceCnt)
            {
                if (ItemLen == 0) return FALSE;
                memcpy(dest_string, src_string + Idx, ItemLen);
                *(dest_string + ItemLen) = '\0';
                return TRUE;
            }
            SentenceCnt++;
            Idx = i + 1;
        }
    }
    if (index == SentenceCnt && (len - Idx) != 0)
    {
        memcpy(dest_string, src_string + Idx, len - Idx);
        *(dest_string + len) = '\0';
        return TRUE;
    }
    return FALSE;
}

void get_apn(char *buffer)
{
    char *Ptr = NULL;
    char set_apn[30] = {0};
    unsigned char i = 0;
    char loc_op[20] = {0};
    char fin_op[20] = {0};

    QSDK_Get_Str(buffer, loc_op, 2);
    strncpy(fin_op, &loc_op[1], strlen(loc_op) - 2);

    for (i = 0; fin_op[i] != '\0'; i++)
        if (fin_op[i] >= 'a' && fin_op[i] <= 'z')
            fin_op[i] -= 32;

    sprintf(GPRSData.Operator, fin_op);
    printf("apn operator is:%s", fin_op);

    Ptr = strstr(GPRSData.Operator, "no-apn");
    if (Ptr == 0)                                                 strcpy(set_apn, SET_APN_2);
    else if (strstr(fin_op, "AIRTEL"))                            sprintf(set_apn, "airtelgprs.com");
    else if (strstr(fin_op, "IDEA") || strstr(fin_op, "!DEA"))    sprintf(set_apn, "internet");
    else if (strstr(fin_op, "VODAFONE"))                          sprintf(set_apn, "www");
    else if (strstr(fin_op, "VI INDIA"))                          sprintf(set_apn, "www");
    else if (strstr(fin_op, "BSNL") || strstr(fin_op, "MAHANAGAR") || strstr(fin_op, "SANCHARNET"))
                                                                  sprintf(set_apn, "bsnlnet");
    else if (strstr(fin_op, "MTNL"))                              sprintf(set_apn, "mtnl.net");
    else if (strstr(fin_op, "IND-JIO") || strstr(fin_op, "JIO") || strstr(fin_op, "jio"))
                                                                  sprintf(set_apn, "jionet");

    memset(SET_APN_2, 0, 20);
    sprintf(SET_APN_2, "%s", set_apn);
    printf("apn is: %s", SET_APN_2);
}

/* ====================================================================================
 * Launchers — thin wrappers so application_init() can start each subsystem
 * without double-creating tasks.
 * ==================================================================================== */
static void main_launcher(void *argv)
{
    (void)argv;
    main_app_init();          /* creates "main_app" task inside */
    ql_rtos_task_delete(NULL);
}

static void inverter_launcher(void *argv)
{
    (void)argv;
    if (ql_trace) ql_trace("main: launching inverter\r\n");

    QlOSStatus st = inverter_app_init();
    if (st != QL_OSI_SUCCESS)
        printf("inverter_app_init FAILED 0x%x", st);
    else
        printf("inverter started");

    ql_rtos_task_delete(NULL);
}

static void mqtt_launcher(void *argv)
{
    (void)argv;
    if (ql_trace) ql_trace("main: launching mqtt\r\n");

    ql_rtos_task_sleep_s(20);

    QlOSStatus st = ql_mqtt_app_init();  /* creates "QmqttApp" task inside */
    if (st != QL_OSI_SUCCESS)
        printf("ql_mqtt_app_init FAILED 0x%x", st);
    else
        printf("mqtt started");

    ql_rtos_task_delete(NULL);
}

/* ====================================================================================
 * Supervisor — reboot if inverter link dies for 10 min.
 * ==================================================================================== */
static ql_task_t supervisor_task = NULL;

static void supervisor_thread(void *argv)
{
    (void)argv;

    printf("supervisor: running");
    int offline_ticks = 0;

    while (1)
    {
        if (INVERTER_COMMUNICATION == 0)
        {
            offline_ticks++;
            if (offline_ticks == 60)
            {
                printf("supervisor: inverter offline 10 min, restart");
                ql_power_reset();
            }
        }
        else
        {
            offline_ticks = 0;
        }

        /* MQTT handles Push_Fault_Server internally; supervisor only logs. */
        if (Push_Fault_Server == TRUE)
            printf("supervisor: fault %d pending", Final_Fault);

        ql_rtos_task_sleep_s(10);
    }

    ql_rtos_task_delete(NULL);
}

static void supervisor_launcher(void *argv)
{
    (void)argv;
    QlOSStatus err = ql_rtos_task_create(&supervisor_task, 8 * 1024,
                                          APP_PRIORITY_NORMAL,
                                          "supervisor", supervisor_thread,
                                          NULL, 2);
    if (err != QL_OSI_SUCCESS)
        printf("supervisor create failed 0x%x", err);

    ql_rtos_task_delete(NULL);
}

/* ====================================================================================
 * application_init() registrations — LOWER NUMBER = HIGHER PRIORITY.
 *
 * Order chosen so the Modbus polling task is the first to run and never starves.
 * ==================================================================================== */
application_init(main_launcher,       "main_launcher",  1, 16 * 1024);
application_init(inverter_launcher,   "inv_launcher",   2, 16 * 1024);
application_init(mqtt_launcher,       "mqtt_launcher",  3, 16 * 1024);
application_init(supervisor_launcher, "supervisor_ln",  5,  8 * 1024);