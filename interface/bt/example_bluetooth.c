#include <stdio.h>
#include <string.h>
#include "ql_bt.h"
#include "ql_rtos.h"
#include "ql_log.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_gpio.h"
#include "ql_bt_drive.h"

#include "ql_fs.h"


#define OC_MODE 3 // 0=ec200m 1=800m 2=eg915keu_la 3=ec800k
#define BLE_DEMO_SLAVE
//#define BLE_DEMO_MASTER
#define BLE_DEMO_HCI_CON_HANDLE_INVALID 0xffff

#define LE_NAME "QUEC SLAVE"
#define LE_HID_APPEARANCE 962

//QL_BT_USE_UART 1 main uart
//QL_BT_USE_UART 2 uart3
//QL_BT_USE_UART 3 uart4


#if OC_MODE==0
#define QL_BT_USE_UART 3 //3--uart4
#define BT_REMOTE_HID_HOST 1
#define QL_GPIO_BT_POWER 77
#define QL_GPIO_BT_RESET 126
#define QL_GPIO_BT_LDO_EN 37
#define QL_GPIO_BT_WAKEUP_HOST 117
#define QL_GPIO_HOST_WAKEUP_BT 118 
#elif OC_MODE==1
#define QL_BT_USE_UART 1
#define QL_GPIO_BT_POWER 26
#define QL_GPIO_BT_RESET 24
#define QL_GPIO_BT_LDO_EN 23
#define QL_GPIO_BT_WAKEUP_HOST 25
#define QL_GPIO_HOST_WAKEUP_BT 14
#elif OC_MODE==2
#define QL_BT_USE_UART 2
#define QL_GPIO_BT_RESET   37
#define QL_GPIO_BT_LDO_EN  57
#define QL_GPIO_BT_WAKEUP_HOST 84
#define QL_GPIO_HOST_WAKEUP_BT 89
#define QL_GPIO_BT_POWER   90
#elif OC_MODE==3
#define QL_BT_USE_UART 1
#define QL_GPIO_BT_RESET   84
#define QL_GPIO_BT_LDO_EN  86
#define QL_GPIO_BT_WAKEUP_HOST 56
#define QL_GPIO_HOST_WAKEUP_BT 57
#define QL_GPIO_BT_POWER   90
#endif

void print_int_data(const uint8_t *data, size_t length)
{
    ql_app_log("\r\n");
    for (size_t i = 0; i < length; ++i)
    {
        printf(" %02X ", data[i]);
    }
    ql_app_log("\r\n");
}
//BLE SLAVE
#ifdef BLE_DEMO_SLAVE

#define BAS_SUPPORT 1
#define CUSTOM_SUPPORT 1
#define HID_SUPPORT 0
#include "ql_ble_slave_profile.h"
int user_ble_slave_handle = BLE_DEMO_HCI_CON_HANDLE_INVALID;
volatile uint8_t ble_demo_loop_test_buf[256];
volatile uint16_t user_ble_slave_notify_flag = 0;
volatile uint16_t ble_demo_loop_test_handle = 0;

uint32_t ble_demo_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

uint16_t ble_demo_little_endian_read_16(const uint8_t *buffer, int position)
{
    return (uint16_t)(((uint16_t)buffer[position]) | (((uint16_t)buffer[position + 1]) << 8));
}

// att_read_callback helpers
uint16_t ble_demo_att_read_callback_handle_blob(const uint8_t *blob, uint16_t blob_size, uint16_t offset, uint8_t *buffer,
                                                uint16_t buffer_size)
{

    if (buffer != NULL)
    {
        uint16_t bytes_to_copy = 0;
        if (blob_size >= offset)
        {
            bytes_to_copy = ble_demo_min(blob_size - offset, buffer_size);
            (void)memcpy(buffer, &blob[offset], bytes_to_copy);
        }
        return bytes_to_copy;
    }
    else
    {
        return blob_size;
    }
}

