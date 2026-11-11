/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

/*读取写入速度量化(xxxKB/s)：
1. open a file by w+
2. get time ticks before write data
3. write 4096 byte data 
4. get time ticks after write data,and calculate the speed 
5. get time ticks before read, read data, get time ticks after read
6. calculate read speed
*/

#include <stdio.h>
#include "ql_fs.h"
#include "ql_flash.h"
#include "ql_application.h"
#include "ql_rtos.h"
#include "ql_rtc.h"
#include "ql_fs.h"

#define CHIP_SECTOR_SIZE 		  0x1000

#define TEST_TIME 			0xFFFFFFFF     //总共测试的次数
#define TEST_TIME_DELAY     5         //每次独写之间间隔时间s

#define FLASH_TYPE			"fota_param"      //选择读写的flash分区

#define FILE_CONTENT_LEN    4096 
#define FILE_CONTENT 		"123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"\
                            "123456789012345678901234567890123456789012345678901234567890123"
                            
                            

#define FILE_CONTENT1 	    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"\
                           "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()~<>"
                               
                            



int32_t nor_flash_read(uint32_t offset, void *buf, uint32_t nbytes) 
{
	return ql_norflash_do_read(FLASH_TYPE, offset, buf, nbytes);
    //return hal_flash_read((hal_partition_t)KV_PARTITION, &offset, buf, nbytes);

}
int32_t nor_flash_write(uint32_t offset, void *buf, uint32_t nbytes) 

{

	return ql_norflash_do_write(FLASH_TYPE, offset, buf, nbytes);
    //return hal_flash_read((hal_partition_t)KV_PARTITION, &offset, buf, nbytes);

}
int32_t nor_flash_erase(uint32_t offset, uint32_t size)

{

	return ql_norflash_do_erase(FLASH_TYPE, offset, size);
    //return hal_flash_erase((hal_partition_t)KV_PARTITION, offset, size);

}



static void WR_test_100Free(unsigned int i)
{	
    uint32_t  start_address = 0;
    uint32_t  size = 0;
    char buf[FILE_CONTENT_LEN] = {0};
	int ret = -1;
	float ticks_0_w, ticks_1_w, ticks_2_w, ticks_3_w;
	float speed_w, speed_r;

    ql_norflash_get_addr(FLASH_TYPE, &start_address, &size);
    printf("start_address: 0x%08x, size: %d\r\n", start_address, size);

    
    ticks_0_w = ql_rtc_get_ticks();
	ret = nor_flash_erase(start_address, FILE_CONTENT_LEN);
	ticks_1_w = ql_rtc_get_ticks();
    if(ret == 0)
    {
        printf("flash erase succeed\r\n");
    }
    else 
    {
        printf("flash_erase_error=%d\r\n ", ret);
    }
	printf("[flash_Speed] ========== ql_rtc_get_ticks ticks_1=%.2f\r\n", ticks_1_w);
	printf("[flash_Speed] ========== Delta ticks(Write_100Free)=%.2f\r\n", ticks_1_w - ticks_0_w);
	speed_w = 32768.00*4/(ticks_1_w - ticks_0_w);     //speed_w (KB) = write times * 4096 / delta ticks /32768 /1024
	printf("[flash_Speed] ========== Erase_Speed_100Free=%.2fKB/s\r\n", speed_w);

    memset(buf, 0, FILE_CONTENT_LEN);
   	ret=nor_flash_read(	start_address, buf, FILE_CONTENT_LEN);
                printf("read status: %d\r\n", ret);
                printf("readbuff = %s\r\n",buf);
   
   
    if(i%2)
    {
    	ticks_0_w = ql_rtc_get_ticks();
    	ret=nor_flash_write(start_address, FILE_CONTENT, FILE_CONTENT_LEN);
    	ticks_1_w = ql_rtc_get_ticks();
        printf("write status: %d\r\n", ret);
    	printf("[flash_Speed] ========== ql_rtc_get_ticks ticks_1=%.2f\r\n", ticks_1_w);
    	printf("[flash_Speed] ========== Delta ticks(Write_100Free)=%.2f\r\n", ticks_1_w - ticks_0_w);
    	speed_w = 32768.00*4/(ticks_1_w - ticks_0_w);     //speed_w (KB) = write times * 4096 / delta ticks /32768 /1024
    	printf("[flash_Speed] ========== Write_Speed_100Free=%.2fKB/s\r\n", speed_w);
    }
    else 
    {
        ticks_0_w = ql_rtc_get_ticks();
        ret=nor_flash_write(start_address, FILE_CONTENT1, FILE_CONTENT_LEN);
        ticks_1_w = ql_rtc_get_ticks();
        printf("write status: %d\r\n", ret);
        printf("[flash_Speed] ========== ql_rtc_get_ticks ticks_1=%.2f\r\n", ticks_1_w);
        printf("[flash_Speed] ========== Delta ticks(Write_100Free)=%.2f\r\n", ticks_1_w - ticks_0_w);
        speed_w = 32768.00*4/(ticks_1_w - ticks_0_w);     //speed_w (KB) = write times * 4096 / delta ticks /32768 /1024
        printf("[flash_Speed] ========== Write_Speed_100Free=%.2fKB/s\r\n", speed_w);
    }

    memset(buf, 0, FILE_CONTENT_LEN);
	ticks_2_w = ql_rtc_get_ticks();
	ret=nor_flash_read(	start_address, buf, FILE_CONTENT_LEN);
	ticks_3_w = ql_rtc_get_ticks();
    printf("read status: %d\r\n", ret);
    printf("readbuff = %s\r\n",buf);
    printf("[flash_Speed] ========== ql_rtc_get_ticks ticks_2=%.2f\r\n", ticks_2_w);
	printf("[flash_Speed] ========== ql_rtc_get_ticks ticks_3=%.2f\r\n", ticks_3_w);
	printf("[flash_Speed] ========== Delta ticks(Read_100Free)=%.2f\r\n", ticks_3_w - ticks_2_w);
	speed_r = 32768.00*4/(ticks_3_w - ticks_2_w);
	printf("[flash_Speed] ========== Read_Speed_100Free=%.2fKB/s\r\n", speed_r);


}

