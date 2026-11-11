/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_timer.c
 * @author  Juson.zhang
 * @version V1.0.0
 * @date    2020/04/02
 * @brief   This file tests timer's APIs
 ******************************************************************************
 */

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_type.h"
#include "ql_application.h"
#include "ql_timer.h"
#include "ql_log.h"
#include "ql_uart.h"


#define test_log(fmt, args...)	printf(fmt, ##args)
static ql_timer_t quec_task_timer = NULL;
void ql_task_test_cb(unsigned int param)
{
	test_log("quectel,%s\n",param);
	ql_norflash_do_erase("fota_param",0x00801A4000,0x6000);
}


static void quec_task_timer_test(void * argv)
{
	ql_debug_log_enable();
	ql_task_timer_create(&quec_task_timer);
	ql_task_timer_start(quec_task_timer, 1000,1, ql_task_test_cb,"quec_task_timer");
	while (1) {
		ql_rtos_task_sleep_s(3);
	}
}

static unsigned int g_timer_cnt = 0;
void ql_Acctimer_test_cb(unsigned int param)
{
	g_timer_cnt++;
}

static void quec_timer_test(void * argv)
{
	int timer_id = 0;
	timer_id = ql_start_Acctimer(QL_TIMER_PERIOD, 1000, ql_Acctimer_test_cb, 0);
	test_log("timer_id=%d \n", timer_id);

	while (1) {
		test_log("test 1ms timer, g_timer_cnt=%d \n", g_timer_cnt);
		ql_rtos_task_sleep_s(1);
	}
}

static ql_timer_t quec_rtos_timer = NULL;
static unsigned int rtos_timer_cnt = 0;
void ql_rtos_timer_test_cb(unsigned int param)
{
	rtos_timer_cnt++;
}

static void quec_rtos_timer_test(void * argv)
{
	ql_debug_log_enable();
	ql_rtos_timer_create(&quec_rtos_timer);
	ql_rtos_timer_start(quec_rtos_timer, 1000,1, ql_rtos_timer_test_cb,NULL);
	while (1) {
		test_log("test 1ms timer, rtos_timer_cnt=%d \n", rtos_timer_cnt);
		ql_rtos_task_sleep_s(1);
	}
}

//application_init(quec_timer_test, "quec_timer_test", 2, 0);//硬件定时器
//application_init(quec_rtos_timer_test, "quec_rtos_timer_test", 2, 0);//软件定时器，回调内不能执行延迟操作
//application_init(quec_task_timer_test, "quec_task_timer_test", 2, 0);//task_timer软件定时器,回调内可执行延迟操作