static uint16_t att_read_callback(uint16_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int att_write_callback(uint16_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer,
                              uint16_t buffer_size);

// test param
static uint8_t test_value = 0;
static uint8_t test_buf[256];

static uint8_t service_changed;
static uint16_t service_changed_client_configuration;

#if BAS_SUPPORT==1
static uint8_t battery_value;
static uint16_t battery_value_client_configuration;
#endif

#if CUSTOM_SUPPORT==1
static uint8_t custom_value1[256];
static uint16_t custom_value_client_configuration1;
static uint8_t custom_value2[20];
static uint16_t custom_value_client_configuration2;
#endif

#if HID_SUPPORT ==1
asdas
static uint8_t hid_protocol_mode = 1; // 0: boot mode, 1: report mode
static uint8_t hid_control_point = 1; // 0: suspend, 1: running
static uint16_t hid_report_input_input_value = 0;
static uint16_t hid_report_input_input_value_client_configuration = 0;
static uint16_t hid_report_output_input_value = 0;
static uint16_t hid_report_output_input_value_client_configuration = 0;
static uint16_t hid_report_feature_input_value = 0;
static uint16_t hid_report_feature_input_value_client_configuration = 0;
static uint16_t hid_boot_mouse_input_value = 0;
static uint16_t hid_boot_mouse_input_value_client_configuration = 0;
static uint16_t hid_boot_keyboard_input_value = 0;
static uint16_t hid_boot_keyboard_input_value_client_configuration = 0;
#endif

static uint16_t att_read_callback(uint16_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer,
                                  uint16_t buffer_size)
{

    ql_app_log("[DEMO][%s] connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer: 0x%x, buffer_size: %d", __func__, connection_handle,
               att_handle, offset,
               buffer, buffer_size);

    if (buffer == NULL)
    {
        ql_app_log("[DEMO][%s] used for check value len", __func__);
    }

    switch (att_handle)
    {
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            service_changed++;
            ql_app_log("[DEMO][%s] service_changed_client_configuration: %d, service_changed: %d", __func__, service_changed_client_configuration, service_changed);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&service_changed_client_configuration, sizeof(service_changed_client_configuration), offset,
                                                      buffer, buffer_size);
#if BAS_SUPPORT ==1
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            battery_value++;
            ql_app_log("[DEMO][%s] battery_value: %d", __func__, battery_value);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&battery_value, sizeof(battery_value), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            battery_value++;
            ql_app_log("[DEMO][%s] battery_value_client_configuration: %d, battery_value: %d", __func__, battery_value_client_configuration, battery_value);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&battery_value_client_configuration, sizeof(battery_value_client_configuration), offset, buffer,
                                                      buffer_size);
#endif
#if CUSTOM_SUPPORT==1
    case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            memcpy(custom_value1, test_buf, sizeof(custom_value1));
            ql_app_log("[DEMO][%s] custom_value1: 0x%x", __func__, custom_value1);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)custom_value1, sizeof(custom_value1), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            custom_value1[0]++;
            ql_app_log("[DEMO][%s] custom_value_client_configuration1: %d, custom_value1: %d", __func__, custom_value_client_configuration1, custom_value1[0]);
        }
        // att_server_request_can_send_now_event(connection_handle);
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&custom_value_client_configuration1, sizeof(custom_value_client_configuration1), offset, buffer,
                                                      buffer_size);
    case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            memcpy(custom_value2, test_buf, sizeof(custom_value2));
            ql_app_log("[DEMO][%s] custom_value2: %d", __func__, custom_value2);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)custom_value2, sizeof(custom_value2), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            custom_value2[0]++;
            ql_app_log("[DEMO][%s] custom_value_client_configuration2: %d, custom_value2: %d", __func__, custom_value_client_configuration2, custom_value2[0]);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&custom_value_client_configuration2, sizeof(custom_value_client_configuration2), offset, buffer,
                                                      buffer_size);
#endif
#if HID_SUPPORT ==1
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_protocol_mode: %d", __func__, hid_protocol_mode);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_protocol_mode, sizeof(hid_protocol_mode), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_report_map: %d", __func__, hid_report_map);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_map, sizeof(hid_report_map), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT_01_VALUE_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_control_point: %d", __func__, hid_control_point);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_control_point, sizeof(hid_control_point), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_report_input_input_value_client_configuration: %d", __func__, hid_report_input_input_value_client_configuration);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_input_input_value_client_configuration,
                                                      sizeof(hid_report_input_input_value_client_configuration), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_report_output_input_value_client_configuration: %d", __func__, hid_report_output_input_value_client_configuration);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_output_input_value_client_configuration,
                                                      sizeof(hid_report_output_input_value_client_configuration), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_report_feature_input_value_client_configuration: %d", __func__, hid_report_feature_input_value_client_configuration);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_feature_input_value_client_configuration,
                                                      sizeof(hid_report_feature_input_value_client_configuration), offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_boot_mouse_input_value_client_configuration: %d", __func__, hid_boot_mouse_input_value_client_configuration);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_boot_mouse_input_value_client_configuration,
                                                      sizeof(hid_boot_mouse_input_value_client_configuration),
                                                      offset, buffer, buffer_size);
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer != NULL)
        {
            ql_app_log("[DEMO][%s] hid_boot_keyboard_input_value_client_configuration: %d", __func__, hid_boot_keyboard_input_value_client_configuration);
        }
        return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_boot_keyboard_input_value_client_configuration,
                                                      sizeof(hid_boot_keyboard_input_value_client_configuration), offset, buffer, buffer_size);
#endif
    default:
        break;
    }

    return 0;
}

static int att_write_callback(uint16_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer,
                              uint16_t buffer_size)
{

    ql_app_log("[DEMO][%s] connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer_size: %d", __func__, connection_handle, att_handle, offset, buffer_size);
    ql_app_log("att_write_callback:", buffer, buffer_size);

    switch (att_handle)
    {
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_CLIENT_CONFIGURATION_HANDLE:
        service_changed_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] service_changed_client_configuration: %d", __func__, service_changed_client_configuration);
        return 0;
#if BAS_SUPPORT==1
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_CLIENT_CONFIGURATION_HANDLE:
        battery_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] battery_value_client_configuration: %d", __func__, battery_value_client_configuration);
        return 0;
