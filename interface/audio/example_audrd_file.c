/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_fs.h"

static AmrEncodeHandle  amr_record_handle;
static AmrEncConfigInfo  amr_record_config;
static AmrPlaybackHandle amr_play_handle;
static AmrPlaybackConfigInfo amr_play_config;

static void ql_amr_record_callback(const unsigned char* para1, unsigned int para2)
{
	printf("ql_amr_recoed_callback para1:[%s]  para2:[%d]\n", para1, para2);
}

static void ql_amr_play_event_callback(AmrPlaybackEventType event, int param)
{
    if (event == AMR_PLAYBACK_EVENT_STATUS)
    {
        if (param == AMR_FILE_STATUS_STARTED)
        {
            printf("amr play start\r\n");
           
        }
        if (param == AMR_FILE_STATUS_ENDED)
        {
            printf("amr play end\r\n");
        }
    }

}


static void audio_file_test(void * argv)
{
	int i = 0;
	 
	//等待audio底层初始化完成
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	
	ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
	printf("audio_file_test start ...\r\n");

	ql_set_audio_path_receiver();						//使用receiver录音

	ql_set_volume(10);

	amr_record_config.callback = ql_amr_record_callback;		
	ql_record_amr("U:/111.amr", &amr_record_handle, &amr_record_config); //录音为指定文件名的amr文件
	ql_rtos_task_sleep_s(7); //test only record 7s	
	ql_record_amr_stop(amr_record_handle);

	amr_play_config.listener = ql_amr_play_event_callback;
	while(ql_access("U:/111.amr",0) == 0){
		ql_play_amr("U:/111.amr", &amr_play_handle, &amr_play_config);
		printf("play start\n");
		ql_rtos_task_sleep_s(10);
		ql_stop_amr_play(amr_play_handle, 0);
	}
	
	printf("not find file!\n");

}

void ql_cb_wav_file_play(acm_audio_track_handle wav_handle, acm_audio_track_event_t event)
{
    printf("ql_cb_wav_file_play event = %d\n", event);
}
static audio_track_handle  wav_handle;
static struct audio_track_config wav_configInfo;
void wav_test(void *argv)
{
	//等待audio底层初始化完成
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	
	ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
		printf("wav_test start ...\r\n");

	ql_set_audio_path_receiver();						//使用receiver录音

	ql_set_volume(10);
    
    wav_configInfo.event_cb=ql_cb_wav_file_play;

	while(ql_access("U:/testwav.wav",0) == 0){
        ql_wav_play("U:/testwav.wav", &wav_configInfo, &wav_handle); //play wav file, custoner_fs.bin文件中需包含对应文件名的wav文件
        printf("wav play start ...\r\n");
        ql_rtos_task_sleep_s(20);
        ql_wav_play_stop(wav_handle, 0);
	}
}

// application_init(wav_test, "wav_test", 5, 2);
//application_init(audio_file_test, "audio_file_test", 2, 0);
