#ifndef QL_BT_TYPES_H
#define QL_BT_TYPES_H





#define BT_UART_DMA_RX_DEBUG
#define FIRMWARE_POSITION_SD                0
#define FIRMWARE_POSITION_FLASH_PARTITION   1
#define FIRMWARE_POSITION_MEMORY            2

#define BT_HCI_MODE_H4                      0
#define BT_HCI_MODE_H5                      1

/* user configuration related define */
#define MAX_BT_NAME                 248
#define MAX_BT_PHONE_NUMBER         25
#define MAX_BT_DEVICE_RECORD        10

#define MAX_BT_OBEX_FILE_NAME       256
#define MAX_BT_OBEX_FILE_TYPE_SIZE  32

#define BT_OBEX_FRAME_SIZE          (8087 - MAX_BT_OBEX_FILE_TYPE_SIZE - MAX_BT_OBEX_FILE_NAME - 5)

#define BT_MODE_BR                  0
#define BT_MODE_BR_LE               1
#define BT_MODE_LE                  2

#define BT_ROLE_MASTER              0
#define BT_ROLE_SLAVE               1

#define BT_SNIFF_ON                 1
#define BT_SNIFF_OFF                0

#define BT_FIRMWARE_AT_SD           0
#define BT_FIRMWARE_AT_FLASH        1
#define BT_FIRMWARE_AT_MEMORY       2

typedef enum{
    BT_NONE_PRF = 0,
    BT_A2DP_SOURCE_PRF = (1 << 0),
    BT_A2DP_SINK_PRF   = (1 << 1),
    BT_AVRCP_PRF       = (1 << 2),
    BT_HFP_PRF         = (1 << 3),
    BT_HFP_HF_PRF      = (1 << 4),
    BT_OBEX_PRF        = (1 << 5),
    BT_SPP_PRF         = (1 << 6),
    BT_HID_PRF         = (1 << 7),
    BT_PBAP_PRF        = (1 << 8),
    BT_MAP_PRF         = (1 << 9),
    BT_FTP_PRF         = (1 << 10),
    BT_MAX_PRF         = (1 << 11),
}bt_prf;

#define BT_WAKELOCK_UART_TX_DMA     (1 << 0)
#define BT_WAKELOCK_UART_RECV       (1 << 1)
#define BT_WAKELOCK_UART_RX_DMA     (1 << 2)

#define BT_ERR_CODE_UNKNOWN_CONNECTION_IDENTIFIER           0x02
#define BT_ERR_CODE_HARDWARE_FAILURE                        0x03
#define BT_ERR_CODE_PAGE_TIMEOUT                            0x04
#define BT_ERR_CODE_AUTHENTICATION_FAILURE                  0x05
#define BT_ERR_CODE_PIN_or_KEY_MISSING                      0x06
#define BT_ERR_CODE_CONNECTION_TIMEOUT                      0x08
#define BT_ERR_CODE_CONNECTION_ALREADY_EXISTS               0x0B
#define BT_ERR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED      0x10
#define BT_ERR_CODE_REMOTE_USER_TERMINATED_CONNECTION       0x13
#define BT_ERR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST     0x16
// private error code
#define BT_ERR_CODE_OBEX_SEND_FAILED_BAD_STATUS             0x70
#define BT_ERR_CODE_OBEX_SEND_FAILED_FILE_NAME_TOO_LONG     0x71

//bt user config magic
#define BT_USER_CFG_MAGIC_V1 0x20231018

#define EC800M_INIT_FLAG      1      //新增宏控用来控制init的形态



enum {
    BTTASK_CMD_MODULE_BASE      = 0,
    BTTASK_CMD_MODULE_LE        = 1000,
    BTTASK_CMD_MODULE_CLASSIC   = 2000,
    BTTASK_CMD_MODULE_CUSTOM    = 3000,
    BTTASK_CMD_MODULE_TEST      = 4000,
};



#define BTTASK_CMD_PROFILE_CMD_START (BTTASK_CMD_MODULE_BASE+200)

enum {
    BTTASK_CMD_ACL_CONNECT = BTTASK_CMD_MODULE_BASE,
    BTTASK_CMD_ACL_DISCONNECT,
    BTTASK_CMD_CANCEL_ACL_CONNECT,
    BTTASK_CMD_INQUIRY,
    BTTASK_CMD_INQUIRY_CANCEL,
    BTTASK_CMD_SET_VISIBLE,
    BTTASK_CMD_SET_VISIBLE_CONNECTABLE,
    BTTASK_CMD_SET_LOCAL_NAME,
    BTTASK_CMD_BONDING_ACCEPT,
    BTTASK_CMD_BONDING,
    BTTASK_CMD_UNBONDING, //10
    BTTASK_CMD_PIN_REPLY,
    BTTASK_CMD_PIN_NEGATIVE_REPLY,
    BTTASK_CMD_USER_PASSKEY_REQUEST_REPLY,
    BTTASK_CMD_USER_PASSKEY_REQUEST_NEGATIVE_REPLY,
    BTTASK_CMD_SEND_KEYPRESS_NOTIFICATION,
    BTTASK_CMD_SNIFF_MODE_ENABLE,
    // New command for custom
    BTTASK_CMD_SET_CHECK_CONTROLLER_ALIVE,
    BTTASK_CMD_HCI_CMD_SEND,
    BTTASK_CMD_SET_PAGE_SCAN_TYPE,

