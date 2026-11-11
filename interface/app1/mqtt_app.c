
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mqtt/MQTTClient.h"
#include "ql_log.h"
#include "ql_power.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "sockets.h"
#include "ql_ssl_hal.h"
#include "ql_rtos.h"
#include "main_app.h"
#include "inverter_app.h"
#include "mqtt_app.h"
#include "led_config.h"
#include "ql_sim.h"
#include "FileSystem.h"
#include "fota_http.h"

int  GetKeyValue(char *String);
void Seprate_Key_Value(char *Key, char *Value);
static int is_network_available(void);
static int reestablish_network_connection(void);

#define PROFILE_IDX 1

static ql_task_t mqtt_task = NULL;


typedef struct
{
    char S_PASS[10];
    char S_KEY[10];
    char S_VALUE[25];
    char H_T[4];
    char H_P[2];
} OndemandStoredData;

OndemandStoredData StoredBuffer = {"\0", "\0", "\0", "\0", "\0"};

bool Read_Write_F    = true;
bool Ondemand_Send_F = false;
bool Read_F          = false;
bool Type_F          = false;

char *Str_Strtok_G(char *s, char delim, char **save_ptr);
char *Strtok_G(char *s, char delim);

char RAW_SERVER_DATA[200] = "";
char COTP[10] = "0";
char POTP[10] = "0";

int      Raw_Data_Count = 0;
long int INDEX_NO = 0;

char IMEI_NO[20]  = "869137060313301";
char PASSWORD[20] = "Eastman@123";

char MQTT_CLIENT_ID[50]             = "";
char EASTMAN_INVERTER_MESSAGE[1200] = "";
char EASTMAN_LOGGER_MESSAGE[300]    = "";
char EASTMAN_CONFIG_MESSAGE[200]    = "";

bool STORE_PUB_FLAG  = false;
bool SUCESS_PUB_FLAG = false;

/* ========================================================================= */
const char Eastman_Inerter_Data_Topic[] = "gti-mqtt-1-3-v1";
const char Eastman_Logger_Data_Topic[]  = "logger-mqtt-imei-v1";

#define MQTT_CLIENT_USER   "Eastman"
#define MQTT_SERVER_HOST   "eastmanone.com"
#define MQTT_SERVER_PORT   1883

bool CORRECT_MAC_ID   = FALSE;
bool PARAM_WRITE_FLAG = FALSE;
bool MQTT_STR_FLAG    = FALSE;

int Config_Count      = 0;
int IsConfigTrue      = 0;
int Fota_Restart_Flag = 0;
int Start_Http_Fota = 0;
int Disconnect_Count  = 0;
int Reconnect_Count   = 0;

/* ---- Paho client objects ---- */
/* sendbuf must hold the largest publish (inverter JSON ~1200 B + topic + header) */
#define MQTT_SEND_BUF_SIZE 1536
#define MQTT_READ_BUF_SIZE 512

static MQTTClient client;
static Network    network;
static unsigned char sendbuf[MQTT_SEND_BUF_SIZE];
static unsigned char readbuf[MQTT_READ_BUF_SIZE];

/* ========================================================================= */
/* Incoming message handler (called from MQTTYield in the same task)          */
static void messageArrived(MessageData *data)
{
    static unsigned char rx[256];
    int len = data->message->payloadlen;

    if (len >= (int)sizeof(rx))
        len = sizeof(rx) - 1;

    memcpy(rx, data->message->payload, len);
    rx[len] = '\0';                       /* payload is NOT null-terminated */

    printf("topic: %.*s\r\n", data->topicName->lenstring.len,
           (char *)data->topicName->lenstring.data);
    printf("payload: %s\r\n", rx);

    mqtt_recv(rx);
}

static void ql_nw_status_callback(int profile_idx, int nw_status)
{
    printf("profile(%d) status: %d\r\n", profile_idx, nw_status);
}

/* -------------------------------------------------------------------------
 * Resolve the APN for ql_start_data_call().
 *   1) SET_APN_2 populated by get_apn() in main.c
 *   2) fallback literal
 * ------------------------------------------------------------------------- */
