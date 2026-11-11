/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/


#include "ql_type.h"
#include "ql_func.h"
#include "ql_keypad.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include <stdio.h>

static ql_task_t key_padref = NULL;
static ql_queue_t key_padmsg = NULL;
static unsigned int test_callback(ql_event_data_t event)
{
	ql_rtos_queue_release(key_padmsg, sizeof(ql_event_data_t), (unsigned char *)&event, QL_WAIT_FOREVER);
    return 0;
}

static void key_events(void)
{
	ql_event_data_t key_num = {0};
	while(1){
		ql_rtos_queue_wait(key_padmsg, (unsigned char *)&key_num, sizeof(ql_event_data_t), QL_WAIT_FOREVER);

		printf("param_01:%d, param_02:%d, param_03:%d -%c*\r\n",key_num.param_01,key_num.param_02,key_num.param_03, key_num.event_id==90?'D':'U');
		printf("============================================\n");
		/*do 
			.....
		keypad func*/
	}
}

void keypad_test(void *argv)
{
	int ret = 0;
	printf("keypad_test start\n");
	int level = 2;
	ql_keypad_Register(test_callback);
//	ql_rtos_task_sleep_s(5);

	QL_KPAD_MKOUT_E row[QL_KEYPAD_MATRIX_LENGTH]= {QL_KP_OUT2,QL_KP_OUT0, QL_KP_OUT1, QL_KP_OUT3, QL_KP_OUT4,QL_KP_OUT_NO_VALID,
		                                           QL_KP_OUT_NO_VALID,QL_KP_OUT_NO_VALID};
	QL_KPAD_MKIN_E col[QL_KEYPAD_MATRIX_LENGTH] ={QL_KP_IN2,QL_KP_IN0, QL_KP_IN1, QL_KP_IN3,QL_KP_IN4,QL_KP_IN_NO_VALID, QL_KP_IN_NO_VALID,
		                                          QL_KP_IN_NO_VALID};
    ql_keypad_Config(0, 20, row, col);

	ql_keypad_MultikeyEn(1);

	printf("DBG======R07==FF==keypad_test \r\n");

	ret = ql_rtos_queue_create(&key_padmsg, sizeof(ql_event_data_t), 40);
	if(ret){
		printf("ql_rtos_queue_create error \n");
		return;
	}
	
	ql_rtos_task_create(&key_padref, 1024, 99, "key_events", key_events, NULL);
	ql_autosleep_enable(1);

}

//application_init(keypad_test, "keypad_test", 2, 2);


