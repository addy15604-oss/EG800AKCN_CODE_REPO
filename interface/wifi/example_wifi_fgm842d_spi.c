
/*==========================================================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
===========================================================================================================*/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ql_wifi.h"
#include "ql_application.h"
#include "ql_log.h"
#include "ql_rtos.h"
#include "ql_nw.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_spi.h"
#include "sockets.h"
#include "ip4_addr.h"
#include "ql_data_call.h"
#include "ql_ping.h"
#include "ql_fs.h"
#include "ql_http_client.h"

#if 1 //EG800AKCN
#define QL_WIFI_LDO_GPIO             GPIO_PIN_NO_84     // Connect to WIFI LDO
#define QL_WIFI_CHIP_EN_GPIO         GPIO_PIN_NO_85     // Connect to FGM842D's CHIP_EN
#define QL_WIFI_SPI_PORT             SPI_PORT2
#define QL_WIFI_SPI_CS_GPIO          GPIO_PIN_NO_13
#define QL_WIFI_SPI_SLAVE_CTS_GPIO   GPIO_PIN_NO_90
#define QL_WIFI_SPI_SLAVE_RDY_GPIO   GPIO_PIN_NO_89
#define QL_WIFI_SPI_CLK              SPI_CLK_13MHZ
#define QL_WIFI_UART_PORT            QL_MAIN_UART_PORT
#else
//EC800KCN_LC
#define QL_WIFI_LDO_GPIO             GPIO_PIN_NO_73
#define QL_WIFI_CHIP_EN_GPIO         GPIO_PIN_NO_74     // Connect to FGM842D's CHIP_EN
#define QL_WIFI_SPI_PORT             SPI_PORT2
#define QL_WIFI_SPI_CS_GPIO          GPIO_PIN_NO_13
#define QL_WIFI_SPI_SLAVE_CTS_GPIO   GPIO_PIN_NO_90
#define QL_WIFI_SPI_SLAVE_RDY_GPIO   GPIO_PIN_NO_89
#define QL_WIFI_SPI_CLK              SPI_CLK_13MHZ
#define QL_WIFI_UART_PORT            QL_MAIN_UART_PORT  //QL_BT_UART_PORT
#endif

#define QL_WIFI_LDO_ENABLE           PIN_LEVEL_HIGH
#define QL_WIFI_LDO_DISABLE          PIN_LEVEL_LOW
#define QL_WIFI_ENABLE               PIN_LEVEL_LOW
#define QL_WIFI_DISABLE              PIN_LEVEL_HIGH

#define QL_WIFI_DEMO_SOCKET_BUF_SIZE (128)
#define QL_HTTP_CID 1
#define QL_WIFI_AP_MODE_CID 1
#define QL_WIFI_STA_MODE_CID 2
#define QL_WIFI_PING_NUM 4
#define QL_WIFI_OTA_FILE_NAME "FGM842DAAR02A01M02_SPI_NLbeta0710V01_ota.rbl"
#define QL_WIFI_OTA_FILE_PATH "U:/" QL_WIFI_OTA_FILE_NAME ""

#define QL_WIFI_OTA_DATA_FLAG 0x1
#define QL_WIFI_OTA_FINISH_FLAG 0x2
#define QL_WIFI_OTA_FAIL_FLAG 0x4

static char send_buf[QL_WIFI_DEMO_SOCKET_BUF_SIZE] = {0};
static int send_len = 0;
static char recv_buf[QL_WIFI_DEMO_SOCKET_BUF_SIZE] = {0};
static int recv_len = 0;
static int data_call_state = -1;

ql_queue_t wifi_msg_queue = NULL;
ql_queue_t wifi_ota_queue = NULL;
ql_flag_t wifi_ota_flag = NULL;
ql_task_t wifi_tcp_task = NULL;
ql_sem_t wifi_scan_sem = NULL;


