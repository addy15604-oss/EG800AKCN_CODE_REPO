/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/

#ifndef _QL_BT_H
#define _QL_BT_H
/*> include header files here*/
#include "ql_type.h"
#include "ql_bt_types.h"
#include "ql_ble_api.h"
#include "ql_bt_comm_api.h"
#include "ql_bt_drive.h"
#include "ql_bt_headset_api.h"
#include "ql_bt_hid_api.h"
#include "ql_bt_obex_api.h"
#include "ql_bt_spp_api.h"
#ifdef __cplusplus
extern "C" {
#endif
#define MAX_BT_NAME                 248//48//forrest.liu@20210315 modify for low layer define update

#define BT_MODE_BR                  0
#define BT_MODE_BR_LE               1
#define BT_MODE_LE                  2

#define BT_HCI_MODE_H4              0
#define BT_HCI_MODE_H5              1

#define BT_FIRMWARE_AT_SD           0
#define BT_FIRMWARE_AT_FLASH        1
#define BT_FIRMWARE_AT_MEMORY       2

#define BAUDRATE_9600               9600
#define BAUDRATE_14400      		14400
#define BAUDRATE_19200      		19200
#define BAUDRATE_38400      		38400
#define BAUDRATE_57600      		57600
#define BAUDRATE_115200     		115200
#define BAUDRATE_230400     		230400
#define BAUDRATE_460800     		460800
#define BAUDRATE_921600     		921600
#define BAUDRATE_1842000    		1842000
#define BAUDRATE_3000000    		3000000
#define BAUDRATE_3686400    		3686400
#define MAX_BT_OBEX_FILE_NAME       256
#define MAX_BT_OBEX_FILE_TYPE_SIZE  32

typedef enum
{
  BT_PARITY_NONE = 0,
  BT_PARITY_ODD = 1,
  BT_PARITY_EVEN = 3,
} BT_PARITY_TYPE;

typedef enum
{
  BT_DATA_5 = 0,
  BT_DATA_6 = 1,
  BT_DATA_7 = 2,
  BT_DATA_8 = 3,
} BT_DATA_WIDTH;
  

typedef enum
{
  BT_CONNECT = 0,
  BT_DISCONNECT = 1,
  BLE_CONNECT = 2,
  BLE_DISCONNECT = 3,
} ql_bt_connect_status;

typedef int (*BT_UartIsrCb)(UINT32 port_id);

typedef enum
{
    bt_int_none = 0,
    bt_rx_int,
    bt_tx_int,
    bt_eor_int,
    bt_all_int,
    bt_int_max,
}bt_uartirqtype;

struct appbt_a2dp {
    int sample;
    int channel;
    void (*notify_upper_layer_send_data)(void);
};

typedef struct
{
  UINT32 port_id;
  UINT32 fifo;
  UINT32 dma;
  UINT32 loopback;
  bt_uartirqtype int_tpye;
  UINT32 highspeed;
  UINT32 flowctrl;
  UINT32 baudrate;
  UINT32 datawidth;
  UINT32 parity;
  BT_UartIsrCb rx_callback;
  BT_UartIsrCb tx_callback;
}bt_uart_cfg;

typedef struct _ble_device_info_record{
	unsigned char valid;
	unsigned char address[6];
	unsigned char addr_type;
	unsigned short deviceState;
	unsigned char ltk[16];
	unsigned char keyType;
	unsigned short ediv;
	unsigned char rand[8];
    unsigned char enc_size;
	unsigned char csrk[16];
	unsigned char peer_csrk[16];
	unsigned char signCounter;
	unsigned char irk[16];
	unsigned char peer_irk[16];
}__attribute__((packed)) ble_device_info_record;

enum{
    BT_RESET_PIN = 0,
    BT_PWDN_PIN,
    BT_WAKEUP_HOST_PIN,
    BT_HOST_WAKEUP_BT_PIN,
    BT_PIN_MAX,
};
struct bt_gpio_def_cfg{
    unsigned int mfprx;
    unsigned int dir;
    unsigned int level;
};
struct bt_hardware_cfg {
    int baudrate; // 115200, 921600, 3000000
    bt_uart_cfg *uart_cfg;
    int reset_pin; // -1: not reset BT chip through reset PIN
    int pwdn_pin; // -1: no PWDN pin
    int bt_wakeup_host_pin; // -1: not use wakeup PIN
    int host_wakeup_bt_pin; // -1: not use wakeup PIN
    unsigned int clock32;
    struct bt_gpio_def_cfg bt_pin_def_cfg[BT_PIN_MAX]; //0:reset pin 1:pwdn pin 2:bt wakeup host pin 3:host wakeup bt pin
};
struct bt_br_common_cfg {
    char name[MAX_BT_NAME];
    unsigned char addr[6];
    unsigned char discoverable; // 0: cannot be searched by other device; 1: can be searched
    unsigned char connectable; // 0: non-connectable; 1: connectable
    unsigned char io_capability; // 0: DisplayOnly; 1: DisplayYesNo; 2: KeyboardOnly; 3: NoInputNoOutput
    unsigned short int inquiry_scan_interval;
    unsigned short int inquiry_scan_window;
    unsigned short int page_scan_interval;
    unsigned short int page_scan_window;
    unsigned char is_absolute_volume_enable; //0:a2dp use relatively volume 1:a2dp enable absolute volume
    unsigned int cod; //class of device
    struct bt_hid_att_t* hid_att; ///hid device cfg
    uint16_t sniff_min_interval;
    uint16_t sniff_max_interval;
    uint16_t sniff_attempt;
    uint16_t sniff_timeout;
    uint8_t comm_cfg; //bit 0 for ssp configure
    unsigned char reserved[6]; //add reserved buffer here

};


typedef unsigned short (*att_read_callback_t)(unsigned short con_handle, unsigned short attribute_handle, unsigned short offset, unsigned char * buffer, unsigned short buffer_size);
typedef int (*att_write_callback_t)(unsigned short con_handle, unsigned short attribute_handle, unsigned short transaction_mode, unsigned short offset, unsigned char *buffer, unsigned short buffer_size);

struct bt_le_common_cfg {
    unsigned char random_address[6];
    unsigned char smp_io_capability; // 0: DisplayOnly; 1: DisplayYesNo; 2: KeyboardOnly; 3: NoInputNoOutput
    unsigned char smp_secure_connection;
	unsigned char max_slave_connections;
    unsigned char reserved[23];
};
enum {
    AVRCP_KEY_ENTER = 0x2B,
    AVRCP_KEY_VOLUME_UP = 0x41,
    AVRCP_KEY_VOLUME_DOWN,
    AVRCP_KEY_MUTE,
    AVRCP_KEY_PLAY,
    AVRCP_KEY_STOP,
    AVRCP_KEY_PAUSE,
    AVRCP_KEY_FORWARD = 0x4B,
    AVRCP_KEY_BACKWARD,
};

struct ql_bt_adv_params {
	uint8_t adv_mode;
	uint8_t phy;
    uint8_t* adv_data;
    uint8_t adv_data_len;
    uint8_t* scan_response_data;
    uint8_t scan_response_data_len;
    struct bt_task_le_adv_parameters *adv;
	struct bt_task_le_ext_adv_parameters *ext_adv;
};


struct bt_profile_cfg {
    unsigned int prf;
    unsigned char reserved[4]; //add reserved buffer here
};

struct bt_user_cfg {
    struct bt_hardware_cfg hw;
    struct bt_br_common_cfg br_common;
    struct bt_le_common_cfg le_common;
    struct bt_profile_cfg profile;
    struct bt_device_record *record;
    int firmware_where; // 0: SD; 1: flash; 2: memory
    unsigned char *bt_firmware;
    int bt_firmware_size;
    unsigned char *bt_nvm_lst;
    int bt_nvm_lst_size;
    int h4_h5; // 0: h4; 1: h5.
    int bt_mode; // 0: br only; 1: br + le; 2: le only
    int bt_chip_id; // 5801, 5803
    int xip;
	ble_device_info_record *ble_rec;
};



struct ql_bt_task_le_adv_parameters {
    unsigned short int interval_min;
    unsigned short int interval_max;
    unsigned char advertising_type; // LE_ADV_TYPE_XXX
    unsigned char own_address_type;
    unsigned char peer_address_type;
    struct bt_addr peer_address;
    unsigned char filter; /* 0x00: process scan and connection request from all devices
                             0x01: process connection request from all devices
                                   and scan request only from White List
                             0x02: process scan request from all devices
                                   and conneciton request only from White List
                             0x03: process scan and connection reques only from in the White List
                          */
};



typedef void (*bt_event_handle_t)(struct bt_task_event *event);

typedef void (*ql_cb_bt_open_status)(int result);
typedef void (*ql_cb_bt_scan_status)(struct bt_event_inquiry *inquiry,int result);
typedef void (*ql_cb_bt_connect_status)(int result);





#define LE_HIDS_BCDHID		                0x1011
enum
{
	BOOT_MODE = 0,
	REPORT_MODE,
};

#define LE_ERR_OK                       0x00
#define LE_ERR_INVALID_HANDLE           0x01
#define LE_ERR_READ_NOT_PERMITTED       0x02
#define LE_ERR_WRITE_NOT_PERMITTED      0x03
#define LE_ERR_INVALID_PDU              0x04
#define LE_ERR_INSUFFICIENT_AUTHEN      0x05
#define LE_ERR_REQUEST_NOT_SUPPORT      0x06
#define LE_ERR_INVALID_OFFSET           0x07
#define LE_ERR_INSUFFICIENT_AUTHOR      0x08
#define LE_ERR_PREPARE_QUEUE_FULL       0x09
#define LE_ERR_ATTRIBUTE_NOT_FOUND      0x0A
#define LE_ERR_ATTRIBUTE_NOT_LONG       0x0B
#define LE_ERR_INSUFFICIENT_ENCRY_KEY   0x0C
#define LE_ERR_INVALID_ATTRIBUTE_VALUE  0x0D
#define LE_ERR_UNLIKELY_ERROR           0x0E
#define LE_ERR_INSUFFICIENT_ENCRY       0x0F
#define LE_ERR_UNSUPPORTED_GTOUP_TYPE   0x10
#define LE_ERR_INSUFFICIENT_RESOURCES   0x11
	
#define LE_DEVICE_TYPE_CLASSIC          1
#define LE_DEVICE_TYPE_DUAL             3
#define LE_DEVICE_TYPE_LE               2
#define LE_DEVICE_TYPE_UNKNOWN          0
	
#define LE_ADV_TYPE_IND                 0x00
#define LE_ADV_TYPE_DIRECT_IND          0x01
#define LE_ADV_TYPE_SCAN_IND            0x02
#define LE_ADV_TYPE_NONCONN_IND         0x03
#define LE_ADV_TYPE_DIRECT_IND_LOW_DUTY 0x04
	
#define LE_PDU_TYPE_IND                 0x00
#define LE_PDU_TYPE_DIRECT_IND          0x01
#define LE_PDU_TYPE_NONCONN_IND         0x02
#define LE_PDU_TYPE_SCAN_REQ            0x03
#define LE_PDU_TYPE_SCAN_RESP           0x04
#define LE_EXT_ADV_PROPERTIES_BIT_CONNECTABLE   0
#define LE_EXT_ADV_PROPERTIES_BIT_SCANNABLE     1
#define LE_EXT_ADV_PROPERTIES_BIT_DIRECTED      2
#define LE_EXT_ADV_PROPERTIES_BIT_HIGH_DUTY_DIRECTED_CONNECTABLE  3
#define LE_EXT_ADV_PROPERTIES_BIT_LEGACY        4
#define LE_EXT_ADV_PROPERTIES_BIT_OMIT          5
#define LE_EXT_ADV_PROPERTIES_BIT_TXPOWER       6
	
#define LE_ADDRESS_TYPE_PUBLIC          0x00
#define LE_ADDRESS_TYPE_RANDOM          0x01
	
#define LE_GAP_TYPE_FLAGS                       0x01
#define LE_GAP_TYPE_INCOMPLETE_SERVICE_16BIT_LIST       0x02
#define LE_GAP_TYPE_COMPLETE_SERVICE_LIST       0x03
#define LE_GAP_TYPE_SHORT_NAME                  0x08
#define LE_GAP_TYPE_COMPLETE_NAME               0x09
#define LE_GAP_TYPE_TX_POWER                    0x0A
#define LE_GAP_TYPE_SERVICE_DATA                0x16
#define LE_GAP_TYPE_APPEARANCE                  0x19
#define LE_GAP_TYPE_SPECIFIC_DATA               0xFF
	
#define LE_PASSIVE_SCAN                 0x00
#define LE_ACTIVE_SCAN                  0x01
	
#define LE_ATT_MASTER                   0
#define LE_ATT_SLAVE                    1
	
#define LE_ATT_CHARC_PROP_READ          0x02
#define LE_ATT_CHARC_PROP_WWP           0x04
#define LE_ATT_CHARC_PROP_WRITE         0x08
#define LE_ATT_CHARC_PROP_NOTIFY        0x10
#define LE_ATT_CHARC_PROP_INDICATE      0x20
	
#define LE_ATT_PM_READABLE              0x0001
#define LE_ATT_PM_WRITEABLE             0x0002
#define LE_ATT_PM_AUTHENT_REQUIRED      0x0004
#define LE_ATT_PM_AUTHORIZE_REQUIRED    0x0008
#define LE_ATT_PM_ENCRYPTION_REQUIRED   0x0010
#define LE_ATT_PM_AUTHENT_MITM_REQUERED 0x0020
	
#define LE_ATT_UUID_PRIMARY             0x2800
#define LE_ATT_UUID_CHARC               0x2803
	
	// standard GATT descriptors
#define LE_ATT_UUID_CLIENT_CHARC_CONFIG 0x2902
#define LE_ATT_UUID_CHAR_PRESENT_FORMAT 0x2904
#define LE_ATT_UUID_EXTERNAL_REF        0x2907
#define LE_ATT_UUID_REPORT_REF          0x2908
	
	
#define LE_ATT_UUID_IMMEDIATE_ALERT     0x1802
#define LE_ATT_UUID_DEVICE_INFO         0x180A
#define	LE_ATT_UUID_BATTERY_SERVICE		0x180F
#define	LE_ATT_UUID_HID					0x1812
#define LE_ATT_UUID_MESH_PRO_SERVICE    0x1827
#define LE_ATT_UUID_MESH_PROXY_SERVICE  0x1828
	
	
#define LE_ATT_UUID_ALERT_LEVEL         0x2A06
#define LE_ATT_UUID_BATTERY   			0x2A19
#define LE_ATT_UUID_HIDBOOT_KB_INPUT	0x2A22
#define LE_ATT_UUID_HIDBOOT_KB_OUTPUT	0x2A32
#define LE_UUID_HIDBOOT_MOUSE_INPUT	    0x2A33
	
#define LE_ATT_UUID_HIDINFO   			0x2A4A
#define LE_ATT_UUID_HIDREPORT_MAP		0x2A4B
	
#define LE_ATT_UUID_HIDCONTROLPOINT   	0x2A4C
#define LE_ATT_UUID_PROTOCOL   		    0x2A4E
	
#define LE_ATT_UUID_HIDREPORT   		0x2A4D
#define LE_ATT_UUID_MANUFACTURER_NAME   0x2A29
#define LE_ATT_UUID_MESH_PRO_DATA_IN    0x2ADB
#define LE_ATT_UUID_MESH_PRO_DATA_OUT   0x2ADC
#define LE_ATT_UUID_MESH_PROXY_DATA_IN  0x2ADD
#define LE_ATT_UUID_MESH_PROXY_DATA_OUT 0x2ADE
#define LE_ATT_UUID_PNP_ID   			0x2A50
	
#define LE_CCC_BIT_INDICATED            0x02
#define LE_CCC_BIT_NOTIFIED             0x01
	
#define LE_ATT_UUID_SYSTEM_ID           0x2A23
#define LE_ATT_UUID_MODEL_NUMBER        0x2A24
#define LE_ATT_UUID_SERIAL_NUMBER       0x2A25
#define LE_ATT_UUID_FIRMWARE_VERSION    0x2A26
#define LE_ATT_UUID_HARDWARE_VERSION    0x2A27
#define LE_ATT_UUID_SOFTWARE_VERSION    0x2A28
#define LE_ATT_UUID_MANUFACTURER_NAME   0x2A29
#define LE_ATT_UUID_IEEE_LIST           0x2A2A
#define LE_ATT_UUID_PNP_ID   			0x2A50
	
#define LE_ATT_UUID_MESH_PRO_DATA_IN    0x2ADB
#define LE_ATT_UUID_MESH_PRO_DATA_OUT   0x2ADC
#define LE_ATT_UUID_MESH_PROXY_DATA_IN  0x2ADD
#define LE_ATT_UUID_MESH_PROXY_DATA_OUT 0x2ADE
#define LE_SUPPORT

enum ciev_index {
    CIEV_CALL = 0,
    CIEV_CALLSETUP, /* 0: means not currently in call setup.
                       1: means an incoming call process ongoing
                       2: means an outgoing call setup is ongoing (outgoing call setup in dialing state)
                       3: means remote party being alerted in an outgoing call (outgoing call setup in alerting state) */
    CIEV_SERVICE,
    CIEV_SIGNAL,
    CIEV_ROAMING,
    CIEV_BATTERY_CHAGER,
    CIEV_CALLHELD, /* 0: no call on hold
                      1: a call is placed on hold or active/held calls swapped
                      2: call on hold, no call active */
};


#define LE_UUID_TYPE_16     (2)
#define LE_UUID_TYPE_32     (4)
#define LE_UUID_TYPE_128    (16)



struct ql_bt_task_le_ext_adv_parameters {
    unsigned char handle;
    unsigned short int properties;
    unsigned int interval_min;
    unsigned int interval_max;
    unsigned char channel;
    unsigned char own_address_type;
    unsigned char peer_address_type;
    struct bt_addr peer_address;
    unsigned char filter_policy;
    unsigned char tx_power;
    unsigned char primary_phy;
    unsigned char secondary_adv_max_skip;
    unsigned char secondary_phy;
    unsigned char sid;
    unsigned char scan_request_notification;
};







#ifdef __cplusplus
} /*"C" */
#endif

#endif

