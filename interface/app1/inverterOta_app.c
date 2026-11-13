/*
 * inverterOta.c  --  EG800AK port
 *
 * HTTP layer converted from the old ql_httpc_* (event + write-callback) API to the
 * EG800AK ql_http_client_* API (ql_http_client_init / setopt / request, one
 * response callback with events), as in the Quectel http_client example.
 *
 * Modbus RTU OTA protocol (START -> data -> END -> ACTIVATE) is unchanged.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "ql_rtos.h"
#include "ql_data_call.h"
#include "ql_http_client.h"
#include "ql_application.h"
#include "ql_log.h"
// #include "info.h"        // GPRSData.IMEI
#include "main_app.h"
#include "FileSystem.h"
#include "ql_power.h"
#include "sockets.h"
#include "ql_osi_def.h"

// ==================== LOGGING ====================
#define QL_OTA_LOG(fmt, ...)    printf("[QL_OTA] " fmt "\r\n", ##__VA_ARGS__)

// ==================== PORT LAYER (implement in inverter_app.c / board code) ====================
// These wrap the EG800AK UART + GPIO calls you already use in inverter_app.c.
// Left as extern on purpose: the link fails loudly if one is missing.
//
//  ota_uart_open(baud)  : (re)open the RS485 UART, 8N1, no flow control, and register an RX
//                         callback that calls ota_uart_rx_feed(buf, len) for every chunk read.
//                         Return 0 on success.
//  ota_uart_close()     : close it.
//  ota_uart_write(d,n)  : blocking write, returns bytes written (<0 on error).
//  ota_rede(level)      : drive RS485 DE/RE pin (1 = transmit, 0 = receive).
extern int  ota_uart_open(uint32_t baud);
extern void ota_uart_close(void);
extern int  ota_uart_write(const uint8_t *data, uint32_t len);
extern void ota_rede(int level);

// ==================== SERVER CONFIGURATION ====================
char HTTP_OTA_FILE_URI[200]     = "http://eastmanone.com:3001/download/";
char HTTP_OTA_FILESIZE_URI[200] = "http://eastmanone.com:3001/file-size?filename=";
char INV_OTA_URL_SUFFIX[100]    = "firmware_Z1.0.95.bin";
char HTTP_OTA_STATUS_URI[300]   = "http://eastmanone.com:3001/ota-status?";

bool OTA_START = false;

#define PROFILE_IDX             1       // same profile mqtt_app.c uses
#define OTA_APN                 "airtelgprs.com"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// ==================== FIRMWARE TYPE (server status reporting only) ====================
#define FW_TYPE_MASTER_DSP   0x0002
uint16_t FW_TYPE = FW_TYPE_MASTER_DSP;

// ==================== Modbus RTU OTA v2.1 ====================
// 0 = comm card / dongle itself, 1 = attached control board (DSP)
uint8_t OTA_MODBUS_TARGET = 0;

#define MODBUS_SLAVE_ADDR       0x01

#define REG_OTA_CTRL          0x1000
#define REG_OTA_STATUS        0x1001
#define REG_OTA_ERROR         0x1002
#define REG_OTA_TOTAL_SIZE    0x1003
#define REG_OTA_CRC32         0x1005
#define REG_OTA_TARGET        0x1007
#define REG_OTA_RECV_OFFSET   0x1008
#define REG_OTA_SESSION_SEQ   0x100A
#define REG_OTA_FW_INFO       0x100B
#define REG_OTA_DATA          0x1010

#define REG_SYS_RUN_FLAG      0x0100
#define REG_RUN_STATE         0x0020
#define REG_PROTOCOL_VERSION  0x0099
#define REG_COMCARD_VERSION   0x0090
#define REG_DSP_VERSION       0x0092
#define REG_RUN_TIME          0x0094

#define OTA_CTRL_ABORT        2
#define OTA_CTRL_END          3
#define OTA_CTRL_ACTIVATE     4

#define OTA_STATUS_IDLE        0
#define OTA_STATUS_RECEIVING   1
#define OTA_STATUS_VERIFY_OK   2
#define OTA_STATUS_FLASHING    3
#define OTA_STATUS_DONE        4
#define OTA_STATUS_ERROR       5
#define OTA_STATUS_BUSY        6

#define OTA_MAX_BLOCK           176
#define OTA_START_BLK           176
#define OTA_DATA_BLOCK_SIZE_DEFAULT 60
#define OTA_START_TIMEOUT_MS    120000
#define OTA_START_RETRIES       2
#define OTA_CTRL_TIMEOUT_MS     1000
#define OTA_CTRL_RETRIES        3
#define OTA_READ_TIMEOUT_MS     200
#define OTA_READ_RETRIES        3
#define OTA_BLOCK_TIMEOUT_MS    500
#define OTA_BLOCK_MAX_RETRY     6
#define OTA_STUCK_LIMIT         5

// ==================== BAUD AUTO-DETECT ====================
#define BAUD_PRIMARY            115200
#define BAUD_SECONDARY          460800
#define OTA_DETECT_ROUNDS           3
#define OTA_DETECT_TRIES_PER_BAUD   3
#define OTA_DETECT_WAIT_MS          300

// Optional: set from the inverter task with the baud it already locked on (0 = unknown)
uint32_t OTA_Hint_Baud = 0;

static uint32_t g_ota_baud        = BAUD_PRIMARY;
static bool     g_ota_baud_locked = false;

#define RS485_TX_SETTLE_MS      3
#define MB_INTER_FRAME_GAP_MS   5
#define MAX_FIRMWARE_SIZE       (1572864)   // 1.5MB

// ==================== OTA CONTEXT ====================
typedef struct {
    uint8_t *firmware_buffer;
    uint32_t firmware_size;
    uint32_t total_bytes_received;
    bool     transfer_active;
    uint64_t last_status_report_time;
    bool     status_report_enabled;
} qota_ctx_t;

ql_task_t ota_task = NULL;
qota_ctx_t ota_ctx = {0};

/* Modbus RTU RX accumulation */
#define MB_RX_BUF_SIZE 512
static uint8_t           mb_rx_buf[MB_RX_BUF_SIZE];
static volatile uint16_t mb_rx_len = 0;
static ql_sem_t          mb_rx_sem = NULL;

static uint16_t g_ota_data_block_size = OTA_DATA_BLOCK_SIZE_DEFAULT;
static uint16_t g_ota_start_blk       = OTA_START_BLK;

// ==================== REBOOT HELPER ====================
void bk_reboot(void)
{
    ql_rtos_task_sleep_ms(3000);
    ql_power_reset();
}

// ==================== STUCK WATCHDOG ====================
#define OTA_STUCK_TIMEOUT_MS   (1UL * 60UL * 1000UL)
#define OTA_WATCHDOG_POLL_MS   5000

