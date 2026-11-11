#include <stdio.h>
#include <string.h>
#include "ql_audio.h"
#include "ql_fs.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_uart.h"

#define FILE_NAME			"mp3test.mp3"


#define U_DISK				"U:"
#define U_ROOT_PATH			""U_DISK"/"
#define U_FILE_UNDER_ROOT 	""U_DISK"/"FILE_NAME""

static void list_dir(const char *path)
{
	QDIR * dp = NULL;
	struct lfs_info info = {0};
	int ret;
	
	dp = ql_opendir(path);
	if(dp == NULL)
	{
		printf("[FS] *** dir open fail: %s ***\r\n", path);
		return;
	}
	printf("[FS] dir opened: %s\r\n", path);

	printf("[FS] ----- start dir list -----\r\n");
	printf("[FS] type\tsize\tname\r\n");
	while(1)
	{
		ret = ql_readdir(dp, &info);
		if(ret < 0)
		{
			printf("[FS] *** dir read fail: %s ***\r\n", path);
			break;
		}
		else if(ret == 0)
		{
			printf("[FS] ----- end of dir list -----\r\n");
			break;
		}
		else
		{
			printf("[FS] %-4d\t%-4d\t%s\r\n", info.type, info.size, info.name);
		}
	}

	if(dp)
	{
		ql_closedir(dp);
		printf("[FS] dir closed: %s\r\n", path);
	}
}
void qlasdasd_cb_on_speakerpa(unsigned int on)
{
    printf("pa %d\r\n",on);
}
signed short DSP_gain[12]={-36,-31,-27,-24,-21,-18,-15,-12,-9,-6,-3,0};

void ql_mp3_test(void * argv)
{
    int ret=0;
    int size =0;
    QFILE* fp;
    unsigned char* buffer = malloc(8*4096);
    Mp3PlaybackHandle handle;
    Mp3PlayConfigInfo mp3_configInfo;
    memset(&mp3_configInfo,0,sizeof(mp3_configInfo));
    ql_rtos_task_sleep_s(10);
    // list "U:/"
	list_dir(U_ROOT_PATH);
    while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
    ql_set_audio_path_speaker();
    ql_set_rxcodec_gain_table(1, 0, 36,  1, 1,0, 2);
    ql_set_rxcodec_gain_table(1, 1, 36,  1, 1,0, 2);
    ql_set_rxcodec_gain_table(1, 2, 36,  1, 2,0, 2);
    ql_set_rxcodec_gain_table(1, 3, 36,  1, 2,0, 2);
    ql_set_rxcodec_gain_table(1, 4, 36,  1, 3,0, 2);
    ql_set_rxcodec_gain_table(1, 5, 36,  1, 3,1, 2);
    ql_set_rxcodec_gain_table(1, 6, 36,  1, 4,1, 2);
    ql_set_rxcodec_gain_table(1, 7, 36,  1, 4,1, 2);
    ql_set_rxcodec_gain_table(1, 8, 36,  1, 4,1, 2);
    ql_set_rxcodec_gain_table(1, 9, 36,  1, 4,1, 2);
    ql_set_rxcodec_gain_table(1, 10, 36,  1, 4,1, 2);
    ql_set_rxcodec_gain_table(1, 11, 36,  1, 4,1, 2);
    ql_set_dsp_gain_table(1, DSP_gain);
    ql_bind_speakerpa_cb(qlasdasd_cb_on_speakerpa);
	ql_set_volume(10);
    printf("quec mp3 test start\r\n");
    mp3_configInfo.sync_mode=0;
    ret=ql_play_mp3(U_FILE_UNDER_ROOT,&handle,&mp3_configInfo);
    printf("ql_mp3_file_play ret = %d\r\n",ret);
        #if 0 
        fp = ql_fopen(U_FILE_UNDER_ROOT,"rb");
        do{
        size = ql_fread(buffer,8*4096, 1, fp);
        ret=ql_play_mp3_stream_buffer(handle, buffer, 8*4096);
        printf("ql_play_mp3_stream_buffer ret = %d\r\n",ret);
        ql_rtos_task_sleep_ms(20);
        }while(size>0);

        ret = ql_stop_mp3_play(handle,1);
        ql_set_dtmf_volume(16);
        ql_aud_dtmf_play(AUDIOHAL_DTMF_0, 1);
		ql_rtos_task_sleep_ms(50);
		ql_aud_dtmf_play(AUDIOHAL_DTMF_0, 0);
        printf("ql_stop_mp3_play ret = %d\r\n",ret);
        printf("stop!!");
        ql_fclose(fp);
        #endif

}