#ifndef QL_MQTT_LOG
#define QL_MQTT_LOG(fmt, ...)  printf("[MQTT] " fmt "\r\n", ##__VA_ARGS__)
#endif
static const char *mqtt_get_apn(void)
{
    if (SET_APN_2[0] != '\0' && strcmp(SET_APN_2, "no-apn") != 0)
        return SET_APN_2;
    // return "airtelgprs.com";
    return "www";
}
static void net_led_network_lost(void)
{
    int sim = QL_SIM_STATUS_UNKNOWN;

    if (ql_sim_get_card_status(&sim) == QL_SIM_SUCCESS && sim == QL_SIM_STATUS_NOT_INSERTED)
        led_config_set_state(LED_ST_SIM_MISSING);
    else
        led_config_set_state(LED_ST_SEARCHING);
}
/* ========================================================================= */
/* Data call (EC800AK API)                                                    */
static int datacall_start(void)
{
    struct ql_data_call_info info = {0};
    char ip4_addr_str[16] = {0};
    const char *apn = mqtt_get_apn();

    led_config_set_state(LED_ST_SEARCHING);      /* slow blink while registering + connecting APN */

    if (ql_network_register_wait(120) != 0)
    {
        printf("*** network register fail ***\r\n");
        net_led_network_lost();                  /* SIM missing -> error blink, else slow blink */
        return -1;
    }
    /* registered, but NO state change here: keep slow blinking until the APN is up */

    ql_wan_start(ql_nw_status_callback);
    ql_set_auto_connect(PROFILE_IDX, TRUE);

    printf("===start data call APN=%s====\r\n", apn);
    if (ql_start_data_call(PROFILE_IDX, 0, (char *)apn, NULL, NULL, 0) != 0)
    {
        printf("*** network activated fail ***\r\n");
        led_config_set_state(LED_ST_SEARCHING);  /* still slow blink, will retry */
        return -1;
    }
    led_config_set_state(LED_ST_REGISTERED);     /* APN connected -> GPIO68 off, GPIO70 solid */

    ql_get_data_call_info(PROFILE_IDX, 0, &info);
    printf("info.profile_idx: %d\r\n", info.profile_idx);
    printf("info.ip_version: %d\r\n",  info.ip_version);
    printf("info.v4.state: %d\r\n",    info.v4.state);
    inet_ntop(AF_INET, &info.v4.addr.ip,      ip4_addr_str, sizeof(ip4_addr_str));
    printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);
    inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
    printf("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);
    inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
    printf("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
    return 0;
}

static int is_network_available(void)
{
    int ok = (ql_network_register_wait(5) == 0);
    if (!ok)
        net_led_network_lost();                      /* ADD */
    return ok;
}
static int reestablish_network_connection(void)
{
    const char *apn = mqtt_get_apn();
    int ret;

    printf("Attempting to reestablish network connection...\r\n");

    ql_stop_data_call(PROFILE_IDX, 0);
    led_config_set_state(LED_ST_SEARCHING);         /* ADD: MQTT is down while the call restarts */
    ql_rtos_task_sleep_s(2);

    ret = ql_start_data_call(PROFILE_IDX, 0, (char *)apn, NULL, NULL, 0);
    if (ret != 0)
    {
        printf("Network reestablishment failed (APN=%s): %d\r\n", apn, ret);
        Reconnect_Count++;
        if (Reconnect_Count >= 12)
        {
            printf("===Maximum reconnect attempts reached, restarting module===\r\n");
            ql_rtos_task_sleep_s(5);
            ql_power_reset();
        }
        return 0;
    }
    
    led_config_set_state(LED_ST_REGISTERED);
    Reconnect_Count = 0;
    printf("Network reestablished successfully (APN=%s)\r\n", apn);
    return 1;
}

/* ========================================================================= */
/* MQTT helpers                                                               */
static void mqtt_session_close(void)
{
    led_config_set_state(LED_ST_REGISTERED);         /* ADD: server link gone */
    MQTTDisconnect(&client);
    NetworkDisconnect(&network);
    MQTTClientDeinit(&client);
}

/* returns 0 on success */
static int mqtt_session_open(void)
{
    SSLConfig sslCfg = { .en = 0 };       /* plain MQTT on port 1883 */
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    int rc;

    memset(&client,  0, sizeof(client));
    memset(&network, 0, sizeof(network));

    NetworkInit(&network, &sslCfg, PROFILE_IDX);
    MQTTClientInit(&client, &network, 30000,
                   sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    if ((rc = NetworkConnect(&network, MQTT_SERVER_HOST, MQTT_SERVER_PORT)) != 0)
    {
        printf("NetworkConnect failed rc=%d\r\n", rc);
        MQTTClientDeinit(&client);
        return -1;
    }

    connectData.MQTTVersion         = 4;            /* 3.1.1 (sample used 3 = 3.1) */
    connectData.clientID.cstring    = MQTT_CLIENT_ID;
    connectData.username.cstring    = MQTT_CLIENT_USER;
    connectData.password.cstring    = PASSWORD;
    connectData.keepAliveInterval   = 60;
    connectData.cleansession        = 1;

    printf("Client ID :%s\r\n", connectData.clientID.cstring);
    printf("User Name :%s\r\n", connectData.username.cstring);

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
    {
        printf("MQTTConnect failed rc=%d\r\n", rc);
        NetworkDisconnect(&network);
        MQTTClientDeinit(&client);
        return -1;
    }

    if ((rc = MQTTSubscribe(&client, "writedatasub_v2", 1, messageArrived)) != 0)
    {
        printf("MQTTSubscribe failed rc=%d\r\n", rc);
        mqtt_session_close();
        return -1;
    }

    printf("====MQTT Connected & subscribed====\r\n");
    return 0;
}

/* returns 0 on success */
static int mqtt_publish_str(const char *topic, char *msg)
{
    MQTTMessage m;
    int rc;

    memset(&m, 0, sizeof(m));
    m.qos        = 0;
    m.retained   = 0;
    m.payload    = msg;
    m.payloadlen = strlen(msg);

        rc = MQTTPublish(&client, topic, &m);
    if (rc != 0)
        printf("Publish failed topic=%s rc=%d\r\n", topic, rc);
    else
        led_config_data_activity();                  /* ADD: flicker on every publish */
    return rc;
}

/* ========================================================================= */
static void mqtt_app_thread(void *arg)
{
    int run_num = 1;
    int reconnect_attempt;
    const int max_reconnect_attempts = 5;
    const int reconnect_delay_sec    = 5;

    (void)arg;
    ql_rtos_task_sleep_s(10);
    printf("========== mqtt app start ==========\r\n");
    led_config_set_state(LED_ST_SEARCHING);          /* ADD */
    Disconnect_Count = 0;
     Reconnect_Count  = 0;
    while (datacall_start() != 0)
    {
        printf("data call failed, retrying...\r\n");
        ql_rtos_task_sleep_s(reconnect_delay_sec);
    }

    Create_Client_Id(MQTT_CLIENT_ID);

    while (1)
    {
        printf("==============mqtt_client_test[%d]================\r\n", run_num);

        if (!is_network_available())
        {
            printf("Network not available, attempting to reconnect...\r\n");
            if (!reestablish_network_connection())
            {
                ql_rtos_task_sleep_s(reconnect_delay_sec);
                continue;
            }
        }

        /* ---- connect with retries ---- */
        for (reconnect_attempt = 0; reconnect_attempt < max_reconnect_attempts; reconnect_attempt++)
        {
            if (mqtt_session_open() == 0)
                break;

            printf("===MQTT Reconnection attempt %d/%d, waiting %d seconds===\r\n",
                   reconnect_attempt + 1, max_reconnect_attempts, reconnect_delay_sec);
            ql_rtos_task_sleep_s(reconnect_delay_sec);
        }

        if (reconnect_attempt >= max_reconnect_attempts)
        {
            printf("===Max MQTT reconnection attempts reached, giving up====\r\n");
            ql_rtos_task_sleep_s(reconnect_delay_sec * 2);
            continue;
        }
        led_config_set_state(LED_ST_ONLINE);     /* MQTT connected */

        /* ================================================================
         * Main publish loop
         * ================================================================ */
        while (1)
        {
            /* Service keep-alive + incoming messages (blocks up to 100 ms).
             * Non-zero rc means the link is gone. */
            if (MQTTYield(&client, 100) != 0)
            {
                printf("MQTTYield failed, reconnecting\r\n");
                break;
            }

            /* -------- Immediate fault publish -------- */
            if (Push_Fault_Server == TRUE)
            {
                printf("=== Immediate fault publish FALT=%d ===\r\n", Final_Fault);
                if (Create_inverter_data(EASTMAN_INVERTER_MESSAGE))
                    mqtt_publish_str(Eastman_Inerter_Data_Topic, EASTMAN_INVERTER_MESSAGE);
                Push_Fault_Server = FALSE;
                LAST_REMOTE_CALL  = 0;
            }

            /* -------- Periodic inverter data publish -------- */
            if (LAST_REMOTE_CALL++ > REMOTE_INTERVEL)
            {
                REMOTE_INTERVEL = Final_InverterData_Interval;

                if (!Create_inverter_data(EASTMAN_INVERTER_MESSAGE))
                {
                    printf("====== Create Inverter Data Return =======\r\n");
                    break;
                }
                if (mqtt_publish_str(Eastman_Inerter_Data_Topic, EASTMAN_INVERTER_MESSAGE) != 0)
                    break;
                LAST_REMOTE_CALL = 0;
            }

            /* -------- Periodic logger publish -------- */
            if (LAST_LOGGER_CALL++ > LOGGER_INTERVAL)
            {
                LOGGER_INTERVAL = Final_InverterData_Interval + 1;

                if (!create_logger_data(EASTMAN_LOGGER_MESSAGE))
                {
                    printf("====== Create Logger Data Return =======\r\n");
                    break;
                }
                if (mqtt_publish_str(Eastman_Logger_Data_Topic, EASTMAN_LOGGER_MESSAGE) != 0)
                    break;
                LAST_LOGGER_CALL = 0;
            }

            /* -------- Config response publish (server-triggered) -------- */
            if (IsConfigTrue == 1 && Config_Count++ > 1)
            {
                if (MQTT_STR_FLAG)
                {
                    if (!create_config_data_str(EASTMAN_CONFIG_MESSAGE,
                                                CONFIG_VAR_NAME, CONFIG_VAR_STR))
                    {
                        printf("====== Create Config Data Str Return =======\r\n");
                        break;
                    }
                    if (mqtt_publish_str("writedatapub", EASTMAN_CONFIG_MESSAGE) != 0)
                        break;
                    MQTT_STR_FLAG = FALSE;
                }
                else
                {
                    if (!create_config_data(EASTMAN_CONFIG_MESSAGE,
                                            CONFIG_VAR_NAME, CONFIG_VAR_VALUE))
                    {
                        printf("====== Create Config Data Return =======\r\n");
                        break;
                    }
                    if (mqtt_publish_str("writedatapub", EASTMAN_CONFIG_MESSAGE) != 0)
                        break;
                }

                IsConfigTrue     = 0;
                Config_Count     = 0;
                PARAM_WRITE_FLAG = FALSE;
                 if (Start_Http_Fota == 1)
                {
                    Start_Http_Fota = 0;
                    fota_http_app_init();      /* ack already published, start download task */
                }
                if (Fota_Restart_Flag == 1)
{
    MQTTYield(&client, 500);          /* flush */
    mqtt_session_close();             /* DISCONNECT + close socket */
    ql_rtos_task_sleep_ms(2000);
    ql_power_reset();
}
            }

            ql_rtos_task_sleep_ms(900);   /* ~1 s loop incl. the 100 ms yield */
        }

        mqtt_session_close();
        Disconnect_Count++;

        printf("===Waiting %d seconds before reconnection attempt===\r\n", reconnect_delay_sec);
        ql_rtos_task_sleep_s(reconnect_delay_sec);
        if (Disconnect_Count >= 10)
        {
            printf("===Maximum disconnects reached, restarting module===\r\n");
            ql_rtos_task_sleep_s(5);
            ql_power_reset();
        }
        run_num++;
    }
}

/* ========================================================================= */
void Create_Client_Id(char *id)
{
    memset(id, 0, 50);
    strcpy(id, "EPTPL");
    strcat(id, GPRSData.IMEI);
    printf("CLIENT ID:%s\r\n", id);
}

/* =========================================================================
 * JSON helpers.
 * Each field is emitted exactly as the original strcat chain did:
 *     "KEY":"value",        (last field: no trailing comma)
 * Numbers are formatted with the same printf formats as before
 * (%d / %0.02f; the original %0.2lf is identical to %0.02f for doubles).
 * ========================================================================= */
static void js_str(char *out, const char *key, const char *val, int last)
{
    strcat(out, "\""); strcat(out, key);
    strcat(out, "\":\""); strcat(out, val);
    strcat(out, last ? "\"" : "\",");
}

#define JS_F(k, v)   do { snprintf(data, sizeof(data), "%0.02f", (double)(v)); js_str(out, k, data, 0); } while (0)
#define JS_D(k, v)   do { snprintf(data, sizeof(data), "%d", (int)(v));        js_str(out, k, data, 0); } while (0)

/* =========================================================================
 * Create_inverter_data
 * ========================================================================= */
unsigned char Create_inverter_data(char *Inverter_Data)
{
    char *out = Inverter_Data;
    char data[24];

    out[0] = '\0';
    strcat(out, "{");

    js_str(out, "MAC", GPRSData.IMEI, 0);

    JS_D("INTYP",   Inverter_Basic_Detail.Inverter_Type);
    JS_D("PRODTYP", Inverter_Basic_Detail.Product_Type);
    JS_F("RTPW",    Inverter_Basic_Detail.Rated_Power);
    JS_D("MPTPH",   Inverter_Basic_Detail.MPPT_Phase_No);
    JS_D("RTGRDST", Inverter_Basic_Detail.Rated_Grid_Voltage_Get);
    JS_F("RUNST",   InverterData.RunState);

    JS_F("DCV1", InverterData.DCVoltage1);
    JS_F("DCV2", InverterData.DCVoltage2);
    JS_F("DCV3", InverterData.DCVoltage3);
    JS_F("DCV4", InverterData.DCVoltage4);

    JS_F("DCC1", InverterData.DCCurrent1);
    JS_F("DCC2", InverterData.DCCurrent2);
    JS_F("DCC3", InverterData.DCCurrent3);
    JS_F("DCC4", InverterData.DCCurrent4);

    JS_F("DCP1", InverterData.DCPower1);
    JS_F("DCP2", InverterData.DCPower2);
    JS_F("DCP3", InverterData.DCPower3);
    JS_F("DCP4", InverterData.DCPower4);

    JS_F("GRDVAB", InverterData.GridVoltageAB);
    JS_F("GRDVBC", InverterData.GridVoltageBC);
    JS_F("GRDVAC", InverterData.GridVoltageAC);
    JS_F("GRDVA",  InverterData.GridVoltageA);
    JS_F("GRDVB",  InverterData.GridVoltageB);
    JS_F("GRDVC",  InverterData.GridVoltageC);
    JS_F("GRDCA",  InverterData.GridCurrentA);
    JS_F("GRDCB",  InverterData.GridCurrentB);
    JS_F("GRDCC",  InverterData.GridCurrentC);
    JS_F("GRFREQ", InverterData.GridFrequency);

    JS_F("DYATPW",  InverterData.DayActivePower);
    JS_F("DYRTPW",  InverterData.DayReactivePower);
    JS_F("TTATPW",  InverterData.TotalActivePower);
    JS_F("TTRTPW",  InverterData.TotalReactivePower);
    JS_F("INATPW",  InverterData.InputActivePower);
    JS_F("OTATPW",  InverterData.OutputActivePower);
    JS_F("OTRTPW",  InverterData.OutputReactivePower);
    JS_F("OTAPPW",  InverterData.OutputApparentPower);
    JS_F("TOTWRKTM", InverterData.TotalWorkTime);
    JS_F("WRNG",    InverterData.Warning);
    JS_F("IGBTTEMP", InverterData.IGBTtemperatur);
    JS_F("RADITEMP", InverterData.RadiatorTemperature);

    JS_D("FALT", Final_Fault);

    JS_F("GRDPWL1", Inverter_Output_Grid_Parameter.Grid_Power_L1);
    JS_F("GRDPWL2", Inverter_Output_Grid_Parameter.Grid_Power_L2);
    JS_F("GRDPWL3", Inverter_Output_Grid_Parameter.Grid_Power_L3);

    JS_F("INVOPWL1", Inverter_Output_Grid_Parameter.Inverter_Output_Power_L1);
    JS_F("INVOPWL2", Inverter_Output_Grid_Parameter.Inverter_Output_Power_L2);
    JS_F("INVOPWL3", Inverter_Output_Grid_Parameter.Inverter_Output_Power_L3);

    JS_F("LMTRPWL1", Inverter_Output_Grid_Parameter.Limiter_Power_L1);
    JS_F("LMTRPWL2", Inverter_Output_Grid_Parameter.Limiter_Power_L2);
    JS_F("LMTRPWL3", Inverter_Output_Grid_Parameter.Limiter_Power_L3);

    JS_F("LODPWL1", Inverter_Output_Grid_Parameter.Load_Power_L1);
    JS_F("LODPWL2", Inverter_Output_Grid_Parameter.Load_Power_L2);
    JS_F("LODPWL3", Inverter_Output_Grid_Parameter.Load_Power_L3);

    JS_F("GRDEXTTPW", Inverter_Output_Grid_Parameter.Total_Grd_External_Power);
    JS_F("INVOTTTPW", Inverter_Output_Grid_Parameter.Total_Invt_Output_Power);
    JS_F("LODTTPW",   Inverter_Output_Grid_Parameter.Total_Load_Power);
    JS_F("DYLDPW",    Inverter_Output_Grid_Parameter.Day_Load_Power);
    JS_F("CUMLDPW",   Inverter_Output_Grid_Parameter.Cumulative_Load_Power);
    JS_F("DYGRFDIN",  Inverter_Output_Grid_Parameter.Daily_Grid_Feed_In);
    JS_F("CUMGRFDIN", Inverter_Output_Grid_Parameter.Cumulative_Grid_Feed_In);
    JS_F("DYEGPUR",   Inverter_Output_Grid_Parameter.Daily_Energy_Purchase);
    JS_F("CUMEGPUR",  Inverter_Output_Grid_Parameter.Cumulative_Energy_Purchase);

    /* last field - no trailing comma */
    snprintf(data, sizeof(data), "%0.02f", (double)Inverter_Config_Param.Meter_Enable);
    js_str(out, "METERE", data, 1);

    strcat(out, "}");
    printf("<<<<<Inverter_Data>>>:%s\r\n", Inverter_Data);
    return 1;
}

/* =========================================================================
 * create_logger_data
 * ========================================================================= */
unsigned char create_logger_data(char *Logger_Data)
{
    char *out = Logger_Data;
    char data[24];

    out[0] = '\0';
    strcat(out, "{");

    js_str(out, "MAC", GPRSData.IMEI, 0);

    /* DATETIME = INV_DATE + " " + INV_TIME */
    strcat(out, "\"DATETIME\":\"");
    strcat(out, INV_DATE); strcat(out, " "); strcat(out, INV_TIME);
    strcat(out, "\",");

    js_str(out, "SSID", GPRSData.Operator, 0);

    snprintf(data, sizeof(data), "%s", RSSI);
    js_str(out, "SIGNAL", data, 0);

    JS_D("INTRVL", Final_InverterData_Interval);

    js_str(out, "MODSOFTVER", MOD_SOFTWARE_VERSION, 0);
    js_str(out, "MODFRMVER",  GPRSData.Module_FW,   0);
    js_str(out, "INVTRSRNO",  serial_number,        0);

    JS_D("PROVER",  Inverter_Basic_Detail.Protocal_Version);
    JS_D("CNTFWVR", Inverter_Basic_Detail.Firmware_Ver_Control_Bord);
    JS_D("COMFWVR", Inverter_Basic_Detail.Firmware_Ver_Communication_Bord);

    js_str(out, "LAT", LAT_DATA, 0);
    js_str(out, "LON", LON_DATA, 0);

    /* GSM_DATETIME = APP_Date + " " + APP_Time (last field) */
    strcat(out, "\"GSM_DATETIME\":\"");
    strcat(out, APP_Date); strcat(out, " "); strcat(out, APP_Time);
    strcat(out, "\"");

    strcat(out, "}");
    printf("<<<<<Logger_Data>>>:%s\r\n", Logger_Data);
    return 1;
}

char create_config_data(char *Config_Data, char *Config_Key, double Config_Value)
{
    char Data[24];

    Config_Data[0] = '\0';
    strcat(Config_Data, "{");
    js_str(Config_Data, "SRNO", GPRSData.IMEI, 0);

    snprintf(Data, sizeof(Data), "%0.02f", Config_Value);
    js_str(Config_Data, Config_Key, Data, 1);

    strcat(Config_Data, "}");
    printf("<<<<<Config_Data>>>:%s\r\n", Config_Data);
    return 1;
}

char create_config_data_str(char *Config_Data, char *Config_Key, char *Config_Value)
{
    Config_Data[0] = '\0';
    strcat(Config_Data, "{");
    js_str(Config_Data, "SRNO", GPRSData.IMEI, 0);
    js_str(Config_Data, Config_Key, Config_Value, 1);
    strcat(Config_Data, "}");

    printf("<<<<<Config_Data>>>:%s\r\n", Config_Data);
    return 1;
}

/* =========================================================================
 * mqtt_recv  (unchanged)
 * ========================================================================= */
void mqtt_recv(const unsigned char *buffer)
{
    printf("<== Raw Server Json :%s==>\r\n", buffer);
    CORRECT_MAC_ID   = FALSE;
PARAM_WRITE_FLAG = FALSE;
Read_F           = FALSE;
Raw_Data_Count   = 0;
    int length = strlen((const char *)buffer);
    memset(RAW_SERVER_DATA, '\0', sizeof(RAW_SERVER_DATA));

    for (int i = 0; i < length; i++)
    {
        if (buffer[i] == '{') Read_F = TRUE;
        if (buffer[i] == '}')
        {
            Read_F         = FALSE;
            Raw_Data_Count = 0;
        }
        if (Read_F == TRUE)
        {
            RAW_SERVER_DATA[Raw_Data_Count] = buffer[i];
            Raw_Data_Count++;
        }
    }

    int lengtofdata = strlen(RAW_SERVER_DATA);
    for (int j = 0; j < lengtofdata; j++)
        RAW_SERVER_DATA[j] = RAW_SERVER_DATA[j + 1];

    GetKeyValue(RAW_SERVER_DATA);
}

int GetKeyValue(char *String)
{
    char key[50], value[50];
    int  j, colon, comma;
    int  i = 0;

    while (i < strlen(String))
    {
        colon = -1;
        for (j = i; j < strlen(String); j++)
        {
            if (String[j] == ':') { colon = j; break; }
        }
        if (colon == -1) break;

       int klen = colon - i;
if (klen > 49) klen = 49;
strncpy(key, String + i, klen);
key[klen] = '\0';

        comma = -1;
        for (j = colon + 1; j < strlen(String); j++)
        {
            if (String[j] == ',') { comma = j; break; }
        }
        if (comma == -1) comma = strlen(String);

    int vlen = comma - colon - 1;
if (vlen > 49) vlen = 49;
strncpy(value, String + colon + 1, vlen);
value[vlen] = '\0';

        printf("Key: %s\n", key);
        printf("Value: %s\n", value);
        Seprate_Key_Value(key, value);
        i = comma + 1;
    }
    return 0;
}

/* =========================================================================
 * Seprate_Key_Value  (unchanged)
 * ========================================================================= */
void Seprate_Key_Value(char *Key, char *Value)
{
	 for(int i=0; i<strlen(Key); i++)
	 {
		 Key[i] = Key[i+1];
	 }
	 Key[strlen(Key)-1]='\0';

	 for(int i=0; i<strlen(Value); i++)
	 {
		 Value[i] = Value[i+1];
	 }
	 Value[strlen(Value)-1]='\0';

//////////////////////////////////////////////// Check for Correct MAC ID //////////////////////////////////////////////////
    if(strcmp(Key,"SRNO")==0)
    {
        if(strcmp(Value,GPRSData.IMEI)==0){
           CORRECT_MAC_ID = TRUE;
           QL_MQTT_LOG("CORRECTED SRNO:%s\r\n",Value);
        }
    }
///////////////////////////////////////// Enable Writing Process /////////////////////////////////////////
    if(strcmp(Key,"TYPE")==0 && strcmp(Value,"WRITE")==0 && CORRECT_MAC_ID == TRUE)
    {
        PARAM_WRITE_FLAG = TRUE;
        QL_MQTT_LOG("PARAM_WRITE_FLAG:%d\r\n",PARAM_WRITE_FLAG);
    }

/////////////////////////////////////////// RESTART ///////////////////////////////////////////////////////
    if(strcmp(Key,"RESTART" ) == 0 && (CORRECT_MAC_ID == TRUE))
    {
        QL_MQTT_LOG("RESTART\r\n");
        if (strcmp(Value, "1") == 0)
        {
            if(PARAM_WRITE_FLAG == TRUE){
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "1");
                Fota_Restart_Flag = 1;
            }
            else{
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "0");
            }

        }
         CORRECT_MAC_ID = FALSE;
    }

    ///////////////////////////////////////////// MODULE FOTA //////////////////////////////////////////////////////
    if(strcmp(Key,"START_FOTA" ) == 0 && (CORRECT_MAC_ID == TRUE))
    {
        QL_MQTT_LOG("START_FOTA\r\n");
        if (strcmp(Value, "1") == 0)
        {
            if(PARAM_WRITE_FLAG == TRUE){
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "1");
                Start_Http_Fota = 1;
            }
            else{
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "0");
            }
        }
         CORRECT_MAC_ID = FALSE;
    }

    if(strcmp(Key,"DSP_OTA" ) == 0 && (CORRECT_MAC_ID == TRUE))
    {
        QL_MQTT_LOG("DSP_OTA\r\n");
        if (strcmp(Value, "1") == 0)
        {
            if(PARAM_WRITE_FLAG == TRUE){
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "2");
                Fota_Restart_Flag = 1;
                WriteFileUFS("2",6);
            }
            else{
                IsConfigTrue = 1;
                IsProcessDone = 1;
                MQTT_STR_FLAG = TRUE;
                strcpy(CONFIG_VAR_NAME, Key);
                strcpy(CONFIG_VAR_STR, "0");
            }
        }
         CORRECT_MAC_ID = FALSE;
    }

