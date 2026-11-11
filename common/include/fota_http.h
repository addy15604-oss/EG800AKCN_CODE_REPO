#ifndef _FOTA_HTTP_H
#define _FOTA_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Download URL = HTTP_DLOAD_URL + HTTP_FILE_NAME.
 * Read_Init_Data() overwrites them at boot from UFS key 11 (base url)
 * and key 8 (file name); both buffers are 100 bytes. */
extern char HTTP_DLOAD_URL[100];
extern char HTTP_FILE_NAME[100];

/* Start an HTTP FOTA with the url built from the two globals above
 * (same name and behaviour as before). Does nothing if one is running. */
void fota_http_app_init(void);

/* Same, but with an explicit full url; NULL/"" = use the globals.
 * Returns 0 if the task was started, -1 if already running, -2 on task error. */
int  http_ota_request(const char *url);

/* 1 while an HTTP FOTA task is running */
int  http_ota_is_busy(void);

/* 0..100 while downloading, -1 after a failure */
int  http_ota_get_progress(void);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* _FOTA_HTTP_H */