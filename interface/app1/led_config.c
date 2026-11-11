/*================================================================
  led_config.c - GPIO68 blink LED + GPIO70 solid LED

    SIM missing   : 68 error blink,  70 OFF
    Searching     : 68 slow blink,   70 OFF
    Registered    : 68 OFF,          70 solid ON
    MQTT online   : 68 OFF,          70 solid ON
    On each publish (while registered/online):
                    68 fast blink for LED_ACTIVITY_MS, 70 stays ON
=================================================================*/
#include <stdio.h>
#include <string.h>

#include "ql_rtos.h"
#include "ql_gpio.h"
#include "ql_data_call.h"

#include "led_config.h"

#define LED_TASK_STACK          (4 * 1024)
#define POLL_TASK_STACK         (8 * 1024)
#define LED_TICK_MS             25

typedef struct
{
    unsigned int period_ms;     /* 0 = LED off                     */
    unsigned int on_ms;         /* on_ms >= period_ms = solid on   */
} led_pattern_t;

// static const led_pattern_t PAT_OFF    = { 0, 0 };
static const led_pattern_t PAT_SIMERR = { LED_SIMERR_PERIOD_MS, LED_SIMERR_ON_MS };
static const led_pattern_t PAT_SLOW   = { LED_SLOW_PERIOD_MS,   LED_SLOW_ON_MS   };
static const led_pattern_t PAT_SPEED  = { LED_SPEED_PERIOD_MS,  LED_SPEED_ON_MS  };

static ql_task_t                s_led_task    = NULL;
static volatile led_state_e     s_state       = LED_ST_SEARCHING;
static volatile unsigned int    s_activity_ms = 0;

static int s_blink_ok = 0;      /* 1 = GPIO68 initialised */
static int s_solid_ok = 0;      /* 1 = GPIO70 initialised */

/* stored only so the old API keeps working */
static volatile int             s_inverter_comm_ok = 0;
static volatile int             s_inverter_fault   = 0;
static volatile int             s_inverter_running = 0;

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
void led_config_data_activity(void)
{
    s_activity_ms = LED_ACTIVITY_MS;
}

void led_config_set_state(led_state_e st)
{
    if (s_state != st)
        printf("[LED] state %d -> %d (0=no SIM,1=searching,2=registered,3=online)\r\n",
               (int)s_state, (int)st);
    s_state = st;
}

void led_config_set_inverter_status(int comm_ok, int fault, int running)
{
    s_inverter_comm_ok = comm_ok;
    s_inverter_fault   = fault;
    s_inverter_running = running;
}

#if LED_POLL_ENABLE
static ql_task_t s_poll_task = NULL;

static led_state_e read_net_state(void)
{
    struct ql_data_call_info info;

#if LED_SIM_CHECK_ENABLE
    if (!LED_SIM_PRESENT())
        return LED_ST_SIM_MISSING;
#endif

    if (ql_network_register_wait(LED_REG_POLL_TIMEOUT_S) != 0)
        return LED_ST_SEARCHING;

    memset(&info, 0, sizeof(info));
    ql_get_data_call_info(LED_NET_PROFILE_IDX, 0, &info);
    if (info.v4.state == 1)
        return LED_ST_ONLINE;

    return LED_ST_REGISTERED;
}

static void net_state_thread(void *param)
{
    (void)param;
    while (1)
    {
        led_config_set_state(read_net_state());
        ql_rtos_task_sleep_ms(LED_POLL_INTERVAL_MS);
    }
}
#endif /* LED_POLL_ENABLE */

/* -------------------------------------------------------------------------
 * GPIO init
 * ------------------------------------------------------------------------- */
static int led_gpio_init_one(int gpio, int use_pin_func, int pin_num, int pin_func)
{
    int ret;

    if (use_pin_func)
        ql_pin_set_func(pin_num, pin_func);

    ql_gpio_deinit(gpio);
    ret = ql_gpio_init(gpio, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, LED_INIT_LEVEL);
    if (ret < 0)
        printf("LED gpio %d init failed: %d\r\n", gpio, ret);
    return ret;
}