//     if(strcmp(Key,"HMI_OTA" ) == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//         QL_MQTT_LOG("HMI_OTA\r\n");
//         if (strcmp(Value, "1") == 0)
//         {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME, Key);
//                 strcpy(CONFIG_VAR_STR, "1");
//                 Fota_Restart_Flag = 1;
//                 WriteFileUFS("1",6);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME, Key);
//                 strcpy(CONFIG_VAR_STR, "0");
//             }
//         }
//          CORRECT_MAC_ID = FALSE;
//     }

// //////////////////////////////////////// HTTP FILE PATH /////////////////////////////////////////////////
//     if (strcmp(Key, "OTA_FILE") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//                 WriteFileUFS(Value,7);
//                 strcpy(INV_OTA_URL_SUFFIX,Value);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,INV_OTA_URL_SUFFIX);
//             }
//             CORRECT_MAC_ID = FALSE;
//     }

// //////////////////////////////////////// HTTP URL /////////////////////////////////////////////////
//     if (strcmp(Key, "HTTP_URL") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//                 WriteFileUFS(Value,11);
//                 strcpy(HTTP_DLOAD_URL,Value);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,HTTP_DLOAD_URL);
//             }
//             CORRECT_MAC_ID = FALSE;
//     }

// //////////////////////////////////////// HTTP FILE PATH /////////////////////////////////////////////////
//     if (strcmp(Key, "FILE_NAME") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//                 WriteFileUFS(Value,8);
//                 strcpy(file_full_path,Value);
//                 strcpy(HTTP_FILE_NAME,Value);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,HTTP_FILE_NAME);
//             }
//             CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "HTTP_PORT") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,"3001");
//             }
//             CORRECT_MAC_ID = FALSE;
//     }