    //a2dp
    BTTASK_CMD_CONNECT_A2DP = BTTASK_CMD_PROFILE_CMD_START,
    BTTASK_CMD_DISCONNECT_A2DP,
    BTTASK_CMD_A2DP_SEND_START,
    BTTASK_CMD_A2DP_SEND_SUSPEND,
    BTTASK_CMD_A2DP_SEND_MEDIA_DATA,
    //avrcp
    BTTASK_CMD_CONNECT_AVRCP,
    BTTASK_CMD_DISCONNECT_AVRCP,
    BTTASK_CMD_AVRCP_PLAY,
    BTTASK_CMD_AVRCP_PAUSE,
    BTTASK_CMD_AVRCP_STOP,
    BTTASK_CMD_AVRCP_FORWARD, //BTTASK_CMD_PROFILE_CMD_START+10
    BTTASK_CMD_AVRCP_BACKWARD,
    BTTASK_CMD_VOLUME_KEY,
    BTTASK_CMD_SET_ABSOLUTE_VOL,
    //hfp
    BTTASK_CMD_CONNECT_HFP,
    BTTASK_CMD_DISCONNECT_HFP,
    BTTASK_CMD_HFP_ACCEPT_CONNECTION,
    BTTASK_CMD_HFP_REJECT_CONNECTION,
    BTTASK_CMD_HFP_UPDATE_CALLSTATUS,
    BTTASK_CMD_HFP_UPDATE_PHONE_NUMBER,
    BTTASK_CMD_HFP_UPDATE_CALLHOLD, //BTTASK_CMD_PROFILE_CMD_START+20
    BTTASK_CMD_HFP_SET_SPEAKER_GAIN,
    BTTASK_CMD_HFP_SET_MICROPHONE_GAIN,
    BTTASK_CMD_HFP_SEND_AT_RAWDATA,
    //obex
    BTTASK_CMD_CONNECT_OBEX_OPP,
    BTTASK_CMD_DISCONNECT_OBEX_OPP,
    BTTASK_CMD_OBEX_OPP_SEND_FILE_START,
    BTTASK_CMD_OBEX_OPP_SEND_FILE,
    BTTASK_CMD_OBEX_RESPONSE,
    //sco
    BTTASK_CMD_SETUP_SCO,
    BTTASK_CMD_DISCONNECT_SCO, //BTTASK_CMD_PROFILE_CMD_START+30
    BTTASK_CMD_SCO_DATA_SEND,
    //spp
    BTTASK_CMD_SPP_CONNECT,
    BTTASK_CMD_SPP_DISCONNECT,
    BTTASK_CMD_SPP_SEND_DATA,
    //hid
    BTTASK_CMD_HID_CONNECT,
    BTTASK_CMD_HID_DISCONNECT,
    BTTASK_CMD_HID_SEND_DATA,
    //HFP HF
    BTTASK_CMD_CONNECT_HFP_HF,
    BTTASK_CMD_DISCONNECT_HFP_HF,
    BTTASK_CMD_COPS,
    BTTASK_CMD_ATA,
    BTTASK_CMD_ATH, //BTTASK_CMD_PROFILE_CMD_START+40
    BTTASK_CMD_DIAL,
    BTTASK_CMD_DIAL_MEMORY,
    BTTASK_CMD_SPEAKER_GAIN,
    BTTASK_CMD_MIC_GAIN,
    BTTASK_CMD_CHLD0,
    BTTASK_CMD_CHLD1,
    BTTASK_CMD_CHLD2,
    BTTASK_CMD_CHLD3,
    BTTASK_CMD_CHLD4,
    BTTASK_CMD_BLDN, //BTTASK_CMD_PROFILE_CMD_START+50
    BTTASK_CMD_CLCC,
    BTTASK_CMD_CHLD1X,
    BTTASK_CMD_CHLD2X,
    BTTASK_CMD_BVRA,
    //a2dp sink
    BTTASK_CMD_A2DP_SINK_CONNECT,
    BTTASK_CMD_A2DP_SINK_DISCONNECT,
    BTTASK_CMD_A2DP_SINK_SRV_REG,
    //pbap client
    BTTASK_CMD_PBAP_CLIENT_CONNECT,
    BTTASK_CMD_PBAP_CLIENT_DISCONNECT,
    BTTASK_CMD_PBAP_CLIENT_FLOW_CONTROL, //BTTASK_CMD_PROFILE_CMD_START+60
    BTTASK_CMD_PBAP_CLIENT_SET_PATH,
    BTTASK_CMD_PBAP_CLIENT_GET_PB_SIZE,
    BTTASK_CMD_PBAP_CLIENT_PULL_PB,
    BTTASK_CMD_PBAP_CLIENT_PULL_NEXT_PB,
    BTTASK_CMD_PBAP_CLIENT_LOOKUP_NUMBER,
    BTTASK_CMD_PBAP_CLIENT_ABORT,

    //pbap client
    BTTASK_CMD_MAP_CLIENT_CONNECT,
    BTTASK_CMD_MAP_CLIENT_DISCONNECT,
    BTTASK_CMD_MAP_CLIENT_FLOW_CONTROL,
    BTTASK_CMD_MAP_CLIENT_GET_NEXT,
    BTTASK_CMD_MAP_CLIENT_NOTIFICATION,
    BTTASK_CMD_MAP_CLIENT_SET_FOLDER, //BTTASK_CMD_PROFILE_CMD_START+70
    BTTASK_CMD_MAP_CLIENT_GET_FOLDER_LISTING,
    BTTASK_CMD_MAP_CLIENT_GET_MESSAGE_LISTING,
    BTTASK_CMD_MAP_CLIENT_GET_MESSAGE,
    BTTASK_CMD_MAP_CLIENT_PUSH_MESSAGE,

    //ftp client
    BTTASK_CMD_FTP_CONNECT,
    BTTASK_CMD_FTP_DISCONNECT,
    BTTASK_CMD_FTP_FOLDER_BROWSING,
    BTTASK_CMD_FTP_SET_FOLDER,
    BTTASK_CMD_FTP_SEND_FILE,
    BTTASK_CMD_FTP_GET_FILE,
    BTTASK_CMD_FTP_GET_NEXT,
    BTTASK_CMD_FTP_DELETE,

    //le define
    BTTASK_CMD_LE_SET_RANDOM_ADDRESS = BTTASK_CMD_MODULE_LE,
    BTTASK_CMD_LE_SET_ADV_PARAMETERS,
    BTTASK_CMD_LE_SET_ADV_DATA,
    BTTASK_CMD_LE_SET_SCAN_RESPONSE,
    BTTASK_CMD_LE_SET_ADV_ENABLE,
    BTTASK_CMD_LE_SET_ADV_SET_RANDOM_ADDRESS,
    BTTASK_CMD_LE_SET_EXT_ADV_PARAMETERS,
    BTTASK_CMD_LE_SET_EXT_ADV_DATA,
    BTTASK_CMD_LE_SET_EXT_SCAN_RESPONSE,
    BTTASK_CMD_LE_SET_EXT_ADV_ENABLE,
    BTTASK_CMD_LE_READ_ADV_PHY_TXPOWER,

    BTTASK_CMD_LE_NOTIFY,
    BTTASK_CMD_LE_INDICATE,

    BTTASK_CMD_LE_SCAN,
    BTTASK_CMD_LE_SCAN_STOP,
    BTTASK_CMD_LE_CONNECT,
    BTTASK_CMD_LE_DISCONNECT,
    BTTASK_CMD_LE_CLEAR_WHITE_LIST,
    BTTASK_CMD_LE_SET_WHITE_LIST,
    BTTASK_CMD_LE_REMOVE_WHITE_LIST,