#define QUEC_AUDIO_MP3_CTRL
#define SERIAL_RX_BUFFER_LEN  2048
static Mp3PlaybackHandle  		mp3_handle;
static Mp3PlayConfigInfo  		mp3_configInfo;
static AmrPlaybackHandle  		amr_handle;
static AmrPlaybackConfigInfo  	amr_configInfo;
static unsigned int mp3pcm_handle=0;
static unsigned int amrpcm_handle=0;
static unsigned int mp3_play_stop=0;
static unsigned int amr_play_stop=0;
static u8 m_RxBuf_Uart1[SERIAL_RX_BUFFER_LEN];

static void quec_main_uart_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	unsigned int pcm_handle = 0;
	
#ifdef QUEC_AUDIO_MP3_CTRL
	ql_mp3_get_pcm_handle(mp3_handle, &mp3pcm_handle);
	pcm_handle = mp3pcm_handle;
#else
	ql_amr_get_pcm_handle(amr_handle, &amrpcm_handle);
	pcm_handle = amrpcm_handle;
#endif // QUEC_AUDIO_MP3_CTRL
	printf("pcm handle=: %d\n",pcm_handle);
	unsigned int  totalBytes = ql_uart_read(port, m_RxBuf_Uart1, sizeof(m_RxBuf_Uart1));
	if (totalBytes <= 0)
	{
		printf("<-- No data in UART buffer! -->\r\n");
		return;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_SUSPEND",24)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(pcm_handle, AUDIO_TRACK_CTRL_SUSPEND, 0);
		goto end;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_RESUME",23)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(pcm_handle, AUDIO_TRACK_CTRL_RESUME, 0);
		goto end;
	}
	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_MUTE_ON",24)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(pcm_handle, AUDIO_TRACK_CTRL_MUTE_ON, 0);
		goto end;
	}
	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_MUTE_OFF",25)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(pcm_handle, AUDIO_TRACK_CTRL_MUTE_OFF, 0);
		goto end;
	}
	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_GAIN",21)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(pcm_handle, AUDIO_TRACK_CTRL_GAIN, 1000);
		goto end;
	}
	
end:
	ql_uart_write(port,m_RxBuf_Uart1,totalBytes);
	memset(m_RxBuf_Uart1,0,SERIAL_RX_BUFFER_LEN);
	return;
}
#ifdef QUEC_AUDIO_MP3_CTRL
static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if(event == MP3_PLAYBACK_EVENT_STATUS){
       if(val == MP3_PLAYBACK_STATUS_STARTED)
       {
            printf("mp3 play start\r\n");
       }
       if(val == MP3_PLAYBACK_STATUS_ENDED)
       {
            printf("mp3 play end\r\n");
            // ql_stop_mp3_play(mp3_handle,0);  //Disable this interface in the callback.
			mp3_play_stop = 0;       
       }
    }
}
#else
static void ql_AmrPlaybackEvent(AmrPlaybackEventType event, int val)
{
    if(event == AMR_PLAYBACK_EVENT_STATUS){
       if(val == AMR_FILE_STATUS_STARTED)
       {
            printf("amr play start\r\n");
       }
       if(val == AMR_FILE_STATUS_ENDED)
       {
            printf("amr play end\r\n");
            ql_stop_amr_play(amr_handle,0);  
			amr_play_stop = 0;       
       }
    }
}
#endif // QUEC_AUDIO_MP3_CTRL
void ql_audio_contorl_test(void * argv)
{
	int ret =0;
    // list "U:/"
	list_dir(U_ROOT_PATH);
    while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	ret = ql_uart_open(QL_MAIN_UART_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
	if (ret) 
	{
		printf("open uart[%d] failed! \n", QL_MAIN_UART_PORT);
		return;
	}
    ql_uart_register_cb(QL_MAIN_UART_PORT, quec_main_uart_callback);	//use callback to read uart data

	ql_uart_write(QL_MAIN_UART_PORT,"MAIN_UART_REDY\r\n",16);
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
    ql_set_audio_path_receiver();
	ql_set_volume(10);

    printf("quec audio ctrl test start\r\n");

#ifdef QUEC_AUDIO_MP3_CTRL
	mp3_configInfo.listener = ql_Mp3PlaybackEvent;
	while (1)
	{
		if (!mp3_play_stop)
		{
			mp3_play_stop = 1;
			ql_play_mp3("U:/testmp3.mp3",&mp3_handle,&mp3_configInfo);
		}
		ql_rtos_task_sleep_ms(500);
	}
#else
	amr_configInfo.listener = ql_AmrPlaybackEvent;
	while (1)
	{
		if (!amr_play_stop)
		{
			amr_play_stop = 1;
			ql_play_amr("U:/testamr.amr",&amr_handle,&amr_configInfo);
		}
		ql_rtos_task_sleep_ms(500);
	}
#endif // QUEC_AUDIO_MP3_CTRL
}
// application_init(ql_audio_contorl_test, "ql_audio_contorl_test", 10, 2);
//application_init(ql_mp3_test, "mp3_test", 10, 2);

