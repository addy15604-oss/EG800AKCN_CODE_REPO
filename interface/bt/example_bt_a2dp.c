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
struct bt_addr test_addr = {{0XF4, 0X4E, 0XFD, 0X03, 0X86, 0X03}};
//struct bt_addr test_addr = {{0X40, 0Xdc, 0Xa5, 0X10, 0X1a, 0X67}};
#define FILE_TEST 1 // 1 -MP3 2-WAV 3-AMR
//#define STREM_TEST 1 // 1 -MP3 2-AMR

#define OC_MODE 1 //1=ec200m 0=800m
#if OC_MODE
#define QL_BT_USE_UART 3 //3--uart4
#define BT_REMOTE_HID_HOST 1
#define QL_GPIO_BT_POWER 77
#define QL_GPIO_BT_RESET 126
#define QL_GPIO_BT_LDO_EN 37
#define QL_GPIO_BT_WAKEUP_HOST 117
#define QL_GPIO_HOST_WAKEUP_BT 118
#else
#define QL_BT_USE_UART 1 //
#define BT_REMOTE_HID_HOST 1
//#define QL_GPIO_BT_POWER   77
#define QL_GPIO_BT_RESET 25
#define QL_GPIO_BT_LDO_EN 26
#define QL_GPIO_BT_WAKEUP_HOST 22
#define QL_GPIO_HOST_WAKEUP_BT 21

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
        .inquiry_scan_interval = 0x1000, // 2560ms
        .inquiry_scan_window = 0x12,     // 11.25ms
        .page_scan_interval = 0x800,     // range:0x12-0x1000
        .page_scan_window = 0x12,        // range:0x11-0x1000
        .io_capability = 3,
        .is_absolute_volume_enable = 1,
        .sniff_min_interval = 400,
        .sniff_max_interval = 800,
        .sniff_attempt = 4,
        .hid_att = NULL,
        .sniff_timeout = 1,
        .comm_cfg = BT_SSP_ENABLE,

    },
    .bt_chip_id = 5801,
    .xip = 0,
};
static ql_task_t p_quec_bt_a2dp_test_task = NULL;
static ql_queue_t quec_bt_a2dp_test_MsgQ = NULL;
static ql_flag_t a2dp_test_strem_flag = NULL;
static int appbt_a2dp_get_buffer_size(int rate, int chan, int *min, int *max)
{
    if (rate > 48000 || chan > 2 || !min || !max)
    {
        ql_app_log("rate: %d, chan: %d, min: %p, max: %p\r\n",
                   rate, chan, min, max);
        return -1;
    }

    *min = 40 * rate / 1000 * chan * 2; // 40ms data in bytes
    *max = 80 * rate / 1000 * chan * 2; // 80ms data in bytes
    return 0;
}
static void a2dp_test_send_continue(void)
{
    int min_buf_size = 0, max_buf_size = 0, error_code = -1;
#ifdef STREM_TEST
    ql_rtos_flag_release(a2dp_test_strem_flag, 0x2, QL_FLAG_OR_CLEAR);
#endif

    appbt_a2dp_get_buffer_size(44100, 2, &min_buf_size, &max_buf_size);
    unsigned char *buf = (unsigned char *)malloc(min_buf_size);
    unsigned int buf_size = min_buf_size;

    error_code = ql_audio_track_pipeline_read(buf, &buf_size);
    if (error_code == 0)
    {
        ql_bt_a2dp_send_pcm_data(buf, buf_size);
    }
    
    free(buf);
}
#if STREM_TEST == 1
static Mp3PlayConfigInfo mp3_configInfo;
static Mp3PlaybackHandle mp3_handle;
static unsigned char is_mp3_playing;
static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if (event == MP3_PLAYBACK_EVENT_STATUS)
    {
        if (val == MP3_PLAYBACK_STATUS_STARTED)
        {
            is_mp3_playing = 1;
            printf("mp3 play start\n");
        }
        if (val == MP3_PLAYBACK_STATUS_ENDED)
        {
            printf("mp3 play end\n");
            ql_bt_a2dp_send_suspend();
            is_mp3_playing = 0;
        }
    }
}
#endif
#if STREM_TEST == 2
static AmrPlaybackHandle amr_handle;
static AmrPlaybackConfigInfo amr_configInfo;
static unsigned char is_amr_playing;
static void ql_AmrPlaybackEvent(AmrPlaybackEventType event, int val)
{
    if (event == AMR_PLAYBACK_EVENT_STATUS)
    {
        if (val == AMR_FILE_STATUS_STARTED)
        {
            printf("amr play start\r\n");
            is_amr_playing = 1;
        }
        if (val == AMR_FILE_STATUS_ENDED)
        {
            printf("amr play end\r\n");
            ql_bt_a2dp_send_suspend();
            is_amr_playing = 0;
        }
    }
}
#endif
#if FILE_TEST == 1
static Mp3PlayConfigInfo mp3_configInfo;
static Mp3PlaybackHandle mp3_handle;
static unsigned char is_mp3_playing;
static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if (event == MP3_PLAYBACK_EVENT_STATUS)
    {
        if (val == MP3_PLAYBACK_STATUS_STARTED)
        {
            is_mp3_playing = 1;
            printf("mp3 play start\n");
        }
        if (val == MP3_PLAYBACK_STATUS_ENDED)
        {
            printf("mp3 play end\n");
            ql_bt_a2dp_send_suspend();
            is_mp3_playing = 0;
        }
    }
}
static int user_mp3_file_play(char *file_name)
{
    if (!file_name)
    {
        printf("mp3Start_nameNULL\n");
        return -1;
    }

    if (is_mp3_playing == 1)
    {
        printf("error, mp3 is in playing\n");
        return -2;
    }
    mp3_configInfo.listener = ql_Mp3PlaybackEvent;
    mp3_configInfo.option = 0x3;
    ql_play_mp3(file_name, &mp3_handle, &mp3_configInfo);
}
#endif // DEBUG