static void ql_wifi_ldo_ctrl(bool enable)
{
    printf("wifi powers:%d", enable);
    static bool is_init = false;

    if (is_init == false)
    {
        is_init = true;

        ql_gpio_init(QL_WIFI_LDO_GPIO, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, QL_WIFI_LDO_DISABLE);
        ql_gpio_init(QL_WIFI_CHIP_EN_GPIO, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, QL_WIFI_DISABLE);
    }
 
    if (enable)
    {
        ql_gpio_set_level(QL_WIFI_LDO_GPIO, QL_WIFI_LDO_ENABLE);
        ql_gpio_set_level(QL_WIFI_CHIP_EN_GPIO, QL_WIFI_ENABLE);
    }
    else
    {
        ql_gpio_set_level(QL_WIFI_LDO_GPIO, QL_WIFI_LDO_DISABLE);
        ql_gpio_set_level(QL_WIFI_CHIP_EN_GPIO, QL_WIFI_DISABLE);
    }
}


static void ql_wifi_hardware_reset(void)
{
    //wifi的en引脚是低电平关机, 高电平开机.
    //需要注意硬件电路是否对蜂窝模组给出的gpio的高低电平进行反向. 如果反向下述代码需要做修改.
    ql_gpio_set_level(QL_WIFI_CHIP_EN_GPIO, QL_WIFI_DISABLE);
    ql_rtos_task_sleep_ms(100);
    ql_gpio_set_level(QL_WIFI_CHIP_EN_GPIO, QL_WIFI_ENABLE);
}

static void ql_wifi_callback(ql_wifi_ind_t *ctx)
{
    ql_wifi_ind_t wifi_ind = {0};
    memcpy(&wifi_ind, ctx, sizeof(ql_wifi_ind_t));
    if (wifi_msg_queue)
    {
        ql_rtos_queue_release(wifi_msg_queue, sizeof(ql_wifi_ind_t), (uint8 *)&wifi_ind, QL_NO_WAIT);
    }
    return;
}

static void ql_wifi_scan_callback(void *ctx)
{
    ql_wifi_scan_result_s *scan_result = (ql_wifi_scan_result_s *)ctx;
    ql_wifi_ap_info_s *ApList = scan_result->ApList;
    printf("scan_result->ApNum:%d\n", scan_result->ApNum);
    for (int i = 0; i < scan_result->ApNum; i++)
    {
        printf("ssid:%s,ap_power:%d,bssid=%02X:%02X:%02X:%02X:%02X:%02X,channel=%d,security=%s\n",
               ApList->ssid,
               ApList->ap_power,
               ApList->bssid[0],ApList->bssid[1],ApList->bssid[2],ApList->bssid[3],ApList->bssid[4],ApList->bssid[5],
               ApList->channel,
               ApList->security);

        ApList++;
    }
}

static void ql_wifi_data_call_cb(int profile_idx, int nw_status)
{
    data_call_state = nw_status;

    printf("ql_wifi_data_call_cb: profile(%d) status(%d) \r\n", profile_idx, data_call_state);
}

static void ql_wifi_data_call(uint8_t cid)
{
    int ret = 0;
    struct ql_data_call_info info = {0};
    char ip4_addr_str[16] = {0};
    QL_NW_REG_STATUS_INFO_T ql_nw_reg_status = {0};
    // Wait register network ok
    while (1)
    {
        printf("wait nw register\n");
        ql_nw_get_reg_status(&ql_nw_reg_status);
        if (ql_nw_reg_status.data_reg.state == QL_NW_REG_STATE_HOME_NETWORK ||
            ql_nw_reg_status.data_reg.state == QL_NW_REG_STATE_ROAMING ||
            ql_nw_reg_status.data_reg.state == QL_NW_REG_STATE_HOME_NETWORK_CSFB_NOT_PREFERRED ||
            ql_nw_reg_status.data_reg.state == QL_NW_REG_STATE_ROAMING_CSFB_NOT_PREFERRED)
        {
            printf("nw register ok\n");
            break;
        }
        ql_rtos_task_sleep_s(1);
    }

    ql_set_data_call_asyn_mode(1, ql_wifi_data_call_cb);

    // Start data call
    ret = ql_start_data_call(cid, 0, NULL, NULL, NULL, 0);
    printf("ql_start_data_call  ret=%d\r\n", ret);

    // Wait data call ok
    while (data_call_state == -1)
    {
        ql_rtos_task_sleep_s(1);
        printf("Wait data call ok\n");
    }

    ql_get_data_call_info(cid, 0, &info);
    inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
    printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);
}