// ///////////////////////////////////////// Interval //////////////////////////////////////////////////
//     if (strcmp(Key, "INTERVAL") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//                 WriteFileUFS(Value,5);
//                 Final_InverterData_Interval = atoi(Value);
//             }
//             else{
//                 char INV_INTERVAL[5];
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 sprintf(INV_INTERVAL,"%d",Final_InverterData_Interval);
//                 strcpy(CONFIG_VAR_STR,INV_INTERVAL);

//             }
//             CORRECT_MAC_ID = FALSE;
//     }

// ////////////////////////////////// STORING DATA INTERVAL///////////////////////////////////////////////////
//     if (strcmp(Key, "STINTERVAL") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,Value);
//                 WriteFileUFS(Value,9);
//                 Max_S_Count = atoi(Value);
//             }
//             else{
//                 char ST_INTERVAL[5];
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 sprintf(ST_INTERVAL,"%d",Max_S_Count);
//                 strcpy(CONFIG_VAR_STR,ST_INTERVAL);

//             }
//             CORRECT_MAC_ID = FALSE;
//     }

// ////////////////////////////////// WRITE DATE TIME ////////////////////////////////////////////////////////

//     if (strcmp(Key, "WR_DATE_TIME") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 CONFIG_WRITE_VAR = WR_DATE_TIME;
//                 PARAM_CONFIG_MODE = 2;
//                 WRITE_DATE_TIME = 1;
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//                 strcpy(RX_CONFIG_VALUE,Value);
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 QL_MQTT_LOG("WR_DATE_TIME:%s\r\n",RX_CONFIG_VALUE);