    BTTASK_CMD_RAW_VENDOR_CMD,
    BTTASK_CMD_TEST_READ_RSSI,
    BTTASK_CMD_TEST_HCI_RESET,
    BTTASK_CMD_TEST_SET_COEXIST_MODE,
    BTTASK_CMD_LE_SET_PASSKEY_FROM_LOCAL,
    BTTASK_CMD_LE_SET_PAIR_ENABLE, // enable normal pair rsp
    BTTASK_CMD_LE_SET_DEFAULT_PHY,
    BTTASK_CMD_LE_SCAN_WITH_FILTER,
    BTTASK_CMD_LE_INPUT_PASSKEY,
    BTTASK_CMD_LE_CANCEL_CONNECT,

    // le custom
    BTTASK_CMD_LE_REGISTER_ATT_SERVER = BTTASK_CMD_MODULE_CUSTOM + 1,
    BTTASK_CMD_LE_SECURITY_REQUEST,
    BTTASK_CMD_LE_DB_CONTROL_GET_INFO,
    BTTASK_CMD_LE_DB_CONTROL_REMOVE_INDEX,

    BTTASK_CMD_LE_CUSTOM_READ_REQUEST,
    BTTASK_CMD_LE_CUSTOM_WRITE_REQUEST,
    BTTASK_CMD_LE_CUSTOM_WRITE_COMMAND,
    BTTASK_CMD_LE_CUSTOM_MTU_REQUEST,
    BTTASK_CMD_LE_CUSTOM_GATT_SCAN_START,
    BTTASK_CMD_LE_CUSTOM_GATT_SCAN_SUBSCRIBE_DEFAULT,
    BTTASK_CMD_LE_CONNECTION_PARAMETER_UPDATE_REQUEST,
    BTTASK_CMD_LE_CUSTOM_SET_EXTENDED_ADV_SUPPORTED,
    BTTASK_CMD_LE_CUSTOM_SCAN_START,
    BTTASK_CMD_LE_CUSTOM_GET_SCAN_RESULT,

    //bt test
    BTTASK_CMD_TEST_DUT = BTTASK_CMD_MODULE_TEST + 1,
    BTTASK_CMD_TEST_TX_TONE,
    BTTASK_CMD_TEST_TX_PACKET,
    BTTASK_CMD_TEST_RX_PACKET,
    BTTASK_CMD_TEST_LE_ENHANCED_RECEIVER,
    BTTASK_CMD_TEST_LE_ENHANCED_TRANSMITTER,
    BTTASK_CMD_TEST_LE_END,

};


enum {
    BTTASK_IND_TYPE_COMMON,
    BTTASK_IND_TYPE_ACL,
    BTTASK_IND_TYPE_SCO,
    BTTASK_IND_TYPE_A2DP,
    BTTASK_IND_TYPE_AVRCP,
    BTTASK_IND_TYPE_HFP,
    BTTASK_IND_TYPE_OBEX,
    BTTASK_IND_TYPE_SPP,
    BTTASK_IND_TYPE_LE,
    BTTASK_IND_TYPE_A2DP_SINK,
    BTTASK_IND_TYPE_HFP_HF,
    BTTASK_IND_TYPE_HID,
    BTTASK_IND_TYPE_PBAP,
    BTTASK_IND_TYPE_MAP,
    BTTASK_IND_TYPE_FTP,
};

enum {
    BTTASK_IND_INQUIRY_RESULT,
    BTTASK_IND_INQUIRY_COMPLETE,
    BTTASK_IND_PAIRING_REQUEST,
    BTTASK_IND_PAIRED,
    BTTASK_IND_PIN_REQUEST,
    BTTASK_IND_USER_PASSKEY_NOTIFICATION,
    BTTASK_IND_USER_PASSKEY_REQUEST,
    BTTASK_IND_POWERUP_COMPLETE,
    BTTASK_IND_POWERUP_FAILED,
    BTTASK_IND_SHUTDOWN_COMPLETE,
    BTTASK_IND_BTFIRMWARE_ASSERT,
    BTTASK_IND_HCI_COMPLETE_EVENT,
    BTTASK_IND_NULL, // message was eaten by upper layer
    BTTASK_IND_NAME,
    BTTASK_IND_RSSI,
    BTTASK_IND_TEMPERATURE_CHECK,
    BTTASK_IND_PCM_MODE,
    BTTASK_IND_BTRX_RESULT,
    BTTASK_IND_AUTH_COMPLETE,
}; // common define

enum {
    BTTASK_IND_ACL_CONNECTED,
    BTTASK_IND_ACL_DISCONNECTED,
    BTTASK_IND_ACL_CONNECT_FAILED,
}; // ACL define

enum {
    BTTASK_IND_SCO_CONNECTED,
    BTTASK_IND_SCO_DISCONNECTED,
    BTTASK_IND_SCO_CONNECT_FAILED,
    BTTASK_IND_HFP_SCO_DATA,
}; // SCO define

enum {
    BTTASK_IND_A2DP_CONNECTED,
    BTTASK_IND_A2DP_MEIDA_CONNECTED,
    BTTASK_IND_A2DP_DISCONNECTED,
    BTTASK_IND_A2DP_START,
    BTTASK_IND_A2DP_MEDIA_SEND_COMPLETE,
    BTTASK_IND_REMOTE_SBC_CAPABILITIES,
    BTTASK_IND_A2DP_MEDIA_MTU,
    BTTASK_IND_A2DP_SUSPEND,
    BTTASK_IND_A2DP_STOP,
    BTTASK_IND_A2DP_REJECTED,
    BTTASK_IND_A2DP_CONNECT_FAIL,
    BTTASK_IND_A2DP_MEDIA_DISCONNECTED,
}; // a2dp define


enum {
    BTTASK_IND_A2DP_SINK_CONNECTED,
    BTTASK_IND_A2DP_SINK_CONNECT_FAIL,
    BTTASK_IND_A2DP_SINK_MEIDA_CONNECTED,
    BTTASK_IND_A2DP_SINK_DISCONNECTED,
    BTTASK_IND_A2DP_SINK_REMOTE_SBC_CAPABILITIES,
    BTTASK_IND_A2DP_SINK_MEDIA_PAYLOAD,
    BTTASK_IND_A2DP_SINK_PLAY_STARTED,
    BTTASK_IND_A2DP_SINK_PLAY_SUSPENDED,
    BTTASK_IND_A2DP_SINK_PLAY_STOPPED,
}; // a2dp sink define