#if FILE_TEST == 2
static struct audio_track_config wav_config;
static audio_track_handle wav_handle;
static unsigned char is_wav_file_playing;
static void wav_play_callback(acm_audio_track_handle handle, acm_audio_track_event_t event)
{
    if (handle == wav_handle)
    {
        if (event == AUDIO_TRACK_EVENT_STARTED)
        {
            is_wav_file_playing = 1;
            printf("wav play start\r\n");
        }
        if (event == AUDIO_TRACK_EVENT_CLOSED)
        {
            is_wav_file_playing = 0;
            ql_bt_a2dp_send_suspend();
            printf("wav play end\r\n");
        }
    }
}
static int user_wav_file_play(char *file_name)
{
    if (!file_name)
    {

        printf("%s:file_name NULL\r\n", __func__);
        return -1;
    }
    if (is_wav_file_playing == 1)
    {
        printf("%s:wav file is in playing\r\n", __func__);
        return -2;
    }
    wav_config.event_cb = wav_play_callback;
    wav_config.option = 0x3;
    return ql_wav_play(file_name, &wav_config, &wav_handle);
}
#endif // 0

#if FILE_TEST == 3
static AmrPlaybackHandle amr_handle;
static AmrPlaybackConfigInfo amr_configInfo;
static unsigned char is_amr_playing;
static void ql_AmrPlaybackEvent(AmrPlaybackEventType event, int val)
{
    if (event == AMR_PLAYBACK_EVENT_STATUS)
    {
        if (val == AMR_FILE_STATUS_STARTED)
        {

            printf("amr play start\r\n");
            is_amr_playing = 1;
        }
        if (val == AMR_FILE_STATUS_ENDED)
        {
            printf("amr play end\r\n");
            ql_bt_a2dp_send_suspend();
            is_amr_playing = 0;
        }
    }
}
static int user_amr_file_play(char *file_name)
{
    if (!file_name)
    {
        printf("%s:file_name NULL\r\n", __func__);
        return -1;
    }
    if (is_amr_playing == 1)
    {
        printf("%s:wav file is in playing\r\n", __func__);
        return -2;
    }
    amr_configInfo.listener = ql_AmrPlaybackEvent;
    amr_configInfo.option = 0x3;
    return ql_play_amr(file_name, &amr_handle, &amr_configInfo);
}
#endif

