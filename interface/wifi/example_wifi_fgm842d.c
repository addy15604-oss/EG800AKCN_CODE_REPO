
/*==========================================================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
===========================================================================================================*/

#include <stdio.h>
#include "ql_uart.h"
#include "ql_application.h"
#include "ql_log.h"
#include "ql_rtos.h"

#define WIFI_AT_PORT       QL_MAIN_UART_PORT
#define DATA_QUEUE_SIZE    200

typedef enum
{
    WLAN_DISCONNECTED,
    WLAN_CONNECTED,
    GOT_IP,
    SCAN_NO_AP
}ql_wifi_sta_status_t;

typedef enum
{
    SOCKET_DISCONNECTED,
    SOCKET_CONNECTED
}ql_wifi_socket_status_t;

typedef enum
{
    AT_CMD_MODE,
    DATA_MODE
}ql_wifi_access_mode_t;

typedef struct
{
    int len;
    uint8_t data[DATA_QUEUE_SIZE];
}ql_wifi_data_queue_t;

ql_queue_t ql_wifi_at_queue = NULL;
ql_queue_t ql_wifi_tcp_queue = NULL;
ql_queue_t ql_wifi_mqtt_queue = NULL;

ql_sem_t  ql_wifi_status_semaRef = NULL;

ql_wifi_sta_status_t g_ql_wifi_sta_status = WLAN_DISCONNECTED;
ql_wifi_access_mode_t g_ql_wifi_access_mode = AT_CMD_MODE;
ql_wifi_socket_status_t g_ql_wifi_tcp_status = SOCKET_DISCONNECTED;

static char *mqtt_ca = 
"-----BEGIN CERTIFICATE-----\n\
MIIEhDCCAuwCCQDuE1BpeAeMwzANBgkqhkiG9w0BAQsFADCBgjELMAkGA1UEBhMC\n\
Q04xCzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFTDEL\n\
MAkGA1UECwwCU1QxFjAUBgNVBAMMDTExMi4zMS44NC4xNjQxIjAgBgkqhkiG9w0B\n\
CQEWE2VkZGllLnpoYW5nQHF1ZWN0ZWwwIBcNMjIwMTI1MDcyMzI3WhgPMjEyMjAx\n\
MDEwNzIzMjdaMIGCMQswCQYDVQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcM\n\
AkhGMRAwDgYDVQQKDAdRVUVDVEVMMQswCQYDVQQLDAJTVDEWMBQGA1UEAwwNMTEy\n\
LjMxLjg0LjE2NDEiMCAGCSqGSIb3DQEJARYTZWRkaWUuemhhbmdAcXVlY3RlbDCC\n\
AaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoCggGBAMt3cjY0eLEDqv8Y7FomA+7N\n\
G5ztAbR7+P/WxjPlodqRDZ5HQORkfAr44gAZcWsKoo4DHTInwr9JBbBnETBMnL8+\n\
13h1PRp5CfwXKFvjppWYvBZfeTwhWQYbSMKINoS+d1Zl11jg/+ZbSd7Fi0bYq8ip\n\
Hbt30H+NANQZP1XQdsCf5/kvn+vXiP4EgJc56JQ9L6ALIF2Q6F3G/PTaYItg463N\n\
lv/S+eRi1VMDSs8Qc+DTlVwlgZZJdSlC8Yjr5pVqoyXm8ENKfSTrdhrLiKSWJTz9\n\
JUr04E7SJ+CoBAnLYNPHR2y0CFS/15aCa1JbK27ZJ/0cvBvzpWdkcgrDtKIcxNYM\n\
9QFPpehb1N4pgqi0NPhCkc/BasfmXUaTwM4ghhi4tQRptKMdTN/kdyC+V5a8Hyhb\n\
Nvw5qeJlLJKpgZ9X3HQzuKstKMkxLNuDIzK9TvO7zLowr+0BetUdllq+fDjXQM0M\n\
+9P3Xv2VmDwGRkmZ0IjYpDjm+qqGTFVLzzVwEqVD6wIDAQABMA0GCSqGSIb3DQEB\n\
CwUAA4IBgQAuNVwkBhd5nyWMmV/ESNxy59Sz+5FcesGclKjs4YocgcKbLD2bS+LN\n\
lKk6zenES7Cq6+l3NMAxxh/QhgHUCThAfREzfPXbmiicrUfaudN4YFivpoFwKIAs\n\
NczsL9S3FPbzAB4nLDATacc2BK0//aKMOU2t3KLNNomKbzlR+EW3wd0F1GoZ9SY6\n\
sCQeLa8Wp1KarOmbvgoFL/DAiTSqjjsU/Lq24dOCCctmG+qXRZxQa4npHD4xJwQJ\n\
qzA0JLu4n+DgoJftm1KpvB0wuzTn6M9+wnk5rv/fGc2t4Zra8B4prEReZZVfy65d\n\
cb8pBdb20Yrmznj+6DR50X/o/8Qzoyj9XpxtjwF23ql0XPYCI7kB03Ms9euP0btc\n\
HFacHapm0qBKx+vWy0V2Qf482OWSbewqaRbud44sErNoKqpqm02yN8PpsCywpFUj\n\
UC5G5DzxzYspMzQv/yidti0scMSKFObseZmNGlRYymCWhXnxmoCFjLpw5RnJSB2+\n\
cZ/1KFFHHZI=\n\
-----END CERTIFICATE-----\n";