enum {
    BTTASK_IND_AVRCP_CONNECTED,
    BTTASK_IND_AVRCP_DISCONNECTED,
    BTTASK_IND_AVRCP_KEY_PRESSED,
    BTTASK_IND_AVRCP_KEY_RELEASED,
    BTTASK_IND_AVRCP_VOLUME_CHANGED_SUPPORT,
    BTTASK_IND_AVRCP_VOLUME_CHANGED,
    BTTASK_IND_AVRCP_CONNECT_FAIL,
}; // avrcp define

enum {
    BTTASK_IND_HFP_CONNECTION_REQUEST, // HF initate a connection to AG
    BTTASK_IND_HFP_CONNECTED, // connection complete, HF or AG initate this connection
    BTTASK_IND_HFP_SDP_FAILED, // remote device not support HFP
    BTTASK_IND_HFP_DISCONNECTED,
    BTTASK_IND_HFP_ATA,
    BTTASK_IND_HFP_ATCHUP,
    BTTASK_IND_HFP_DIAL,
    BTTASK_IND_HFP_ATCHLD,
    BTTASK_IND_HFP_VOICE_RECOGNITION,
    BTTASK_IND_HFP_SPEAKER_VOLUME,
    BTTASK_IND_HFP_MICROPHONE_GAIN,
    BTTASK_IND_HFP_CLCC,
    BTTASK_IND_HFP_RESPONSE_AND_HOLD,
    BTTASK_IND_HFP_CIND,
}; // hfp ag define

enum {
    BTTASK_IND_HFP_HF_CONNECTED, // connection complete, HF or AG initate this connection
    BTTASK_IND_HFP_HF_DISCONNECTED,
    BTTASK_IND_HFP_HF_SPEAKER_VOLUME,
    BTTASK_IND_HFP_HF_MICROPHONE_GAIN,
    BTTASK_IND_HFP_HF_RING,
    BTTASK_IND_HFP_HF_AG_INDICATOR_STATUS_CHANGED,
    BTTASK_IND_HFP_HF_NETWORK_OPERATOR_CHANGED,
    BTTASK_IND_HFP_HF_CLIP,
    BTTASK_IND_HFP_HF_CLCC,
    BTTASK_IND_HFP_HF_VOICE_RECOGNITION_ACTIVATED,
    BTTASK_IND_HFP_HF_VOICE_RECOGNITION_DEACTIVATED,
    BTTASK_IND_HFP_HF_CMD_COMP,
}; // hfp hf define

enum {
    BTTASK_IND_OBEX_OPP_SERVER_CONNECTED,
    BTTASK_IND_OBEX_OPP_SERVER_DISCONNECTED,
    BTTASK_IND_OBEX_OPP_SERVER_OBJECT_FILE_INFO,
    BTTASK_IND_OBEX_OPP_SERVER_OBJECT_RECIEVE_DATA,
    BTTASK_IND_OBEX_OPP_SERVER_OBJECT_RECIEVE_FINISH,
    BTTASK_IND_OBEX_OPP_SERVER_OBJECT_RECIEVE_ABORT,
    BTTASK_IND_OBEX_OPP_CLIENT_CONNECTED,
    BTTASK_IND_OBEX_OPP_CLIENT_DISCONNECTED,
    BTTASK_IND_OBEX_OPP_CLIENT_OBJECT_PUT_FAILED,
    BTTASK_IND_OBEX_OPP_CLIENT_OBJECT_PUT_SUCCESS,
    BTTASK_IND_OBEX_OPP_CLIENT_OBJECT_PUT_CONTINUE,
    BTTASK_IND_OBEX_PBAP_SERVER_CONNECTED,
}; // obex define

enum {
    BTTASK_IND_SPP_CONNECT_IND,
    BTTASK_IND_SPP_CONNECT_CNF,
    BTTASK_IND_SPP_DISCONNECT_IND,
    BTTASK_IND_SPP_DISCONNECT_CNF,
    BTTASK_IND_SPP_DATA_IND,
    BTTASK_IND_SPP_DATA_CNF,
    BTTASK_IND_SPP_FLOW_IND,
 }; // SPP define

enum {
    BTTASK_IND_PBAP_CLIENT_CONNECTED,
    BTTASK_IND_PBAP_CLIENT_DISCONNECTED,
    BTTASK_IND_PBAP_CLIENT_PB_SIZE,
    BTTASK_IND_PBAP_CLIENT_OPERATION_COMPLETED,
    BTTASK_IND_PBAP_CLIENT_LOOKUP_NAME,
    BTTASK_IND_PBAP_CLIENT_DATA,
}; //pbap

enum {
    BTTASK_IND_HID_CONNECTED,
    BTTASK_IND_HID_DISCONNECTED,
};

enum {
    BTTASK_IND_MAP_CLIENT_CONNECTED,
    BTTASK_IND_MAP_CLIENT_DISCONNECTED,
    BTTASK_IND_MAP_CLIENT_OPERATION_COMPLETED,
    BTTASK_IND_MAP_CLIENT_DATA,
    BTTASK_IND_MAP_MNS_CONNECTED,
    BTTASK_IND_MAP_MNS_DISCONNECTED,
    BTTASK_IND_MAP_MNS_EVENT_REPORT_START,
    BTTASK_IND_MAP_MNS_EVENT_REPORT_FINISH,
}; //map

enum {
    BTTASK_IND_FTP_CLIENT_CONNECTED,
    BTTASK_IND_FTP_CLIENT_DISCONNECTED,
    BTTASK_IND_FTP_CLIENT_PULL_CONTINUE,
    BTTASK_IND_FTP_CLIENT_PUT_CONTINUE,
    BTTASK_IND_FTP_CLIENT_PKT_DATA,
    BTTASK_IND_FTP_CLIENT_OPERATION_COMPLETED,
}; //ftp

enum {
    BTTASK_IND_LE_SCAN_EVENT,
    BTTASK_IND_LE_GATT_CONNECTED,
    BTTASK_IND_LE_GATT_DISCONNECTED,
    BTTASK_IND_LE_MTU_EXCHANGED, // this indication means MTU be changed (no matter master or slave)
    BTTASK_IND_LE_CLIENT_HANDLE_NOTIFY,
    BTTASK_IND_LE_CLIENT_HANDLE_INDIATION,
    BTTASK_IND_LE_WHITE_LIST_SIZE,
    BTTASK_IND_SMP_PASSKEY,
    BTTASK_IND_ADV_PHY_TXPOWER,
    BTTASK_IND_SLAVE_LE_BOND_COMPLETE, //slave role, bond