static void ql_bt_a2dp_pcm_process(void *ptr)
{
    QuecOSStatus err = kNoErr;
    unsigned char play_test_msg = 0;
    QFILE *fp;
    int user_file_size;
    unsigned char *buffer = malloc(4096);
    int size = 0;
    int ret = 0, event;
    while (1)
    {
        err = ql_rtos_queue_wait(quec_bt_a2dp_test_MsgQ, (void *)&play_test_msg, 1, QL_WAIT_FOREVER);
        if (kNoErr != err)
        {
            ql_app_log("[QEUC_BT]quec_bt_a2dp_test_MsgQ wait failed");
            continue;
        }

        ql_app_log("[QEUC_BT][A2DP-PCM]recv msg ID %d\r\n", play_test_msg);

        switch (play_test_msg)
        {
        case 1:
        {

            struct appbt_a2dp a2dp_info = {44100, 2, a2dp_test_send_continue};

            ql_bt_a2dp_send_start(&a2dp_info);
            ql_rtos_task_sleep_ms(100);
#if FILE_TEST == 1
            user_mp3_file_play("U:/testmp3.mp3");
#elif FILE_TEST == 2
            user_wav_file_play("U:/testwav.wav");
#elif FILE_TEST == 3
            user_amr_file_play("U:/testamr.amr");
#endif

#if STREM_TEST == 1
            fp = ql_fopen("U:/testmp3.mp3", "rb");
            user_file_size = ql_fsize(fp);
            if (fp == NULL)
            {
                printf("[FS]ERROR!!! *** file open fail***\r\n");
            }
            printf("[FS]user_file_size %d\r\n",user_file_size);
            mp3_configInfo.listener = ql_Mp3PlaybackEvent;
            mp3_configInfo.option = 0x3;
            ret = ql_play_mp3(NULL, &mp3_handle, &mp3_configInfo);
            printf("ql_play_mp3= %d\r\n", ret);
#elif STREM_TEST == 2
            fp = ql_fopen("U:/testamr.amr", "rb");
            user_file_size = ql_fsize(fp);
            if (fp == NULL)
            {
                printf("[FS]ERROR!!! *** file open fail***\r\n");
            }
            amr_configInfo.listener = ql_AmrPlaybackEvent;
            amr_configInfo.option = 0x3;

            ret = ql_play_amr(NULL, &amr_handle, &amr_configInfo);
            printf("ql_play_amr= %d\r\n", ret);
#endif
#ifdef STREM_TEST

            while (1)
            {
                ql_rtos_flag_wait(a2dp_test_strem_flag, 0x2, QL_FLAG_OR_CLEAR, &event, 0xffffffff);
                size = ql_fread(buffer, 4096, 1, fp);
                printf("size=%d!!\n", size);
                if (size <= 0)
                {
#if STREM_TEST == 1
                    ql_stop_mp3_play(mp3_handle, 1);
#elif STREM_TEST == 2
                    ql_stop_amr_play(amr_handle, 1);
#endif
                    ql_fclose(fp);
                    break;
                }
#if STREM_TEST == 1
                ql_play_mp3_stream_buffer(mp3_handle, buffer, size);
#elif STREM_TEST == 2
                ql_play_amr_stream_buffer(amr_handle, buffer, size);
#endif

                ql_rtos_task_sleep_ms(20);
            }
#endif
            break;
        }
        case 2:
        {

            ql_bt_a2dp_send_suspend();
        }
        default:
            break;
        }
    }
}

