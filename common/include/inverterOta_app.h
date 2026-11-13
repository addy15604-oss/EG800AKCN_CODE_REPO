#ifndef INVERTER_OTA_H
#define INVERTER_OTA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- Public state / config (defined in inverterOta.c) ---------------- */
extern bool     OTA_START;              /* true while an OTA is running; pause the inverter task on it */
extern uint8_t  OTA_MODBUS_TARGET;      /* 0 = comm card / dongle, 1 = attached control board (DSP)    */
extern uint16_t FW_TYPE;                /* firmware type reported to the server status API             */
extern uint32_t OTA_Hint_Baud;          /* baud the inverter task locked on (0 = unknown)              */

extern char HTTP_OTA_FILE_URI[200];     /* e.g. "http://eastmanone.com:3001/download/"                 */
extern char HTTP_OTA_FILESIZE_URI[200];
extern char INV_OTA_URL_SUFFIX[100];    /* firmware file name on the server                            */
extern char HTTP_OTA_STATUS_URI[300];

/* ---------------- Public API ---------------- */
void ql_http_down_app_init(void);               /* start the OTA task                                  */
void make_file_size_url(char *FileName);        /* rebuild HTTP_OTA_FILESIZE_URI for a file name        */
void bk_reboot(void);                           /* 3 s delay, then normal module reset                  */

/* Call from your UART RX callback for every chunk read from the RS485 port */
void ota_uart_rx_feed(const uint8_t *data, uint32_t len);

/* ---------------- Port layer: implement these in inverter_app.c / board code ----------------
 * ota_uart_open  : (re)open RS485 UART at 'baud', 8N1, no flow control, register an RX
 *                  callback that calls ota_uart_rx_feed(). Return 0 on success.
 * ota_uart_close : close the UART.
 * ota_uart_write : blocking write, returns bytes written (<0 on error).
 * ota_rede       : drive RS485 DE/RE pin (1 = transmit, 0 = receive).
 */
int  ota_uart_open(uint32_t baud);
void ota_uart_close(void);
int  ota_uart_write(const uint8_t *data, uint32_t len);
void ota_rede(int level);

#ifdef __cplusplus
}
#endif

#endif /* INVERTER_OTA_H */