//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,"");
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,INV_DATE);
//                 strcat(CONFIG_VAR_STR," ");
//                 strcat(CONFIG_VAR_STR,INV_TIME);
//             }

//             CORRECT_MAC_ID = FALSE;
//     }
// ///////////////////////////////////////////////// GET INVERTER SERIAL NO. /////////////////////////////////////////
//     if (strcmp(Key, "INV_SERIAL") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,serial_number);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,serial_number);
//             }
//             CORRECT_MAC_ID = FALSE;
//     }

//     ///////////////////////////////////////// Get Software Version //////////////////////////////////////////////////
//     if (strcmp(Key, "SOFT_VER") == 0 && (CORRECT_MAC_ID == TRUE))
//     {
//             if(PARAM_WRITE_FLAG == TRUE){
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,MOD_SOFTWARE_VERSION);
//             }
//             else{
//                 IsConfigTrue = 1;
//                 IsProcessDone = 1;
//                 MQTT_STR_FLAG = TRUE;
//                 strcpy(CONFIG_VAR_NAME,Key);
//                 strcpy(CONFIG_VAR_STR,MOD_SOFTWARE_VERSION);
//             }
//             CORRECT_MAC_ID = FALSE;
//     }


// //////////////////////////////////////////////////// Configuration ///////////////////////////////////////