    // add for btstack
    BTTASK_IND_LE_ATT_EVENT_CAN_SEND_NOW,
    BTTASK_IND_LE_ATT_EVENT_CAN_WRITE_NOW,
    BTTASK_IND_LE_IDENTITY_INFO,
    BTTASK_IND_LE_DB_CONTROL_GET_INFO,
    BTTASK_IND_LE_ATT_EVENT_READ_DATA_RESULT,
    BTTASK_IND_LE_ATT_EVENT_READ_OVER_RESULT,
    BTTASK_IND_LE_GATT_SCAN_RESULT,
    BTTASK_IND_LE_GATT_SCAN_DUMP_SERVICE,
    BTTASK_IND_LE_GATT_SCAN_DUMP_CHARACTERISTIC,
    BTTASK_IND_LE_GATT_SCAN_DUMP_DESCRIPTOR,
    BTTASK_IND_LE_CONNECTION_UPDATE_COMPLETE,
    BTTASK_IND_LE_ATT_INDICATION_COMPLETE,
    BTTASK_IND_LE_ENCRYPTION_CHANGE,
    BTTASK_IND_LE_ATT_EVENT_NOTIFY_FLOW_CONTROL,
    BTTASK_IND_LE_ATT_EVENT_WRITE_FLOW_CONTROL,
    BTTASK_IND_LE_SMP_PASSKEY_INPUT_NUMBER,
    BTTASK_IND_LE_PAIR_COMPLETE,
    BTTASK_IND_LE_HCI_COMMAND_COMPLETE,
    BTTASK_IND_LE_CUSTOM_SCAN_EVENT,
    BTTASK_IND_LE_CONNECTION_COMPLETE,
    BTTASK_IND_LE_DISCONNECTION_COMPLETE,
}; // le define

enum {
    BT_LE_ADV_LEGACY_MODE = 0,
    BT_LE_EXTENDED_ADV_LEGACY_MODE,
    BT_LE_EXTENDED_ADV_AUX_MODE,
}; // le adv mode

struct bt_addr {
    unsigned char bytes[6];
};

//classic bt define
enum{
    BT_NOT_VISABLE_NOT_CONNECTABLE = 0,             //no visable - no connectable
    BT_VISABLE_CONNECTABLE,                         //visable - connectable
    BT_VISABLE_NOT_CONNECTABLE,                     //visable - not connectable
    BT_NOT_VISABLE_CONNECTABLE                      //no visable - connectable
};

enum{
    BT_HFP_HF_CMD_NONE,
    BT_HFP_HF_CMD_ATD,
    BT_HFP_HF_CMD_ATA,
    BT_HFP_HF_CMD_CHUP,
    BT_HFP_HF_CMD_CHLD,
};

enum{
    BT_SSP_DISABLE = 0,
    BT_SSP_ENABLE = 1,
};

/* ------------------------------- typedef summary start ---------------------------*/
typedef struct {
    unsigned short start_group_handle;
    unsigned short end_group_handle;
    unsigned short uuid16;
    unsigned char  uuid128[16];
} gatt_client_asr_service_t;

typedef struct {
    unsigned short start_handle;
    unsigned short value_handle;
    unsigned short end_handle;
    unsigned short properties;
    unsigned short uuid16;
    unsigned char  uuid128[16];
} gatt_client_asr_characteristic_t;

typedef struct {
    unsigned short handle;
    unsigned short uuid16;
    unsigned char  uuid128[16];
} gatt_client_asr_characteristic_descriptor_t;

typedef void (*bt_stack_shutdown_clearup)(void);

/**
 * @param result: pair result, 0: success, 1: fail
 * @param reason: reason when result is fail
 */
typedef void (*pair_result_callback)(unsigned char result, unsigned char reason);

/**
 * @brief ATT Client Read Callback for Dynamic Data
 * - if buffer == NULL, don't copy data, just return size of value
 * - if buffer != NULL, copy data and return number bytes copied
 * @param con_handle of hci le connection
 * @param attribute_handle to be read
 * @param offset defines start of attribute value
 * @param buffer
 * @param buffer_size
 * @return size of value if buffer is NULL, otherwise number of bytes copied
 */
typedef unsigned short (*att_read_callback_t)(unsigned short con_handle, unsigned short attribute_handle, unsigned short offset, unsigned char *buffer, unsigned short buffer_size);

/**
 * @brief ATT Client Write Callback for Dynamic Data
 * Each Prepared Write Request triggers a callback with transaction mode ATT_TRANSACTION_MODE_ACTIVE.
 * On Execute Write, the callback will be called with ATT_TRANSACTION_MODE_VALIDATE and allows to validate all queued writes and return an application error.
 * @param con_handle of hci le connection
 * @param attribute_handle to be written
 * @param transaction - ATT_TRANSACTION_MODE_NONE for regular writes. For prepared writes: ATT_TRANSACTION_MODE_ACTIVE, ATT_TRANSACTION_MODE_VALIDATE, ATT_TRANSACTION_MODE_EXECUTE, ATT_TRANSACTION_MODE_CANCEL
 * @param offset into the value - used for queued writes and long attributes
 * @param buffer
 * @param buffer_size
 * @param signature used for signed write commmands
 * @return 0 if write was ok, ATT_ERROR_PREPARE_QUEUE_FULL if no space in queue, ATT_ERROR_INVALID_OFFSET if offset is larger than max buffer
 */
typedef int (*att_write_callback_t)(unsigned short con_handle, unsigned short attribute_handle, unsigned short transaction_mode, unsigned short offset, unsigned char *buffer,
                                    unsigned short buffer_size);

/* ------------------------------- typedef summary end ---------------------------*/

//struct definition
struct bt_task_event {
    unsigned short int event_type;
    unsigned short int event_id;
    int payload_length;
    void *payload;
};

typedef void (*bt_event_handle_t)(struct bt_task_event *event);

