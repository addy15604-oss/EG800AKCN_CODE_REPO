#ifndef _INVERTER_OTA_H_
#define _INVERTER_OTA_H_

#include <stdint.h>
#include <stdbool.h>

/* Starts the OTA task (download firmware over HTTP, push it to the inverter over Modbus RTU). */
void ql_http_down_app_init(void);

void make_file_size_url(char *FileName);

/* true while an OTA is running: inverter_app.c must stop polling/using the UART meanwhile */
extern bool    OTA_START;

extern uint8_t OTA_MODBUS_TARGET;          /* 0 = comm card/dongle, 1 = control board */
extern char    INV_OTA_URL_SUFFIX[100];    /* firmware file name on the server        */
extern char    HTTP_OTA_FILE_URI[200];
extern char    HTTP_OTA_FILESIZE_URI[200];
extern char    HTTP_OTA_STATUS_URI[300];

/* defined in main_app.c */
extern int     INVERTER_OTA_FLAG;

#endif /* _INVERTER_OTA_H_ */