//     if(strcmp(Key,"SYSRUNFLG")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_RUN_FLAG;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_RUN_FLAG:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_RUN_FLAG;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"SYSRSTFCTRY")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_RESET_FACTORY;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_RESET_FACTORY:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_RESET_FACTORY;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"ZEROXPORTMOD")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_ZERO_EXPORT_MODE;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_ZERO_EXPORT_MODE:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_ZERO_EXPORT_MODE;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"PAUSETIME")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_PAUSE_TIME_SET;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_PAUSE_TIME_SET:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_PAUSE_TIME_SET;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"ACTIVEPOWERCTRL")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_ACTIVE_POWER_CTRL;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_ACTIVE_POWER_CTRL:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_ACTIVE_POWER_CTRL;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"REACTIVEPOWERCTRL")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_REACTIVE_POWER_CTRL;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_REACTIVE_POWER_CTRL:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_REACTIVE_POWER_CTRL;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"ISODETECTION")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_ISO_DETECTION;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_ISO_DETECTION:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_ISO_DETECTION;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"RCDETECTION")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_RC_DETECTION;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_RC_DETECTION:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_RC_DETECTION;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"ISLANDETECTION")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_ISLANDING_DETECTION;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_ISLANDING_DETECTION:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_ISLANDING_DETECTION;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"METERENABLE")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_METER_ENABLE;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_METER_ENABLE:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_METER_ENABLE;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"WINDENABLE")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_WIND_ENABLE;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_WIND_ENABLE:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_WIND_ENABLE;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"GRIDSTANDARD")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_GRID_STANDERD;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_GRID_STANDERD:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_GRID_STANDERD;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     //////////////////////////////// Level 1 Configuration /////////////////////////////////////////////
//     if(strcmp(Key,"OVPOINTLV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_POINT_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_POINT_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_POINT_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OVTIMELV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_TIME_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_TIME_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_TIME_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVPOINTLV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_POINT_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_POINT_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_POINT_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVTIMELV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_TIME_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_TIME_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_TIME_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFPOINTLV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_POINT_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_POINT_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_POINT_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFTIMELV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_TIME_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_TIME_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_TIME_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFPOINTLV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_POINT_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_POINT_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_POINT_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFTIMELV1")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_TIME_LV1;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_TIME_LV1:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_TIME_LV1;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     /////////////////////////////////////////// Level 2 Configuration /////////////////////////////////////////////

