/*================================================================
  fota_http.c - HTTP FOTA for Quectel EG800AK-CN (EG800AKCN91LC)

  APIs   : ql_http_client_* / ql_fota_image_* (EG800AK SDK)
  Layout : your existing http fota module (HTTP_DLOAD_URL +
           HTTP_FILE_NAME, fota_http_app_init(), retry loop, one task).

  The package is streamed straight into the FOTA partition
  (ql_fota_image_write), so no UFS temp file is needed.
  Resume (HTTP Range) is therefore not possible: every retry
  starts from byte 0.
=================================================================*/
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_data_call.h"
#include "sockets.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_application.h"
#include "ql_fota.h"
#include "ql_http_client.h"
#include "fota_http.h"

#define OTA_LOG(fmt, ...)  printf("[HTTP_OTA][%s,%d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* ---------------- configuration ---------------- */
#define OTA_PROFILE_IDX      1
#define OTA_APN              "www"   /* MUST match the APN used in mqtt_app.c */
#define OTA_URL_MAX          256
#define OTA_TRY_TIMES        10
#define OTA_RETRY_DELAY_S    5
#define OTA_NET_WAIT_S       120

#define OTA_TASK_STACK       (1024 * 16)
#define OTA_TASK_PRIO        100            /* same scale as ql_rtos_task_create in the SDK examples */

/* Adjust to your SDK: some headers declare ql_power_reset(void),
 * others ql_power_reset(int type) -> then use ql_power_reset(RESET_NORMAL). */
#define OTA_SYSTEM_RESET()   ql_power_reset()

/* exported (see fota_http.h) - FileSystem.c strcpy()s into these at boot */
char HTTP_DLOAD_URL[100] = "http://eastmanone.com:3001/download/";
char HTTP_FILE_NAME[100] = "fbf.bin";

/* ---------------- state ---------------- */
typedef enum
{
    OTA_STAGE_IDLE = 0,
    OTA_STAGE_DOWNLOADING,
    OTA_STAGE_VERIFYING,
    OTA_STAGE_DONE,
    OTA_STAGE_FAILED
} ota_stage_e;

typedef struct
{
    ota_stage_e stage;
    int   total_len;      /* Content-Length, 0 if unknown (chunked) */
    int   recv_len;       /* bytes written to FOTA partition        */
    int   progress;       /* 0..100, -1 on error                    */
    int   last_logged;    /* last progress step printed             */
    int   body_done;      /* RECV_BODY_FINISHED seen                */
    int   failed;         /* any error inside the callback          */
} ota_ctx_t;

static ota_ctx_t           s_ota;
static qlFotaImgProcCtxPtr s_fota_ctx = NULL;
static ql_task_t           s_task     = NULL;
static volatile int        s_running  = 0;
static char                s_url[OTA_URL_MAX];

/* ---------------- public helpers ---------------- */
int http_ota_get_progress(void)
{
    return s_ota.progress;
}

int http_ota_is_busy(void)
{
    return s_running;
}

/* url given -> use it; otherwise HTTP_DLOAD_URL + HTTP_FILE_NAME (one '/' between) */
static void http_ota_build_url(const char *url)
{
    const char *name = HTTP_FILE_NAME;
    size_t n = strlen(HTTP_DLOAD_URL);
    const char *sep = "";

    if (url && url[0])
    {
        snprintf(s_url, sizeof(s_url), "%s", url);
        return;
    }

    if (n > 0 && HTTP_DLOAD_URL[n - 1] == '/' && name[0] == '/')
        name++;                                   /* avoid "//" */
    else if (n > 0 && HTTP_DLOAD_URL[n - 1] != '/' && name[0] != '/')
        sep = "/";                                /* add the missing "/" */

    snprintf(s_url, sizeof(s_url), "%s%s%s", HTTP_DLOAD_URL, sep, name);
}

