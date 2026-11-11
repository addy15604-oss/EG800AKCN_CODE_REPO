/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include "ql_type.h"
#include "ql_func.h"
#include "ql_atcmd.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include <stdio.h>


static ql_atcmd_asyc_cb ql_atcmd_test_cb(QL_AT_RESP_STATUS_E errno,char * buffer,unsigned int len)
{
	printf("atcmd_test_cb:errno:%d, len:%d\r\nbuf: %s\r\n",errno, len, buffer);
}
static void ql_atcmd_test(void * argv)
{
	int ret = 0,i = 0;
	char test_at[][30] = {"ati\r\n","AT+CSQ\r\n","at+qeng=\"servingcell\"\r\n","AT+CFUN?\r\n","AT+MEDCR=0,88,3\r\n", "at*GRIP=?\r\n"};
	printf("atcmd test start\r\n");
	
	for(; i < sizeof(test_at)/30; i++){
		ret = ql_atcmd_async_init(NULL, 1024*4, ql_atcmd_test_cb);
		if(ret){
			printf("atcmd init fail:%d\r\n",ret);
			break;
		}
		ret = ql_atcmd_send_async(&test_at[i], sizeof(test_at[i]));
		if(ret){
			printf("%s send fail:%d\r\n",&test_at[i], ret);
			break;
		}
		ql_rtos_task_sleep_s(5);
		ret = ql_atcmd_async_close();
		if(ret){
			printf("atcmd close fail:%d\r\n",ret);
			break;
		}
	}
	
	printf("atcmd test end");
}
//application_init(ql_atcmd_test, "ql_atcmd_test", 2, 0);