static char *mqtt_cc = 
"-----BEGIN CERTIFICATE-----\n\
MIIEhDCCAuwCCQDuE1BpeAeMwzANBgkqhkiG9w0BAQsFADCBgjELMAkGA1UEBhMC\n\
Q04xCzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFTDEL\n\
MAkGA1UECwwCU1QxFjAUBgNVBAMMDTExMi4zMS44NC4xNjQxIjAgBgkqhkiG9w0B\n\
CQEWE2VkZGllLnpoYW5nQHF1ZWN0ZWwwIBcNMjIwMTI1MDcyMzI3WhgPMjEyMjAx\n\
MDEwNzIzMjdaMIGCMQswCQYDVQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcM\n\
AkhGMRAwDgYDVQQKDAdRVUVDVEVMMQswCQYDVQQLDAJTVDEWMBQGA1UEAwwNMTEy\n\
LjMxLjg0LjE2NDEiMCAGCSqGSIb3DQEJARYTZWRkaWUuemhhbmdAcXVlY3RlbDCC\n\
AaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoCggGBAMt3cjY0eLEDqv8Y7FomA+7N\n\
G5ztAbR7+P/WxjPlodqRDZ5HQORkfAr44gAZcWsKoo4DHTInwr9JBbBnETBMnL8+\n\
13h1PRp5CfwXKFvjppWYvBZfeTwhWQYbSMKINoS+d1Zl11jg/+ZbSd7Fi0bYq8ip\n\
Hbt30H+NANQZP1XQdsCf5/kvn+vXiP4EgJc56JQ9L6ALIF2Q6F3G/PTaYItg463N\n\
lv/S+eRi1VMDSs8Qc+DTlVwlgZZJdSlC8Yjr5pVqoyXm8ENKfSTrdhrLiKSWJTz9\n\
JUr04E7SJ+CoBAnLYNPHR2y0CFS/15aCa1JbK27ZJ/0cvBvzpWdkcgrDtKIcxNYM\n\
9QFPpehb1N4pgqi0NPhCkc/BasfmXUaTwM4ghhi4tQRptKMdTN/kdyC+V5a8Hyhb\n\
Nvw5qeJlLJKpgZ9X3HQzuKstKMkxLNuDIzK9TvO7zLowr+0BetUdllq+fDjXQM0M\n\
+9P3Xv2VmDwGRkmZ0IjYpDjm+qqGTFVLzzVwEqVD6wIDAQABMA0GCSqGSIb3DQEB\n\
CwUAA4IBgQAuNVwkBhd5nyWMmV/ESNxy59Sz+5FcesGclKjs4YocgcKbLD2bS+LN\n\
lKk6zenES7Cq6+l3NMAxxh/QhgHUCThAfREzfPXbmiicrUfaudN4YFivpoFwKIAs\n\
NczsL9S3FPbzAB4nLDATacc2BK0//aKMOU2t3KLNNomKbzlR+EW3wd0F1GoZ9SY6\n\
sCQeLa8Wp1KarOmbvgoFL/DAiTSqjjsU/Lq24dOCCctmG+qXRZxQa4npHD4xJwQJ\n\
qzA0JLu4n+DgoJftm1KpvB0wuzTn6M9+wnk5rv/fGc2t4Zra8B4prEReZZVfy65d\n\
cb8pBdb20Yrmznj+6DR50X/o/8Qzoyj9XpxtjwF23ql0XPYCI7kB03Ms9euP0btc\n\
HFacHapm0qBKx+vWy0V2Qf482OWSbewqaRbud44sErNoKqpqm02yN8PpsCywpFUj\n\
UC5G5DzxzYspMzQv/yidti0scMSKFObseZmNGlRYymCWhXnxmoCFjLpw5RnJSB2+\n\
cZ/1KFFHHZI=\n\
-----END CERTIFICATE-----\n";

