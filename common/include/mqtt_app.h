/**
  @file    mqtt_app.h
  @brief   MQTT client task + JSON payload builders for Eastman One server.
           Ported from EC200 to EG800.
*/

/*============================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 ============================================================================*/

#ifndef MQTT_APP_H
#define MQTT_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ql_type.h"

/* =========================================================================
 * Shared globals between main.c / inverter.c / mqtt.c
 * ========================================================================= */

/* Connection state (read-only for other modules) */
extern int  mqtt_connected;              /* 1 when connected to broker */

/* Broker credentials — defined in mqtt.c */
extern char IMEI_NO[20];
extern char PASSWORD[20];

/* Buffers holding the most recent JSON payload built by the Create_* helpers.
 * Exposed here so the server-triggered config publish path in mqtt.c can see
 * them, and so any future command handler in another module can inspect them. */
extern char MQTT_CLIENT_ID[50];
extern char EASTMAN_INVERTER_MESSAGE[1200];
extern char EASTMAN_LOGGER_MESSAGE[300];
extern char EASTMAN_CONFIG_MESSAGE[200];

/* =========================================================================
 * Task creation
 * ========================================================================= */

/* Called by main.c's mqtt_launcher. Creates the "QmqttApp" RTOS task which
 * runs mqtt_app_thread(). Returns QL_OSI_SUCCESS on success.
 * This is the entry point the application_init() pattern uses. */
int ql_mqtt_app_init(void);

/* Legacy alias — kept because your original project may still call it from
 * older code paths. Functionally identical to ql_mqtt_app_init(). */
int ql_mqtt_test_init(void);

/* =========================================================================
 * JSON payload builders
 *
 * Each returns 1 on success and writes a NUL-terminated JSON object into the
 * caller-supplied buffer. Sizing:
 *   - Create_inverter_data:  needs EASTMAN_INVERTER_MESSAGE (1200 B)
 *   - create_logger_data:    needs EASTMAN_LOGGER_MESSAGE   (300 B)
 *   - create_config_data*:   need  EASTMAN_CONFIG_MESSAGE   (200 B)
 * ========================================================================= */

/* Full inverter telemetry snapshot (MAC, all voltages/currents/powers/faults). */
unsigned char Create_inverter_data(char *Inverter_Data);

/* Logger payload (MAC, datetime, operator, RSSI, firmware, GPS, interval). */
unsigned char create_logger_data(char *Logger_Data);

/* Server response for a numeric config-parameter read (e.g. "DCVUPLI1":42.5). */
char create_config_data(char *Config_Data, char *Config_Key, double Config_Value);

/* Server response for a string config-parameter read (e.g. "INV_SERIAL"). */
char create_config_data_str(char *Config_Data, char *Config_Key, char *Config_Value);

/* =========================================================================
 * Incoming message handling
 * ========================================================================= */

/* Entry point for the MQTT in-publish callback. Extracts the JSON object
 * from `buffer`, calls GetKeyValue() → Seprate_Key_Value() which sets the
 * PARAM_CONFIG_MODE / CONFIG_WRITE_VAR / RX_CONFIG_VALUE flags consumed by
 * the inverter state machine. */
void mqtt_recv(const unsigned char *buffer);

/* Key/value extractor — public for debugging. Normally only mqtt_recv()
 * calls this. */
int  GetKeyValue(char *String);

/* Callback payload handler — for each "key":"value" pair, decides which
 * inverter parameter is being read or written. */
void Seprate_Key_Value(char *Key, char *Value);

/* =========================================================================
 * Client ID helper
 * ========================================================================= */

/* Builds "EPTPL" + IMEI into `id` (max 50 chars). */
void Create_Client_Id(char *id);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_APP_H */