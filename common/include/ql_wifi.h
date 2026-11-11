/*==========================================================================================================
  Copyright (c) 2024, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
===========================================================================================================*/

/*==========================================================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------
25/06/2024        marvin.ning  

===========================================================================================================*/

#ifndef _QL_WIFI_H_
#define _QL_WIFI_H_

#include "ql_type.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_spi.h"

#ifdef __cplusplus
	 extern "C" {
#endif

#define QL_WIFI_SSID_MAX_LEN     32
#define QL_WIFI_PASSWORD_MAX_LEN 64
#define QL_WIFI_OTA_DATA_LEN     2048
#define QL_WIFI_OTA_HEAD_LEN     12
#define QL_WIFI_MAC_STR_MAX      18

typedef enum
{
    QL_WIFI_MODE_STA,
    QL_WIFI_MODE_AP
} ql_wifi_mode_e;

typedef enum
{
    QL_WIFI_AUTH_TYPE_NONE_SHARE,
    QL_WIFI_AUTH_TYPE_NONE,
    QL_WIFI_AUTH_TYPE_SHARE,
    QL_WIFI_AUTH_TYPE_WPA,
    QL_WIFI_AUTH_TYPE_WPA2,
    QL_WIFI_AUTH_TYPE_WPA_WPA2,
    QL_WIFI_AUTH_TYPE_MAX
} ql_wifi_auth_e;

typedef enum{
    QL_WIFI_ENCRYPT_TYPE_NONE,
    QL_WIFI_ENCRYPT_TYPE_WEP,
    QL_WIFI_ENCRYPT_TYPE_TKIP,
    QL_WIFI_ENCRYPT_TYPE_AES,
    QL_WIFI_AUTH_TYPE_TKIP_ASE,
    QL_WIFI_encrypt_TYPE_MAX
}ql_wifi_encrypt_e;

typedef enum{
    QL_WIFI_PORT_SPI_UART,
    QL_WIFI_PORT_SPI,
    QL_WIFI_PORT_CFG_MAX
}ql_wifi_port_cfg_e;

typedef struct
{
    SPI_PORT_E port;
    SPI_CLK_E  clk;
    SPI_MODE_E mode;
    GPIO_PIN_NUMBER_E cs;
    GPIO_PIN_NUMBER_E slave_rdy;//When the host is about to send data, use this gpio to detect whether the slave is in the ready state
    GPIO_PIN_NUMBER_E slave_cts;//The slave uses this gpio to notify the host to read spi data.
}ql_wifi_spi_config_t;


typedef struct
{
    ql_wifi_mode_e mode;
    uint8 simid;
    uint8 cid;
    ql_wifi_auth_e auth_type;
    ql_wifi_encrypt_e encrypt_type;//not used
    char ssid[QL_WIFI_SSID_MAX_LEN];
    char password[QL_WIFI_PASSWORD_MAX_LEN];

    ql_wifi_port_cfg_e port_cfg;
    ql_wifi_spi_config_t spi_config;
    QL_UART_PORT_NUMBER_E uart_port;
}ql_wifi_config_t;

typedef enum{
    QL_WIFI_OTA_WRITE_NORMAL,
    QL_WIFI_OTA_WRITE_DONE,
    QL_WIFI_OTA_WRITE_NOSPACE,
    QL_WIFI_OTA_WRITE_ERROR,
    QL_WIFI_OTA_REBOOT_DONE,
    QL_WIFI_OTA_MAX
}ql_wifi_ota_status;

typedef struct
{
    char ssid[32];
    char ap_power;                       // Signal strength, min:0, max:100
    uint8_t bssid[6];                    // The BSSID of an access point
    char channel;                        // The RF frequency, 1-13
    char security[16];                // Security type, @ref wlan_sec_type_t
} ql_wifi_ap_info_s;

typedef struct
{
    char ApNum; // The number of access points found in scanning
    ql_wifi_ap_info_s *ApList;
} ql_wifi_scan_result_s;

typedef enum
{
    QL_WIFI_IND_STATE_DISCONNECT,
    QL_WIFI_IND_STATE_CONNECTED
} ql_wifi_ind_state_e;

typedef enum
{
    QL_WIFI_IND_TYPE_DEVICE,
    QL_WIFI_IND_TYPE_CLIENT
} ql_wifi_ind_e;

typedef struct
{
    ql_wifi_ind_e ind_type;
    ql_wifi_ind_state_e state;
    char mac_str[QL_WIFI_MAC_STR_MAX];
} ql_wifi_ind_t;

#pragma pack(4)
typedef struct{
    uint32 total_size; // 文件总共大小
    uint32 dload_size; // 已经下载大小
    uint32 data_len;   // 用于记录当前缓存在p中的数据长度
    union {
        uint8_t  data;
        uint8_t  status;//ql_wifi_ota_status
    }data;
}ql_wifi_ota_data_head;
#pragma pack()

typedef void (*ql_wifi_ind_cb)(ql_wifi_ind_t *ctx);
typedef void (*wifi_ota_callback)(void *ctx);
typedef void (*wifi_scan_callback)(void *ctx);
/*****************************************************************
* Function: ql_wifi_open
*
* Description: 
*
* Parameters:
* 		 wifi_cb 	[in] 	回调函数,用于通知状态变化
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_open(ql_wifi_ind_cb wifi_cb);

/*****************************************************************
* Function: ql_wifi_close
* Description: 关闭wifi
* Parameters:
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_close(void);


/*****************************************************************
* Function: ql_wifi_close
* Description: 
*
* Parameters:
* 		 wifi_cfg 	[in] 	ql_wifi_config_t
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_set_config(ql_wifi_config_t *wifi_cfg);


/*****************************************************************
* Function: ql_wifi_soft_reset
*
* Description: 软复位wifi
*
* Parameters:
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_soft_reset(void);

/*****************************************************************
* Function: ql_wifi_get_version
*
* Description: 获取wifi版本信息
*
* Parameters:
* 	version	  		[out] 	WIFI的固件版本，字符串。
* 	len        	  	[in] 	version 数组的长度 
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_get_version(char *version, int len);

/*****************************************************************
* Function: ql_wifi_ota_register_cb
*
* Description: 注册ota回调函数
*
* Parameters:
* 	cb	  		[in] 	回调函数,用于通知OTA状态变化
*
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_ota_register_cb(wifi_ota_callback cb);

/*****************************************************************
* Function: ql_wifi_ota_data_send
*
* Description: 传输ota包给wifi
*
* Parameters:
* 	ota_data	  	[in] 	ql_wifi_ota_data_head结构体数据
*
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_ota_data_send(ql_wifi_ota_data_head *ota_data);

/*****************************************************************
* Function: ql_wifi_scan
*
* Description: 扫描wifi热点
*
* Parameters:
* 	cb	  		[in] 	回调函数,扫描到的结果通过该回调函数返回
*
* Return:
* 	0			成功。
*	-1 			失败。
*****************************************************************/
int ql_wifi_scan(wifi_scan_callback cb);

#ifdef __cplusplus
	} /*"C" */
#endif

#endif /* _QL_WIFI_H_ */