static char *mqtt_ck = 
"-----BEGIN RSA PRIVATE KEY-----\n\
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\n\
-----END RSA PRIVATE KEY-----\n";


static char *http_ca = 
"-----BEGIN CERTIFICATE-----\n\
MIIE4zCCA0ugAwIBAgIJAIaV+KPqwKDTMA0GCSqGSIb3DQEBCwUAMIGGMQswCQYD\n\
VQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcMAkhGMRAwDgYDVQQKDAdRVUVD\n\
VEVMMQswCQYDVQQLDAJTVDEWMBQGA1UEAwwNMTEyLjMxLjg0LjE2NDEmMCQGCSqG\n\
SIb3DQEJARYXZWRkaWUuemhhbmdAcXVlY3RlbC5jb20wIBcNMjIwMTI1MDYyMTA2\n\
WhgPMjEyMjAxMDEwNjIxMDZaMIGGMQswCQYDVQQGEwJDTjELMAkGA1UECAwCQUgx\n\
CzAJBgNVBAcMAkhGMRAwDgYDVQQKDAdRVUVDVEVMMQswCQYDVQQLDAJTVDEWMBQG\n\
A1UEAwwNMTEyLjMxLjg0LjE2NDEmMCQGCSqGSIb3DQEJARYXZWRkaWUuemhhbmdA\n\
cXVlY3RlbC5jb20wggGiMA0GCSqGSIb3DQEBAQUAA4IBjwAwggGKAoIBgQDLd3I2\n\
NHixA6r/GOxaJgPuzRuc7QG0e/j/1sYz5aHakQ2eR0DkZHwK+OIAGXFrCqKOAx0y\n\
J8K/SQWwZxEwTJy/Ptd4dT0aeQn8Fyhb46aVmLwWX3k8IVkGG0jCiDaEvndWZddY\n\
4P/mW0nexYtG2KvIqR27d9B/jQDUGT9V0HbAn+f5L5/r14j+BICXOeiUPS+gCyBd\n\
kOhdxvz02mCLYOOtzZb/0vnkYtVTA0rPEHPg05VcJYGWSXUpQvGI6+aVaqMl5vBD\n\
Sn0k63Yay4ikliU8/SVK9OBO0ifgqAQJy2DTx0dstAhUv9eWgmtSWytu2Sf9HLwb\n\
86VnZHIKw7SiHMTWDPUBT6XoW9TeKYKotDT4QpHPwWrH5l1Gk8DOIIYYuLUEabSj\n\
HUzf5HcgvleWvB8oWzb8OaniZSySqYGfV9x0M7irLSjJMSzbgyMyvU7zu8y6MK/t\n\
AXrVHZZavnw410DNDPvT9179lZg8BkZJmdCI2KQ45vqqhkxVS881cBKlQ+sCAwEA\n\
AaNQME4wHQYDVR0OBBYEFNUvIPGNPgsC8f1SRtlwaRCLj1eYMB8GA1UdIwQYMBaA\n\
FNUvIPGNPgsC8f1SRtlwaRCLj1eYMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEL\n\
BQADggGBAGEiAVP/Z2odReW2NmwvZ9vtccTe7eQX4XpQqrpgnnRu59bldPi9IXBc\n\
4VDtmJeVnGEYSIAgkG5u8kItKdeYqHpU4ujXGxbqV2fR8o8+7ywrDWVAn/01guUF\n\
eZ4CtHnXse1i1Jr73CRlskYYajJrE6wwe1GdcBolzXa/2I//C9hdonfiUNz/kJsL\n\
4Wn2/tacgXWbBlVqZvoKpqap2halu9zC/vZNTWj8/AV1pXBZGAcTeffdF35O/Aeo\n\
+5QIhUPJgezuNunyIHyk7wOyDVC6BmgcF+633zVJcz7WUPPXOZNOQg0zijCkwYOg\n\
uv9L7syz2k82TW0GaYz7YGAp4Ocfza9ltj73QDIQpa70HVFgLTr0VAvN5xLY0nDB\n\
Ff+2jSOx087lVK++QfgDuRtQ2iWljCSab5Tdfp7TPR4LrljaXE4C2IXDi9FJ+vuL\n\
P/+3qn1h1ZMzpLVTLPt8iqLJv0soAKeHXB96TETAZDa0pk+4VhadOo8WoKuAm3k9\n\
EdJNFsgC7A==\n\
-----END CERTIFICATE-----\n";

