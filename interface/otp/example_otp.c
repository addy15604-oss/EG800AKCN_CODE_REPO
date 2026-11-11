/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include "ql_application.h"
#include "ql_otp.h"

//NOTES:If secboot is enabled, operation of bank1 and bank2 areas is prohibited 
//如果使能了secboot,禁止操作bank1和bank2区域
/*Fuse的每个Bit写1有效，而且只能写一次*/
/*当板子使能了secboot，禁止向bank1、bank2写数据，会导致板子启动不了且不能恢复！！！*/
/*当板子使能了secboot，禁止向bank1、bank2写数据，会导致板子启动不了且不能恢复！！！*/
/*当板子使能了secboot，禁止向bank1、bank2写数据，会导致板子启动不了且不能恢复！！！*/
void quec_otp_bank1_test(void * argv)
{
	int ret = 0;
	unsigned int bank1_key1_t1=0;
	unsigned int bank1_key1_t2=0;
	
	unsigned int bank1_key1=0;
	
	ql_rtos_task_sleep_s(10);
	printf("<------quec_otp_bank1_test------>\r\n");
	
	ret = ql_otp_read(GEU_FUSE_VAL_OEM_HASH_KEY1,&bank1_key1);
	if(ret<0)
		printf("ERR:first:ql_otp_read()-GEU_FUSE_VAL_ROOT_KEY1 fail ret=%d\r\n",ret);
	
	printf("Get GEU_FUSE_VAL_ROOT_KEY1 bank1_key1=0x%x\r\n",bank1_key1);

	bank1_key1_t1 |= (1<<31);//set bit31 to 1
	printf("Will Set BANK1 Bit31 to 1\r\n");
	ret = ql_otp_write(GEU_FUSE_VAL_OEM_HASH_KEY1,bank1_key1_t1);
	if(ret<0)
		printf("ERR:first:ql_otp_write()-GEU_FUSE_VAL_ROOT_KEY1 fail ret=%d\r\n",ret);
	bank1_key1_t1 = 0;
	ret = ql_otp_read(GEU_FUSE_VAL_OEM_HASH_KEY1,&bank1_key1_t1);
	if(ret<0)
		printf("ERR:second:ql_otp_read()-GEU_FUSE_VAL_ROOT_KEY1 fail ret=%d\r\n",ret);

	printf("Get bank1_key1_t1(0x%x)\r\n",bank1_key1_t1);

	bank1_key1_t2 |= (1<<0);//set bit0 to 1
	printf("Will Set BANK1 Bit0 to 1\r\n");
	ret = ql_otp_write(GEU_FUSE_VAL_OEM_HASH_KEY1,bank1_key1_t2);
	if(ret<0)
		printf("ERR:second:ql_otp_write()-GEU_FUSE_VAL_ROOT_KEY1 fail ret=%d\r\n",ret);

	ret = ql_otp_read(GEU_FUSE_VAL_OEM_HASH_KEY1,&bank1_key1);
	if(ret<0)
		printf("ERR:3:ql_otp_read()-GEU_FUSE_VAL_ROOT_KEY1 fail ret=%d\r\n",ret);
	
	printf("Get bank1_key1(0x%x)\r\n",bank1_key1);

	for(;;)
	{
		if(1 == ((bank1_key1>>31)&1))
			printf("This is 'A'\r\n");

		if(1 == ((bank1_key1>>0)&1))
			printf("This is 'B'\r\n");

		if((1 == ((bank1_key1>>0)&1))&&(1 == ((bank1_key1>>31)&1)))
			printf("This is 'C'\r\n");
		
 		ql_rtos_task_sleep_s(3);
		printf("this is bank1 tast\r\n");
	}
}


//read-write interface to OTP
static void quec_otp_test(void * argv)
{
	ql_rtos_task_sleep_s(10);
	printf("<------example_otp.c------>\r\n");

	unsigned int fuse_id = 0;
	ql_otp_read(GEU_FUSE_ADDR_READ_USE1,&fuse_id);
	printf("0x%x\n",fuse_id);

	fuse_id = 0x1;
	ql_otp_write(GEU_FUSE_ADDR_WRITE_USE1,fuse_id);
	fuse_id = 0;	
	
	ql_otp_read(GEU_FUSE_ADDR_READ_USE1,&fuse_id);
	printf("0x%x\n",fuse_id);
}

// application_init(quec_otp_test, "quec_otp_test", 2, 0);
// application_init(quec_otp_bank1_test, "quec_otp_test", 2, 0);