static volatile uint32_t g_ota_progress_seq = 0;
static ql_task_t         ota_watchdog_task  = NULL;

static void ota_touch_progress(void) { g_ota_progress_seq++; }

static void ota_watchdog_thread(void *arg)
{
    (void)arg;
    uint32_t last_seen_seq = g_ota_progress_seq;
    uint32_t idle_ms = 0;

    QL_OTA_LOG("OTA watchdog started");

    while (OTA_START) {
        ql_rtos_task_sleep_ms(OTA_WATCHDOG_POLL_MS);
        if (!OTA_START) break;

        uint32_t seq_now = g_ota_progress_seq;
        if (seq_now != last_seen_seq) { last_seen_seq = seq_now; idle_ms = 0; }
        else                          { idle_ms += OTA_WATCHDOG_POLL_MS; }

        if (idle_ms >= OTA_STUCK_TIMEOUT_MS) {
            QL_OTA_LOG("=== OTA WATCHDOG: no progress for %lu ms, rebooting ===", (unsigned long)idle_ms);
    ql_power_reset();
        }
    }
    ql_rtos_task_delete(ota_watchdog_task);
}

// ==================== CRC ====================
static uint16_t mb_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= data[pos];
        for (int i = 0; i < 8; i++)
            crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    return crc;
}

static uint32_t crc32_mpeg2(const uint8_t *p, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= ((uint32_t)p[i]) << 24;
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80000000U) ? ((crc << 1) ^ 0x04C11DB7U) : (crc << 1);
    }
    return crc;
}

/* ================================================================
 *  HTTP LAYER  (EG800AK ql_http_client_* API)
 *
 *  One generic GET helper. The response callback copies the body into
 *  a caller buffer. Completion is signalled by a flag + semaphore, so it
 *  works whether ql_http_client_request() blocks until the response is
 *  done or returns early and finishes from another task.
 * ================================================================ */
typedef struct {
    ql_sem_t      sem;
    volatile int  status_code;
    volatile bool body_done;
    volatile bool failed;
    volatile bool done;

    uint8_t      *buf;          // body destination (may be NULL)
    uint32_t      cap;
    uint32_t      len;
    bool          allow_trunc;  // small replies: silently truncate instead of failing
    bool          overflow;
    bool          touch_wd;     // feed the stuck-watchdog while receiving
} http_ctx_t;

static void http_ctx_finish(http_ctx_t *c)
{
    c->done = true;
    if (c->sem) ql_rtos_semaphore_release(c->sem);
}

static int http_response_cb(QL_HTTP_CLIENT_T *client,
                            QL_HTTP_CLIENT_EVENT_E event,
                            int status_code,
                            char *data, int data_len,
                            void *private_data)
{
    http_ctx_t *c = (http_ctx_t *)private_data;
    (void)client;
    if (c == NULL) return 0;

    switch (event) {
        case QL_HTTP_CLIENT_EVENT_SEND_FAIL:
            QL_OTA_LOG("http send failed");
            c->failed = true;
            http_ctx_finish(c);
            break;

        case QL_HTTP_CLIENT_EVENT_SEND_SUCCESSED:
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FAIL:
            QL_OTA_LOG("http response header parse failed");
            c->failed = true;
            http_ctx_finish(c);
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FINISHED:
            c->status_code = status_code;
            QL_OTA_LOG("http header: status=%d header_len=%d", status_code, data_len);
            return 1;   // 1 = keep receiving the body

        case QL_HTTP_CLIENT_EVENT_RECV_BODY:
            if (data != NULL && data_len > 0) {
                if (c->buf != NULL) {
                    uint32_t room = (c->len < c->cap) ? (c->cap - c->len) : 0;
                    uint32_t n    = (uint32_t)data_len;
                    if (n > room) {
                        c->overflow = true;
                        n = room;
                    }
                    if (n > 0) {
                        memcpy(c->buf + c->len, data, n);
                        c->len += n;
                    }
                }
                if (c->touch_wd) ota_touch_progress();
            }
            return 1;

        case QL_HTTP_CLIENT_EVENT_RECV_BODY_FINISHED:
            c->body_done = true;
            http_ctx_finish(c);
            break;

        case QL_HTTP_CLIENT_EVENT_DISCONNECTED:
            // After a normal body-finished this is just the server closing; harmless.
            if (!c->body_done) {
                QL_OTA_LOG("http closed by server before body finished");
                c->failed = true;
            }
            http_ctx_finish(c);
            break;

        default:
            break;
    }
    return 0;
}

/* Returns 0 on HTTP 200 + complete body, -1 otherwise. *out_len = body bytes stored. */
static int http_get(const char *url, uint8_t *buf, uint32_t cap, bool allow_trunc,
                    bool touch_wd, uint32_t timeout_ms, uint32_t *out_len, int *out_status)
{
    http_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.buf         = buf;
    ctx.cap         = cap;
    ctx.allow_trunc = allow_trunc;
    ctx.touch_wd    = touch_wd;

    if (ql_rtos_semaphore_create(&ctx.sem, 0) != 0) {
        QL_OTA_LOG("http: semaphore create failed");
        return -1;
    }

    QL_HTTP_CLIENT_T *client = ql_http_client_init();
    if (client == NULL) {
        QL_OTA_LOG("http: client init failed");
        ql_rtos_semaphore_delete(ctx.sem);
        return -1;
    }

    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PDP_CID,      PROFILE_IDX);
    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PROTOCOL_VER, 1);   // HTTP/1.0: no chunked encoding

    int ret = ql_http_client_request(client, (char *)url,
                                     QL_HTTP_CLIENT_REQUEST_GET,
                                     QL_HTTP_CLIENT_AUTH_TYPE_NONE,
                                     NULL, NULL, NULL, 0,
                                     http_response_cb, &ctx);
    QL_OTA_LOG("http: request ret=%d", ret);

    if (!ctx.done)
        ql_rtos_semaphore_wait(ctx.sem, timeout_ms);

    ql_http_client_release(client);
    ql_rtos_semaphore_delete(ctx.sem);

    if (out_len)    *out_len    = ctx.len;
    if (out_status) *out_status = ctx.status_code;

    bool overflow_bad = ctx.overflow && !ctx.allow_trunc;
    if (ctx.status_code == 200 && ctx.body_done && !ctx.failed && !overflow_bad)
        return 0;

    QL_OTA_LOG("http: failed status=%d body_done=%d failed=%d overflow=%d",
               ctx.status_code, ctx.body_done, ctx.failed, ctx.overflow);
    return -1;
}

