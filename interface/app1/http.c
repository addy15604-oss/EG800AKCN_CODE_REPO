/*==========================================================================================================
  http_app.c — periodic FOTA check over HTTP for EG800AK.

  Every FOTA_INTERVAL_S seconds the task POSTs
      {"IMEI":"..","CURR_SOFT_VER":"..","CURR_FILE_PATH":"..","DATE_TIME":"YYYY-MM-DD HH:MM:SS"}
  to FOTA_URL and looks for two keys in the JSON reply:
      "NEWFILEPATH":"<path>"  -> saved to file_full_path and UFS:FILE_8.txt
      "FOTA":"TRUE"           -> UFS:FILE_1.txt = "1", then the module restarts (FOTA runs at boot)

  Same behaviour as the old module. Differences:
    * Only http_post_json() (and its two callbacks) touch the SDK HTTP/RTOS API. If your EG800AK
      headers differ from the assumptions marked "SDK PORT", change only that section.
    * JSON handling is bounds-checked (old parser could overflow its 400/50-byte buffers).
    * The AT+QSPN / APN code was dropped: it was already disabled and main.c has its own copy.
===========================================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_http_client.h"            /* SDK PORT: HTTP client header */
#include "main_app.h"
#include "FileSystem.h"
#include "http_app.h"

/* ------------------------------------- settings ---------------------------------------------- */
#define FOTA_URL            "http://eastmanone.com/eastmen_inverter/inveter_software_update.php"
#define FOTA_FIRST_DELAY_S  6          /* first check, after the 10 s start-up delay            */
#define FOTA_INTERVAL_S     63         /* old loop ran ~every 63 s; raise this for production   */
#define HTTP_SIM_ID         0
#define HTTP_PROFILE        1          /* PDP context / profile index                           */
#define HTTP_TIMEOUT_MS     60000

#define HTTP_BODY_MAX       400
#define HTTP_RESP_MAX       512
#define HTTP_PATH_MAX       100        /* file_full_path must hold at least this many bytes     */

#define HTTP_LOG(fmt, ...)  printf("[HTTP] " fmt "\r\n", ##__VA_ARGS__)

/* ---- provided elsewhere in the project (main.c / OTA code) ---- */
extern char MOD_SOFTWARE_VERSION[];
extern char APP_Date[];
extern char APP_Time[];
extern char file_full_path[];          /* >= HTTP_PATH_MAX bytes */
/* GPRSData.IMEI comes from main_app.h */

static ql_task_t http_task = NULL;
static char HTTP_STR[HTTP_BODY_MAX];
static char HTTP_RESP[HTTP_RESP_MAX];

/* =========================================================================================== */
/* ================================ SDK PORT: HTTP transfer ================================== */
/* =========================================================================================== */
enum { HTTP_EVT_END = 1001 };

typedef struct { unsigned int id; unsigned int param1; } http_evt_t;

typedef struct
{
    http_client_t  client;
    ql_queue_t     queue;
    const char    *req;                /* request body being uploaded          */
    int            req_off;
    char          *resp;               /* where the reply is collected         */
    int            resp_size;
    int            resp_len;
    int            resp_code;
} http_ctx_t;

static http_ctx_t ctx;

static void http_event_cb(http_client_t *client, int evt, int evt_code, void *arg)
{
    http_ctx_t *c = (http_ctx_t *)arg;
    http_evt_t ev = {HTTP_EVT_END, 0};

    if (c == NULL || *client != c->client) return;
    HTTP_LOG("evt:%d code:%d", evt, evt_code);

    switch (evt)
    {
        case HTTP_EVENT_SESSION_ESTABLISH:
            if (evt_code != HTTP_SUCCESS)
            {
                HTTP_LOG("session create failed");
                ql_rtos_queue_release(c->queue, sizeof(ev), (uint8 *)&ev, 0);
            }
            break;

        case HTTP_EVENT_RESPONE_STATE_LINE:
            if (evt_code == HTTP_SUCCESS)
            {
                int code = 0;
                ql_httpc_getinfo(client, HTTP_INFO_RESPONSE_CODE, &code);
                c->resp_code = code;
                HTTP_LOG("response code:%d", code);
            }
            break;

        case HTTP_EVENT_SESSION_DISCONNECT:
            HTTP_LOG(evt_code == HTTP_SUCCESS ? "transfer end" : "transfer exception");
            ev.param1 = (unsigned int)evt_code;
            ql_rtos_queue_release(c->queue, sizeof(ev), (uint8 *)&ev, 0);
            break;
    }
}

/* reply body arrives here, possibly in several pieces */
static int http_write_response_data(http_client_t *client, void *arg, char *data, int size, unsigned char end)
{
    http_ctx_t *c = (http_ctx_t *)arg;
    (void)end;

    if (c == NULL || *client != c->client) return 0;

    int room = c->resp_size - 1 - c->resp_len;
    int n = (size < room) ? size : room;
    if (n > 0)
    {
        memcpy(c->resp + c->resp_len, data, n);
        c->resp_len += n;
        c->resp[c->resp_len] = '\0';
    }
    return size;                       /* report everything consumed (extra bytes are dropped) */
}

/* request body is pulled from here, possibly in several pieces */
static int http_read_request_data(http_client_t *client, void *arg, char *data, int size)
{
    http_ctx_t *c = (http_ctx_t *)arg;
    (void)client;

    int left = (int)strlen(c->req) - c->req_off;
    int n = (size < left) ? size : left;
    if (n <= 0) return 0;

    memcpy(data, c->req + c->req_off, n);
    c->req_off += n;
    return n;
}