/* ---------------- HTTP callback ---------------- */
static int http_ota_resp_cb(QL_HTTP_CLIENT_T *client,
                            QL_HTTP_CLIENT_EVENT_E event,
                            int status_code,
                            char *data,
                            int data_len,
                            void *private_data)
{
    int ret = 1;                 /* 1 = keep receiving, 0 = abort */
    char para[32] = {0};
    int  hret, wret;

    (void)client;
    (void)private_data;

    switch (event)
    {
        case QL_HTTP_CLIENT_EVENT_SEND_FAIL:
            OTA_LOG("http send failed");
            s_ota.failed = 1;
            ret = 0;
            break;

        case QL_HTTP_CLIENT_EVENT_SEND_SUCCESSED:
            OTA_LOG("http send ok");
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FAIL:
            OTA_LOG("http parse header failed");
            s_ota.failed = 1;
            ret = 0;
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FINISHED:
            OTA_LOG("http status:%d header_len:%d", status_code, data_len);
            if (status_code != 200)
            {
                OTA_LOG("unexpected status, abort");
                s_ota.failed = 1;
                ret = 0;
                break;
            }
            hret = ql_http_client_get_header(data, "Content-Length", 0, para, sizeof(para));
            if (hret > 0)
                s_ota.total_len = atoi(para);
            OTA_LOG("content-length:%d", s_ota.total_len);
            s_ota.stage = OTA_STAGE_DOWNLOADING;
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_BODY:
            if (s_ota.failed || s_fota_ctx == NULL)
            {
                ret = 0;
                break;
            }
            wret = ql_fota_image_write(s_fota_ctx, (void *)data, data_len);
            if (wret)
            {
                OTA_LOG("*** fota image write fail (%d) ***", wret);
                s_ota.failed = 1;
                ret = 0;
                break;
            }
            s_ota.recv_len += data_len;
            if (s_ota.total_len > 0)
            {
                s_ota.progress = (int)(((long long)s_ota.recv_len * 100) / s_ota.total_len);
                if (s_ota.progress >= s_ota.last_logged + 10)
                {
                    s_ota.last_logged = s_ota.progress - (s_ota.progress % 10);
                    OTA_LOG("progress %d%% (%d/%d)", s_ota.progress, s_ota.recv_len, s_ota.total_len);
                }
            }
            break;

        case QL_HTTP_CLIENT_EVENT_RECV_BODY_FINISHED:
            OTA_LOG("http body finished, %d bytes", s_ota.recv_len);
            s_ota.body_done = 1;
            break;

        case QL_HTTP_CLIENT_EVENT_DISCONNECTED:
            /* normal after a finished body; only an error if body is incomplete */
            if (!s_ota.body_done)
            {
                OTA_LOG("disconnected before body finished");
                s_ota.failed = 1;
            }
            break;

        default:
            break;
    }
    return ret;
}

/* ---------------- network ---------------- */
static int http_ota_data_call_up(void)
{
    struct ql_data_call_info info = {0};

    if (ql_get_data_call_info(OTA_PROFILE_IDX, 0, &info) == 0 && info.v4.state == 1)
        return 1;
    return 0;
}

/* mqtt_app.c normally already holds the data call. Reuse it; only dial
 * ourselves if it is not up after a grace period.
 * NOTE: ql_wan_start() is deliberately NOT called here so we do not
 * replace the network callback registered by mqtt_app.c. */
static int http_ota_net_ready(void)
{
    int i;

    if (http_ota_data_call_up())
        return 0;

    OTA_LOG("wait for network register");
    if (ql_network_register_wait(OTA_NET_WAIT_S) != 0)
    {
        OTA_LOG("*** network register fail ***");
        return -1;
    }

    for (i = 0; i < 15; i++)           /* give mqtt_app up to ~30 s */
    {
        if (http_ota_data_call_up())
            return 0;
        ql_rtos_task_sleep_s(2);
    }

    OTA_LOG("starting data call");
    if (ql_start_data_call(OTA_PROFILE_IDX, 0, OTA_APN, NULL, NULL, 0) != 0)
    {
        OTA_LOG("*** data call fail ***");
        return -1;
    }
    return 0;
}