#if EC800M_INIT_FLAG
struct bt_user_init_cfg {
    char name[MAX_BT_NAME];
	unsigned char io_capability; // 0: DisplayOnly; 1: DisplayYesNo; 2: KeyboardOnly; 3: NoInputNoOutput
	unsigned short int inquiry_scan_interval;
    unsigned short int inquiry_scan_window;
	unsigned short int page_scan_interval;
    unsigned short int page_scan_window;
    unsigned char is_absolute_volume_enable;
    unsigned short sniff_min_interval; //0x0006-0x0540
    unsigned short sniff_max_interval; //0x0006-0x0540
    unsigned short sniff_attempt; //0x0001-0x7FFF
    unsigned short sniff_timeout; //0x0001-0x7FFF
    unsigned char addr[6];
    unsigned char reserved[6]; //add reserved buffer
    unsigned int bt_cfg_magic;

};
#else
struct bt_user_init_cfg {
    char name[MAX_BT_NAME];
    unsigned char discover_connect_able;
    int bt_wakeup_host_pin;
    unsigned char *bt_firmware;
    int bt_firmware_size;
    unsigned char *bt_nvm_lst;
    int bt_nvm_lst_size;
    int firmware_where;
    unsigned char io_capability;
    unsigned char is_absolute_volume_enable;
    unsigned short sniff_min_interval; //0x0006-0x0540
    unsigned short sniff_max_interval; //0x0006-0x0540
    unsigned short sniff_attempt; //0x0001-0x7FFF
    unsigned short sniff_timeout; //0x0001-0x7FFF
    unsigned char addr[6];
    unsigned char reserved[6]; //add reserved buffer
    unsigned int bt_cfg_magic;
};
#endif


struct bt_device_record {
    char name[MAX_BT_NAME];
    unsigned int cod;
    unsigned char addr[6];
    unsigned char linkkey[16];
    int linkkey_type;
    int hfp_volume;
    int a2dp_volume;
    struct bt_device_record *next;
};

/* command and event related define */
struct bt_task_command {
    int command_id;
    void (*execute_cb)(void *arg);
    int payload_length;
    void *payload;
};

/* ------------------------------- classic command struct define start ---------------------------*/
struct bt_task_inquiry {
    unsigned char inquiry_length;
    unsigned char num_responses;
};

struct bt_task_bonding_accept {
    unsigned char addr[6];
    int accept;
};

struct bt_task_pin_reply {
    struct bt_addr addr;
    char pin[64];
};

struct bt_task_pin_negative_reply {
    struct bt_addr addr;
};

struct bt_task_user_passkey_request_reply {
    struct bt_addr addr;
    unsigned int numeric_value; /*valid values are decimal 000000 to 999999*/
};

struct bt_task_user_passkey_request_negative_reply {
    struct bt_addr addr;
};

struct bt_task_send_keypress_notification {
    struct bt_addr addr;
    unsigned char notification_type;
};

struct bt_task_a2dp_media_data {
    unsigned int length;
    unsigned char *data;
    unsigned int timestamp;
    unsigned int seq_num;
    unsigned char payload_type;
    unsigned char frames;
};

struct bt_task_hfp_connect {
    unsigned char addr[6];
    int call_status;
};

struct bt_task_hfp_call_status {
    int ciev_index;
    int value;
    int delay;
};

struct bt_task_hfp_update_phone_number {
    char number[MAX_BT_PHONE_NUMBER];
    unsigned char type;
};

struct bt_task_obex_file_send {
    unsigned int total; //total file size
    unsigned char *data;
    unsigned int payload_size; //data size
    char type[MAX_BT_OBEX_FILE_TYPE_SIZE];
    char name[MAX_BT_OBEX_FILE_NAME]; // name: unicode type, should be uint16
    unsigned int name_size;
    int final; //final file packet
    int tid;
};

struct bt_task_sco_data_send {
    unsigned short int size;
    unsigned char* data;
};

struct bt_task_obex_send_response {
    int tid;
    unsigned char response_code;
};

struct bt_task_hid_connect {
    unsigned char addr[6];
    unsigned char remote_role;
};

struct bt_task_hid_data_send{
    unsigned short int type;
    unsigned short int size;
    unsigned char* data;
};

struct bt_task_map_get_message {
    unsigned char with_attachment;
    unsigned char handle_len;
    char* handle;
};

struct bt_task_map_push_message {
    unsigned char box_type;
    unsigned char msg_type;
    char* dst_num;
    unsigned short int buf_length;
    char *msg_content_buf;
};

struct bt_task_spp_send_data {
    unsigned short int length;
    unsigned char* data;
    unsigned short int spp_port;
};

struct bt_task_ftp_name {
    unsigned short int tid;
    unsigned short int name_len;
    /*
     * name is 2 byte utf-8
     * backward set name: ..
     * root set name: /
     */
    unsigned short int* name;
};

/* ------------------------------- classic command struct define end ---------------------------*/