// ==================== STATUS REPORTING ====================
static int send_ota_status_to_server(int status_code, const char *error_msg)
{
    char MAC_ADD[20] = {0};
    memcpy(MAC_ADD, GPRSData.IMEI, sizeof(GPRSData.IMEI) - 1);

    if (!ota_ctx.status_report_enabled)
        return -1;

    char     url[600];
    uint32_t percentage = 0;

    if (ota_ctx.firmware_size > 0) {
        percentage = (uint32_t)(((uint64_t)ota_ctx.total_bytes_received * 100) / ota_ctx.firmware_size);
        if (percentage > 100) percentage = 100;
    }

    if (error_msg != NULL && strlen(error_msg) > 0) {
        char  encoded_error[200] = {0};
        char *dest = encoded_error;
        for (int i = 0; error_msg[i] && (dest - encoded_error) < (int)(sizeof(encoded_error) - 3); i++) {
            if (error_msg[i] == ' ') { *dest++ = '%'; *dest++ = '2'; *dest++ = '0'; }
            else                     { *dest++ = error_msg[i]; }
        }
        *dest = '\0';

        snprintf(url, sizeof(url),
            "%smac=%s&status=%d&downloaded=%lu&total=%lu&percentage=%lu&fwtype=%d&error=%s",
            HTTP_OTA_STATUS_URI, MAC_ADD, status_code,
            (unsigned long)ota_ctx.total_bytes_received,
            (unsigned long)ota_ctx.firmware_size,
            (unsigned long)percentage, FW_TYPE, encoded_error);
    } else {
        snprintf(url, sizeof(url),
            "%smac=%s&status=%d&downloaded=%lu&total=%lu&percentage=%lu&fwtype=%d",
            HTTP_OTA_STATUS_URI, MAC_ADD, status_code,
            (unsigned long)ota_ctx.total_bytes_received,
            (unsigned long)ota_ctx.firmware_size,
            (unsigned long)percentage, FW_TYPE);
    }

    QL_OTA_LOG("Sending status update: %s", url);
    ql_rtos_task_sleep_ms(100);

    uint8_t resp[64];
    int ret = http_get(url, resp, sizeof(resp), true, false, 10000, NULL, NULL);
    QL_OTA_LOG("Status update %s", ret == 0 ? "sent" : "failed");
    return ret;
}

static void report_ota_progress(void)
{
    if (!ota_ctx.status_report_enabled) return;

    uint64_t now = (uint64_t)ql_rtos_get_system_tick();
    if ((now - ota_ctx.last_status_report_time) < 5000)    // at most one report / 5 s
        return;

    ota_ctx.last_status_report_time = now;
    send_ota_status_to_server(0, NULL);
}

static void report_ota_error(const char *error_msg)
{
    if (!ota_ctx.status_report_enabled) return;
    QL_OTA_LOG("OTA ERROR: %s", error_msg);
    send_ota_status_to_server(2, error_msg);
    ql_rtos_task_sleep_ms(500);
    send_ota_status_to_server(2, error_msg);
}

static void report_ota_success(void)
{
    if (!ota_ctx.status_report_enabled) return;
    QL_OTA_LOG("OTA SUCCESS");
    send_ota_status_to_server(1, NULL);
    ql_rtos_task_sleep_ms(500);
    send_ota_status_to_server(1, NULL);
}

// ==================== UART RX FEED (call from your UART RX callback) ====================
void ota_uart_rx_feed(const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (mb_rx_len < MB_RX_BUF_SIZE) {
            mb_rx_buf[mb_rx_len++] = data[i];
        } else {
            mb_rx_len = 0;     // overflow: resync
        }
    }
    if (mb_rx_sem)
        ql_rtos_semaphore_release(mb_rx_sem);
}

static int uart_send_buf(const uint8_t *data, uint32_t len)
{
    int ret;
    ota_rede(1);
    ql_rtos_task_sleep_ms(1);
    ret = ota_uart_write(data, len);
    ql_rtos_task_sleep_ms(RS485_TX_SETTLE_MS);
    ota_rede(0);
    return ret;
}

// ==================== MODBUS RTU TRANSPORT ====================
typedef struct {
    uint8_t  addr;
    uint8_t  fc;
    bool     is_exception;
    uint8_t  exc_code;
    uint8_t  data[300];
    uint16_t data_len;
} mb_resp_t;

static int mb_try_parse_frame(const uint8_t *buf, uint16_t len, uint16_t *frame_len_out)
{
    if (len < 2) return 0;

    uint8_t  addr = buf[0];
    uint8_t  fc   = buf[1];
    uint16_t need;

    if (fc & 0x80)                    need = 5;
    else if (fc == 0x03) { if (len < 3) return 0; need = 3 + buf[2] + 2; }
    else if (fc == 0x06 || fc == 0x10) need = 8;
    else return -1;

    if (len < need) return 0;

    uint16_t crc_calc = mb_crc16(buf, need - 2);
    uint16_t crc_recv = (uint16_t)buf[need - 2] | ((uint16_t)buf[need - 1] << 8);

    if (crc_calc != crc_recv) return -1;
    if (addr != MODBUS_SLAVE_ADDR) return -1;

    *frame_len_out = need;
    return 1;
}

static int mb_transact(const uint8_t *pdu, uint16_t pdu_len, uint32_t timeout_ms, mb_resp_t *resp)
{
    uint8_t frame[300];

    if (pdu_len + 3 > sizeof(frame)) {
        QL_OTA_LOG("mb_transact: pdu too large (%u)", pdu_len);
        return -1;
    }

    frame[0] = MODBUS_SLAVE_ADDR;
    memcpy(&frame[1], pdu, pdu_len);
    uint16_t crc = mb_crc16(frame, 1 + pdu_len);
    frame[1 + pdu_len]     = (uint8_t)(crc & 0xFF);
    frame[1 + pdu_len + 1] = (uint8_t)((crc >> 8) & 0xFF);
    uint16_t frame_len = 1 + pdu_len + 2;

    mb_rx_len = 0;
    uart_send_buf(frame, frame_len);
    ql_rtos_task_sleep_ms(MB_INTER_FRAME_GAP_MS);

    uint64_t deadline = (uint64_t)ql_rtos_get_system_tick() + timeout_ms;

    for (;;) {
        int64_t remain = (int64_t)(deadline - (uint64_t)ql_rtos_get_system_tick());
        if (remain <= 0) return -1;

        if (mb_rx_sem)
            ql_rtos_semaphore_wait(mb_rx_sem, (uint32_t)remain);

        for (;;) {
            uint16_t frame_out_len = 0;
            int r = mb_try_parse_frame(mb_rx_buf, mb_rx_len, &frame_out_len);

            if (r == 1) {
                resp->addr = mb_rx_buf[0];
                resp->fc   = mb_rx_buf[1];
                if (resp->fc & 0x80) {
                    resp->is_exception = true;
                    resp->exc_code     = mb_rx_buf[2];
                    resp->data_len     = 0;
                } else {
                    resp->is_exception = false;
                    resp->data_len = frame_out_len - 2 - 2;
                    if (resp->data_len > sizeof(resp->data)) resp->data_len = sizeof(resp->data);
                    memcpy(resp->data, &mb_rx_buf[2], resp->data_len);
                }
                memmove(mb_rx_buf, &mb_rx_buf[frame_out_len], mb_rx_len - frame_out_len);
                mb_rx_len -= frame_out_len;
                return 0;
            } else if (r == -1) {
                if (mb_rx_len > 0) {
                    memmove(mb_rx_buf, &mb_rx_buf[1], mb_rx_len - 1);
                    mb_rx_len--;
                    continue;
                }
                break;
            } else {
                break;
            }
        }
    }
}