static void ql_wifi_test_assert()
{
    while(1)
    {
        ql_rtos_task_sleep_s(5);
        printf("ql_wifi_test_assert");
    }
}

static void ql_wifi_ap_test(void *argv)
{
    int ret = 0;
    char *ssid = "ningjw";
    char *password = "13535374141";
    ql_wifi_ind_t ql_wifi_status = {0};
    char wifi_version[64] = {0};

    ql_wifi_config_t wifi_config;
    /* Hardware-related configuration*/
    wifi_config.port_cfg = QL_WIFI_PORT_SPI_UART;
    wifi_config.spi_config.port = QL_WIFI_SPI_PORT;
    wifi_config.spi_config.clk = QL_WIFI_SPI_CLK;
    wifi_config.spi_config.mode = SPI_MODE0;
    wifi_config.spi_config.cs = QL_WIFI_SPI_CS_GPIO;
    wifi_config.spi_config.slave_rdy = QL_WIFI_SPI_SLAVE_RDY_GPIO;
    wifi_config.spi_config.slave_cts = QL_WIFI_SPI_SLAVE_CTS_GPIO;
    wifi_config.uart_port = QL_WIFI_UART_PORT;

    /* Software-related configuration*/
    wifi_config.mode = QL_WIFI_MODE_AP;
    wifi_config.simid = 0;
    wifi_config.cid = QL_WIFI_AP_MODE_CID;
    wifi_config.auth_type = QL_WIFI_AUTH_TYPE_WPA_WPA2;
    memcpy(wifi_config.ssid, ssid, strlen(ssid) + 1);
    memcpy(wifi_config.password, password, strlen(password) + 1);

    // Set log port
    ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART);
    printf("ql_wifi_ap_test start\n");

    /* Create msg queue. */
    if (0 != ql_rtos_queue_create(&wifi_msg_queue, sizeof(ql_wifi_ind_t), 10))
    {
        printf("ql_rtos_queue_create failed");
    }

    /* 842D uses MAIN UART. Make sure the MAIN UART is not used before WIFI init. */
    ql_uart_close(QL_WIFI_UART_PORT);

    ret = ql_wifi_set_config(&wifi_config);
    if (ret == -1)
    {
        printf("ql_wifi_set_config failed\n");
        /* The process should not continue if initialization fails. */
        //ql_wifi_test_assert();
    }

#if 1
    ql_wifi_ldo_ctrl(true);
    ql_wifi_hardware_reset();
#else
    ql_wifi_soft_reset();
#endif
    printf("reset wifi\n");
    // wait wifi reset ok
    ql_rtos_task_sleep_s(1);

    ql_wifi_get_version(wifi_version, sizeof(wifi_version));
    printf("wifi version=%s\n", wifi_version);

    ql_wifi_data_call(wifi_config.cid);

    // Start wifi as ap mode
    ret = ql_wifi_open(ql_wifi_callback);
    printf("ql_wifi_open %d\n", ret);
    while (1)
    {
        ql_rtos_queue_wait(wifi_msg_queue, (uint8_t *)&ql_wifi_status, sizeof(ql_wifi_ind_t), QL_WAIT_FOREVER);

        printf("ind_type=%d, wifi status=%d, mac=%s\n", ql_wifi_status.ind_type ,ql_wifi_status.state, ql_wifi_status.mac_str);

        switch (ql_wifi_status.state)
        {
        case QL_WIFI_IND_STATE_CONNECTED:
        {
        }
        break;
        case QL_WIFI_IND_STATE_DISCONNECT:
        {
        }
        break;
        default:
        {
            break;
        }
        }
    }
}