static char *http_cc = 
"-----BEGIN CERTIFICATE-----\n\
MIIEjDCCAvQCCQDUEg/AtXowWjANBgkqhkiG9w0BAQsFADCBhjELMAkGA1UEBhMC\n\
Q04xCzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFTDEL\n\
MAkGA1UECwwCU1QxFjAUBgNVBAMMDTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0B\n\
CQEWF2VkZGllLnpoYW5nQHF1ZWN0ZWwuY29tMCAXDTIyMDEyNTA2MjI0MloYDzIx\n\
MjIwMTAxMDYyMjQyWjCBhjELMAkGA1UEBhMCQ04xCzAJBgNVBAgMAkFIMQswCQYD\n\
VQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFSzELMAkGA1UECwwCU1QxFjAUBgNVBAMM\n\
DTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0BCQEWF2VkZGllLnpoYW5nQHF1ZWN0\n\
ZWwuY29tMIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEA46qNt9Xs+qis\n\
XaMaE3shUIYQQq1V0cbkwK3eMlK10B3hsx+/i8sH98T07b0tdDoBpzpwlBFx6LhO\n\
zypJRcPgIiDIEwfB+0oj8BG2/b3BDBDjbMNnFG/vcR7al8HFeqcThtXFTRlaIHay\n\
bRrxOQ/477VIBPpsw3G3YGpFFyKaIW2yw9GlGBAc0AETGKACkEXf8mgtLhTq0XGS\n\
9bNMT8H1o40cyP6AxXdzF/jCKOy1YxtJ1CLBHXfNObnMhV0LwGcf2xkv2i5XHlnL\n\
KHImNDeZmpFuYboHOg1EORRx0Vz27IzX/OC370j4y3FAhPBgx7NPV0/glNejM8VZ\n\
sxE62OezRenOGMxTvvG5uGNoiyRUWTQfzM2Jv+Ym5RCNhVnW5RLiwDi2OuCt1xU2\n\
x5d+ncETcltznhiUY7e01LP9nXQllFW/h6jSOcdxEvBRNdlpIbdY5ZBI4XuH1riN\n\
6Rq3VKSdDJhoXdmhA2471lF3mYyhdy/3Isjez0nElrHQFVRtTYatAgMBAAEwDQYJ\n\
KoZIhvcNAQELBQADggGBAEEKY4568flzjFCYjjD5CEzIuLpFUyAtmIx6J8dLaEY1\n\
vyl3+JcYZqERJTfJu4Au0CjeAN3oNis0uKnjONszLU8uBFybqvcG2C45RsmkymEM\n\
TwiElLLk7yiVhjscjK91AIxRMLgBd6WraNSM7l88LI8SAOjGx/p3GIhHZzSMF1TN\n\
TKGGNM2lBg/MKwulvvAuqRIHyuElwakcW7z3AF9cUtRe6yeyMp8keJgtCXb71VHz\n\
LoSRx7yBwxTpownquy2zWMf1LqKQk1fddth7+ZSj2ft4k7BferFEnBALV2ws3pBX\n\
g3THUydNZyOIoB/iCXza2+daroizGDBiRY4zoLE9azlzLttfPCpISRQEdaX87K2i\n\
J6OhjDsaobOPoYSu8d11fI7vOuoIjk5VS2UKBfnaGWlF3xHxXJYHyhE6/rijtvyi\n\
AhWLjWtIIftg0Njmt0YVTWen1Iczd4oanYwPzVFs9AmHhyBJDsFRm5STcDyDbjzy\n\
Q+Ep7YHxMYXseaUKVeZ8+Q==\n\
-----END CERTIFICATE-----\n";

static char *http_ck = 
"-----BEGIN RSA PRIVATE KEY-----\n\
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\n\
-----END RSA PRIVATE KEY-----\n";

static char *http_test_data = "START123456789abcdefghijklmnopqrstuvwxyzEND";

