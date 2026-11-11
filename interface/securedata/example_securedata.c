/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_securedata.c
 * @author  yang.liu
 * @version V1.0.0
 * @date    2021/10/27
 * @brief   This file tests securedata
 ******************************************************************************
 */
#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_securedata.h"

static void ql_securedata_task_entry(void * argv)
{
	char a = 'a'; 
	int i = 1;
	printf("-----------------start---------------------\n");
	int cnt = 0;
	while(cnt++ < 10)
	{
		char *p1 = malloc(52);
		memset(p1,a++,52);
		ql_securedata_store(i,p1,52);
		ql_rtos_task_sleep_ms(100);
		printf("wid %d buf:%s\n",i,p1);
		free(p1);
		
		char *p2 = malloc(52);
		memset(p2,0,52);
		ql_securedata_read(i,p2,52);
		ql_rtos_task_sleep_ms(100);
		printf("rid %d: buf:%s\n",i,p2);
		free(p2);
	}
	
	printf("-----------------stop---------------------\n");
}

//application_init(ql_securedata_task_entry, "quec_securedata_test", 2, 0);