static int mb_read_holding(uint16_t addr, uint16_t qty, uint16_t *out, uint32_t timeout_ms, int retries)
{
    uint8_t pdu[5];
    pdu[0] = 0x03;
    pdu[1] = (addr >> 8) & 0xFF; pdu[2] = addr & 0xFF;
    pdu[3] = (qty  >> 8) & 0xFF; pdu[4] = qty  & 0xFF;

    mb_resp_t resp;
    int attempt = 0;
    do {
        int r = mb_transact(pdu, sizeof(pdu), timeout_ms, &resp);
        if (r == 0) {
            ota_touch_progress();
            if (resp.is_exception) return (int)resp.exc_code;
            uint8_t  bc = resp.data[0];
            uint16_t n  = bc / 2;
            if (n > qty) n = qty;
            for (uint16_t i = 0; i < n; i++)
                out[i] = ((uint16_t)resp.data[1 + i * 2] << 8) | resp.data[2 + i * 2];
            return 0;
        }
        attempt++;
    } while (attempt <= retries);

    return -1;
}

static int mb_write_single(uint16_t addr, uint16_t value, uint32_t timeout_ms, int retries)
{
    uint8_t pdu[5];
    pdu[0] = 0x06;
    pdu[1] = (addr  >> 8) & 0xFF; pdu[2] = addr  & 0xFF;
    pdu[3] = (value >> 8) & 0xFF; pdu[4] = value & 0xFF;

    mb_resp_t resp;
    int attempt = 0;
    do {
        int r = mb_transact(pdu, sizeof(pdu), timeout_ms, &resp);
        if (r == 0) {
            ota_touch_progress();
            if (resp.is_exception) return (int)resp.exc_code;
            return 0;
        }
        attempt++;
    } while (attempt <= retries);

    return -1;
}

static int mb_write_multiple(uint16_t addr, const uint8_t *payload, uint16_t len,
                             uint32_t timeout_ms, int retries)
{
    uint8_t pdu[6 + 256];
    if (len % 2 != 0 || len > 256) {
        QL_OTA_LOG("mb_write_multiple: bad len %u", len);
        return -1;
    }
    uint16_t qty = len / 2;

    pdu[0] = 0x10;
    pdu[1] = (addr >> 8) & 0xFF; pdu[2] = addr & 0xFF;
    pdu[3] = (qty  >> 8) & 0xFF; pdu[4] = qty  & 0xFF;
    pdu[5] = (uint8_t)len;
    memcpy(&pdu[6], payload, len);

    mb_resp_t resp;
    int attempt = 0;
    do {
        int r = mb_transact(pdu, 6 + len, timeout_ms, &resp);
        if (r == 0) {
            ota_touch_progress();
            if (resp.is_exception) {
                QL_OTA_LOG("mb_write_multiple: exception 0x%02X", resp.exc_code);
                return (int)resp.exc_code;
            }
            return 0;
        }
        attempt++;
    } while (attempt <= retries);

    return -1;
}

// ==================== BAUD AUTO-DETECT ====================
static int ota_detect_baud(void)
{
    uint32_t list[2] = { BAUD_PRIMARY, BAUD_SECONDARY };

    if (OTA_Hint_Baud == BAUD_SECONDARY) {
        list[0] = BAUD_SECONDARY;
        list[1] = BAUD_PRIMARY;
    }

    g_ota_baud_locked = false;

    for (int round = 0; round < OTA_DETECT_ROUNDS && !g_ota_baud_locked; round++) {
        for (int idx = 0; idx < 2 && !g_ota_baud_locked; idx++) {
            QL_OTA_LOG("Baud detect: round %d, trying %lu", round + 1, (unsigned long)list[idx]);

            if (ota_uart_open(list[idx]) != 0)
                continue;
            mb_rx_len = 0;
            ql_rtos_task_sleep_ms(50);

            for (int attempt = 0; attempt < OTA_DETECT_TRIES_PER_BAUD; attempt++) {
                uint16_t ver = 0;
                ota_touch_progress();

                int r = mb_read_holding(REG_PROTOCOL_VERSION, 1, &ver, OTA_DETECT_WAIT_MS, 0);
                if (r != -1) {      // 0 = ok, >0 = valid exception frame
                    g_ota_baud        = list[idx];
                    g_ota_baud_locked = true;
                    QL_OTA_LOG("Baud LOCKED at %lu (protocol ver 0x%04X, ret=%d)",
                               (unsigned long)g_ota_baud, ver, r);
                    break;
                }
                ql_rtos_task_sleep_ms(100);
            }
            if (!g_ota_baud_locked)
                ql_rtos_task_sleep_ms(200);
        }
    }

    if (!g_ota_baud_locked) {
        QL_OTA_LOG("Baud detect FAILED on all rates");
        return -1;
    }

    mb_rx_len = 0;
    return 0;
}

// ==================== OTA HELPERS ====================
static int ota_read_status(uint16_t *status, uint16_t *error)
{
    uint16_t regs[2];
    int ret = mb_read_holding(REG_OTA_STATUS, 2, regs, OTA_READ_TIMEOUT_MS, OTA_READ_RETRIES);
    if (ret != 0) return ret;
    *status = regs[0];
    *error  = regs[1];
    return 0;
}

static int ota_read_recv_offset(uint32_t *offset)
{
    uint16_t regs[2];
    int ret = mb_read_holding(REG_OTA_RECV_OFFSET, 2, regs, OTA_READ_TIMEOUT_MS, OTA_READ_RETRIES);
    if (ret != 0) return ret;
    *offset = ((uint32_t)regs[0] << 16) | regs[1];
    return 0;
}