/*
* This function does not take into account all urc, such as bluetooth-related urc, please add it yourself if necessary
*/
static int ql_wifi_urc_process(char *data)
{
    if(strstr(data, "+QSTASTAT:") != NULL)
    {
        if(strstr(data, "GOT_IP"))
        {
            g_ql_wifi_sta_status = GOT_IP;
            ql_rtos_semaphore_release(ql_wifi_status_semaRef);
        }
        else if(strstr(data, "WLAN_DISCONNECTED"))
        {
            g_ql_wifi_sta_status = WLAN_DISCONNECTED;
        }
        else if(strstr(data, "WLAN_CONNECTED")){
            g_ql_wifi_sta_status = WLAN_CONNECTED;
        }
        else if(strstr(data, "SCAN_NO_AP"))
        {
            g_ql_wifi_sta_status = SCAN_NO_AP;
        }
        printf("g_ql_wifi_sta_status = %d \n", g_ql_wifi_sta_status);
    }
    else if(strstr(data, "+QAPSTAT:") != NULL)
    {

    }
    else if(strstr(data, "+QOTASTAT:") != NULL)
    {

    }
    else if(strstr(data, "+QIURC:") != NULL)
    {
        if(strstr(data, "recv"))
        {
            ql_wifi_data_queue_t tcp_data = {0};
            char *token;
            // 解析数据长度
            token = strtok(data, ",");
            token = strtok(NULL, ",");
            token = strtok(NULL, "\r\n");
            tcp_data.len = atoi(token);

            // 解析数据内容
            token = strtok(NULL, "\r\n");
            memcpy(tcp_data.data, token, tcp_data.len);

            printf("tcp_data.len = %d\n", tcp_data.len);  // 输出: 0
            printf("tcp_data.data = %s\n", tcp_data.data);  // 输出: Hello World
            ql_rtos_queue_release(ql_wifi_tcp_queue, sizeof(ql_wifi_data_queue_t), (u8*)(&tcp_data), 0);
        }
    }
    else if(strstr(data, "+QSSLURC:") != NULL)
    {

    }
    else if(strstr(data, "+QMTSTAT:") != NULL)
    {

    }
    else if(strstr(data, "+QMTRECV:") != NULL)
    {
        ql_wifi_data_queue_t mqtt_data = {0};
        mqtt_data.len = strlen(data);
        strcpy(mqtt_data.data, data);
        ql_rtos_queue_release(ql_wifi_mqtt_queue, sizeof(ql_wifi_data_queue_t), (u8*)(&mqtt_data), 0);
    }
    else if(strstr(data, "+QHTTPURC:") != NULL)
    {

    }
    else if(strstr(data, "RDY") != NULL)
    {

    }
    else
    {
        return FALSE;
    }

    printf("%s, %s\n", __FUNCTION__, data);
    return TRUE;
}


static bool ql_wifi_send_at_sync(char *at_cmd, char *expected_resp, char *resp)
{
    ql_wifi_data_queue_t msg = {0};

    printf("%s, at cmd:%s\n", __FUNCTION__, at_cmd);

    /*send at cmd*/
    ql_uart_write(WIFI_AT_PORT, at_cmd, strlen(at_cmd));

    if(expected_resp)
    {
WAIT_RESP:
        /*wait response*/
        ql_rtos_queue_wait(ql_wifi_at_queue, (u8*)(&msg), sizeof(msg), 0xFFFFFFFF);

        printf("%s,at resp:%s\n", __FUNCTION__, msg.data, msg.len);

        /*Judge whether it is expected response*/
        if(strstr(msg.data, expected_resp) == NULL)
        {
            goto WAIT_RESP;
        }
    }

    if(resp)
    {
        strcpy(resp, msg.data);
    }

    return TRUE;
}

static void ql_wifi_uart_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	ql_wifi_data_queue_t msg = {0};
	msg.len = ql_uart_read(port, msg.data, sizeof(msg.data));

    /*processing urc from wifi*/
    if(ql_wifi_urc_process(msg.data) == FALSE)
    {
        /*if it is not urc, send queue to ql_wifi_send_at_sync_sync*/
        ql_rtos_queue_release(ql_wifi_at_queue, sizeof(ql_wifi_data_queue_t), (u8*)(&msg), 0);
    }
}