static void ql_wifi_tcp_client_thread(void)
{
    int ret = 0;
    int i = 0;
    int flags = 0;
    int connected = 0;
    int socket_fd = -1;
    fd_set read_fds;
    fd_set write_fds;
    fd_set exp_fds;
    int fd_changed = 0;
    int closing = false;
    struct sockaddr_in local4, server_ipv4;
    ip_addr_t ip = {0};
    char ip4_addr_str[16] = {0};
    struct ql_data_call_info info = {0};

    uint8_t destip[] = {112, 31, 84, 164};
    uint16_t destport = 8305;
    ip_addr_t int_destip;
    IP4_ADDR(&int_destip, destip[0], destip[1], destip[2], destip[3]);

    ql_get_data_call_info(QL_WIFI_STA_MODE_CID, 0, &info);
    inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
    printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);

    inet_aton(ip4_addr_str, &ip);

    memset(&local4, 0x00, sizeof(struct sockaddr_in));
    local4.sin_family = AF_INET;
    local4.sin_port = 2345;
    memcpy(&local4.sin_addr, &ip.addr, sizeof(ip_addr_t));
    // inet_aton(ip_ntoa(&ip.addr), (ip_addr_t*)&local4.sin_addr);

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exp_fds);

    printf("socket start!\r\n");
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("*** socket create fail ***\r\n");
        goto exit;
    }
    ret = bind(socket_fd, (struct sockaddr *)&local4, sizeof(struct sockaddr));
    if (ret < 0)
    {
        printf("*** bind fail ***\r\n");
        close(socket_fd);
        socket_fd = -1;
        goto exit;
    }
    flags |= O_NONBLOCK;
    fcntl(socket_fd, F_SETFL, flags);
    memset(&server_ipv4, 0x00, sizeof(struct sockaddr_in));
    server_ipv4.sin_family = AF_INET;
    server_ipv4.sin_port = htons(destport);
    memcpy(&(server_ipv4.sin_addr), &int_destip, sizeof(int_destip));
    ret = connect(socket_fd, (struct sockaddr *)&server_ipv4, sizeof(server_ipv4));
    if (ret == 0)
    {
        connected = 1;
        printf("tcp connected\r\n");
    }
    else
    {
        if (ql_get_soc_errno() != EINPROGRESS)
        {
            printf("tcp connect failed\r\n");
            close(socket_fd);
            socket_fd = -1;
            goto exit;
        }

        FD_SET(socket_fd, &write_fds);
    }

    FD_SET(socket_fd, &exp_fds);

    while (1)
    {
        fd_changed = select(socket_fd + 1, &read_fds, &write_fds, &exp_fds, NULL);
        if (fd_changed > 0)
        {
            if (FD_ISSET(socket_fd, &write_fds))
            {
                FD_CLR(socket_fd, &write_fds);
                if (connected == 0)
                {
                    int value = 0;
                    char con[128] = {0};
                    u32_t len = sizeof(value);
                    getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &value, &len);
                    if (value == 0 || value == EISCONN)
                    {
                        printf("tcp connected");
                        connected = 1;
                        FD_SET(socket_fd, &read_fds);
                        len = snprintf(con, 128, "tcp connected\r\n");
                        write(socket_fd, con, len);
                    }
                    else
                    {
                        printf("tcp connect failed\r\n");
                        close(socket_fd);
                        socket_fd = -1;
                        break;
                    }
                }
                else
                {
                    memset(send_buf, 0x00, 128);
                    send_len = snprintf(send_buf, 128, "%d%s%d\r\n", i, "startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA#!&*1234end", i);
                    write(socket_fd, send_buf, send_len);
                    i++;
                }
            }
            else if (FD_ISSET(socket_fd, &read_fds))
            {
                FD_CLR(socket_fd, &read_fds);
                memset(recv_buf, 0x00, 128);
                recv_len = read(socket_fd, recv_buf, 128);
                if (recv_len > 0)
                {
                    printf("recv: %s\r\n", recv_buf);
                    memset(send_buf, 0x00, 128);
                    send_len = snprintf(send_buf, 128, "%d%s%d\r\n", i, "startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA#!&*1234end", i);
                    write(socket_fd, send_buf, send_len);
                    i++;
                    ql_rtos_task_sleep_ms(500);
                    FD_SET(socket_fd, &read_fds);
                }
                else if (recv_len == 0)
                {
                    if (closing == false)
                    {
                        printf("shut down\r\n");
                        shutdown(socket_fd, SHUT_WR);
                        closing = true;
                        FD_SET(socket_fd, &read_fds);
                    }
                    else
                    {
                        close(socket_fd);
                        socket_fd = -1;
                        break;
                    }
                }
                else
                {
                    if (ql_get_soc_errno() == EAGAIN)
                    {
                        FD_SET(socket_fd, &read_fds);
                    }
                    else
                    {
                        close(socket_fd);
                        socket_fd = -1;
                        break;
                    }
                }
            }
            else if (FD_ISSET(socket_fd, &exp_fds))
            {
                FD_CLR(socket_fd, &exp_fds);
                close(socket_fd);
                socket_fd = -1;
                break;
            }
        }
    }