/* returns reply length (>=0) or -1 on failure */
static int http_post_json(const char *url, const char *body, char *resp, int resp_size)
{
    http_evt_t ev;
    int ret = -1;

    memset(&ctx, 0, sizeof(ctx));
    ctx.req = body;
    ctx.resp = resp;
    ctx.resp_size = resp_size;
    resp[0] = '\0';

    if (ql_rtos_queue_create(&ctx.queue, sizeof(http_evt_t), 4) != 0)
    {
        HTTP_LOG("queue create failed");
        return -1;
    }

    if (ql_httpc_new(&ctx.client, http_event_cb, (void *)&ctx) != HTTP_SUCCESS)
    {
        HTTP_LOG("client create failed");
        ql_rtos_queue_delete(ctx.queue);
        return -1;
    }

    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_SIM_ID, HTTP_SIM_ID);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_PDPCID, HTTP_PROFILE);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_WRITE_FUNC, http_write_response_data);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_WRITE_DATA, (void *)&ctx);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_METHOD, HTTP_METHOD_POST);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_READ_FUNC, http_read_request_data);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_READ_DATA, (void *)&ctx);
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_UPLOAD_LEN, (int)strlen(body));
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_REQUEST_HEADER, "Content-type:application/json");
    ql_httpc_formadd(&ctx.client, HTTP_FORM_CONTENT_TYPE, "application/json");
    ql_httpc_setopt(&ctx.client, HTTP_CLIENT_OPT_URL, (char *)url);

    if (ql_httpc_perform(&ctx.client) == HTTP_SUCCESS)
    {
        memset(&ev, 0, sizeof(ev));
        if (ql_rtos_queue_wait(ctx.queue, (uint8 *)&ev, sizeof(ev), HTTP_TIMEOUT_MS) == 0
            && ev.param1 == HTTP_SUCCESS)
            ret = ctx.resp_len;
        else
            HTTP_LOG("transfer failed/timeout");
    }
    else
    {
        HTTP_LOG("perform failed");
    }

    ql_httpc_release(&ctx.client);
    ql_rtos_queue_delete(ctx.queue);
    ctx.queue = NULL;
    return ret;
}
/* ================================== end of SDK PORT ======================================== */

/* ------------------------------------ request body ------------------------------------------- */
static void Create_Fota_String(char *out, int out_size)
{
    snprintf(out, out_size,
             "{\"IMEI\":\"%s\",\"CURR_SOFT_VER\":\"%s\",\"CURR_FILE_PATH\":\"%s\",\"DATE_TIME\":\"%s %s\"}",
             GPRSData.IMEI, MOD_SOFTWARE_VERSION, file_full_path, APP_Date, APP_Time);
    HTTP_LOG("httpString:%s", out);
}

/* ------------------------------------- reply parsing ----------------------------------------- */
/* finds "key" : "value" (or bare value) in a flat JSON string; returns 1 if found */
static int json_get_string(const char *json, const char *key, char *out, size_t out_size)
{
    char pat[48];
    snprintf(pat, sizeof(pat), "\"%s\"", key);

    const char *p = strstr(json, pat);
    if (!p) return 0;

    p += strlen(pat);
    while (*p == ' ' || *p == '\t') p++;
    if (*p != ':') return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;

    int quoted = (*p == '"');
    if (quoted) p++;

    size_t n = 0;
    while (*p && n < out_size - 1)
    {
        if (quoted ? (*p == '"') : (*p == ',' || *p == '}' || *p == ' ')) break;
        out[n++] = *p++;
    }
    out[n] = '\0';
    return n > 0;
}

static int is_true(const char *s)
{
    return toupper((unsigned char)s[0]) == 'T' && toupper((unsigned char)s[1]) == 'R' &&
           toupper((unsigned char)s[2]) == 'U' && toupper((unsigned char)s[3]) == 'E' && s[4] == '\0';
}

static void handle_server_reply(const char *json)
{
    char val[HTTP_PATH_MAX];
    HTTP_LOG("<== HTTP Raw Server Json :%s==>", json);

    if (json_get_string(json, "NEWFILEPATH", val, sizeof(val)))
    {
        strncpy(file_full_path, val, HTTP_PATH_MAX - 1);
        file_full_path[HTTP_PATH_MAX - 1] = '\0';
        WriteFileUFS(file_full_path, 8);
        HTTP_LOG("RECEIVED FILE PATH:%s", file_full_path);
    }

    if (json_get_string(json, "FOTA", val, sizeof(val)) && is_true(val))
    {
        HTTP_LOG("STARTING FOTA..........");
        WriteFileUFS("1", 1);
        ql_rtos_task_sleep_s(5);
        ql_power_reset();                 /* same call main.c uses */
    }
}

/* ------------------------------------------ task --------------------------------------------- */
static void http_app_thread(void *arg)
{
    (void)arg;
    int wait_s = FOTA_FIRST_DELAY_S;

    ql_rtos_task_sleep_s(10);
    HTTP_LOG("========== http app start ==========");

    while (1)
    {
        ql_rtos_task_sleep_s(1);
        if (--wait_s > 0) continue;
        wait_s = FOTA_INTERVAL_S;

        Create_Fota_String(HTTP_STR, sizeof(HTTP_STR));

        int len = http_post_json(FOTA_URL, HTTP_STR, HTTP_RESP, sizeof(HTTP_RESP));
        if (len > 0)
            handle_server_reply(HTTP_RESP);
        else
            HTTP_LOG("no reply (len=%d), retry in %d s", len, FOTA_INTERVAL_S);
    }
}

void ql_http_init(void)
{
    QlOSStatus err = ql_rtos_task_create(&http_task, 8 * 1024, APP_PRIORITY_NORMAL,
                                         "QhttpApp", http_app_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
        HTTP_LOG("created task failed 0x%x", err);
}