static int ota_check_dongle_present(void)
{
    uint16_t ver = 0;
    int ret = mb_read_holding(REG_PROTOCOL_VERSION, 1, &ver, 300, 2);
    if (ret == 0) {
        QL_OTA_LOG("RS485 dongle responded, protocol version 0x%04X", ver);
        return 0;
    }
    QL_OTA_LOG("RS485 dongle did not respond (ret=%d)", ret);
    return -1;
}

static int ota_poll_until(uint16_t wanted, uint32_t timeout_s, uint16_t *out_status, uint16_t *out_error)
{
    uint64_t deadline = (uint64_t)ql_rtos_get_system_tick() + ((uint64_t)timeout_s * 1000UL);
    while ((uint64_t)ql_rtos_get_system_tick() < deadline) {
        uint16_t status, error;
        if (ota_read_status(&status, &error) == 0) {
            if (status == wanted || status == OTA_STATUS_ERROR) {
                *out_status = status;
                *out_error  = error;
                return 0;
            }
        }
        ql_rtos_task_sleep_ms(1000);
    }
    return -1;
}

static void ota_probe_log(void)
{
    uint16_t ver = 0, fwinfo = 0, status = 0, error = 0;

    if (mb_read_holding(REG_PROTOCOL_VERSION, 1, &ver, 200, 3) == 0)
        QL_OTA_LOG("Dongle protocol version: 0x%04X", ver);

    if (mb_read_holding(REG_OTA_FW_INFO, 1, &fwinfo, 200, 3) == 0) {
        uint32_t max_block = (fwinfo & 0xFF) + 64;
        QL_OTA_LOG("OTA_FW_INFO=0x%04X max_block=%luB target0=%s target1=%s",
                   fwinfo, (unsigned long)max_block,
                   (fwinfo & 0x100) ? "yes" : "no",
                   (fwinfo & 0x200) ? "yes" : "no");
    }

    if (ota_read_status(&status, &error) == 0)
        QL_OTA_LOG("OTA_STATUS=%d OTA_ERROR=%d", status, error);
}

static void ota_negotiate_block_size(void)
{
    uint16_t fwinfo = 0;

    if (mb_read_holding(REG_OTA_FW_INFO, 1, &fwinfo, 200, 3) == 0) {
        uint32_t max_block = (fwinfo & 0xFF) + 64;
        uint32_t requested = MIN((uint32_t)OTA_START_BLK, max_block);
        if (requested < 64) requested = 64;
        if (requested % 2) requested -= 1;

        g_ota_start_blk       = (uint16_t)requested;
        g_ota_data_block_size = (uint16_t)(requested - 4);

        QL_OTA_LOG("Negotiated OTA block: blk=%u data=%u", g_ota_start_blk, g_ota_data_block_size);
    } else {
        g_ota_start_blk       = 64;
        g_ota_data_block_size = OTA_DATA_BLOCK_SIZE_DEFAULT;
        QL_OTA_LOG("FW_INFO read failed, fallback blk=%u data=%u", g_ota_start_blk, g_ota_data_block_size);
    }
}

static int ota_send_start(uint32_t size, uint32_t crc32, uint8_t target, uint8_t seq, uint8_t blk)
{
    uint8_t payload[12];
    payload[0]  = (size >> 24) & 0xFF;  payload[1] = (size >> 16) & 0xFF;
    payload[2]  = (size >> 8)  & 0xFF;  payload[3] = size & 0xFF;
    payload[4]  = (crc32 >> 24) & 0xFF; payload[5] = (crc32 >> 16) & 0xFF;
    payload[6]  = (crc32 >> 8)  & 0xFF; payload[7] = crc32 & 0xFF;
    payload[8]  = target;
    payload[9]  = seq;
    payload[10] = blk;
    payload[11] = 0x00;

    for (int attempt = 0; attempt <= OTA_START_RETRIES; attempt++) {
        if (ota_check_dongle_present() != 0) {
            QL_OTA_LOG("ota_send_start: dongle presence check failed (attempt %d), aborting", attempt + 1);
            return -1;
        }
        ota_touch_progress();

        int ret = mb_write_multiple(REG_OTA_TOTAL_SIZE, payload, sizeof(payload), OTA_START_TIMEOUT_MS, 0);
        if (ret == 0) return ret;

        QL_OTA_LOG("ota_send_start: attempt %d/%d failed (ret=%d)", attempt + 1, OTA_START_RETRIES + 1, ret);
    }
    return -1;
}

static int ota_send_data_block(uint32_t offset, const uint8_t *data, uint16_t len)
{
    uint8_t payload[4 + OTA_MAX_BLOCK];
    payload[0] = (offset >> 24) & 0xFF; payload[1] = (offset >> 16) & 0xFF;
    payload[2] = (offset >> 8)  & 0xFF; payload[3] = offset & 0xFF;
    memcpy(&payload[4], data, len);

    return mb_write_multiple(REG_OTA_DATA, payload, 4 + len, OTA_BLOCK_TIMEOUT_MS, 0);
}

static int ota_send_end(void)      { return mb_write_single(REG_OTA_CTRL, OTA_CTRL_END,      OTA_CTRL_TIMEOUT_MS, OTA_CTRL_RETRIES); }
static int ota_send_activate(void) { return mb_write_single(REG_OTA_CTRL, OTA_CTRL_ACTIVATE, OTA_CTRL_TIMEOUT_MS, OTA_CTRL_RETRIES); }
static int ota_send_abort(void)    { return mb_write_single(REG_OTA_CTRL, OTA_CTRL_ABORT,    OTA_CTRL_TIMEOUT_MS, OTA_CTRL_RETRIES); }

