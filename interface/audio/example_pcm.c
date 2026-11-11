/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/


#include "ql_type.h"
#include "ql_func.h"
#include "ql_audio.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include <stdio.h>
#include "ql_uart.h"

#define SERIAL_RX_BUFFER_LEN  2048
static char *tmp_buf = NULL;
static	audio_track_handle track_hd;
static	audio_record_handle record_hd;
/*option*/
	/** bit0~bit1, dest play end, 0->near,1->far,2->both, refer @ref acm_audio_track_direction_t  0：近端播放，1：远端播放，2：同时*/
	/** bit2, override voice if any, 0->false,1->true */
	/** bit3, near codec or vocoder, 0->codec,1->vocoder */
	/** bit4, reserved */
	/** bit5, play current file in cyclic mode, 0->false,1->true */
	/** bit6~bit7, schedule mode, 0->combine,1->exclusive,2->preempt, refer @ref acm_audio_track_mode_t */
	/** bit8~bit11, eq effect index, 0 for EQ off */
	/** bit12~bit20, speed factor in Q8, 0 for normal*/
	/** bit21~bit23, write mode, refer @ref acm_audio_track_write_mode_t  0：异步模式，1：同步模式，2：异步，不阻塞不返回错误码*/
	/** bit24~bit27, pcm format, refer @ref acm_audio_track_format_t*/
	/** bit28, turn off audio effect */
	/** bit29~bit31, reserved */
	/** bit32~bit41, how many frames can be held*/
	/** bit42~bit45, how many frames to keep dynamically, level 0~7*/
	/** bit46~bit48, how many mixed frames to keep dynamically, level 0~7*/

static void ql_audio_track_callback(acm_audio_track_handle handle, acm_audio_track_event_t event)
{
	printf("audio_track_callback event:%d\r\n", event);
}

static void ql_audio_record_callback(acm_audio_track_handle handle, acm_audio_record_event_t event)
{
	printf("audio_record_callback event:%d\r\n", event);
}
static u8 m_RxBuf_Uart1[SERIAL_RX_BUFFER_LEN];

static void quec_main_uart_callback(QL_UART_PORT_NUMBER_E port, void *para)
{

	unsigned int  totalBytes = ql_uart_read(port, m_RxBuf_Uart1, sizeof(m_RxBuf_Uart1));
	if (totalBytes <= 0)
	{
		printf("<-- No data in UART buffer! -->\r\n");
		return;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_SUSPEND",24)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(track_hd, AUDIO_TRACK_CTRL_SUSPEND, 0);
		goto end;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_RESUME",23)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(track_hd, AUDIO_TRACK_CTRL_RESUME, 0);
		goto end;
	}
	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_TRACK_CTRL_MUTE_ON",24)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_track_ctrl(track_hd, AUDIO_TRACK_CTRL_MUTE_ON, 0);
		goto end;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_RECORD_CTRL_SUSPEND",25)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_record_ctrl(record_hd, AUDIO_RECORD_CTRL_SUSPEND, 0);
		goto end;
	}

	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_RECORD_CTRL_RESUME",24)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_record_ctrl(record_hd, AUDIO_RECORD_CTRL_RESUME, 0);
		goto end;
	}
	if(0 == memcmp(m_RxBuf_Uart1,"AUDIO_RECORD_CTRL_MUTE_ON",25)) 
	{
		printf("<-%s -->\r\n",m_RxBuf_Uart1);
		ql_audio_record_ctrl(record_hd, AUDIO_RECORD_CTRL_MUTE_ON, 0);
		goto end;
	}
	
end:
	ql_uart_write(port,m_RxBuf_Uart1,totalBytes);
	memset(m_RxBuf_Uart1,0,SERIAL_RX_BUFFER_LEN);
	return;
}

void pcm_test(void *arg)
{
	int ret =0;
	int read_ret = -1, write_ret = -1;

    unsigned int record_len;
	unsigned int rate = 16000; 
	unsigned int channels = 2;
	unsigned long long option = 0;
	//等待audio初始化好
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

	printf("pcm_test start...\r\n");
	ql_set_audio_path_receiver();
	ql_set_volume(10);
	record_len = (channels*rate*2*20)/1000;
	tmp_buf = malloc(record_len);

	track_hd = ql_audio_track_open(rate, channels, 0x100, ql_audio_track_callback);

	record_hd = ql_audio_record_open(rate, channels, 0, ql_audio_record_callback);

	while (1) 
	{
		read_ret = ql_audio_record_read(record_hd, tmp_buf, &record_len);                  
		printf("ql_pcm_read, pcm read:%d %d\n", read_ret,record_len);
		if ((read_ret == 0) && record_len > 0) {
			write_ret = ql_audio_track_write(track_hd, tmp_buf, record_len);		  
			printf("ql_pcm_write, pcm write:%d\n", write_ret);
		}		
	}
	ql_audio_track_close(track_hd, 0);
	ql_audio_record_close(record_hd, 0);
}
void amrenc_test(void *arg)
{
	int ret =0;
	int read_ret = -1, write_ret = -1;
	unsigned char out_buf[5*1024] = {0};
	int out_len = 0;
    unsigned int record_len;
	unsigned int rate = 8000; 
	unsigned int channels = 1;
	unsigned long long option = 0;
	//???audio???????
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}

	ql_codec_choose(AUD_INTERNAL_CODEC, NULL);

	printf("amrenc_test start...\r\n");
	ql_set_audio_path_receiver();
	ql_set_volume(10);
	record_len = (channels*rate*2*20)/1000;
	tmp_buf = malloc(record_len);

	record_hd = ql_audio_record_open(rate, channels, 0, ql_audio_record_callback);
	ql_amrnbenc_init(0,0);
	while (1) 
	{
		read_ret = ql_audio_record_read(record_hd, tmp_buf, &record_len);                  
		printf("ql_pcm_read, pcm read:%d %d\n", read_ret,record_len);
		if ((read_ret == 0) && record_len > 0) {
			ret = ql_amrnbenc_do(tmp_buf, out_buf+out_len);
			if (ret > 0)
			{
				out_len += ret;
				if (out_len > sizeof(out_buf))
				{
					break;
				}
			}
		}	
		ql_rtos_task_sleep_ms(20);
	}
	ql_audio_record_close(record_hd, 0);
	ql_amrnbenc_deinit();
	ql_amr_stream_write(out_buf, out_len);
}
// application_init(amrenc_test, "amrenc_test", 20, 2);
// application_init(pcm_test, "pcm_test", 10, 2);