static void ql_bt_usr_handle_common(int status, void *event_data)
{
    char buffer[256] = {0};
    switch (status)
    {

    case BTTASK_IND_PAIRING_REQUEST:
    {
        struct bt_event_pairing_request *request =
            (struct bt_event_pairing_request *)event_data;
        ql_app_log("[QEUC_BT]handle pairing request %02x%02x%02x%02x%02x%02x. "
                   "cod %08x. IO capability %d. Numeric %d\r\n",
                   request->addr.bytes[0], request->addr.bytes[1],
                   request->addr.bytes[2], request->addr.bytes[3],
                   request->addr.bytes[4], request->addr.bytes[5],
                   request->cod, request->io_capability,
                   request->numeric_value);
        // user can reject or accept pairing request here
        ql_rtos_task_sleep_ms(1000); // avoid to fast
        ql_bt_accept_bonding(request->addr, 1);
        break;
    }

    case BTTASK_IND_PAIRED:
    {

        struct bt_event_paired *paired =
            (struct bt_event_paired *)event_data;
        ql_app_log("BTTASK_IND_PAIRED");
        break;
    }

    case BTTASK_IND_PIN_REQUEST:
    {
        struct bt_event_pin_request *temp =
            (struct bt_event_pin_request *)event_data;
        // user add pin code repley here
        char *test_pin_code = "0000";
        ql_app_log("handle PIN code request. from device %s. cod %08x\r\n",
                   temp->name, temp->cod);

        ql_rtos_task_sleep_ms(200); // avoid to fast
        ql_bt_pin_reply(temp->addr, test_pin_code);
        //appbt_pin_negative_reply(temp->addr);

        break;
    }
    case BTTASK_IND_NAME:
    {
        struct bt_event_name_indication *name =
            (struct bt_event_name_indication *)event_data;

        ql_app_log("handle name indication len:%d: %s, %02x:%02x:%02x:%02x:%02x:%02x",
                   name->name_length,
                   name->name,
                   name->addr.bytes[0], name->addr.bytes[1], name->addr.bytes[2],
                   name->addr.bytes[3], name->addr.bytes[4], name->addr.bytes[5]);
        break;
    }

    default:
      //  ql_app_log("[QEUC_BT]handle unknow COMMON event %d", status);
        break;
    }
}

