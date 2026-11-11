#ifndef _FOTA_F_DEMO_H
#define _FOTA_F_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Remote path of the firmware package on the FTP server, e.g.
 * "newout2.pack" or "fw/newout2.pack". Overwritten at boot from UFS
 * key 8 by Read_Init_Data(), so the buffer is 128 bytes (>= 100 + '\0'). */
extern char file_full_path[128];

/* Start the FTP FOTA task (runs once, then ends). Safe to call again:
 * it does nothing while a FOTA task is already running. */
void ql_fota_app_init(void);

/* 1 while the FOTA task is running */
int  fota_f_is_busy(void);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* _FOTA_F_DEMO_H */