static int ql_wifi_init(void)
{
    int ret = -1;
    ql_uart_config_t dcb;

    ql_rtos_queue_create(&ql_wifi_at_queue, sizeof(ql_wifi_data_queue_t), 10);

    ql_rtos_semaphore_create(&ql_wifi_status_semaRef, 0);

    /*Init uart port, the wifi uart baud rate is 115200*/
    ret = ql_uart_open(WIFI_AT_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
	if (ret) {
		printf("open uart[%d] failed! %d \n", WIFI_AT_PORT, ret);
		return ret;
	}

    /*register callback to read uart data*/
    ql_uart_register_cb(WIFI_AT_PORT, ql_wifi_uart_callback);

    /*Set wifi uart baud rate to 1842000*/
    ql_wifi_send_at_sync("AT+QSETBAND=1842000\r\n","OK", NULL);

    /*Set 4G-module uart baud rate to 1842000*/
    ql_uart_get_dcbconfig(WIFI_AT_PORT, &dcb);
	dcb.baudrate = QL_UART_BAUD_1842000;
	ql_uart_set_dcbconfig(WIFI_AT_PORT, &dcb);

    ql_wifi_send_at_sync("ATE0\r\n", "OK", NULL);

    /*set wifi to station mode and connect ap: AT+QSTAAPINFO=<SSID>[,<pwd>]*/
    ql_wifi_send_at_sync("AT+QSTAAPINFO=\"Visitor-Quectel\",\"v@quectel\"\r\n","OK", NULL);

    /*wait sta connected and got ip*/
    ql_rtos_semaphore_wait(ql_wifi_status_semaRef, QL_WAIT_FOREVER);
    
    printf("<--------------sta connected and got ip---------------->\r\n");
}

static void ql_wifi_tcp_test(void * argv)
{
	int write_bytes = 0;
    char *send_data = "Hello World";
    char at_cmd_buf[128] = {0};
    ql_wifi_data_queue_t tcp_data = {0};

	printf("<-----------------ql_wifi_tcp_test start---------------------->\r\n");

    ql_rtos_queue_create(&ql_wifi_tcp_queue, sizeof(ql_wifi_data_queue_t), 10);

    ql_wifi_init();

    /*connect to tcp server: AT+QIOPEN=<connectID>,<service_type>,<IP_address>/<domain_name>,<remote_port>[,<local_port>[,<access_mode>]]*/
    ql_wifi_send_at_sync("AT+QIOPEN=0,\"TCP\",\"112.31.84.164\",8305,8000,1\r\n", "+QIOPEN: 0,0", NULL);

    /*send data to tcp server: AT+QISEND=<connectID>,<send_length>*/
    sprintf(at_cmd_buf, "AT+QISEND=0,%d\r\n", strlen(send_data));
    ql_wifi_send_at_sync(at_cmd_buf, ">", NULL);
    ql_wifi_send_at_sync(send_data, "SEND OK", NULL);

    /*The tcp server we used will send the received data back to the tcp client.*/
    ql_rtos_queue_wait(ql_wifi_tcp_queue, (u8*)(&tcp_data), sizeof(tcp_data), 0xFFFFFFFF);
    if(strstr(send_data, tcp_data.data)){
        printf("<-----------------ql_wifi_tcp_test success---------------------->\r\n");
    }else{
        printf("<-----------------ql_wifi_tcp_test fail---------------------->\r\n");
    }

    /*disconnect to tcp server: AT+QICLOSE=<connectID>[,<timeout>]*/
    ql_wifi_send_at_sync("AT+QICLOSE=0\r\n","OK", NULL);

    printf("<-----------------ql_wifi_tcp_test end---------------------->\r\n");


    ql_rtos_queue_delete(ql_wifi_tcp_queue);
    ql_rtos_task_delete(NULL);
}

static ql_wifi_file_create(char *file_name, char *file_data, int file_len)
{
    char at_cmd_buf[64] = {0};

    sprintf(at_cmd_buf, "AT+QFDEL=\"%s\"\r\n",file_name);
    ql_wifi_send_at_sync(at_cmd_buf,NULL, NULL);

    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    sprintf(at_cmd_buf, "AT+QFOPEN=\"%s\"\r\n",file_name);
    ql_wifi_send_at_sync(at_cmd_buf, "OK", NULL);

    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    sprintf(at_cmd_buf, "AT+QFWRITE=1,%d\r\n",file_len);
    ql_wifi_send_at_sync(at_cmd_buf, "CONNECT", NULL);

    ql_wifi_send_at_sync(file_data, "OK", NULL);

    ql_wifi_send_at_sync("AT+QFCLOSE=1\r\n", "OK", NULL);
}

static void ql_wifi_mqtt_test(void * argv)
{
#define MQTT_SSL_LEVEL 2  //Can be set to 0 / 1 / 2

    ql_wifi_data_queue_t mqtt_data = {0};
    printf("<-----------------ql_wifi_mqtt_test start---------------------->\r\n");

    ql_rtos_queue_create(&ql_wifi_mqtt_queue, sizeof(ql_wifi_data_queue_t), 10);

    ql_wifi_init();

#if MQTT_SSL_LEVEL == 0
    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",0,0\r\n","OK", NULL);
#elif MQTT_SSL_LEVEL == 1
    /*Upload CA certificate*/
    ql_wifi_file_create("mqtt_ca.pem", mqtt_ca, strlen(mqtt_ca));

    /*Configure CA certificate.*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"cacert\",0,\"mqtt_ca.pem\"\r\n","OK", NULL);

    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",0,1\r\n","OK", NULL);

#elif MQTT_SSL_LEVEL == 2
    /*Upload CA / CC / CK certificate*/
    ql_wifi_file_create("mqtt_ca", mqtt_ca, strlen(mqtt_ca));
    ql_wifi_file_create("mqtt_cc", mqtt_cc, strlen(mqtt_cc));
    ql_wifi_file_create("mqtt_ck", mqtt_ck, strlen(mqtt_ck));

    /*Configure CA certificate.*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"cacert\",0,\"mqtt_ca\"\r\n","OK", NULL);
    ql_wifi_send_at_sync("AT+QSSLCFG=\"clientcert\",0,\"mqtt_cc\"\r\n","OK", NULL);
    ql_wifi_send_at_sync("AT+QSSLCFG=\"clientkey\",0,\"mqtt_ck\"\r\n","OK", NULL);

    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",0,2\r\n","OK", NULL);
#endif

    ql_wifi_send_at_sync("AT+QFLST\r\n","OK", NULL);

    /*Configure ssl cipher suites. AT+QSSLCFG="ciphersuite",<SSL_ctxID>[,<cipher_suites>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"ciphersuite\",0,0xFFFF\r\n","OK", NULL);
    
    /*AT+QSSLCFG="ignorelocaltime",<SSL_ctxID>[,<ignore_ltime>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"ignorelocaltime\",0,1\r\n","OK", NULL);

#if MQTT_SSL_LEVEL == 0
    /*Disable the ssl function of the mqtt client with id 0. AT+QMTCFG="ssl",<client_idx>[,<SSL_enable>[,<SSL_ctxID>]]*/
    ql_wifi_send_at_sync("AT+QMTCFG=\"ssl\",0,0,0\r\n","OK", NULL);

    /*Create an MQTT client with id 0.  AT+QMTOPEN=<client_idx>,<host_name>,<port> , +QMTOPEN: <client_idx>,<result>*/
    ql_wifi_send_at_sync("AT+QMTOPEN=0,\"112.31.84.164\",8306\r\n","+QMTOPEN: 0,0", NULL);
#elif MQTT_SSL_LEVEL == 1
    /*Enable the ssl function of the mqtt client with id 0. AT+QMTCFG="ssl",<client_idx>[,<SSL_enable>[,<SSL_ctxID>]]*/
    ql_wifi_send_at_sync("AT+QMTCFG=\"ssl\",0,1,0\r\n","OK", NULL);

    /*Create an MQTT client with id 0.  AT+QMTOPEN=<client_idx>,<host_name>,<port> , +QMTOPEN: <client_idx>,<result>*/
    ql_wifi_send_at_sync("AT+QMTOPEN=0,\"112.31.84.164\",8307\r\n","+QMTOPEN: 0,0", NULL);
#elif MQTT_SSL_LEVEL == 2
    /*Enable the ssl function of the mqtt client with id 0. AT+QMTCFG="ssl",<client_idx>[,<SSL_enable>[,<SSL_ctxID>]]*/
    ql_wifi_send_at_sync("AT+QMTCFG=\"ssl\",0,1,0\r\n","OK", NULL);

    /*Create an MQTT client with id 0.  AT+QMTOPEN=<client_idx>,<host_name>,<port> , +QMTOPEN: <client_idx>,<result>*/
    ql_wifi_send_at_sync("AT+QMTOPEN=0,\"112.31.84.164\",8308\r\n","+QMTOPEN: 0,0", NULL);
#endif

    /*
     Establish an MQTT connection that identify is 100. 
     AT+QMTCONN=<client_idx>,<clientID>[,<username>,<password>]
     +QMTCONN: <client_idx>,<result>[,<retcode>]
     */
    ql_wifi_send_at_sync("AT+QMTCONN=0,\"100\"\r\n","+QMTCONN: 0,0,0", NULL);

    /*Subscribe to Topic. AT+QMTSUB=<client_idx>,<msgID>,<topic1>,<qos1>[,<topic2>,<qos2>[,..]]*/
    ql_wifi_send_at_sync("AT+QMTSUB=0,1,\"topic/test\",2\r\n","+QMTSUB:", NULL);

    /*Publish Topic. AT+QMTPUB=<client_idx>,<msgID>,<qos>,<retain>,<topic>[,<length>]*/
    ql_wifi_send_at_sync("AT+QMTPUB=0,1,2,0,\"topic/test\",4\r\n",">", NULL);
    ql_wifi_send_at_sync("25.8", "+QMTPUB", NULL);

    /*Wait +QMTRECV: 0,1,"topic/test","25.8"  URC*/
    ql_rtos_queue_wait(ql_wifi_mqtt_queue, (u8*)(&mqtt_data), sizeof(mqtt_data), 0xFFFFFFFF);
    if(strstr(mqtt_data.data, "25.8")){
        printf("<-----------------ql_wifi_mqtt_test success---------------------->\r\n");
    }else{
        printf("<-----------------ql_wifi_mqtt_test fail---------------------->\r\n");
    }

    /*Close the MQTT client. AT+QMTCLOSE=<client_idx>*/
    ql_wifi_send_at_sync("AT+QMTCLOSE=0\r\n","OK", NULL);

    printf("<-----------------ql_wifi_mqtt_test end---------------------->\r\n");

    ql_rtos_queue_delete(ql_wifi_mqtt_queue);
    ql_rtos_task_delete(NULL);
}