//     if(strcmp(Key,"OVPOINTLV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_POINT_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_POINT_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_POINT_LV2_R;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OVTIMELV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_TIME_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_TIME_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_TIME_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVPOINTLV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_POINT_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_POINT_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_POINT_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVTIMELV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_TIME_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_TIME_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_TIME_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFPOINTLV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_POINT_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_POINT_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_POINT_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFTIMELV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_TIME_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_TIME_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_TIME_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFPOINTLV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_POINT_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_POINT_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_POINT_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFTIMELV2")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_TIME_LV2;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_TIME_LV2:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_TIME_LV2;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     /////////////////////////////////////////// Level 3 Configuration /////////////////////////////////////////////
//     if(strcmp(Key,"OVPOINTLV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_POINT_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_POINT_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_POINT_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OVTIMELV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OV_TIME_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OV_TIME_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OV_TIME_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVPOINTLV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_POINT_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_POINT_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_POINT_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UVTIMELV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UV_TIME_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UV_TIME_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UV_TIME_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFPOINTLV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_POINT_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_POINT_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_POINT_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"OFTIMELV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = OF_TIME_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("OF_TIME_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = OF_TIME_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFPOINTLV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_POINT_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_POINT_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_POINT_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"UFTIMELV3")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = UF_TIME_LV3;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("UF_TIME_LV3:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = UF_TIME_LV3;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"SYSTMDLYRUNTIME")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_RUNNING_DELAY_TIME;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_RUNNING_DELAY_TIME:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_RUNNING_DELAY_TIME;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"SYSTEMLANGUAGE")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_LANGUAGE;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_LANGUAGE:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_LANGUAGE;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"SYSTEMLCDBRIGHT")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = SYSTEM_LCD_BRIGHT_TIME;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("SYSTEM_LCD_BRIGHT_TIME:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = SYSTEM_LCD_BRIGHT_TIME;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"BEEPONERRORENABLE")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = BEEP_ON_ERROR_ENABLE;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("BEEP_ON_ERROR_ENABLE:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = BEEP_ON_ERROR_ENABLE;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     if(strcmp(Key,"ZEROREFPOWER")==0 && CORRECT_MAC_ID == TRUE)
//     {
//         if(PARAM_WRITE_FLAG == TRUE){
//             CONFIG_WRITE_VAR = ZERO_REF_POWER;
//             PARAM_CONFIG_MODE = 2;
//             IsConfigTrue = 1;
//             memset(RX_CONFIG_VALUE,'\0',sizeof(RX_CONFIG_VALUE));
//             strcpy(RX_CONFIG_VALUE,Value);
//             QL_MQTT_LOG("ZERO_REF_POWER:%s\r\n",RX_CONFIG_VALUE);
//         }else{
//             PARAM_CONFIG_MODE = 1;
//             CONFIG_IS_READ = 1;
//             strcpy(CONFIG_VAR_NAME,Key);
//             PARAM_WRITE_FLAG = FALSE;
//             CONFIG_WRITE_VAR = ZERO_REF_POWER;
//             CONFIG_VAR_VALUE = 0.0;
//         }
//         CORRECT_MAC_ID = FALSE;
//     }