// ==================== MAIN OTA TRANSFER (Modbus, unchanged logic) ====================
static int run_modbus_ota_transfer(void)
{
    if (ota_ctx.firmware_buffer == NULL || ota_ctx.firmware_size == 0) {
        QL_OTA_LOG("No firmware to send");
        return -1;
    }

    uint32_t fw_len      = ota_ctx.firmware_size;
    uint32_t padded_len  = fw_len;
    uint8_t *padded_buf  = ota_ctx.firmware_buffer;
    bool     free_padded = false;

    if (fw_len % 2 != 0) {
        padded_len = fw_len + 1;
        padded_buf = (uint8_t *)malloc(padded_len);
        if (!padded_buf) {
            report_ota_error("OOM padding firmware to even length");
            return -1;
        }
        memcpy(padded_buf, ota_ctx.firmware_buffer, fw_len);
        padded_buf[fw_len] = 0x00;
        free_padded = true;
    }

#define OTA_FAIL(msg)  do { report_ota_error(msg); if (free_padded) free(padded_buf); return -1; } while (0)

    uint32_t crc32 = crc32_mpeg2(padded_buf, padded_len);
    uint8_t  seq   = (uint8_t)(ql_rtos_get_system_tick() & 0xFF);

    QL_OTA_LOG("Phase4: UART locked at baud %lu", (unsigned long)g_ota_baud);
    ota_negotiate_block_size();
    ota_touch_progress();
    uint8_t blk = (uint8_t)g_ota_start_blk;

    QL_OTA_LOG("=== Modbus OTA: size=%lu (padded %lu) crc32=0x%08lX target=%d seq=0x%02X blk=%d data=%d ===",
               (unsigned long)fw_len, (unsigned long)padded_len, (unsigned long)crc32,
               OTA_MODBUS_TARGET, seq, blk, g_ota_data_block_size);

    ota_probe_log();
    ota_touch_progress();

    /* Clear any stale session */
    {
        uint16_t pre_status = 0, pre_error = 0;
        if (ota_read_status(&pre_status, &pre_error) == 0 &&
            (pre_status == OTA_STATUS_ERROR || pre_status == OTA_STATUS_BUSY ||
             pre_status == OTA_STATUS_FLASHING || pre_status == OTA_STATUS_VERIFY_OK)) {

            QL_OTA_LOG("Stale OTA session (STATUS=%d ERROR=%d) - sending ABORT", pre_status, pre_error);
            int abort_ret = ota_send_abort();
            ota_touch_progress();
            if (abort_ret != 0)
                QL_OTA_LOG("ABORT failed (ret=%d)", abort_ret);

            uint16_t cur_status = pre_status, cur_error = pre_error;
            uint64_t clear_deadline = (uint64_t)ql_rtos_get_system_tick() + 10000;
            while ((uint64_t)ql_rtos_get_system_tick() < clear_deadline) {
                if (ota_read_status(&cur_status, &cur_error) == 0) {
                    ota_touch_progress();
                    if (cur_status == OTA_STATUS_IDLE) break;
                }
                ql_rtos_task_sleep_ms(500);
            }
            if (cur_status != OTA_STATUS_IDLE)
                OTA_FAIL("Dongle stuck in non-idle OTA state, ABORT did not clear it");

            QL_OTA_LOG("Stale session cleared");
        }
    }

    /* Target capability check */
    {
        uint16_t fwinfo_chk = 0;
        if (mb_read_holding(REG_OTA_FW_INFO, 1, &fwinfo_chk, 200, 3) != 0)
            OTA_FAIL("FW_INFO read failed (target capability check)");

        uint16_t need_bit = (OTA_MODBUS_TARGET == 0) ? 0x100 : 0x200;
        if (!(fwinfo_chk & need_bit))
            OTA_FAIL("Target not supported by dongle firmware");

        QL_OTA_LOG("Target=%d support confirmed (FW_INFO=0x%04X)", OTA_MODBUS_TARGET, fwinfo_chk);
    }

    if (OTA_MODBUS_TARGET == 1) {
        QL_OTA_LOG("Target=1: requesting run-stop...");
        mb_write_single(REG_SYS_RUN_FLAG, 0, 500, 3);

        uint64_t deadline = (uint64_t)ql_rtos_get_system_tick() + 10000;
        bool stopped = false;
        while ((uint64_t)ql_rtos_get_system_tick() < deadline) {
            uint16_t run_state;
            if (mb_read_holding(REG_RUN_STATE, 1, &run_state, 200, 3) == 0) {
                if (run_state != 8) { stopped = true; break; }
            }
            ql_rtos_task_sleep_ms(500);
        }
        if (!stopped)
            QL_OTA_LOG("warning: run-state did not leave NORMAL within 10s, continuing");
        ota_touch_progress();
    }

    QL_OTA_LOG("Sending OTA START (erase may take 20-40s)...");
    int sret = ota_send_start(padded_len, crc32, OTA_MODBUS_TARGET, seq, blk);
    ota_touch_progress();
    if (sret != 0)
        OTA_FAIL("Modbus OTA START rejected");

    uint16_t status = 0, error = 0;
    uint32_t recv_offset = 0;
    ota_read_recv_offset(&recv_offset);
    ota_read_status(&status, &error);
    QL_OTA_LOG("After START: STATUS=%d ERROR=%d RECV_OFFSET=%lu", status, error, (unsigned long)recv_offset);

    if (status != OTA_STATUS_RECEIVING)
        OTA_FAIL("Unexpected STATUS after OTA START");

    uint32_t offset     = recv_offset;
    uint32_t stuck_at   = 0xFFFFFFFFUL;
    int      stuck_count = 0;

    ota_ctx.firmware_size        = padded_len;
    ota_ctx.total_bytes_received = offset;

    while (offset < padded_len) {
        uint32_t remaining = padded_len - offset;
        uint16_t chunk_len = (remaining > g_ota_data_block_size) ? g_ota_data_block_size : (uint16_t)remaining;

        int  attempts  = 0;
        bool advanced  = false;
        bool realigned = false;

        while (attempts < OTA_BLOCK_MAX_RETRY && !advanced && !realigned) {
            int r = ota_send_data_block(offset, &padded_buf[offset], chunk_len);

            if (r == 0) {
                offset += chunk_len;
                advanced = true;
                ota_touch_progress();
                stuck_at = 0xFFFFFFFFUL;
                stuck_count = 0;
                ota_ctx.total_bytes_received = offset;
                report_ota_progress();

                if ((offset & 0x3FFF) == 0 || offset >= padded_len) {
                    QL_OTA_LOG("Modbus OTA: %lu/%lu bytes (%lu%%)",
                               (unsigned long)offset, (unsigned long)padded_len,
                               (unsigned long)(((uint64_t)offset * 100) / padded_len));
                }
            } else if (r == 0x03) {
                uint32_t new_offset = offset;
                ota_read_recv_offset(&new_offset);
                ota_read_status(&status, &error);
                QL_OTA_LOG("Block @%lu rejected(0x03). STATUS=%d ERR=%d RECV_OFFSET=%lu",
                           (unsigned long)offset, status, error, (unsigned long)new_offset);

                if (new_offset == stuck_at) stuck_count++;
                else { stuck_at = new_offset; stuck_count = 1; }

                if (stuck_count >= OTA_STUCK_LIMIT) {
                    ota_send_abort();
                    OTA_FAIL("Modbus OTA stuck realigning offset");
                }
                offset = new_offset;
                realigned = true;
            } else if (r == -1) {
                attempts++;
                QL_OTA_LOG("Block @%lu timeout, retry %d/%d", (unsigned long)offset, attempts, OTA_BLOCK_MAX_RETRY);
                ql_rtos_task_sleep_ms(50);
            } else {
                QL_OTA_LOG("Block @%lu exception 0x%02X", (unsigned long)offset, r);
                ota_send_abort();
                OTA_FAIL("Modbus OTA data block exception");
            }
        }

        if (!advanced && !realigned) {
            ota_send_abort();
            OTA_FAIL("Modbus OTA block send failed");
        }
    }

    QL_OTA_LOG("All OTA data sent (%lu bytes). Sending END...", (unsigned long)padded_len);
    if (ota_send_end() != 0) {
        ota_send_abort();
        OTA_FAIL("Modbus OTA END failed");
    }

    if (ota_poll_until(OTA_STATUS_VERIFY_OK, 30, &status, &error) != 0 || status != OTA_STATUS_VERIFY_OK) {
        QL_OTA_LOG("OTA verify failed: STATUS=%d ERROR=%d", status, error);
        OTA_FAIL("Modbus OTA verify failed");
    }

    QL_OTA_LOG("Verify OK. Sending ACTIVATE...");
    if (ota_send_activate() != 0)
        OTA_FAIL("Modbus OTA ACTIVATE failed");

    if (OTA_MODBUS_TARGET == 0) {
        QL_OTA_LOG("Target=0: dongle resets ~1s after ACTIVATE. Waiting 3.5s...");
        ql_rtos_task_sleep_ms(3500);

        uint16_t ver_after[2] = {0};
        int vret = mb_read_holding(REG_COMCARD_VERSION, 2, ver_after, 1000, 5);
        if (vret != 0) {
            QL_OTA_LOG("Post-activate: no response (ret=%d)", vret);
            OTA_FAIL("No response from dongle after ACTIVATE reboot");
        }
        QL_OTA_LOG("Post-activate COMCARD_VERSION: [0x%04X 0x%04X]", ver_after[0], ver_after[1]);

        uint16_t verregs[2];
        if (mb_read_holding(REG_RUN_TIME, 2, verregs, 500, 3) == 0)
            QL_OTA_LOG("Post-activate Run_Time: [0x%04X 0x%04X]", verregs[0], verregs[1]);
    } else {
        QL_OTA_LOG("Target=1: polling until done (up to 200s)...");
        if (ota_poll_until(OTA_STATUS_DONE, 200, &status, &error) != 0 || status != OTA_STATUS_DONE) {
            QL_OTA_LOG("OTA finished with STATUS=%d ERROR=%d", status, error);
            OTA_FAIL("Modbus OTA activation/delivery failed");
        }

        uint16_t dsp_ver[2] = {0};
        int vret = mb_read_holding(REG_DSP_VERSION, 2, dsp_ver, 1000, 5);
        if (vret != 0)
            OTA_FAIL("DSP version read failed after activate");
        QL_OTA_LOG("Post-activate DSP_VERSION: [0x%04X 0x%04X]", dsp_ver[0], dsp_ver[1]);
    }

#undef OTA_FAIL
    QL_OTA_LOG("=== Modbus OTA sequence completed successfully! ===");
    if (free_padded) free(padded_buf);
    return 0;
}