static void ql_wifi_http_test(void * argv)
{
#define HTTP_SSL_LEVEL 0  //Can be set to 0 / 1 / 2

    ql_wifi_data_queue_t http_data = {0};
    char at_cmd_buf[128] = {0};

    printf("<-----------------ql_wifi_http_test start---------------------->\r\n");

    ql_wifi_init();
    
#if HTTP_SSL_LEVEL == 0
    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",1,0\r\n","OK", NULL);
#elif HTTP_SSL_LEVEL == 1
    /*Upload CA certificate*/
    ql_wifi_file_create("http_ca.pem", http_ca, strlen(http_ca));

    /*Configure CA certificate.*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"cacert\",1,\"http_ca.pem\"\r\n","OK", NULL);

    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",1,1\r\n","OK", NULL);

#elif HTTP_SSL_LEVEL == 2
    /*Upload CA / CC / CK certificate*/
    ql_wifi_file_create("http_ca", http_ca, strlen(http_ca));
    ql_wifi_file_create("http_cc", http_cc, strlen(http_cc));
    ql_wifi_file_create("http_ck", http_ck, strlen(http_ck));

    /*Configure CA certificate.*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"cacert\",1,\"http_ca\"\r\n","OK", NULL);
    ql_wifi_send_at_sync("AT+QSSLCFG=\"clientcert\",1,\"http_cc\"\r\n","OK", NULL);
    ql_wifi_send_at_sync("AT+QSSLCFG=\"clientkey\",1,\"http_ck\"\r\n","OK", NULL);

    /*AT+QSSLCFG="seclevel",<SSL_ctxID>[,<seclevel>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"seclevel\",1,2\r\n","OK", NULL);
#endif

    ql_wifi_send_at_sync("AT+QFLST\r\n","OK", NULL);

    /*Configure ssl cipher suites. AT+QSSLCFG="ciphersuite",<SSL_ctxID>[,<cipher_suites>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF\r\n","OK", NULL);
    
    /*AT+QSSLCFG="ignorelocaltime",<SSL_ctxID>[,<ignore_ltime>]*/
    ql_wifi_send_at_sync("AT+QSSLCFG=\"ignorelocaltime\",1,1\r\n","OK", NULL);

    /*AT+QHTTPCFG="sslctxid"[,<sslctxID>]*/
    ql_wifi_send_at_sync("AT+QHTTPCFG=\"sslctxid\",1\r\n","OK", NULL);

