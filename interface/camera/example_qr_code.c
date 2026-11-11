/*---------------------------------------------------------------------------
Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
Quectel Wireless Solution Proprietary and Confidential.
---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ql_application.h"

#include "ql_rtos.h"
#include "ql_gpio.h"
#include "ql_camera.h"
#include "ql_uart.h"



#define APP_DEBUG(fmt, args...)	printf(fmt, ##args)
#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart1[SERIAL_RX_BUFFER_LEN];

#define DECODER_COUNT 20

static ql_timer_t timerRef;
bool camera_close = 0;
int camera_open = 1;
ql_task_t  taskRef;
#define __align(x) __attribute__ ((aligned (x)))

//static __align(32) unsigned char camdecodebuf[640*480*3/2*2]={0};
#define CAMERA_PREVIEW_TASK_PRIORITY	218 
#define CAMERA_DECODE_TASK_PRIORITY		227  
#define PREVIEW_BUF_CNT 2
void cam_callback_preview(P_CAL_IDENTITY_RESULT_STRUCT outdata);



static CAM_DECODE_CONFIG_CTX camconfig_test =
{
	.camwidth      =240,     //
	.camheight     =320,
	.dectaskprio   =CAMERA_DECODE_TASK_PRIORITY,
	.decbufcnt     =1,      //解码buffer的数量
	.decbufaddr    =NULL,   //解码buffer的首地址
	.decodecb      =cam_callback_preview,   //解码成功的回调
	.preview       =0,      //是否需要进行预览
	.pretaskprio   =CAMERA_PREVIEW_TASK_PRIORITY,//预览任务的优先级
	.prebufcnt     =PREVIEW_BUF_CNT, //预览的buffer数量
	.prebufaddr    =NULL,   //预览的buffer首地址---用于Camera预览数据（NV12）保存
	.lcdmemeryaddr =NULL,   //LCD的显存首地址,预览的时候,需要将预览的数据转化到此buffer,600n此处不需要传入
	.lcdprewidth   =320,    //预览图像的宽度
	.lcdpreheight  =240,    //预览图像的高度
};

/******************************************************************
ENGLISH:Please do not run time-consuming functions in this function
中文：请不要在此函数内运行比较耗时的代码或者函数
如果扫码成功，此回调函数将会被调用
attention:please don't "use ql_qr_camera_close" as callback in "ql_qr_camera_open".
注意：不要将use ql_qr_camera_close函数作为ql_qr_camera_open的回调函数。
回调类型为task级别
******************************************************************/
void cam_callback_preview(P_CAL_IDENTITY_RESULT_STRUCT outdata)
{
	printf("<-- result=%d\r\n",outdata->result);
	printf("<-- databuf=%s\r\n",outdata->DataBuf);
	//解码成功，发送消息，告诉通讯线程进行数据通信,将解码结果上传到交易中心
	//More.....
	//关闭Camera
	if(outdata->result==0){
	camera_close =1;
    }
}

/*Fias.fan 2021.11.17 测试解码功能和补光灯控制*/
//#define TEST_FILL_LIGHT		//测试当有物体靠近摄像头时，打开补光灯
static void qr_code_light_test(void *arg)
{
    int ret=0;   
	unsigned int value[10]={0};
	unsigned int threshold=0x5;		//阈值设置，当前环境光和遮挡时的环境光差值，可根据读取寄存器0x14环境光平均值，自行设定。
    unsigned int current_val=0,last_val=0,last_last_val=0;
	

	ql_rtos_task_sleep_s(5);			//这里目前必须要延时，给camera时间初始化。
	printf("<----------Camera qr_code_light_test++++++++++++++>\n");

	//@20221208 初始化camera IO供电使能引脚（EC600M）
	ql_gpio_init(GPIO_PIN_NO_77, PIN_DIRECTION_IN,PIN_PULL_PU,PIN_LEVEL_HIGH);
	//Ql_Cam_Identify(1);//打开解码
	ql_set_decode_mirror(AUTO_MODE);	 //此接口表示自动检测非镜像和镜像二维码
	 printf("[OPEN-START] Start Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());

	 ret = ql_qr_camera_open(&camconfig_test); 
       if(ret != CAM_DECODE_SUCESS)
        {
           printf("camera code open failed\r\n");
           printf("[OPEN-CAM] Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
        }
       else
        {
            printf("camera code open successd\r\n");
            printf("[OPEN-CAM]  Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
        }
	
	ql_rtos_task_sleep_s(5);	
	/*

	ret=ql_qr_camera_close();
	   if(ret != CAM_DECODE_SUCESS)
	   {
		  printf("camera code close failed，ret=%d\r\n",ret);
		  printf("[OPEN-CAM]  Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
	   }
	   else
	   {
			printf("camera code close success\r\n");
			printf("[OPEN-CAM]	Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
	   }
	 */ 


}

//application_init(qr_code_light_test, "qr_code_light_test", 2, 0);