static int led_gpio_init(void)
{
    s_blink_ok = (led_gpio_init_one(LED_BLINK_Pin, LED_BLINK_USE_PIN_FUNC,
                                    LED_BLINK_PIN_NUM, LED_BLINK_PIN_FUNC) >= 0);
    s_solid_ok = (led_gpio_init_one(LED_SOLID_Pin, LED_SOLID_USE_PIN_FUNC,
                                    LED_SOLID_PIN_NUM, LED_SOLID_PIN_FUNC) >= 0);

    printf("[LED] init: blink(GPIO%d)=%s solid(GPIO%d)=%s\r\n",
           (int)LED_BLINK_Pin, s_blink_ok ? "OK" : "FAIL",
           (int)LED_SOLID_Pin, s_solid_ok ? "OK" : "FAIL");

    return (s_blink_ok || s_solid_ok) ? 0 : -1;
}

/* -------------------------------------------------------------------------
 * Drive GPIO68 from a pattern at time t_ms
 * ------------------------------------------------------------------------- */
static void blink_led_write(const led_pattern_t *p, unsigned int t_ms)
{
    if (!s_blink_ok)
        return;

    if (p->period_ms == 0)
        LED_BLINK_OFF;
    else if (p->on_ms >= p->period_ms)
        LED_BLINK_ON;
    else if ((t_ms % p->period_ms) < p->on_ms)
        LED_BLINK_ON;
    else
        LED_BLINK_OFF;
}

/* -------------------------------------------------------------------------
 * LED task
 * ------------------------------------------------------------------------- */
static void led_thread(void *param)
{
    unsigned int t_ms = 0;
    const led_pattern_t *last_p = NULL;
    (void)param;

    if (led_gpio_init() < 0)
    {
        printf("[LED] both pins failed, LED task exiting\r\n");
        ql_rtos_task_delete(NULL);
        return;
    }

    /* boot self-test: both LEDs ON for 1 s */
    if (s_blink_ok) LED_BLINK_ON;
    if (s_solid_ok) LED_SOLID_ON;
    ql_rtos_task_sleep_ms(1000);
    if (s_blink_ok) LED_BLINK_OFF;
    if (s_solid_ok) LED_SOLID_OFF;

    while (1)
    {
        led_state_e st = s_state;
        const led_pattern_t *p;
        int solid_on;

        switch (st)
        {
            case LED_ST_SIM_MISSING:
                p = &PAT_SIMERR;
                solid_on = 0;
                s_activity_ms = 0;
                break;

            case LED_ST_SEARCHING:
                p = &PAT_SLOW;
                solid_on = 0;
                s_activity_ms = 0;
                break;

            case LED_ST_REGISTERED:
            case LED_ST_ONLINE:
            default:
                solid_on = 1;
                if (s_activity_ms > 0)
                {
                    p = &PAT_SPEED;      /* fast blink after a publish */
                    s_activity_ms = (s_activity_ms > LED_TICK_MS) ?
                                    (s_activity_ms - LED_TICK_MS) : 0;
                }
                else
                {
                    p = &PAT_SLOW;
                }
                break;
        }

        if (p != last_p)
        {
            last_p = p;
            t_ms = 0;                    /* restart the pattern on a change */
        }

        blink_led_write(p, t_ms);

        if (s_solid_ok)
        {
            if (solid_on) LED_SOLID_ON;
            else          LED_SOLID_OFF;
        }

        t_ms += LED_TICK_MS;
        if (t_ms >= 60000)               /* multiple of every period used */
            t_ms = 0;

        ql_rtos_task_sleep_ms(LED_TICK_MS);
    }
}

/* -------------------------------------------------------------------------
 * Called from inverter_app_init()
 * ------------------------------------------------------------------------- */
int led_config_init(void)
{
    int err;

    printf("[LED] led_config_init called\r\n");

    err = ql_rtos_task_create(&s_led_task, LED_TASK_STACK, 100,
                              "net_led", led_thread, NULL, 1);
    if (err != 0)
    {
        printf("LED task create failed 0x%x\r\n", err);
        return err;
    }

#if LED_POLL_ENABLE
    err = ql_rtos_task_create(&s_poll_task, POLL_TASK_STACK, 100,
                              "net_state", net_state_thread, NULL, 1);
    if (err != 0)
        printf("LED state task create failed 0x%x\r\n", err);
#endif

    return err;
}