#if HTTP_SSL_LEVEL == 0
    /*AT+QHTTPCFG="url"[,<URL_string>]*/
    ql_wifi_send_at_sync("AT+QHTTPCFG=\"url\",\"http://112.31.84.164:8300/160B.txt\"\r\n","OK", NULL);
#elif HTTP_SSL_LEVEL == 1
    /*AT+QHTTPCFG="url"[,<URL_string>]*/
    ql_wifi_send_at_sync("AT+QHTTPCFG=\"url\",\"https://112.31.84.164:8301/160B.txt\"\r\n","OK", NULL);
#elif HTTP_SSL_LEVEL == 2
    /*AT+QHTTPCFG="url"[,<URL_string>]*/
    ql_wifi_send_at_sync("AT+QHTTPCFG=\"url\",\"https://112.31.84.164:8303/160B.txt\"\r\n","OK", NULL);
#endif

    /*AT+QHTTPGET[=<rsptime>],  +QHTTPGET: <err>[,<httprspcode>[,<content_length>]]*/
    ql_wifi_send_at_sync("AT+QHTTPGET=60\r\n", "+QHTTPGET:", NULL);

    /*AT+QHTTPREAD[=<wait_time>]*/
    ql_wifi_send_at_sync("AT+QHTTPREAD=60\r\n", "+QHTTPREAD:", NULL);

    printf("<-----------------ql_wifi_http_test end---------------------->\r\n");
exit:
    ql_rtos_task_delete(NULL);
}

// application_init(ql_wifi_tcp_test, "tcp_test", 10, 0);
// application_init(ql_wifi_mqtt_test, "mqtt_test", 10, 0);
// application_init(ql_wifi_http_test, "http_test", 10, 0);