#define TONGJI_UNIT 50

static void WR_test_E_W_R_JiSuan_Ms_Time(void)
{	
    uint32_t  start_address = 0;
    uint32_t  size = 0;
    unsigned int offset=0;
	unsigned int cnt = 0;
    char buf[FILE_CONTENT_LEN+1] = {0};
    char w_buf_0[FILE_CONTENT_LEN+1] = {0};
    char w_buf_1[FILE_CONTENT_LEN+1] = {0};
	
	int ret = -1;
	float ticks_0_w, ticks_1_w, ticks_2_w, ticks_3_w;
	float speed_w, speed_r;

	float speed_e_50 =0,speed_w_50=0, speed_r_50=0;

   	memset(buf, 0, FILE_CONTENT_LEN);
   	memset(w_buf_0, 0x55, FILE_CONTENT_LEN);
   	memset(w_buf_1, 0xAA, FILE_CONTENT_LEN);

	printf("[flash_Speed] ========== flash_speed test Start\r\n");

    ql_norflash_get_addr(FLASH_TYPE, &start_address, &size);
    printf("start_address: 0x%08x, size: %d\r\n", start_address, size);
	
	for (unsigned int i = 1; i < TEST_TIME; i++)
	{
        if(offset<size-4096)
        {
            offset+=4096;
        }
        else 
        {
            offset=0;
        }
		/////1、擦除时间统计
	    ticks_0_w = ql_rtc_get_ticks();
		ret = nor_flash_erase(start_address+offset, FILE_CONTENT_LEN);
		ticks_1_w = ql_rtc_get_ticks();
	    if(ret == 0)
	    {
	        //printf("flash erase succeed\r\n");
	    }
	    else 
	    {
	        printf("ERR:flash_erase_error=%d\r\n ", ret);
			while(1)
			{
				ql_rtos_task_sleep_s(1);
			}

	    }
		speed_w = ((ticks_1_w - ticks_0_w)*1000)/32768.00;     //擦除4K 花费的时间
		//printf("[flash_Speed] ========== Erase_Speed_100Free=%.2fms\r\n", speed_w);
		speed_e_50+=speed_w;

		/////2、写时间统计
		ticks_0_w = ql_rtc_get_ticks();
	    if(i%2)
	    	ret=nor_flash_write(start_address+offset, w_buf_0, FILE_CONTENT_LEN);
	    else 
	        ret=nor_flash_write(start_address+offset, w_buf_1, FILE_CONTENT_LEN);
		
		ticks_1_w = ql_rtc_get_ticks();
		speed_w = ((ticks_1_w - ticks_0_w)*1000)/32768.00;	  //写4K 花费的时间
		//printf("[flash_Speed] ========== Write_Speed_100Free=%.2fms\r\n", speed_w);
		speed_w_50 +=speed_w;

		////3、读时间统计
	    memset(buf, 0, FILE_CONTENT_LEN);
		ticks_2_w = ql_rtc_get_ticks();
		ret=nor_flash_read(	start_address+offset, buf, FILE_CONTENT_LEN);
		ticks_3_w = ql_rtc_get_ticks();
		speed_r = ((ticks_3_w - ticks_2_w)*1000)/32768.00;
		//printf("[flash_Speed] ========== Read_Speed_100Free=%.2fms\r\n", speed_r);
		speed_r_50 += speed_r;
		
		////4、写入和读出对比
		if((i%2))
		{
			if(0 != memcmp(buf,w_buf_0,FILE_CONTENT_LEN))
			{
                printf("R_buf:");
                for(int j=0; j<4096;j++)
                {
				    printf("%X",buf[j]);
                }
                printf("\r\n");
				printf("w_buf_0:");
                for(int j=0; j<4096;j++)
                {
				    printf("%X",w_buf_0[j]);
                }
                printf("\r\n");
				while(1)
				{
					printf("ERR:R != W, Test End!! CNT(%d)\r\n",i);
					ql_rtos_task_sleep_ms(1000); 
				}
			}
		}
		else
		{
			if(0 != memcmp(buf,w_buf_1,FILE_CONTENT_LEN))
			{
				printf("R_buf:%s\r\n",buf);
				printf("W_buf:%s\r\n",w_buf_1);
				while(1)
				{
					printf("ERR:R != W, Test End!! CNT(%d)\r\n",i);
					ql_rtos_task_sleep_ms(1000); 
				}
			}
		}
		
		if(i%TONGJI_UNIT == 0)
		{
			printf("[flash_Speed] ========== Erase_Speed_100Free=%.2fms\r\n", speed_e_50/TONGJI_UNIT);
			printf("[flash_Speed] ========== Write_Speed_100Free=%.2fms\r\n", speed_w_50/TONGJI_UNIT);
			printf("[flash_Speed] ========== Read_Speed_100Free=%.2fms\r\n", speed_r_50/TONGJI_UNIT);
			printf("[flash_Speed] ========== flash_speed_100free test RunTime-%d\r\n", i);//这里是擦写的次数
			speed_e_50 =0;
			speed_w_50 =0;
			speed_r_50 =0; 
		}
		
		ql_rtos_task_sleep_ms(TEST_TIME_DELAY); 
	}
}

