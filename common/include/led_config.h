/*================================================================
  led_config.h - two status LEDs

    LED_BLINK_Pin (GPIO68): slow blink  = searching for network
                            fast blink  = MQTT data just published
                            error blink = SIM missing
    LED_SOLID_Pin (GPIO70): solid ON    = network registered / internet up

  All prototypes and led_state_e live here only.
=================================================================*/
#ifndef _LED_CONFIG_H_
#define _LED_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ql_gpio.h"

/* =========================================================================
 * LED pins
 * ========================================================================= */
#define LED_BLINK_Pin           GPIO_PIN_NO_68
#define LED_SOLID_Pin           GPIO_PIN_NO_70

#define LED_BLINK_ON            ql_gpio_set_level(LED_BLINK_Pin, PIN_LEVEL_HIGH)
#define LED_BLINK_OFF           ql_gpio_set_level(LED_BLINK_Pin, PIN_LEVEL_LOW)
#define LED_SOLID_ON            ql_gpio_set_level(LED_SOLID_Pin, PIN_LEVEL_HIGH)
#define LED_SOLID_OFF           ql_gpio_set_level(LED_SOLID_Pin, PIN_LEVEL_LOW)

/* Level at init (= the OFF level). Active-low LEDs: swap HIGH/LOW in the
 * four macros above and set this to PIN_LEVEL_HIGH. */
#define LED_INIT_LEVEL          PIN_LEVEL_LOW

/* =========================================================================
 * Pin mux: ql_pin_set_func(module_pin_number, func) before ql_gpio_init.
 * These are MODULE pin numbers (see the EG800AK hardware design PDF),
 * not GPIO numbers.
 * ========================================================================= */
#define LED_BLINK_USE_PIN_FUNC  1
#define LED_BLINK_PIN_NUM       4       /* module pin that carries GPIO68 */
#define LED_BLINK_PIN_FUNC      0

#define LED_SOLID_USE_PIN_FUNC  0       /* set to 1 once you know the module pin for GPIO70 */
#define LED_SOLID_PIN_NUM       0
#define LED_SOLID_PIN_FUNC      0

/* =========================================================================
 * Network state (set from mqtt_app.c)
 * ========================================================================= */
typedef enum
{
    LED_ST_SIM_MISSING = 0,     /* GPIO68 error pattern, GPIO70 off           */
    LED_ST_SEARCHING,           /* GPIO68 slow blink,    GPIO70 off           */
    LED_ST_REGISTERED,          /* GPIO70 solid ON (internet available)       */
    LED_ST_ONLINE               /* GPIO70 solid ON (MQTT connected)           */
} led_state_e;

/* =========================================================================
 * Options
 * ========================================================================= */
#define LED_POLL_ENABLE         0       /* 0 = state comes from mqtt_app.c events */

/* Only used when LED_POLL_ENABLE is 1 */
#define LED_NET_PROFILE_IDX         1
#define LED_POLL_INTERVAL_MS        2000
#define LED_REG_POLL_TIMEOUT_S      1
#define LED_SIM_CHECK_ENABLE        0
#define LED_SIM_PRESENT()           (1)

/* How long the fast publish blink stays after each publish */
#define LED_ACTIVITY_MS             1000

/* =========================================================================
 * Blink patterns on GPIO68: period / on-time in ms
 * ========================================================================= */
#define LED_SIMERR_PERIOD_MS    600
#define LED_SIMERR_ON_MS        100

#define LED_SLOW_PERIOD_MS      900    /* searching */
#define LED_SLOW_ON_MS          300

#define LED_SPEED_PERIOD_MS     125     /* publish: fast blink */
#define LED_SPEED_ON_MS         62

/* =========================================================================
 * API
 * ========================================================================= */
int  led_config_init(void);                     /* from inverter_app_init()      */
void led_config_set_state(led_state_e st);      /* from mqtt_app.c               */
void led_config_data_activity(void);            /* from mqtt_app.c, per publish  */

/* Kept so inverter_app.c still links. Values are stored but not used by
 * the LEDs in this version. */
void led_config_set_inverter_status(int comm_ok, int fault, int running);

#ifdef __cplusplus
}
#endif

#endif /* _LED_CONFIG_H_ */