exit:
    printf("wifi tcp client end\r\n");
    close(socket_fd);
    wifi_tcp_task = NULL;
    ql_rtos_task_delete(NULL);
}

static void ql_wifi_sta_test(void *argv)
{
    int ret = 0;
    char *ssid = "Visitor-Quectel";
    char *password = "v@quectel";
    ql_wifi_ind_t ql_wifi_status = {0};
    char wifi_version[64] = {0};

    ql_wifi_config_t wifi_config;
    /* Hardware-related configuration*/
    wifi_config.port_cfg = QL_WIFI_PORT_SPI_UART;
    wifi_config.spi_config.port = QL_WIFI_SPI_PORT;
    wifi_config.spi_config.clk = QL_WIFI_SPI_CLK;
    wifi_config.spi_config.mode = SPI_MODE0;
    wifi_config.spi_config.cs = QL_WIFI_SPI_CS_GPIO;
    wifi_config.spi_config.slave_rdy = QL_WIFI_SPI_SLAVE_RDY_GPIO;
    wifi_config.spi_config.slave_cts = QL_WIFI_SPI_SLAVE_CTS_GPIO;
    wifi_config.uart_port = QL_WIFI_UART_PORT;

    /* Software-related configuration */
    wifi_config.mode = QL_WIFI_MODE_STA;
    wifi_config.simid = 0;
    wifi_config.cid = QL_WIFI_STA_MODE_CID;
    wifi_config.auth_type = QL_WIFI_AUTH_TYPE_WPA_WPA2;
    memcpy(wifi_config.ssid, ssid, strlen(ssid) + 1);
    memcpy(wifi_config.password, password, strlen(password) + 1);

    // Set log port
    ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART);
    printf("ql_wifi_sta_test\n");

    /* Create msg queue. */
    if (0 != ql_rtos_queue_create(&wifi_msg_queue, sizeof(ql_wifi_ind_t), 10))
    {
        printf("ql_rtos_queue_create failed");
    }

    /* 842D uses MAIN UART. Make sure the MAIN UART is not used before WIFI init. */
    ql_uart_close(QL_WIFI_UART_PORT);

    ret = ql_wifi_set_config(&wifi_config);
    if (ret == -1)
    {
        printf("ql_wifi_set_config failed\n");
        /* The process should not continue if initialization fails. */
        //ql_wifi_test_assert();
    }

#if 0
    ql_wifi_hardware_reset();
#else
    ql_wifi_soft_reset();
#endif
    printf("reset wifi\n");
    // wait wifi reset ok
    ql_rtos_task_sleep_s(1);

    ql_wifi_get_version(wifi_version, sizeof(wifi_version));
    printf("wifi version=%s\n", wifi_version);
#if 0
    // Start wifi scan
    ql_wifi_scan(ql_wifi_scan_callback);
