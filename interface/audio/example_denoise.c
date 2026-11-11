/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_dtmf.c
 * @author  august.yang
 * @version V1.0.0
 * @date    2020/08/21
 * @brief   This file contains the audio play pcm functions's definitions
 ******************************************************************************
 */

#include "ql_type.h"
#include "ql_func.h"
#include "ql_audio.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include <stdio.h>
#include "ql_gpio.h"
#include "ql_platform.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "ql_audio_denoise.h"



#define QL_PCM_BLOCK_FLAG (0x01)
#define QL_PCM_NONBLOCK_FLAG (0x02)
#define QL_PCM_READ_FLAG (0x04)
#define QL_PCM_WRITE_FLAG (0x08)

//#define OVERLOP_SAMPLE 320      
#define OVERLOP_SAMPLE 160          


#ifndef QUEC_ARR_SIZE
#define QUEC_ARR_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

static unsigned char tmp_buf[1024] = {0};
void example_denoise(void *arg)
{
    int read_ret = -1, write_ret = -1;
    int ret = -1;
    unsigned int size = OVERLOP_SAMPLE *2;
    PCM_HANDLE_T read_hdl, write_hdl;
    char denoise_version[64] = {0};

    QL_PCM_CONFIG_T pcm_config = {1, 8000, 0};

    //等待audio底层初始化完成
    
    printf("example_denoise start ...\r\n");
    while (!ql_get_audio_state())
    {
        ql_rtos_task_sleep_ms(50);
    }
    
    printf("pcm test start ...\r\n");     
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
   
	ql_set_audio_path_receiver();
	ql_set_volume(10);

    printf("get denoise switch:%d ...\r\n",ql_get_denoise_flag());
    ql_set_denoise_flag(1);//打开降噪
    printf("get denoise switch:%d ...\r\n",ql_get_denoise_flag());
    ql_denoise_get_version(denoise_version);
    printf("denoise_version :%s\n",denoise_version);
    
    read_hdl = ql_pcm_open(&pcm_config, QL_PCM_READ_FLAG | QL_PCM_BLOCK_FLAG);
    if (read_hdl == NULL)
    {
        printf("ql_pcm_open read fail\n");
        return;
    }
    write_hdl = ql_pcm_open(&pcm_config, QL_PCM_WRITE_FLAG | QL_PCM_BLOCK_FLAG);
    if (write_hdl == NULL)
    {
        printf("ql_pcm_open write fail\n");
        return;
    }

    
    ret = ql_denoise_licenses_verify();
    if(ret == 0)
    {
        ql_trace("ql_denoise_licenses_verify ok:%d\n",ret); 
    }
    else 
    {
        
        ql_trace("ql_denoise_licenses_verify fail:%d\n",ret); 
    }

    unsigned int curr_pp = 0;
    unsigned int PPx = QL_PP5;

    ql_rtos_task_sleep_s(3);


    curr_pp = ql_cpu_fre_get();
    printf("pre  PP: %d\n", curr_pp);

    ql_cpu_fre_set(PPx); // PP1-PP5

    curr_pp = ql_cpu_fre_get();
    printf("currtnt  PP: %d\n", curr_pp);
    while (1)
    {
        read_ret = ql_pcm_read(read_hdl, tmp_buf, size); //录音
        if (read_ret >= 0)
        {
            write_ret = ql_pcm_write(write_hdl, tmp_buf, read_ret); //播音
        }
    }
    ql_pcm_close(read_hdl);
    ql_pcm_close(write_hdl);
}

 //application_init(example_denoise, "example_denoise", 12, 100);