/* ---------------- one download attempt ---------------- */
static int http_ota_download_once(const char *url)
{
    QL_HTTP_CLIENT_T *client;
    int ret;

    memset(&s_ota, 0, sizeof(s_ota));
    s_ota.stage = OTA_STAGE_DOWNLOADING;

    s_fota_ctx = ql_fota_init();
    if (!s_fota_ctx)
    {
        OTA_LOG("*** fota init fail ***");
        s_ota.stage = OTA_STAGE_FAILED;
        s_ota.progress = -1;
        return -1;
    }

    client = ql_http_client_init();
    if (!client)
    {
        OTA_LOG("*** http client init fail ***");
        goto fail;
    }

    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_ASYN, 0);              /* synchronous */
    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PDP_CID, OTA_PROFILE_IDX);
    ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PROTOCOL_VER, 1);

    OTA_LOG("GET %s", url);
    ret = ql_http_client_request(client, (char *)url,
                                 QL_HTTP_CLIENT_REQUEST_GET,
                                 QL_HTTP_CLIENT_AUTH_TYPE_NONE,
                                 NULL, NULL, NULL, 0,
                                 http_ota_resp_cb, NULL);
    OTA_LOG("request returned %d", ret);
    ql_http_client_release(client);

    if (s_ota.failed || !s_ota.body_done)
    {
        OTA_LOG("download incomplete");
        goto fail;
    }
    if (s_ota.total_len > 0 && s_ota.recv_len != s_ota.total_len)
    {
        OTA_LOG("size mismatch %d/%d", s_ota.recv_len, s_ota.total_len);
        goto fail;
    }

    s_ota.stage = OTA_STAGE_VERIFYING;
    if (ql_fota_image_flush(s_fota_ctx))
    {
        OTA_LOG("*** fota image flush fail ***");
        goto fail;
    }
    if (ql_fota_image_verify(s_fota_ctx))
    {
        OTA_LOG("*** fota image verify fail ***");
        goto fail;
    }

    s_ota.progress = 100;
    s_ota.stage = OTA_STAGE_DONE;
    ql_fota_deinit(s_fota_ctx);
    s_fota_ctx = NULL;
    OTA_LOG("package verified, update flag set");
    return 0;

fail:
    s_ota.stage = OTA_STAGE_FAILED;
    s_ota.progress = -1;
    ql_fota_deinit(s_fota_ctx);
    s_fota_ctx = NULL;
    return -1;
}

/* ---------------- worker task (runs once, then ends) ---------------- */
static void http_ota_task(void *argv)
{
    int tries, ok = 0;
    ql_task_t self;


    (void)argv;
    ql_rtos_task_sleep_s(6); 
    OTA_LOG("http ota start: %s", s_url);

    for (tries = 1; tries <= OTA_TRY_TIMES; tries++)
    {
        OTA_LOG("attempt %d/%d", tries, OTA_TRY_TIMES);

        if (http_ota_net_ready() == 0 && http_ota_download_once(s_url) == 0)
        {
            ok = 1;
            break;
        }
        ql_rtos_task_sleep_s(OTA_RETRY_DELAY_S);
    }

    if (ok)
    {
        OTA_LOG("OTA ok, rebooting to apply update");
        ql_rtos_task_sleep_s(5);
        OTA_SYSTEM_RESET();
        /* not reached */
    }

    OTA_LOG("*** OTA failed after %d attempts, staying on current firmware ***", OTA_TRY_TIMES);

    self = s_task;
    s_task = NULL;
    s_running = 0;                      /* the rest of the application keeps running */
    ql_rtos_task_delete(self);
}

/* ---------------- entry points ---------------- */
int http_ota_request(const char *url)
{
    int err;

        if (s_running)
    {
        OTA_LOG("HTTP FOTA already running, stage=%d recv=%d/%d",
                s_ota.stage, s_ota.recv_len, s_ota.total_len);
        return -1;
    }

    http_ota_build_url(url);
    s_running = 1;
    s_ota.progress = 0;

    err = ql_rtos_task_create(&s_task, OTA_TASK_STACK, OTA_TASK_PRIO,
                              "QfotaHttp", http_ota_task, NULL);
    if (err != 0)
    {
        OTA_LOG("created task failed (%d)", err);
        s_task = NULL;
        s_running = 0;
        return -2;
    }
    return 0;
}

void fota_http_app_init(void)
{
    http_ota_request(NULL);
}