#endif
    // Start wifi as station mode
    ret = ql_wifi_open(ql_wifi_callback);
    printf("ql_wifi_open %d\n", ret);
    while (1)
    {
        ql_rtos_queue_wait(wifi_msg_queue, (uint8_t *)&ql_wifi_status, sizeof(ql_wifi_ind_t), QL_WAIT_FOREVER);
        printf("ind_type=%d, wifi status=%d, mac=%s\n", ql_wifi_status.ind_type ,ql_wifi_status.state, ql_wifi_status.mac_str);

        switch (ql_wifi_status.state)
        {
        case QL_WIFI_IND_STATE_CONNECTED:
        {
            if (wifi_tcp_task == NULL)
            {
                ret = ql_rtos_task_create(&wifi_tcp_task, 4096, 100, "wifi_tcp", ql_wifi_tcp_client_thread, NULL, 0);
            }
        }
        break;
        case QL_WIFI_IND_STATE_DISCONNECT:
        {
        }
        break;
        default:
        {
            break;
        }
        }
    }
}

static void ql_wifi_ota_callback(void *ctx)
{
    uint8_t ota_status = *(uint8_t *)ctx;
    printf("ota status = %d\n", ota_status);
    switch (ota_status)
    {
    case QL_WIFI_OTA_WRITE_NORMAL:
        if (wifi_ota_flag)
        {
            ql_rtos_flag_release(wifi_ota_flag, QL_WIFI_OTA_DATA_FLAG, QL_FLAG_OR);
        }
        break;
    case QL_WIFI_OTA_WRITE_DONE:
        printf("All ota data write completed, wait wifi auto restart for Upgrade\n");
        break;
    case QL_WIFI_OTA_REBOOT_DONE:
        if (wifi_ota_flag)
        {
            ql_rtos_flag_release(wifi_ota_flag, QL_WIFI_OTA_FINISH_FLAG, QL_FLAG_OR);
        }
        break;
    default:
        printf("wifi OTA failed\n");
        break;
    }
}
static int http_response_cb(QL_HTTP_CLIENT_T *client, QL_HTTP_CLIENT_EVENT_E event, int status_code, char *data, int data_len, void *private_data)
{
    int ret = 0;
    uint32 ota_flag = 0;
    ql_wifi_ota_data_head *ota_data = (ql_wifi_ota_data_head *)private_data;
    ql_wifi_ota_data_head ql_wifi_ota_status = {0};

    switch (event)
    {
    case QL_HTTP_CLIENT_EVENT_SEND_FAIL:
        printf("http send failed!\n");
        break;
    case QL_HTTP_CLIENT_EVENT_SEND_SUCCESSED:
        printf("http send successed!\n");
        break;
    case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FAIL:
        printf("http parse response header failed!\n");
        break;
    case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FINISHED:
        printf("http recv header,status_code=%d\n", status_code);
        if (status_code == 200)
        {
            ota_data->total_size = client->response.response_data_length;
            printf("total_size=%d\n", ota_data->total_size);
        }
        else
        {
            ql_rtos_flag_release(wifi_ota_flag, QL_WIFI_OTA_FAIL_FLAG, QL_FLAG_OR);
            printf("http download firmware fail");
        }

        ret = 1; // 继续接受body数据
        break;
    case QL_HTTP_CLIENT_EVENT_RECV_BODY:
        /*
         *在传输ota数据给wifi模块时, 非最后一包数据,必须要传输2048字节的数据.
         *而http 模块接收数据时, 并不是一次获取2048字节的数据, 所以要做如下判断处理.
         */
        if (status_code == 200)
        {
            int out_size = data_len;
            int offset_size = 0;
            printf("data_len = %d\n", data_len);
            while (out_size > 0)
            {
                printf("ota_data->data_len=%d, out_size=%d, %d\n", ota_data->data_len, out_size, ota_data->data_len + out_size);
                // 缓存的数据+当前接受到的数据>2048, 可以发送给wifi了.
                if (ota_data->data_len + out_size >= QL_WIFI_OTA_DATA_LEN)
                {
                    // 计算将缓存填满,需要使用多少字节
                    offset_size = QL_WIFI_OTA_DATA_LEN - ota_data->data_len;

                    // 将数据追加到缓存后面, 让数据刚好有2048字节
                    memcpy(&ota_data->data.data + ota_data->data_len, data + (data_len - out_size), offset_size);

                    // 在调用ql_wifi_ota_data_send前设置ota_data->data_len的值, 用于表示本次发送给wifi的数据长度, ql_wifi_ota_data_send接口使用
                    // 其他时候ota_data->data_len理解为缓存在ota_data->data中的数据长度
                    ota_data->data_len = QL_WIFI_OTA_DATA_LEN;
                    ql_wifi_ota_data_send(ota_data);

                    // 等待wifi返回成功
                    ql_rtos_flag_wait(wifi_ota_flag, QL_WIFI_OTA_DATA_FLAG, QL_FLAG_OR_CLEAR, &ota_flag, QL_WAIT_FOREVER);

                    out_size -= offset_size;                      // 计算本次http下载的数据,还剩余多数数据未处理
                    ota_data->dload_size += QL_WIFI_OTA_DATA_LEN; // 已经发送给wifi的数据大小
                    ota_data->data_len = 0;                       // 缓存中的数据已发给wifi,清0
                }
                // 数据已经全部下载完了, 将剩余数据发送给wifi
                else if (ota_data->dload_size >= ota_data->total_size || ota_data->dload_size + out_size >= ota_data->total_size)
                {
                    memcpy(&ota_data->data.data + ota_data->data_len, data + (data_len - out_size), out_size);

                    ota_data->data_len = out_size;

                    // 将剩余数据发送给wifi
                    ql_wifi_ota_data_send(ota_data);

                    // 等待wifi返回成功
                    ql_rtos_flag_wait(wifi_ota_flag, QL_WIFI_OTA_DATA_FLAG, QL_FLAG_OR_CLEAR, &ota_flag, QL_WAIT_FOREVER);

                    ota_data->dload_size += out_size; // 已经发送给wifi的数据大小
                    ota_data->data_len = 0;           // 缓存中的数据已发给wifi,清0
                    printf("dload file finished\n");
                    break;
                }
                // 剩余数据不足2048 且 还有数据未下载
                else
                {
                    // 将剩余数据暂时保存在ota_data->data中
                    memcpy(&ota_data->data.data + ota_data->data_len, data + (data_len - out_size), out_size);
                    ota_data->data_len += out_size; // 记录保存在缓存中的数据长度
                    break;
                }
            }
        }
        else
        {
            ql_rtos_flag_release(wifi_ota_flag, QL_WIFI_OTA_FAIL_FLAG, QL_FLAG_OR);
        }
        ret = 1; // 继续接受body数据
        break;
    case QL_HTTP_CLIENT_EVENT_RECV_BODY_FINISHED:
        printf("http recv body finished!\n");
        break;
    case QL_HTTP_CLIENT_EVENT_DISCONNECTED:
        printf("http be closed by server!\n");
        break;
    default:
        break;
    }
    return ret;
}