// ==================== FILE SIZE API ====================
static uint32_t parse_file_size_from_response(const char *response)
{
    if (!response || response[0] == '\0') return 0;

    QL_OTA_LOG("Parsing file size: %s", response);

    const char *size_key = "\"size\":";
    const char *size_ptr = strstr(response, size_key);
    if (size_ptr) {
        size_ptr += strlen(size_key);
        while (*size_ptr == ' ' || *size_ptr == '\t' || *size_ptr == '\n' || *size_ptr == '\r')
            size_ptr++;
        return (uint32_t)atoi(size_ptr);
    }
    return (uint32_t)atoi(response);
}

void make_file_size_url(char *FileName)
{
    snprintf(HTTP_OTA_FILESIZE_URI, sizeof(HTTP_OTA_FILESIZE_URI),
             "http://eastmanone.com:3001/file-size?filename=%s", FileName);
    QL_OTA_LOG("File-size URL: %s", HTTP_OTA_FILESIZE_URI);
}

static uint32_t get_firmware_size_from_api(void)
{
    char     resp[256] = {0};
    uint32_t got = 0;
    int      status = 0;

    QL_OTA_LOG("=== Querying firmware size ===");
    make_file_size_url(INV_OTA_URL_SUFFIX);
    ql_rtos_task_sleep_ms(200);

    if (http_get(HTTP_OTA_FILESIZE_URI, (uint8_t *)resp, sizeof(resp) - 1, true, false,
                 30000, &got, &status) != 0 || got == 0) {
        report_ota_error("Failed to get firmware size");
        return 0;
    }
    resp[got] = '\0';

    uint32_t file_size = parse_file_size_from_response(resp);
    if (file_size == 0) {
        report_ota_error("Invalid firmware size");
        return 0;
    }

    QL_OTA_LOG("Firmware size: %lu bytes", (unsigned long)file_size);
    if (file_size > MAX_FIRMWARE_SIZE) {
        QL_OTA_LOG("ERROR: Firmware too large! %lu > %lu", (unsigned long)file_size, (unsigned long)MAX_FIRMWARE_SIZE);
        report_ota_error("Firmware too large");
        return 0;
    }
    return file_size;
}

// ==================== DOWNLOAD FIRMWARE ====================
static int download_firmware(void)
{
    char     url[256] = {0};
    uint32_t got = 0;
    int      status = 0;

    snprintf(url, sizeof(url), "%s%s", HTTP_OTA_FILE_URI, INV_OTA_URL_SUFFIX);
    QL_OTA_LOG("Download URL: %s", url);

    ota_ctx.firmware_buffer = (uint8_t *)malloc(ota_ctx.firmware_size);
    if (ota_ctx.firmware_buffer == NULL) {
        QL_OTA_LOG("Failed to allocate %lu bytes", (unsigned long)ota_ctx.firmware_size);
        report_ota_error("Failed to allocate firmware buffer");
        return -1;
    }
    memset(ota_ctx.firmware_buffer, 0, ota_ctx.firmware_size);
    ota_ctx.total_bytes_received = 0;

    QL_OTA_LOG("=== PHASE 3: Downloading firmware (%lu bytes) ===", (unsigned long)ota_ctx.firmware_size);
    ql_rtos_task_sleep_ms(200);

    // Exact-size buffer, overflow = failure (allow_trunc=false), watchdog fed per chunk.
    int ret = http_get(url, ota_ctx.firmware_buffer, ota_ctx.firmware_size,
                       false, true, 120000, &got, &status);
    ota_ctx.total_bytes_received = got;

    if (ret != 0) {
        QL_OTA_LOG("Download failed (status=%d, got=%lu)", status, (unsigned long)got);
        report_ota_error("Firmware download failed");
        goto fail;
    }
    if (got != ota_ctx.firmware_size) {
        QL_OTA_LOG("Download size mismatch: expected %lu, got %lu",
                   (unsigned long)ota_ctx.firmware_size, (unsigned long)got);
        report_ota_error("Download size mismatch");
        goto fail;
    }

    QL_OTA_LOG("Download completed: %lu bytes", (unsigned long)got);
    ota_touch_progress();
    report_ota_progress();      // report 100% from the OTA thread (not from the HTTP callback)
    return 0;

fail:
    free(ota_ctx.firmware_buffer);
    ota_ctx.firmware_buffer = NULL;
    return -1;
}

