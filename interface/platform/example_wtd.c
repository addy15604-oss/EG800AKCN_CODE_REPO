/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_wtd.c
 * @author  
 * @version V1.0.0
 * @date    2023/10/09
 * @brief   This file tests WTD APIs
 ******************************************************************************
 */

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_wtd.h"
#include "ql_power.h"
#include "ql_dev.h"

ql_timer_t wtd_timer;
#define WTD_FEED_DOG_PERIOD 10

//@ It should be noted that in sleep mode, the hardware watchdog is used to ensure that it has not woken up after in sleep mode, 
//as long as there is a wake up, the watchdog timeout count will be reset
//Observe whether the module restarts if the watchdog times out
static void quec_wtd_sleep_test(void *argv)
{
	printf("========hardware watchdog test===== !\n");
	printf("========WATCHDOG OPEN===== !\n");
	ql_wtd_timeoutperiod_set(5);//feed dog 32s period
	ql_wtd_faultwake_enable(1);
	ql_wtd_enable(1);
	ql_wtd_feed();//after starting the WTD,the dog needs to be fed once before the timeout set by the regiter 0x11 will load
	ql_autosleep_enable(1);
	ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);//set cfun0
}

void quec_wtd_feed_timer(u32 argv)
{
	ql_wtd_feed();
}

static void quec_wtd_active_test(void * argv)
{
	int time= 0;
	//create a timer to feed dog regularly
	ql_rtos_timer_create(&wtd_timer);
	while(1)
	{
		printf("========hardware watchdog test===== !\n");	
		printf("========WATCHDOG OPEN :[%d]===== !\n",time++);
		//enable watchdog
	    ql_wtd_timeoutperiod_set(4);//feed dog 16s period
		ql_wtd_faultwake_enable(1);
		ql_wtd_enable(1);
		ql_wtd_feed();//after starting the WTD,the dog needs to be fed once before the timeout set by the regiter 0x11 will load
		//start timer to feed dog
		ql_rtos_timer_start(wtd_timer, WTD_FEED_DOG_PERIOD, 1, quec_wtd_feed_timer, 0);
		ql_rtos_task_sleep_s(60);
		//diable watchdog
		printf("========WATCHDOG CLOSE!!===== !\n");
		ql_wtd_faultwake_enable(0);
		ql_wtd_enable(0);
		//stop timer
		ql_rtos_timer_stop(wtd_timer);
		ql_rtos_task_sleep_s(2);
	}
}

//application_init(quec_wtd_active_test, "quec_wtd_test", 2, 10);