static const unsigned int crc32_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

static unsigned int
crc32(unsigned int* val, const void *ss, int len)
{
  const unsigned char *s = ss;

  while(--len >= 0)
    *val = crc32_table[(*val ^ *s++) & 0xff] ^ (*val >> 8);
  return *val;
}

static char *patition_arry[]={"bootloader","ptable","rd","cp","dsp","rfbin","customer_app","factory_a"};
static unsigned int ro_crc(void)
{
    int i = 0 ;
    unsigned int crc;
    unsigned int start_address=0,size=0,offset=0;
    char* flash_cache = malloc(4096);
    for(i=0;i<(sizeof(patition_arry)/4);i++)
    {
        printf("===============[%d],%s==============\r\n",i,patition_arry[i]);
        ql_norflash_get_addr(patition_arry[i], &start_address, &size);
        for(offset=0;offset<size;offset=offset+4096)
        {   //printf("[addr]:%x\r\n",start_address+offset);
            ql_norflash_do_read(patition_arry[i],start_address+offset,flash_cache,4096);
            crc32(&crc,flash_cache,4096);
        }
    }

    return crc;
}
#define FILE_NAME			"crc32.txt"

#define U_DISK				"U:"
#define U_ROOT_PATH			""U_DISK"/"
#define U_FILE_UNDER_ROOT 	""U_DISK"/"FILE_NAME""

static void rtos_test_sub_task(void * argv)
{
    unsigned int ccrrcc=0;
    int ferr=0;
    int ret=0;
    QFILE * fp=0;
    unsigned int fcrc32 = 0;
    ferr = ql_access(U_FILE_UNDER_ROOT, 777);
    printf("ql_access%d",ferr);
    if(!ferr)
    {
        ccrrcc = ro_crc();
        fp = ql_fopen(U_FILE_UNDER_ROOT, "rb");
        ql_fread(&fcrc32,sizeof(fcrc32),1,fp);
        ql_fclose(fp);
        printf("%x==%x",fcrc32,ccrrcc);
        if(fcrc32==ccrrcc)
        {
            printf("check_ok\r\n");
        }
        else 
        {
            while(1)
            {
                printf("check_err\r\n");
                ql_rtos_task_sleep_s(10);
            }
        }
    }
    else 
    {
       ccrrcc = ro_crc();
       printf("===========crc32:%x==============\r\n",ccrrcc);
       fp=ql_fopen(U_FILE_UNDER_ROOT, "w+");
       ret=ql_fwrite(&ccrrcc,sizeof(ccrrcc),1,fp);
       printf("ql_fwrite:%d,%d",ret,fp);
       ql_fclose(fp);
    }
	WR_test_E_W_R_JiSuan_Ms_Time();
}

static ql_task_t test_task_ref = NULL;
static void flash_speed_test(void * argv)
{
	ql_rtos_task_create(&test_task_ref, 20*1024, 67, "FlashTest", rtos_test_sub_task,NULL);

    while(1)
    {
        //printf("flash  speed test over\r\n" );
        ql_rtos_task_sleep_s(1);
    }
}
//application_init(flash_speed_test, "flashtest", 20, 10);