/* ------------------------------- le command struct define start ---------------------------*/
struct bt_task_le_adv_parameters {
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

struct bt_task_le_white_list {
    struct bt_addr addr;
    unsigned char type;
};

struct bt_task_le_ext_adv_parameters {
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

struct bt_task_le_ext_adv_enable {
    unsigned char enable;
    unsigned char number;
    unsigned char *handle;
    unsigned short *duration;
    unsigned char *max_event;
};

struct bt_task_le_set_adv_random_address {
    unsigned char handle;
    struct bt_addr addr;
};

struct bt_task_le_set_random_address {
    struct bt_addr addr;
};

struct bt_task_le_adv_data {
    unsigned char data[31];
    unsigned char length;
};

struct bt_task_le_ext_adv_data {
    unsigned char data[251];
    unsigned char length;
    unsigned char handle;
    unsigned char operation;
    unsigned char fragment;
};

struct bt_task_le_scan {
    unsigned char type;
    unsigned short int interval;
    unsigned short int window;
    unsigned char own_address_type;
};

struct bt_task_le_scan_with_filter {
    unsigned char type;
    unsigned short int interval;
    unsigned short int window;
    unsigned char own_address_type;
    unsigned char filter; // 0: accept all, 1: accept white list only
};

struct bt_task_le_custom_scan_start {
    unsigned char type;
    unsigned short int interval;
    unsigned short int window;
    unsigned char own_address_type;
    signed char rssi; // if rssi >= 0, stop filter rssi;  if rssi < 0, adv rssi < config rssi, ignore.
    unsigned char max_size; // max list size(1 ~ 255)
    unsigned char notify_size; // when list size == notify size, send all list data to BT_Device.
};

struct bt_task_le_custom_get_scan_result {
    unsigned char clean_flag;
};

struct bt_task_le_notify_indicate {
    unsigned short acl;
    unsigned short att;
    unsigned char *data;
    int size;
};

struct bt_task_le_connect {
    struct bt_addr addr;
    int type;
};

struct bt_task_le_passkey {
    unsigned char set_enable; // 0: disable 1: enable
    unsigned int passkey;
    void *callback;
};

struct bt_task_le_input_passkey {
    unsigned short connection_handle;
    unsigned int passkey;
};

struct bt_task_le_security_request {
    unsigned short connection_handle;
};

struct bt_task_le_register_att_server {
    unsigned char *profile_data;
    unsigned short profile_data_len;
    att_read_callback_t read_callback;
    att_write_callback_t write_callback;
};

struct bt_task_le_custom_command {
    unsigned int param1;
    unsigned int param2;
    unsigned int param3;
    unsigned char *data;
    unsigned short size;
};

struct bt_task_le_custom_write_command {
    unsigned short conn_handle;
    unsigned short att_handle;
    unsigned char *data;
    unsigned short length;
};

struct bt_task_le_custom_write_request {
    unsigned short conn_handle;
    unsigned short att_handle;
    unsigned char *data;
    unsigned short length;
};

struct bt_task_le_custom_read_request {
    unsigned short conn_handle;
    unsigned short att_handle;
};

struct bt_task_le_custom_mtu_request {
    unsigned short conn_handle;
    unsigned short mtu;
};

struct bt_task_le_connection_parameter_update_request {
    unsigned short conn_handle;
    unsigned short conn_interval;
    unsigned short slave_latency;
    unsigned short supervision_timeout;
};

struct bt_task_le_custom_gatt_scan {
    unsigned short conn_handle;
};

struct bt_task_le_custom_gatt_scan_subscribe_default {
    unsigned char enable;
};

struct bt_task_le_set_default_phy {
    unsigned char all_phys;
    unsigned char tx_phys;
    unsigned char rx_phys;
};

/* ------------------------------- le command struct define end ---------------------------*/


/* ------------------------------- test command struct define start ---------------------------*/
struct bt_task_test_le_enhanced_receiver {
    unsigned char channel;
    unsigned char PHY;
    unsigned char modulation_index;
};

struct bt_task_test_le_enhanced_transmitter {
    unsigned char channel;
    unsigned char length_of_test_data;
    unsigned char packet_payload;
    unsigned char PHY;
};

struct bt_task_test_tx_tone {
    unsigned char start_stop;
    unsigned char channel;
    unsigned char power;
    unsigned char type;
};

struct bt_task_test_tx_packet {
    unsigned char start_stop;
    unsigned char channel;
    unsigned char power;
    unsigned char modulation;
    unsigned char dh_type;
    unsigned char data_pattern;
    struct bt_addr addr;
};

struct bt_task_test_rx_packet {
    unsigned char start_stop;
    unsigned char channel;
    unsigned char packet_type;
    unsigned char data_pattern;
    struct bt_addr addr;
};

#define MAX_BT_VENDOR_CMD_PARAMETERS 16
struct bt_task_vendor_cmd {
    unsigned int cmd;
    unsigned int parameters[MAX_BT_VENDOR_CMD_PARAMETERS];
    unsigned char valid_parametes;
};
/* ------------------------------- test command struct define end ---------------------------*/

/* ------------------------------- classic event struct define start ---------------------------*/
struct bt_event_inquiry {
    char name[MAX_BT_NAME];
    int length;
    unsigned int cod;
    unsigned char addr[6];
    char rssi;
};

struct bt_event_pairing_request {
    unsigned char name[MAX_BT_NAME];
    struct bt_addr addr;
    unsigned int numeric_value;
    unsigned int cod;
    unsigned char io_capability;
};

struct bt_event_paired {
    unsigned char name[MAX_BT_NAME];
    unsigned char addr[6];
    unsigned linkey[16];
    int linkkey_type;
    unsigned int cod;
};

struct bt_event_auth{
    struct bt_addr addr;
    int status;
};

struct bt_event_pin_request {
    struct bt_addr addr;
    unsigned char name[MAX_BT_NAME];
    unsigned int cod;
};

struct bt_event_user_passkey_notification {
    struct bt_addr addr;
    unsigned int passkey;
};

struct bt_event_user_passkey_request {
    struct bt_addr addr;
};

struct bt_event_name_indication {
    struct bt_addr addr;
    unsigned char name[MAX_BT_NAME];
    int name_length;
};

struct bt_event_acl_connect {
    unsigned char addr[6];
    unsigned short int handle;
};

struct bt_event_acl_disconnect {
    unsigned short int reason;
    unsigned short int handle;
};

struct bt_event_sco {
    unsigned char addr[6];
    unsigned int handle;
    unsigned int role;
    unsigned char is_hfp;
};

//a2dp source
struct bt_event_a2dp_connect {
    int cid;
    unsigned char addr[6];
};

struct bt_event_a2dp_disconnect {
    int cid;
};

struct bt_event_a2dp_media_mtu {
    int cid;
    unsigned int mtu;
};

struct bt_event_sbc_capabilities {
    unsigned int remote_device_samples_support;
    unsigned int config_samples;
    unsigned int config_sbc_bitpool;
    unsigned int config_channel_mode;
    unsigned int config_allocation;
    unsigned int config_subbands;
    unsigned int config_blocks;
};

//a2dp sink struct
struct bt_event_a2dp_sink_connect {
    int cid;
    unsigned char addr[6];
};

struct bt_event_a2dp_sink_disconnect {
    int cid;
};

struct bt_event_a2dp_sink_media_payload {
    unsigned char payload_type;
    unsigned char number_of_frames;
    unsigned short int sequence_number;
    unsigned short int length;
    void* data;
};

struct bt_event_a2dp_sink_sbc_capabilities {
    int reconfigure;

    int num_channels;
    int sampling_frequency;
    int block_length;
    int subbands;
    int bitpool_value;
    int channel_mode;
    int allocation_method;
};

//avrcp
struct bt_event_avrcp_connect {
    unsigned char addr[6];
};

struct bt_event_hfp_connect {
    unsigned char addr[6];
};

struct bt_event_hfp_dial {
    int type; // 0: number; 1: redial; 2: memory
    unsigned char number[MAX_BT_PHONE_NUMBER];
    int length;
};
//hfp hf
struct bt_event_hfp_hf_connect {
    unsigned char addr[6];
    unsigned char status; //0:success
};

struct bt_event_hfp_hf_dial {
    int type; // 0: number; 1: redial; 2: memory
    unsigned char number[MAX_BT_PHONE_NUMBER];
    int length;
};

struct bt_event_hfp_hf_ag_ind_status {
    unsigned char index;
    unsigned char status;
    unsigned char min_range;
    unsigned char max_range;
    char name[20];
};

struct bt_event_hfp_hf_network_status {
    unsigned char mode;
    unsigned char format;
    char name[17];
};

struct bt_event_hfp_hf_clip {
    unsigned char type;
    char number[25];
};

struct bt_event_hfp_hf_clcc {
    unsigned char idx;
    unsigned char dir;
    unsigned char status;
    unsigned char mode;
    unsigned char mpty;
    unsigned char number[MAX_BT_PHONE_NUMBER];
    unsigned char type;
};

struct bt_event_hfp_sco_data {
    unsigned short int size;
    unsigned char* sco_data;
};

struct bt_event_hfp_hf_comp {
    unsigned char cmd;
    unsigned int status;
};

//hid
struct bt_event_hid_connectted {
    unsigned char addr[6];
    unsigned int result;
};

//obex
struct bt_event_obex_connected{
    int cid;
    unsigned char status;
    unsigned short max_obex;
};

struct bt_event_obex_file_info {
    char file_name[MAX_BT_OBEX_FILE_NAME];
    int file_name_size;
    char file_type[MAX_BT_OBEX_FILE_TYPE_SIZE]; // UTF-8 not need size
    unsigned int file_size;
};

struct bt_event_obex_receive_finish {
    //file_info
    struct bt_event_obex_file_info file_info;

