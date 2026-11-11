/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_tts.c
 * @author  august.yang
 * @version V1.0.0
 * @date    2022/04/15
 * @brief   This file tests TTS APIs
 ******************************************************************************
 */


#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_fs.h"
#include "ql_spi_nor.h"

//#define TTS_SEPARATE_TEST


void test_cb(QL_TTS_ENVENT_E event, char *str)
{
	printf("user_cb :event = %d,str = %s\n",event,str);
}

#ifdef TTS_SEPARATE_TEST
static unsigned int tts_address = 0;
int read_cb(void *pParameter, void * pBuffer, unsigned long iPos, unsigned long nSize)
{
	ql_spi_nor_read_fifo(EXTERNAL_NORFLASH_PORT33_36, pBuffer, tts_address+iPos, nSize);
	return 1;
}
#endif
static void quec_tts_test(void * argv)
{
	int port_index = EXTERNAL_NORFLASH_PORT33_36;
	int clk = _APBC_SSP_FNCLKSEL_26MHZ_;
	unsigned char *id = NULL;
	int i =0,ret = 0;
	unsigned int tts_size = 0;
	//等待audio初始化好
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	
	ql_set_audio_path_receiver();
	
	ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
#ifdef TTS_SEPARATE_TEST
	ql_spi_nor_init(port_index, clk);
	ql_norflash_get_addr("quectel_resource_tts", &tts_address, &tts_size);
	tts_address &= 0x0FFFFFFF;
	if(tts_address == NULL){
		printf("error!! no tts resource \r\n");
		while(1);
	}
	printf("quectel_resource_tts adress:0x%x, size:%d \r\n",tts_address, tts_size);
	
	ql_tts_separate_callback(read_cb);
#endif
	ql_tts_init(test_cb);
	ql_set_volume(8);
	AUDIO_PLAY_OPTION prop = {0};	
	//查询tts播放选项	
	prop.option = ql_get_tts_play_option();
	printf(">>>>>>>>>>>>>option=0x%016llX\r\n",prop.option);
	//设置tts播放选项，以下是举例，以实际开发需求为准
	prop.dest_end = 1;//TTS播放到远端
	ql_set_tts_play_option(prop.option);
	printf(">>>>>>>>>>>>>option=0x%016llX\r\n",prop.option);
	
	ql_tts_set_volume(0);									//设置tts音量，参数范围-32768~32767	
	while(1)
	{
		ql_tts_play(2,"支付宝到账一百万元");					//mode2,使用GBK编码方式
		ql_rtos_task_sleep_s(7); 							//等待5S,由于tts为异步播放，连续调用会直接打断
	}
}
//application_init(quec_tts_test, "quec_tts_test", 2, 0);