static void ql_bt_usr_a2dp_handle(int status, void *event_data)
{

    ql_app_log("a2dp event:%d", status);
    switch (status)
    {
    case BTTASK_IND_A2DP_CONNECTED:
    {
        struct bt_event_a2dp_connect *connect =
            (struct bt_event_a2dp_connect *)event_data;

        ql_app_log("handle A2DP connect msg: CID %d, "
                   "address %02x%02x%02x%02x%02x%02x\r\n",
                   connect->cid,
                   connect->addr[0], connect->addr[1], connect->addr[2],
                   connect->addr[3], connect->addr[4], connect->addr[5]);

        break;
    }
    case BTTASK_IND_A2DP_DISCONNECTED:
    {
        struct bt_event_a2dp_disconnect *disconnect =
            (struct bt_event_a2dp_disconnect *)event_data;
        ql_app_log("handle A2DP disconnect msg: CID %d\r\n",
                   disconnect->cid);
        break;
    }
    case BTTASK_IND_A2DP_START:
        ql_app_log("BTTASK_IND_A2DP_START");
        break;
    case BTTASK_IND_A2DP_STOP:
        ql_app_log("BTTASK_IND_A2DP_STOP");
        break;
    case BTTASK_IND_A2DP_SUSPEND:
        ql_app_log("BTTASK_IND_A2DP_SUSPEND");
        break;
    case BTTASK_IND_A2DP_REJECTED:

        ql_app_log("BTTASK_IND_A2DP_REJECTED");
        break;
    case BTTASK_IND_A2DP_MEDIA_MTU:
    {
        struct bt_event_a2dp_media_mtu *temp =
            (struct bt_event_a2dp_media_mtu *)event_data;
        unsigned int mtu = temp->mtu;

        ql_app_log("handle media connected out mtu %d\r\n", mtu);

        break;
    }
    case BTTASK_IND_A2DP_MEIDA_CONNECTED:
        ql_app_log("a2dp media connected");
        break;

    case BTTASK_IND_A2DP_CONNECT_FAIL:
        ql_app_log("a2dp connected fail");
        break;

    case BTTASK_IND_A2DP_MEDIA_DISCONNECTED:
        ql_app_log("a2dp media disconnected");
        break;
    default:
        //ql_app_log("a2dp event not parsed:%d", msg->event_id);
        break;
    }
}
static void ql_bt_usr_handle_avrcp(int status, void *event_data)
{

    ql_app_log("avrcp event:%d", status);
    switch (status)
    {
    case BTTASK_IND_AVRCP_CONNECTED:
    {
        struct bt_event_avrcp_connect *connect =
            (struct bt_event_avrcp_connect *)event_data;

        ql_app_log("handle AVRCP connect msg: %02x%02x%02x%02x%02x%02x\r\n",
                   connect->addr[0], connect->addr[1], connect->addr[2],
                   connect->addr[3], connect->addr[4], connect->addr[5]);
        break;
    }

    case BTTASK_IND_AVRCP_DISCONNECTED:
    {
        ql_app_log("handle AVRCP disconnect msg\r\n");

        break;
    }

    case BTTASK_IND_AVRCP_KEY_PRESSED:
    {

        unsigned char *id = (unsigned char *)event_data;
        ql_app_log("handle AVRCP key pressed %02x\r\n", *id);

        break;
    }

    case BTTASK_IND_AVRCP_KEY_RELEASED:
    {
        unsigned char *id = (unsigned char *)event_data;
        unsigned char play_test_msg = 0;

        ql_app_log("[QEUC_BT]handle AVRCP key released %02x\r\n", *id);
        switch (*id)
        {
        // user handle key here
        case AVRCP_KEY_PLAY:
        {
            // start media data send
            play_test_msg = 1;
            ql_bt_a2dp_set_volume(120);
            ql_rtos_queue_release(quec_bt_a2dp_test_MsgQ, 1, &play_test_msg, 10);
            break;
        }

        case AVRCP_KEY_PAUSE:
        {
            // stop
            play_test_msg = 2;
            ql_rtos_queue_release(quec_bt_a2dp_test_MsgQ, 1, &play_test_msg, 10);
            break;
        }
        case AVRCP_KEY_STOP:
        {
            // stop
            play_test_msg = 0;
            ql_rtos_queue_release(quec_bt_a2dp_test_MsgQ, 1, &play_test_msg, 10);
            break;
        }

        case AVRCP_KEY_FORWARD:
            // start media data send
            play_test_msg = 3;
            ql_rtos_queue_release(quec_bt_a2dp_test_MsgQ, 1, &play_test_msg, 10);
            break;

        case AVRCP_KEY_BACKWARD:
            break;

        default:
            break;
        }
        break;
    }
    case BTTASK_IND_AVRCP_VOLUME_CHANGED:
    {
        uint8_t *volume = (uint8_t *)event_data;
        ql_app_log("handle avrcp volume changed %d", *volume);

        break;
    }
    case BTTASK_IND_AVRCP_VOLUME_CHANGED_SUPPORT:
    {
        unsigned char *enable = (unsigned char *)event_data;
        ql_app_log("handle avrcp support volume changed %d", *enable);

        break;
    }
    default:
        ql_app_log("[USER]handle unknow AVRCP event %d", status);
        break;
    }
}
static QuecOSStatus ql_bt_a2dp_test_init(void)
{
    QuecOSStatus err = kNoErr;

    if (NULL != p_quec_bt_a2dp_test_task)
    {
        ql_app_log("[QEUC_BT]p_quec_bt_a2dp_test_task is not NULL");
        return kGeneralErr;
    }
    err = ql_rtos_queue_create(&quec_bt_a2dp_test_MsgQ, 1, 6);
    if (kNoErr != err)
    {
        ql_app_log("[QEUC_BT]quec_bt_a2dp_test_MsgQ create failed");
        return err;
    }
#ifdef STREM_TEST

    ql_rtos_flag_create(&a2dp_test_strem_flag);
#endif
    /* create bt task */
    err = ql_rtos_task_create(&p_quec_bt_a2dp_test_task, 8192, 100, "quec_a2dp_test_task", ql_bt_a2dp_pcm_process, NULL);
    if (kNoErr != err)
    {
        p_quec_bt_a2dp_test_task = NULL;
        ql_app_log("[QEUC_BT]p_quec_bt_a2dp_test_task init failed");
        return err;
    }

    ql_app_log("[QEUC_BT]p_quec_bt_a2dp_test_task init done");

    return err;
}
static void ql_get_bt_scan_result(struct bt_event_inquiry *inquiry, int result)
{
    if (result == 0)
    {
        ql_app_log("inquiry complete\r\n");
    }
    else
    {
        if (inquiry->name[0] > 0x7f)
        {
            int i = 0;
            ql_app_log("[QEUC_BT]handle inquiry result(chinese name): %02x%02x%02x%02x%02x%02x\r\n",
                       inquiry->addr[0], inquiry->addr[1], inquiry->addr[2],
                       inquiry->addr[3], inquiry->addr[4], inquiry->addr[5]);
            ql_app_log("\r\n");
        }
        else
        {
            ql_app_log("[QEUC_BT]handle inquiry result: %s, %02x%02x%02x%02x%02x%02x\r\n",
                       inquiry->name,
                       inquiry->addr[0], inquiry->addr[1], inquiry->addr[2],
                       inquiry->addr[3], inquiry->addr[4], inquiry->addr[5]);
        }

        //find test device address
        if ((inquiry->addr[0] == 0xCD && inquiry->addr[5] == 0xAA) || (inquiry->addr[0] == 0x67 && inquiry->addr[5] == 0x40) || (inquiry->addr[0] == 0xFc && inquiry->addr[5] == 0xBA) || (inquiry->addr[0] == 0x44 && inquiry->addr[5] == 0xE2) || (inquiry->addr[0] == 0xAA && inquiry->addr[5] == 0xCD) || (inquiry->addr[0] == 0x40 && inquiry->addr[5] == 0x67) || (inquiry->addr[0] == 0xBA && inquiry->addr[5] == 0xFC) || (inquiry->addr[0] == test_addr.bytes[0] && inquiry->addr[5] == test_addr.bytes[5]))
        {
            struct bt_addr bt_addr_get;

            ql_bt_inquiry_cancel();

            memcpy(&bt_addr_get, inquiry->addr, sizeof(struct bt_addr));
            ql_bt_connect_headset(bt_addr_get, 0);
        }
    }
}