static void ql_wifi_ota_test(void *argv)
{
    int ret = 0;
    uint32 ota_flag = 0;
    struct http_client *client = NULL;
    char *ssid = "ningjw";
    char *password = "13535374141";
    char wifi_old_version[64] = {0};
    char wifi_new_version[64] = {0};
    // char *url = "http://112.31.84.164:8300/fgm842d/FGM842DAAR02A01M02_SPI_NLbeta0710V01_ota.rbl";
    char *url = "http://112.31.84.164:8300/fgm842d/FGM842DAAR02A01M02_SPI_NLbeta0717V01_ota.rbl";

    // Set log port
    ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART);
    printf("ql_wifi_ota_test\n");

    ql_wifi_config_t wifi_config;
    /* Hardware-related configuration*/
    wifi_config.port_cfg = QL_WIFI_PORT_SPI_UART;
    wifi_config.spi_config.port = QL_WIFI_SPI_PORT;
    wifi_config.spi_config.clk = QL_WIFI_SPI_CLK;
    wifi_config.spi_config.mode = SPI_MODE0;
    wifi_config.spi_config.cs = QL_WIFI_SPI_CS_GPIO;
    wifi_config.spi_config.slave_rdy = QL_WIFI_SPI_SLAVE_RDY_GPIO;
    wifi_config.spi_config.slave_cts = QL_WIFI_SPI_SLAVE_CTS_GPIO;
    wifi_config.uart_port = QL_WIFI_UART_PORT;

    /* Software-related configuration*/
    wifi_config.mode = QL_WIFI_MODE_STA; //
    wifi_config.simid = 0;
    wifi_config.cid = QL_WIFI_AP_MODE_CID;
    wifi_config.auth_type = QL_WIFI_AUTH_TYPE_WPA_WPA2;
    memcpy(wifi_config.ssid, ssid, strlen(ssid) + 1);
    memcpy(wifi_config.password, password, strlen(password) + 1);

    /* 842D uses MAIN UART. Make sure the MAIN UART is not used before WIFI init. */
    ql_uart_close(QL_WIFI_UART_PORT);

    ret = ql_wifi_set_config(&wifi_config);
    if (ret == -1)
    {
        printf("ql_wifi_set_config failed\n");
        /* The process should not continue if initialization fails. */
        //ql_wifi_test_assert();
    }