// ==================== DATA CALL (EG800AK API) ====================
static void ota_datacall_status_cb(int profile_idx, int status)
{
    QL_OTA_LOG("profile(%d) status: %d", profile_idx, status);
}

static int ota_ensure_data_call(void)
{
    struct ql_data_call_info info = {0};
    char ip4_addr_str[16] = {0};
    int  ret, i = 0;

    while ((ret = ql_network_register_wait(120)) != 0 && i < 10) {
        i++;
        QL_OTA_LOG("Network register attempt %d, ret=%d", i, ret);
        ql_rtos_task_sleep_ms(1000);
    }
    if (ret != 0) {
        QL_OTA_LOG("Network registration failed");
        return -1;
    }
    QL_OTA_LOG("Network registered");

    // profile 1 is the one mqtt_app.c already brings up - reuse it when active
    ql_get_data_call_info(PROFILE_IDX, 0, &info);

    if (info.v4.state == 1) {
        QL_OTA_LOG("Data call already active on profile %d - reusing", PROFILE_IDX);
    } else {
        QL_OTA_LOG("No active data call on profile %d - starting one", PROFILE_IDX);
        ql_wan_start(ota_datacall_status_cb);
        ql_set_auto_connect(PROFILE_IDX, TRUE);
        extern char SET_APN_2[20];
        const char *apn = (SET_APN_2[0] && strcmp(SET_APN_2, "no-apn") != 0) ? SET_APN_2 : "www";
        ret = ql_start_data_call(PROFILE_IDX, 0, (char *)apn, NULL, NULL, 0);
                if (ret != 0) {
            QL_OTA_LOG("Data call failed: %d", ret);
            return -1;
        }
        ql_rtos_task_sleep_ms(2000);

        memset(&info, 0, sizeof(info));
        ql_get_data_call_info(PROFILE_IDX, 0, &info);
        if (info.v4.state != 1) {
            QL_OTA_LOG("Data call did not come up (state=%d)", info.v4.state);
            return -1;
        }
    }

    QL_OTA_LOG("info.profile_idx: %d, ip_version: %d, v4.state: %d",
               info.profile_idx, info.ip_version, info.v4.state);
    inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
    QL_OTA_LOG("info.v4.addr.ip: %s", ip4_addr_str);
    inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
    QL_OTA_LOG("info.v4.addr.pri_dns: %s", ip4_addr_str);
    return 0;
}

// ==================== MAIN OTA THREAD ====================
static void ql_http_down_thread(void *arg)
{
    (void)arg;

    OTA_START = true;
    ota_touch_progress();

    if (ql_rtos_task_create(&ota_watchdog_task, 2048, APP_PRIORITY_NORMAL,
                            "QotaWatchdog", ota_watchdog_thread, NULL, 5) != QL_OSI_SUCCESS)
        QL_OTA_LOG("OTA stuck-watchdog task create failed (continuing without it)");

    QL_OTA_LOG("========== OTA Firmware Update Start ==========");

    memset(&ota_ctx, 0, sizeof(ota_ctx));
    ota_ctx.status_report_enabled = true;
    mb_rx_len = 0;
    g_ota_baud_locked = false;

    if (ota_ensure_data_call() != 0)
        goto exit;

    QL_OTA_LOG("Waiting for network stack to stabilize...");
    ql_rtos_task_sleep_ms(3000);

    // ===== PHASE 1: firmware size =====
    ota_ctx.firmware_size = get_firmware_size_from_api();
    if (ota_ctx.firmware_size == 0) {
        QL_OTA_LOG("Invalid firmware size - aborting");
        goto exit;
    }
    ota_touch_progress();
    send_ota_status_to_server(0, NULL);

    // ===== PHASE 2: baud detect + dongle presence =====
    if (mb_rx_sem == NULL) {
        if (ql_rtos_semaphore_create(&mb_rx_sem, 0) != 0) {
            report_ota_error("Failed to create Modbus RX semaphore");
            goto exit;
        }
    }

    if (ota_detect_baud() != 0) {
        report_ota_error("RS485 dongle not responding at any baud");
        goto exit;
    }
    ota_touch_progress();

    if (ota_check_dongle_present() != 0) {
        report_ota_error("RS485 dongle not responding");
        goto exit;
    }
    ota_touch_progress();

    // ===== PHASE 3: download =====
    if (download_firmware() != 0) {
        QL_OTA_LOG("Firmware download failed - aborting");
        goto exit;
    }

    // ===== PHASE 4: Modbus OTA =====
    QL_OTA_LOG("=== PHASE 4: Modbus RTU OTA transfer (baud %lu) ===", (unsigned long)g_ota_baud);
    ota_ctx.transfer_active = true;

    if (run_modbus_ota_transfer() != 0) {
        ota_ctx.transfer_active = false;
        QL_OTA_LOG("=== Modbus OTA transfer FAILED - firmware NOT updated ===");
        goto exit;
    }
    ota_ctx.transfer_active = false;

    QL_OTA_LOG("=== FIRMWARE UPDATE COMPLETED SUCCESSFULLY! ===");
    report_ota_success();

exit:
    if (ota_ctx.firmware_buffer) {
        free(ota_ctx.firmware_buffer);
        ota_ctx.firmware_buffer = NULL;
    }

    OTA_START = false;

    ota_uart_close();
    if (mb_rx_sem != NULL) {
        ql_rtos_semaphore_delete(mb_rx_sem);
        mb_rx_sem = NULL;
    }

    // Data call is intentionally NOT stopped: mqtt_app.c shares profile 1, and we reboot anyway.

    QL_OTA_LOG("=== OTA PROCESS COMPLETED ===");

    ql_rtos_task_sleep_ms(5000);
    bk_reboot();

    ql_rtos_task_delete(ota_task);
}

// ==================== OTA INITIALIZATION ====================
void ql_http_down_app_init(void)
{
    QlOSStatus err = ql_rtos_task_create(&ota_task, 8192, APP_PRIORITY_NORMAL,
                                         "QotaApp", ql_http_down_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
        QL_OTA_LOG("OTA task create failed: 0x%x", err);
}