static void ql_get_bt_start_result(int status, void *event_data)
{
    if (status == 0)
    {
        struct bt_device_record *device_record = NULL;
        uint8 find_record = 0;
        ql_app_log("OPEN BT sucess can do next step\r\n");
        struct bt_addr addr = {0};
        ql_bt_get_bt_address(&addr);
        ql_app_log("[QEUC_BT]get local bluetooth address: %02x%02x%02x%02x%02x%02x\r\n",
                   addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], addr.bytes[4], addr.bytes[5]);
        ql_bt_register_event_handler(QL_BTTASK_IND_TYPE_COMMON, ql_bt_usr_handle_common);
        ql_bt_register_event_handler(QL_BTTASK_IND_TYPE_A2DP,   ql_bt_usr_a2dp_handle);
        ql_bt_register_event_handler(QL_BTTASK_IND_TYPE_AVRCP,  ql_bt_usr_handle_avrcp);
        ql_bt_a2dp_test_init();
        ql_bt_get_device_records(&device_record);
        for (uint8 i = 0; i < 10; i++)
        {
            ql_app_log("handle record: %02x:%02x:%02x:%02x:%02x:%02x (%s)\r\n",
                       device_record[i].addr[0], device_record[i].addr[1],
                       device_record[i].addr[2], device_record[i].addr[3],
                       device_record[i].addr[4], device_record[i].addr[5],
                       device_record[i].name);
            if ((device_record[i].addr[0] == test_addr.bytes[0]) && (device_record[i].addr[5] == test_addr.bytes[5]))
            {
                find_record = 1;
                ql_app_log(" Please initiate the connection by turning on the headphones.");

                break;
            }
        }

		#if 1
        if (find_record != 1)
        {
            ql_app_log(" No headphone records found, searching is required.");
            ql_bt_inquiry_ex(0x30, 10, ql_get_bt_scan_result);
        }
		#else
		ql_bt_inquiry_ex(0x30, 10, ql_get_bt_scan_result);
		#endif
    }
    else if(status == 1) {
        ql_app_log("OPEN BT failed please check SW config\r\n");
    } 
    else if(status == 2){
     ql_app_log("BT  BTFIRMWARE_ASSERT\r\n");
    }

}
static QuecOSStatus ql_bt_a2dp_init(void)
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
}
//application_init(ql_bt_a2dp_init, "ql_bt_a2dp_init", 4, 0);