#if 0
    ql_wifi_hardware_reset();
#else
    ql_wifi_soft_reset();
#endif
    printf("reset wifi\n");
    // wait wifi reset ok
    ql_rtos_task_sleep_s(1);

    ql_wifi_get_version(wifi_old_version, sizeof(wifi_old_version));
    printf("wifi_old_version=%s\n", wifi_old_version);

    ql_wifi_data_call(QL_HTTP_CID);

    client = ql_http_client_init();
    if (client == NULL)
    {
        printf("ql_http_client_init failed\n");
        goto exit;
    }

    if (0 != ql_rtos_flag_create(&wifi_ota_flag))
    {
        printf("ql_rtos_flag_create failed");
    }

    ql_wifi_ota_register_cb(ql_wifi_ota_callback);

    ql_wifi_ota_data_head *private_data = calloc(1, QL_WIFI_OTA_HEAD_LEN + QL_WIFI_OTA_DATA_LEN);
    printf("private_data = %p", private_data);

    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PDP_CID, QL_HTTP_CID); /*set PDP cid,if not set,using default PDP*/
    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PROTOCOL_VER, 1);      /*"0" is HTTP 1.1, "1" is HTTP 1.0*/
    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_ENABLE_COOKIE, 1);

    ret = ql_http_client_request(client, url, QL_HTTP_CLIENT_REQUEST_GET, QL_HTTP_CLIENT_AUTH_TYPE_NONE, NULL, NULL, NULL, 0, http_response_cb, private_data);
    if (ret == QL_HTTP_CLIENT_ERR_LAST_REQUEST_NOT_FINISH)
    {
        printf("last request not finish, can not to request again!\n");
    }

    ql_rtos_flag_wait(wifi_ota_flag, QL_WIFI_OTA_FINISH_FLAG | QL_WIFI_OTA_FAIL_FLAG, QL_FLAG_OR_CLEAR, &ota_flag, QL_WAIT_FOREVER);

    /*
     * 接受到QUEC_WIFI_OTA_REBOOT_DONE事件后,通过版本号判断是否升级成功.
     */
    ql_wifi_get_version(wifi_new_version, sizeof(wifi_new_version));
    printf("wifi_old_version=%s, wifi_new_version = %s\n", wifi_old_version, wifi_new_version);
    if (ota_flag & QL_WIFI_OTA_FINISH_FLAG)
    {
        if (strcmp(wifi_new_version, wifi_old_version) == 0)
        {
            printf("OTA upgrade failed\n");
        }
        else
        {
            printf("OTA upgrade successful\n");
        }
    }
    if (ota_flag & QL_WIFI_OTA_FAIL_FLAG)
    {
        printf("OTA upgrade failed\n");
    }
exit:
    printf("\n\n==============ql_wifi_ota_test end================\n");
    ql_rtos_flag_delete(wifi_ota_flag);
    ql_rtos_task_delete(NULL);
}

// application_init(ql_wifi_ap_test, "wifi_ap", 10, 0);
// application_init(ql_wifi_sta_test, "wifi_sta", 10, 0);
// application_init(ql_wifi_ota_test, "wifi_ota", 10, 0);