//     ////////////////////////////////////////// Meter /////////////////////////////
//     if (strcmp(Key, "METERVOLTAGE") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%.1f",
//                 Inverter_Metering_Parameter.MeterVoltage);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERCURRENT") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%.2f",
//                 Inverter_Metering_Parameter.MeterCurrent);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERATPWR") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%ld",
//                 (long)Inverter_Metering_Parameter.MeterActivePower);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERRATPW") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%ld",
//                 (long)Inverter_Metering_Parameter.MeterReactivePower);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERPWFTR") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%.2f",
//                 Inverter_Metering_Parameter.MeterPowerFactor);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERTTENRG") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%.1f",
//                 Inverter_Metering_Parameter.MeterTotalEnergy);

//         CORRECT_MAC_ID = FALSE;
//     }

//     if (strcmp(Key, "METERPSTVENRG") == 0 && CORRECT_MAC_ID == TRUE)
//     {
//         IsConfigTrue = 1;
//         IsProcessDone = 1;
//         MQTT_STR_FLAG = TRUE;
//         strcpy(CONFIG_VAR_NAME, Key);

//         snprintf(CONFIG_VAR_STR,
//                 sizeof(CONFIG_VAR_STR),
//                 "%.1f",
//                 Inverter_Metering_Parameter.MeterPositiveEnergy);

//         CORRECT_MAC_ID = FALSE;
//     }

    // if (strcmp(Key, "METERNEGENRG") == 0 && CORRECT_MAC_ID == TRUE)
    // {
    //     IsConfigTrue = 1;
    //     IsProcessDone = 1;
    //     MQTT_STR_FLAG = TRUE;
    //     strcpy(CONFIG_VAR_NAME, Key);

    //     snprintf(CONFIG_VAR_STR,
    //             sizeof(CONFIG_VAR_STR),
    //             "%.1f",
    //             Inverter_Metering_Parameter.MeterNegativeEnergy);

    //     CORRECT_MAC_ID = FALSE;
    // }

}


int ql_mqtt_app_init(void)
{
    int err;
    err = ql_rtos_task_create(&mqtt_task, 16 * 1024, 100, "QmqttApp", mqtt_app_thread, NULL);
    if (err != 0) printf("mqtt_app init failed 0x%x\r\n", err);
    return err;
}