    //data info
    int data_len;
    void* data;
};

struct bt_event_spp_event {
    unsigned char addr[6];
    unsigned char port;
    unsigned char result;
    unsigned short int max_frame_size;
    unsigned short int data_len;
    unsigned char* data;
};

//pbap client
struct bt_event_pbap_client_pb_size {
    unsigned char status;
    unsigned char pb_path[32];
    unsigned short int pb_size;
};

struct bt_event_pbap_client_packet_data {
    unsigned short int size;
    unsigned char* data;
};

//map client
struct bt_event_map_client_packet_data {
    unsigned short int size;
    unsigned char* data;
};

struct bt_event_ftp_client_connect{
    unsigned short int cid;
    unsigned char status;
    int max_packet_length;
};

struct bt_event_ftp_client_status{
    unsigned short int cid;
    unsigned char status;
};

struct bt_event_ftp_client_pkt_data {
    unsigned short int cid;
    unsigned short int size;
    unsigned char* data;
};

struct bt_event_shutdown_complete {
    bt_stack_shutdown_clearup clearup;
};
/* ------------------------------- classic event struct define end ---------------------------*/

/* ------------------------------- le event struct define start ---------------------------*/
struct bt_event_le_scan_event {
    unsigned char event_type;
    unsigned char address_type;
    struct bt_addr address;
    unsigned char length;
    unsigned char data[31];
    signed char rssi;
};

struct bt_event_le_custom_scan_event {
    unsigned char count;
    struct bt_event_le_scan_event *item;
};

typedef void (*le_scan_event_handle_t)(struct bt_event_le_scan_event *event);

typedef void (*le_user_event_handle_t)(struct bt_task_event * msg);

struct bt_event_le_att_connected {
    struct bt_addr addr;
    unsigned char addr_type;
    int acl_handle;
    int role;
    unsigned char peer_irk[16];
};

struct bt_event_le_bond_complete {
    unsigned char address_type;
    struct bt_addr address;
};

struct bt_event_le_pair_complete {
    unsigned short connection_handle;
    unsigned char address_type;
    struct bt_addr address;
    unsigned char status; // error code
    unsigned char reason; // fail reason
};

struct bt_event_le_encryption_change {
    unsigned char status;
    unsigned short connection_handle;
    unsigned char encryption_enabled;
};

struct bt_event_le_error_rsp {
    unsigned char request;
    unsigned short att_handle;
    unsigned char code;
};

struct bt_event_le_mtu_exchange {
    int mut;
    int acl_handle;
};

struct bt_event_le_smp_passkey {
    struct bt_addr addr;
    unsigned int passkey_value;
};

struct bt_event_le_smp_passkey_input_number {
    unsigned short connection_handle;
    struct bt_addr addr;
};

struct bt_event_le_identity_info_event {
    unsigned char success_flag;
    unsigned char index;
    unsigned short connection_handle;
    struct bt_addr address;
    unsigned char address_type;
    struct bt_addr identity_address;
    unsigned char identity_address_type;
};

struct bt_event_le_get_pair_info_event {
    unsigned char valid_count;
    unsigned char max_count;
    unsigned char index;
    struct bt_addr identity_address;
    unsigned char identity_address_type;
    unsigned int seq;
};

struct bt_event_le_client_handle_notify {
    unsigned short connection_handle;
    unsigned short value_handle;
    unsigned char value[255];
    unsigned short value_length;
};

struct bt_event_le_client_handle_indication {
    unsigned short connection_handle;
    unsigned short value_handle;
    unsigned char value[255];
    unsigned short value_length;
};

struct bt_event_le_att_can_write_now {
    unsigned short connection_handle;
    unsigned char write_type;
};

struct bt_event_le_att_read_data_result {
    unsigned short connection_handle;
    unsigned short value_handle;
    unsigned short value_offset;
    unsigned short value_len;
    unsigned char value[255];
};

struct bt_event_le_att_read_over_result {
    unsigned short connection_handle;
    unsigned char att_status;
};

struct bt_event_le_gatt_scan_result {
    unsigned short connection_handle;
    unsigned char status;
};

struct bt_event_le_gatt_scan_dump_service {
    unsigned short connection_handle;
    gatt_client_asr_service_t service;
};

struct bt_event_le_gatt_scan_dump_characteristic {
    unsigned short connection_handle;
    gatt_client_asr_characteristic_t characteristic;
};

struct bt_event_le_gatt_scan_dump_descriptor {
    unsigned short connection_handle;
    gatt_client_asr_characteristic_descriptor_t descriptor;
};

struct bt_event_le_connection_parameter_update {
    unsigned char status;
    unsigned short conn_handle;
    unsigned short conn_interval;
    unsigned short slave_latency;
    unsigned short supervision_timeout;
};

struct bt_event_le_write_flow_control_event {
    unsigned char enable;
};

struct bt_event_le_connection_complete {
    unsigned char status;
    unsigned short connection_handle;
    unsigned char role;
    unsigned char addr_type;
    struct bt_addr addr;
    unsigned short interval;
    unsigned short latency;
    unsigned short timeout;
    unsigned char clock_accuracy;
};

struct bt_event_le_disconnection_complete {
    unsigned char status;
    unsigned short connection_handle;
    unsigned char reason;
};

/* ------------------------------- le event struct define end ---------------------------*/

/* ------------------------------- test event struct define start ---------------------------*/
struct bt_event_hci_command_complete {
    unsigned short opcode;
    unsigned int status;
    unsigned char data[255];
};

struct bt_event_btrx_result {
    unsigned char channel;
    float rssi;
    unsigned int rxok;
    unsigned int rxtotal;
    unsigned int headerok;
    unsigned int syncok;
};

/* ------------------------------- test event struct define end ---------------------------*/

#endif