#endif
#if CUSTOM_SUPPORT ==1
    case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_CLIENT_CONFIGURATION_HANDLE:
        custom_value_client_configuration1 = ble_demo_little_endian_read_16(buffer, 0);

        user_ble_slave_notify_flag = custom_value_client_configuration1;

        ql_app_log("[DEMO][%s] custom_value_client_configuration1: %d", __func__, custom_value_client_configuration1);
        return 0;
    case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_CLIENT_CONFIGURATION_HANDLE:
        custom_value_client_configuration2 = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] custom_value_client_configuration2: %d", __func__, custom_value_client_configuration2);
        return 0;
    case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_VALUE_HANDLE:
        print_int_data(buffer, buffer_size);
    case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE:
        print_int_data(buffer, buffer_size);

        return 0;
#endif
#if HID_SUPPORT==1
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE_01_VALUE_HANDLE:
        hid_protocol_mode = buffer[0];
        ql_app_log("[DEMO][%s] hid_protocol_mode: %d(0: boot mode, 1: report mode)", __func__, hid_protocol_mode);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT_01_VALUE_HANDLE:
        hid_control_point = buffer[0];
        ql_app_log("[DEMO][%s] hid_control_point: %d(0: suspend, 1: running)", __func__, hid_control_point);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        hid_report_input_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] hid_report_input_input_value_client_configuration: %d", __func__, hid_report_input_input_value_client_configuration);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_CLIENT_CONFIGURATION_HANDLE:
        hid_report_output_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] hid_report_output_input_value_client_configuration: %d", __func__, hid_report_output_input_value_client_configuration);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_CLIENT_CONFIGURATION_HANDLE:
        hid_report_feature_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] hid_report_feature_input_value_client_configuration: %d", __func__, hid_report_feature_input_value_client_configuration);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        hid_boot_keyboard_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        return 0;
    case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
        hid_boot_mouse_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
        ql_app_log("[DEMO][%s] hid_boot_mouse_input_value_client_configuration: %d", __func__, hid_boot_mouse_input_value_client_configuration);
        return 0;
#endif
    default:
        break;
    }
    return 0;
}
void ble_slave_demo_register(void)
{
    int i;
    for (i = 0; i < sizeof(test_buf); i++)
    {
        test_buf[i] = i;
    }

    memcpy((uint8_t *)ble_demo_loop_test_buf, test_buf, sizeof(ble_demo_loop_test_buf));
    ble_demo_loop_test_handle = ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE;

    ql_app_log("[DEMO] %s", __func__);
    ql_bt_le_register_att_service(profile_data, sizeof(profile_data), att_read_callback, att_write_callback);
}
#endif

#ifdef BLE_DEMO_MASTER
int user_ble_master_handle = 0xff;
volatile uint8_t ble_demo_master_test_buf[256];
volatile uint8_t user_ble_master_scan_flag = 0;

static unsigned short user_att_name_handle = BLE_DEMO_HCI_CON_HANDLE_INVALID;
#define USER_TEST_ATT_NAME 0X1213
#define USER_TEST_SLAVE_BLE_NAME "test_server"

char *ble_demo_uuid128_to_str(const uint8_t *uuid)
{
    static char appbt_uuid128_to_str_buffer[32 + 4 + 1];
    int i;
    int j = 0;
    // after 4, 6, 8, and 10 bytes = XYXYXYXY-XYXY-XYXY-XYXY-XYXYXYXYXYXY, there's a dash
    const int dash_locations = (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9);
    for (i = 0; i < 16; i++)
    {
        snprintf(appbt_uuid128_to_str_buffer + i * 2 + j, 4, "%02x", uuid[i]);
        if (dash_locations & (1 << i))
        {
            snprintf(appbt_uuid128_to_str_buffer + i * 2 + 2 + j, 4, "-");
            j++;
        }
    }
    return appbt_uuid128_to_str_buffer;
}
char *ble_demo_print_properties(uint16_t properties)
{
    static char properties_log[100] = {0};
    int offset = 0;
    if ((properties & LE_ATT_CHARC_PROP_READ) == LE_ATT_CHARC_PROP_READ)
    {
        snprintf(properties_log + offset, strlen("read | ") + 1, "read | ");
        offset += strlen("read | ");
    }
    if ((properties & LE_ATT_CHARC_PROP_WWP) == LE_ATT_CHARC_PROP_WWP)
    {
        snprintf(properties_log + offset, strlen("write_without_response | ") + 1, "write_without_response | ");
        offset += strlen("write_without_response | ");
    }
    if ((properties & LE_ATT_CHARC_PROP_WRITE) == LE_ATT_CHARC_PROP_WRITE)
    {
        snprintf(properties_log + offset, strlen("write | ") + 1, "write | ");
        offset += strlen("write | ");
    }
    if ((properties & LE_ATT_CHARC_PROP_NOTIFY) == LE_ATT_CHARC_PROP_NOTIFY)
    {
        snprintf(properties_log + offset, strlen("notify | ") + 1, "notify | ");
        offset += strlen("notify | ");
    }
    if ((properties & LE_ATT_CHARC_PROP_INDICATE) == LE_ATT_CHARC_PROP_INDICATE)
    {
        snprintf(properties_log + offset, strlen("indicate | ") + 1, "indicate | ");
        offset += strlen("indicate | ");
    }
    snprintf(properties_log + offset - 3, 1, "~");
    return properties_log;
}
void master_name_scan_event_handle(struct bt_event_le_scan_event *event)
{
    int index = 0;
    int total_size = event->length;
    while (total_size > 0 && (event->event_type == LE_PDU_TYPE_IND || event->event_type == LE_PDU_TYPE_SCAN_RESP))
    {
        int length = event->data[index];
        int type = event->data[index + 1];
        ql_app_log("[DEMO] event_type: %d, address_type: %d, address: %02x%02x%02x%02x%02x%02x, handle type %02x, length %d, index %d , rssi %d",
                   event->event_type, event->address_type,
                   event->address.bytes[0], event->address.bytes[1],
                   event->address.bytes[2], event->address.bytes[3],
                   event->address.bytes[4], event->address.bytes[5],
                   type, length, index, event->rssi);

        switch (type)
        {
        case LE_GAP_TYPE_COMPLETE_NAME:
        {
            char name[31];
            memset(name, 0, 31);
            memcpy(name, &event->data[index + 2], length - 1);
            ql_app_log("[DEMO] handle name %s", name);
            if (strstr(name, USER_TEST_SLAVE_BLE_NAME))
            {
                ql_bt_le_scan_stop();
                ql_app_log("[DEMO][LE-client]found test device. "
                           "address type %d, "
                           "addrss %02x%02x%02x%02x%02x%02x\r\n",
                           event->address_type,
                           event->address.bytes[0], event->address.bytes[1],
                           event->address.bytes[2], event->address.bytes[3],
                           event->address.bytes[4], event->address.bytes[5]);
                ql_bt_le_connect(event->address, event->address_type);
            }
            break;
        }
        default:
            break;
        }
        index += (length + 1);
        total_size -= (length + 1);
    }
}
#endif

