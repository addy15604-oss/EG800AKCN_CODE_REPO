/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_dev.h"



static void ql_dev_test_task(void * argv)
{
	int ret = 0,j=0;
	char data[64] = {0};
	QL_DEV_MODEM_FUNCTION function = 0;
	int current_fun=0;
	int rst = 0;
	ql_rtos_task_sleep_s(5);
	while(j<10000)
	{
	 	ql_rtos_task_sleep_s(5);
		printf("\n==========Device info test[start]==========\n");
		
		ret = ql_dev_get_imei(data,sizeof(data));
		printf("ql_dev_get_imei ret=%d; IMEI:%s \n",ret,data);
		memset(data,0,sizeof(data));

		ret = ql_dev_get_model(data,sizeof(data));
		printf("ql_dev_get_model ret=%d; model:%s \n",ret,data);
		memset(data,0,sizeof(data));

		ret = ql_dev_get_sn(data,sizeof(data));
		printf("ql_dev_get_sn ret=%d; sn:%s \n",ret,data);
		memset(data,0,sizeof(data));

		ret = ql_dev_get_firmware_version(data,sizeof(data));
		printf("ql_dev_get_firmware_version ret=%d; firmware_version:%s \n",ret,data);
		memset(data,0,sizeof(data));
   
        ret = ql_dev_get_product_id(data,sizeof(data));	
		printf("ql_dev_get_product_id ret=%d; product_id:%s \n",ret,data);
		memset(data,0,sizeof(data));

		if(function==5)
			function=0;
		else 
			function++;
		//ret = ql_dev_set_modem_fun(function,rst);
		printf("ql_dev_get_modem_fun ret=%d; modem_fun:%d; rst:%d\n",ret,function,rst);
		printf("==========Device info test[end]==========\n\n");
		j++;
	}
}

//application_init(ql_dev_test_task, "ql_dev_test_task", 2, 0);



static void ql_dev_get_sn2(void * argv)
{
	unsigned int ret=-1;
	unsigned char mac_val[20] = {0};		//至少17个字�?
	printf("\n==========ql_dev_get_sn_2[start]==========\n");
	
	while(1)
	{
		ql_rtos_task_sleep_s(1);
		ret =ql_dev_get_sn_2(mac_val,sizeof(mac_val));
		printf("ret=%d; mac_val:%s \n",ret,mac_val);
	}
}

//application_init(ql_dev_get_sn2, "ql_dev_get_sn2", 2, 0);

/* add  ql_dev_get_qverinfo version function*/
static void quec_version_test(void * argv)
{
	ql_rtos_task_sleep_s(1);
	char tmpBuf1[512]={0};
	int ret= -1;
	ret=ql_dev_get_qverinfo(tmpBuf1,sizeof(tmpBuf1)-1);
	ql_rtos_task_sleep_s(2);
    printf("=========print version test==========\r\n");
	printf("release_version: %s\r\n%d\r\n",tmpBuf1,ret);

  
}
//application_init(quec_version_test, "quec_version_test", 2, 0);