static void ql_bt_open_gpio_set() //?????????pin?????????????GPIO??
{
    ql_gpio_init(QL_GPIO_BT_POWER, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(QL_GPIO_BT_RESET, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(QL_GPIO_BT_LDO_EN, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(QL_GPIO_BT_WAKEUP_HOST, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(QL_GPIO_HOST_WAKEUP_BT, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
}
static bt_uart_cfg ql_bt_uart_port =
    {
        QL_BT_USE_UART,  //you can change it, if you use other UART port
        1,               //do not change it
        0,               //do not change it
        0,               //do not change it
        bt_rx_int,       //do not change it
        0,               //do not change it
        0,               //do not change it
        BAUDRATE_921600, //do not change it. it is a default baudrate. stack will change baudrate accroding bt_user_cfg
        BT_DATA_8,       //do not change it
        BT_PARITY_NONE,  //do not change it
        NULL,            //do not change it. host stack will use it
        NULL,            //do not change it. host stack will use it
};
static struct bt_user_cfg cfg = {
    .bt_mode = BT_MODE_BR_LE,
    .h4_h5 = BT_HCI_MODE_H5,
    .firmware_where = BT_FIRMWARE_AT_FLASH,
    .hw.baudrate = BAUDRATE_921600,
    .hw.uart_cfg = &ql_bt_uart_port,
    .hw.reset_pin = QL_GPIO_BT_RESET,
    .hw.pwdn_pin = QL_GPIO_BT_LDO_EN,
    .hw.host_wakeup_bt_pin = QL_GPIO_HOST_WAKEUP_BT,
    .hw.bt_wakeup_host_pin = QL_GPIO_BT_WAKEUP_HOST,
    .profile = {
        .prf = BT_HFP_PRF | BT_A2DP_SOURCE_PRF | BT_AVRCP_PRF | BT_SPP_PRF | BT_OBEX_PRF,
    },
    .br_common = {
        .name = "QUECTEL",
        .io_capability = 3,
    },

};


static void ql_bt_usr_handle_ble(int status, void *event_data)
{
    char buffer[256] = {0};
    switch (status)
    {

    case BTTASK_IND_LE_GATT_CONNECTED:

    {
        struct bt_event_le_att_connected *connect = (struct bt_event_le_att_connected *)event_data;
        ql_app_log("[DEMO]handle GATT connected(%d) handle %d "
                   "%02x%02x%02x%02x%02x%02x type %d\r\n",
                   connect->role,
                   connect->acl_handle,
                   connect->addr.bytes[0], connect->addr.bytes[1],
                   connect->addr.bytes[2], connect->addr.bytes[3],
                   connect->addr.bytes[4], connect->addr.bytes[5],
                   connect->addr_type);
        if (connect->role == 1)
        {
#ifdef BLE_DEMO_SLAVE
            user_ble_slave_handle = connect->acl_handle;

            ql_bt_le_set_adv_enable(0);
            ql_le_custom_mtu_request(user_ble_slave_handle, 400);
#endif
        }
#ifdef BLE_DEMO_MASTER
        else
        {
            user_ble_master_handle = connect->acl_handle;
            ql_le_custom_mtu_request(user_ble_master_handle, 400);

            ql_le_custom_gatt_scan_start(user_ble_master_handle);
        }
#endif
       
        break;
    }

    case BTTASK_IND_LE_GATT_DISCONNECTED:
    {
        int *conn_handle = (int *)event_data;
#ifdef BLE_DEMO_SLAVE
        if (user_ble_slave_handle == *conn_handle)
        {
            ql_bt_le_set_adv_enable(1);
            user_ble_slave_handle = BLE_DEMO_HCI_CON_HANDLE_INVALID;
        }
#endif
#ifdef BLE_DEMO_MASTER
        if (user_ble_master_handle == *conn_handle)
        {
            user_ble_master_handle = BLE_DEMO_HCI_CON_HANDLE_INVALID;
        }
#endif

        ql_app_log("[DEMO]GATT disconnected, handle: %d\n", *conn_handle);
        break;
    }
    case BTTASK_IND_LE_CUSTOM_SCAN_EVENT:
    {
        struct bt_event_le_custom_scan_event *scan = (struct bt_event_le_custom_scan_event *)event_data;
        struct bt_event_le_scan_event *p_item = scan->item;
        int i;

        ql_app_log("[DEMO]handle BTTASK_IND_LE_CUSTOM_SCAN_EVENT, count: %d: ", scan->count);

        for (i = 0; i < scan->count; i++)
        {
            int total_size = (int)p_item->length;

            ql_app_log("[DEMO]handle BTTASK_IND_LE_CUSTOM_SCAN_EVENT from: "
                       "%02x:%02x:%02x:%02x:%02x:%02x. length %d. type %d, rssi %dr\n",
                       p_item->address.bytes[0], p_item->address.bytes[1],
                       p_item->address.bytes[2], p_item->address.bytes[3],
                       p_item->address.bytes[4], p_item->address.bytes[5],
                       p_item->length, p_item->event_type,
                       p_item->rssi);

            p_item++;
        }

        break;
    }
    case BTTASK_IND_LE_CONNECTION_COMPLETE:
    {

        struct bt_event_le_connection_complete *le_conn_complete = (struct bt_event_le_connection_complete *)event_data;
        ql_app_log("[DEMO]le connection complete, status: %d, conn_handle: %d, role: %d, param: %d, %d, %d, clock_accuracy %d\r\n",
                   le_conn_complete->status, le_conn_complete->connection_handle, le_conn_complete->role, le_conn_complete->interval,
                   le_conn_complete->latency, le_conn_complete->timeout, le_conn_complete->clock_accuracy);

        break;
    }
    case BTTASK_IND_LE_DISCONNECTION_COMPLETE:
    {

        struct bt_event_le_disconnection_complete *le_disconn_complete = (struct bt_event_le_disconnection_complete *)event_data;
        ql_app_log("[DEMO]le disconnection complete, status: %d, conn_handle: %d, reason: 0x%x\r\n",
                   le_disconn_complete->status, le_disconn_complete->connection_handle, le_disconn_complete->reason);

        break;
    }

    case BTTASK_IND_LE_MTU_EXCHANGED:

    {
        struct bt_event_le_mtu_exchange *mtu = (struct bt_event_le_mtu_exchange *)event_data;
        ql_app_log("[DEMO]handle GATT MTU exchange message, mtu: %d, handle: %d", mtu->mut, mtu->acl_handle);

        break;
    }

    case BTTASK_IND_LE_CONNECTION_UPDATE_COMPLETE:
    {
        struct bt_event_le_connection_parameter_update *param = (struct bt_event_le_connection_parameter_update *)event_data;

        ql_app_log("[DEMO]handle LE_CONNECTION_UPDATE_COMPLETE, status: %d, conn_handle: 0x%x, interval: %d, latency: %d, timeout: %d",
                   param->status, param->conn_handle, param->conn_interval, param->slave_latency, param->supervision_timeout);

        break;
    }
    case BTTASK_IND_ADV_PHY_TXPOWER:
    {
        signed char *tx_power = (signed char *)event_data;
        ql_app_log("[DEMO]handle TX power %d\r\n", *tx_power);
        break;
    }
    case BTTASK_IND_SMP_PASSKEY:
    {
        struct bt_event_le_smp_passkey *passkey =
            (struct bt_event_le_smp_passkey *)event_data;

        ql_app_log("[DEMO]handle SMP PASSKEY %d %02X%02X%02X%02X%02X%02X\r\n",
                   passkey->passkey_value,
                   passkey->addr.bytes[0], passkey->addr.bytes[1],
                   passkey->addr.bytes[2], passkey->addr.bytes[3],
                   passkey->addr.bytes[4], passkey->addr.bytes[5]);

        break;
    }
    case BTTASK_IND_LE_ATT_INDICATION_COMPLETE:
    {
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_INDICATION_COMPLETE\n");
        break;
    }
    case BTTASK_IND_LE_ATT_EVENT_CAN_SEND_NOW:
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_EVENT_CAN_SEND_NOW\n");
        break;

    case BTTASK_IND_SLAVE_LE_BOND_COMPLETE:
    {
        struct bt_event_le_bond_complete *event =
            (struct bt_event_le_bond_complete *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_SLAVE_LE_BOND_COMPLETE, type: %d, address: %02x%02x%02x%02x%02x%02x\r\n",
                   event->address_type,
                   event->address.bytes[0], event->address.bytes[1], event->address.bytes[2],
                   event->address.bytes[3], event->address.bytes[4], event->address.bytes[5]);

        break;
    }

    case BTTASK_IND_LE_PAIR_COMPLETE:
    {
        struct bt_event_le_pair_complete *event =
            (struct bt_event_le_pair_complete *)event_data;
        ql_app_log("[DEMO]le pair complete, conn_handle: 0x%x, status: %d, reason: %d",
                   event->connection_handle, event->status, event->reason);

        break;
    }
    case BTTASK_IND_LE_IDENTITY_INFO:
    {
        ql_app_log("[DEMO] BTTASK_IND_LE_IDENTITY_INFO\n");
        struct bt_event_le_identity_info_event *info =
            (struct bt_event_le_identity_info_event *)event_data;
        if (info->success_flag == 1)
        {
            ql_app_log("[DEMO] identity success, index: %d, connection_handle: 0x%x\r\n"
                       "addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n"
                       "identity_addr: %02x%02x%02x%02x%02x%02x, identity_addr_type: %d\r\n",
                       info->index, info->connection_handle,
                       info->address.bytes[0], info->address.bytes[1],
                       info->address.bytes[2], info->address.bytes[3],
                       info->address.bytes[4], info->address.bytes[5],
                       info->address_type,
                       info->identity_address.bytes[0], info->identity_address.bytes[1],
                       info->identity_address.bytes[2], info->identity_address.bytes[3],
                       info->identity_address.bytes[4], info->identity_address.bytes[5],
                       info->identity_address_type);
        }
        else
        {
            ql_app_log(
                "[DEMO] identity fail, index: %d, connection_handle: 0x%x\r\n"
                "addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
                info->index, info->connection_handle,
                info->address.bytes[0], info->address.bytes[1],
                info->address.bytes[2], info->address.bytes[3],
                info->address.bytes[4], info->address.bytes[5],
                info->address_type);
        }

        break;
    }

    case BTTASK_IND_LE_DB_CONTROL_GET_INFO:
    {
        ql_app_log("[DEMO] BTTASK_IND_LE_DB_CONTROL_GET_INFO\n");
        struct bt_event_le_get_pair_info_event *info =
            (struct bt_event_le_get_pair_info_event *)event_data;

        ql_app_log("[DEMO] index: %d, seq: %d, valid count: %d, max count: %d, "
                   "addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
                   info->index, info->seq, info->valid_count, info->max_count,
                   info->identity_address.bytes[0], info->identity_address.bytes[1],
                   info->identity_address.bytes[2], info->identity_address.bytes[3],
                   info->identity_address.bytes[4], info->identity_address.bytes[5],
                   info->identity_address_type);

        break;
    }

#ifdef BLE_DEMO_MASTER
    case BTTASK_IND_LE_GATT_SCAN_RESULT:
    {
        struct bt_event_le_gatt_scan_result *result =
            (struct bt_event_le_gatt_scan_result *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_GATT_SCAN_RESULT, connection_handle: 0x%x, status: %d\n", result->connection_handle, result->status);
        if (0 == result->status)
        {
            ql_app_log("[DEMO] gatt scan success\n");
#ifdef BLE_DEMO_MASTER
            user_ble_master_scan_flag = 1;
#endif
        }
        break;
    }
    case BTTASK_IND_LE_ATT_EVENT_CAN_WRITE_NOW:
    {
        struct bt_event_le_att_can_write_now *event =
            (struct bt_event_le_att_can_write_now *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_EVENT_CAN_WRITE_NOW, conn: 0x%x, write_type: %d\n", event->connection_handle, event->write_type);
        break;
    }

    case BTTASK_IND_LE_ATT_EVENT_WRITE_FLOW_CONTROL:
    {
        struct bt_event_le_write_flow_control_event *event = (struct bt_event_le_write_flow_control_event *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_EVENT_WRITE_FLOW_CONTROL, flow control: %s\n", event->enable ? "wait" : "continue");

        break;
    }

    case BTTASK_IND_LE_ATT_EVENT_READ_DATA_RESULT:
    {
        struct bt_event_le_att_read_data_result *event =
            (struct bt_event_le_att_read_data_result *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_EVENT_READ_DATA_RESULT, connection_handle: 0x%x, value_handle: 0x%x, value_offset: %d, value_length: %d\n",
                   event->connection_handle, event->value_handle, event->value_offset, event->value_len);
        print_int_data(event->value, event->value_len);
        break;
    }
    case BTTASK_IND_LE_ATT_EVENT_READ_OVER_RESULT:
    {
        struct bt_event_le_att_read_over_result *event =
            (struct bt_event_le_att_read_over_result *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_ATT_EVENT_READ_OVER_RESULT, connection_handle: 0x%x, att_status: %d\n",
                   event->connection_handle, event->att_status);

        break;
    }
    case BTTASK_IND_LE_CLIENT_HANDLE_NOTIFY:
    {
        struct bt_event_le_client_handle_notify *notify =
            (struct bt_event_le_client_handle_notify *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_CLIENT_HANDLE_NOTIFY, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\n",
                   notify->connection_handle, notify->value_handle, notify->value_length);
        print_int_data(notify->value, notify->value_length);

        break;
    }
    case BTTASK_IND_LE_CLIENT_HANDLE_INDIATION:
    {
        struct bt_event_le_client_handle_indication *indication =
            (struct bt_event_le_client_handle_indication *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_CLIENT_HANDLE_INDIATION, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\n",
                   indication->connection_handle, indication->value_handle, indication->value_length);
        print_int_data(indication->value, indication->value_length);
        break;
    }
    case BTTASK_IND_LE_GATT_SCAN_DUMP_SERVICE:
    {

        struct bt_event_le_gatt_scan_dump_service *result =
            (struct bt_event_le_gatt_scan_dump_service *)event_data;
        gatt_client_asr_service_t *service = &result->service;
        if (service->uuid16)
        {
            ql_app_log("[DEMO]* service: [0x%x-0x%x], uuid 0x%x",
                       service->start_group_handle, service->end_group_handle, service->uuid16);
        }
        else
        {
            ql_app_log("[DEMO]* service: [0x%x-0x%x], uuid  %s",
                       service->start_group_handle, service->end_group_handle, ble_demo_uuid128_to_str(service->uuid128));
        }

        break;
    }
    case BTTASK_IND_LE_GATT_SCAN_DUMP_CHARACTERISTIC:
    {

        struct bt_event_le_gatt_scan_dump_characteristic *result =
            (struct bt_event_le_gatt_scan_dump_characteristic *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_GATT_SCAN_DUMP_CHARACTERISTIC, connection_handle: 0x%x\n", result->connection_handle);
        gatt_client_asr_characteristic_t *characteristic = &result->characteristic;
        if (characteristic->uuid16)
        {
            ql_app_log("[DEMO]*   characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid 0x%x",
                       characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                       ble_demo_print_properties(characteristic->properties), characteristic->uuid16);
            if (characteristic->uuid16 == USER_TEST_ATT_NAME)
            {
                ql_app_log("[DEMO] find device name att handle: 0x%x\n", characteristic->value_handle);
                user_att_name_handle = characteristic->value_handle;
            }
        }
        else
        {

            ql_app_log("[DEMO]*   characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid %s",
                       characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                       ble_demo_print_properties(characteristic->properties), ble_demo_uuid128_to_str(characteristic->uuid128));
#if 0
                 if (strcmp(ble_demo_uuid128_to_str(characteristic->uuid128), USER_TEST_ATT_NAME) == 0)
                    {
                    ql_app_log("[DEMO] find device name att handle: 0x%x\n", characteristic->value_handle);
                    user_att_name_handle = characteristic->value_handle;
                    }
#endif
        }

        break;
    }
    case BTTASK_IND_LE_GATT_SCAN_DUMP_DESCRIPTOR:
    {
        struct bt_event_le_gatt_scan_dump_descriptor *result =
            (struct bt_event_le_gatt_scan_dump_descriptor *)event_data;
        ql_app_log("[DEMO] BTTASK_IND_LE_GATT_SCAN_DUMP_DESCRIPTOR, connection_handle: 0x%x\n", result->connection_handle);
        gatt_client_asr_characteristic_descriptor_t *descriptor = &result->descriptor;
        if (descriptor->uuid16)
        {
            ql_app_log("[DEMO]* descriptor, handle: 0x%x, uuid 0x%x", descriptor->handle, descriptor->uuid16);
        }
        else
        {

            ql_app_log("[DEMO]* descriptor, handle: 0x%x, uuid %s", descriptor->handle, ble_demo_uuid128_to_str(descriptor->uuid128));
        }
        break;
    }
#endif

    }
}

void ql_bt_le_service_process(void)
{
    struct bt_addr addr = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    struct bt_task_le_adv_parameters adv_params;
    struct ql_bt_adv_params ql_params;

    ql_bt_le_set_random_address(addr);
    memset(&adv_params, 0, sizeof(struct bt_task_le_adv_parameters));
    adv_params.interval_min = 0x30; // 30ms
    adv_params.interval_max = 0x30; // 30ms
    adv_params.advertising_type = LE_ADV_TYPE_IND;
    adv_params.own_address_type = LE_ADDRESS_TYPE_RANDOM;
    adv_params.filter = 0; //
#if 1
        // set adv
    uint8_t adv_data[] = {
        0x02, LE_GAP_TYPE_FLAGS, 0x02,
        0x02, LE_GAP_TYPE_TX_POWER, 0x02,
        0x03, LE_GAP_TYPE_COMPLETE_SERVICE_LIST, LE_ATT_UUID_HID & 0xff, LE_ATT_UUID_HID >> 8,
        0x03, LE_GAP_TYPE_APPEARANCE, LE_HID_APPEARANCE & 0xff, LE_HID_APPEARANCE >> 8, // Appearance: HID Device
    };
    // set response
    uint8_t scan_response_data[] = {
        0x0b, LE_GAP_TYPE_COMPLETE_NAME, 'Q', 'U', 'E', 'C', ' ', 'S', 'L', 'A', 'V', 'E', // Complete Local Name
    };

#else
    unsigned char flags = 0;
    unsigned char txpower = 0;
    unsigned char service_uuid = 0;
    unsigned short uuid = 0;
    unsigned short appearance = 0;
    int adv_size = 0;
    int total = 0;
    unsigned char adv_data[31];
    unsigned char scan_response_data[31];
    int scan_response_size = 0;
    char name[31];
    memset(name, 0, 31);
    memset(scan_response_data, 0, 31);
    memset(adv_data, 0, 31);
    snprintf(name, 31, LE_NAME);


    flags = (1 << 1);
    adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_FLAGS,
                                        &adv_data[total], 31 - total,
                                        &flags, sizeof(flags));
    total += adv_size;

    txpower = 2; // 2dbm
    adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_TX_POWER,
                                        &adv_data[total], 31 - total,
                                        &txpower, sizeof(txpower));
    total += adv_size;

    uuid = LE_ATT_UUID_HID;
    adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_COMPLETE_SERVICE_LIST,
                                        &adv_data[total], 31 - total,
                                        (unsigned char *)&uuid, sizeof(uuid));
    total += adv_size;

    appearance = LE_HID_APPEARANCE;
    adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_APPEARANCE,
                                        &adv_data[total], 31 - total,
                                        (unsigned char *)&appearance, sizeof(appearance));
    total += adv_size;

    scan_response_size += ql_bt_le_create_adv_data(LE_GAP_TYPE_COMPLETE_NAME,
                                                   &scan_response_data[0], 31 - 0,
                                                   (unsigned char *)name, strlen(name));
#endif

    memset(&ql_params, 0, sizeof(struct ql_bt_adv_params));
    ql_params.adv_mode = BT_LE_ADV_LEGACY_MODE;
    ql_params.adv_data = adv_data;
    ql_params.adv_data_len = sizeof(adv_data);
    ql_params.scan_response_data = scan_response_data;
    ql_params.scan_response_data_len = sizeof(scan_response_data);
    ql_params.adv = &adv_params;
    ql_params.ext_adv = NULL; //

    ql_ble_set_adv_cfg_enable(&ql_params, 1);
}

static void ql_get_bt_start_result(int status, void *event_data)
{
    if (status == 0)
    {
        ql_app_log("OPEN BT sucess can do next step\r\n");
        struct bt_addr addr = {0};
        ql_bt_get_bt_address(&addr);
        ql_app_log("[QEUC_BT]get local bluetooth address: %02x%02x%02x%02x%02x%02x\r\n",
                   addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], addr.bytes[4], addr.bytes[5]);
        ql_bt_register_event_handler(QL_BTTASK_IND_TYPE_LE, ql_bt_usr_handle_ble);
        ql_bt_le_set_pair_enable(1);
        for (int i = 0; i < 16; i++)
        {
            ql_le_remove_pair_info(i);
        }
        ql_bt_le_service_process();

#ifdef BLE_DEMO_SLAVE
        ble_slave_demo_register();
#endif

#ifdef BLE_DEMO_MASTER
        ql_bt_le_scan(0x01,   // LE_ACTIVE_SCAN
                      0x1F40, // interval - 5000ms
                      0x3E8,  // window - 1000ms
                      0x01,   //LE_ADDRESS_TYPE_RANDOM
                      master_name_scan_event_handle);
#endif
    }
     else if(status == 1) {
        ql_app_log("OPEN BT failed please check SW config\r\n");
    } 
    else if(status == 2){
        ql_app_log("BT  BTFIRMWARE_ASSERT\r\n");
    }
}

QuecOSStatus ql_bt_task_init(void)
{
    QuecOSStatus err = kNoErr;

    ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART | QL_LOG_PORT_USB);
    ql_app_log("ql_bt_task_init start\r\n");
    ql_rtos_task_sleep_s(10);

    ql_bt_open_gpio_set();
    err = ql_bt_device_open(&cfg, ql_get_bt_start_result);
    if (err != kNoErr)
    {
        ql_app_log("OPEN BT failed please check HW config\r\n");
    }

#ifdef BLE_DEMO_MASTER
    for (int i = 0; i < sizeof(ble_demo_master_test_buf); i++)
    {
        ble_demo_master_test_buf[i] = i;
    }
#endif

    while (1)
    {
#ifdef BLE_DEMO_MASTER
        if (user_ble_master_scan_flag == 1)
        {
            if (user_att_name_handle != BLE_DEMO_HCI_CON_HANDLE_INVALID)
            {
                ql_le_custom_read_request(user_ble_master_handle, user_att_name_handle);
                ql_rtos_task_sleep_s(5);
                ble_demo_master_test_buf[0]++;
                ql_le_custom_write_request(user_ble_master_handle, user_att_name_handle, (uint8 *)ble_demo_master_test_buf, sizeof(ble_demo_master_test_buf));
                ql_rtos_task_sleep_ms(10);
            }
        }
#endif

#ifdef BLE_DEMO_SLAVE
        if (user_ble_slave_notify_flag == 1)
        {
            ble_demo_loop_test_buf[0]++;
            ql_appbt_le_notify(user_ble_slave_handle, ble_demo_loop_test_handle, (uint8_t *)ble_demo_loop_test_buf, sizeof(ble_demo_loop_test_buf));
            ql_rtos_task_sleep_s(5);
        }
#endif
        ql_rtos_task_sleep_s(5);
    }
}
//application_init(ql_bt_task_init, "ql_bt_task_init", 4, 0);
