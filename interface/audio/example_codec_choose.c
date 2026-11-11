/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_iic.h"
#include "ql_gpio.h"

/***************参数定义***************/
#define STATEconfirm		0xFC			//状态机确认 回读STATEconfirm的寄存值确认IC正常工作状态
#define NORMAL_I2S			0x00
#define NORMAL_LJ			0x01
#define NORMAL_DSPA			0x03
#define NORMAL_DSPB			0x23
#define Format_Len24		0x00
#define Format_Len20		0x01
#define Format_Len18		0x02
#define Format_Len16		0x03
#define Format_Len32		0x04

#define VDDA_3V3			0x00
#define VDDA_1V8			0x01
#define MCLK_PIN			0x00
#define SCLK_PIN			0x01
/***************参数定义***************/
/***************参数选择***************/
#define MSMode_MasterSelOn	0				//产品主从模式选择:默认选择0为SlaveMode,打开为1选择MasterMode
#define Ratio 				PCM_FS_64		//实际Ratio=MCLK/LRCK比率，需要和实际时钟比例匹配
#define Format 				NORMAL_DSPA		//数据格式选择,需要和实际时序匹配
#define Format_Len			Format_Len16	//数据长度选择,需要和实际时序匹配
#define SCLK_DIV			4				//SCLK分频选择:(选择范围1~20),SCLK=MCLK/SCLK_DIV，超过后非等比增加具体对应关系见相应DS说明
#define SCLK_INV			0				//默认对齐方式为下降沿,1为上升沿对齐,需要和实际时序匹配
#define MCLK_SOURCE			MCLK_PIN		//是否硬件没接MCLK需要用SCLK当作MCLK

#define ADCChannelSel		1				//单声道ADC输入通道选择是CH1(MIC1P/1N)还是CH2(MIC2P/2N)，
#define DACChannelSel		0				//单声道DAC输出通道选择:默认选择0:L声道,1:R声道	
#define VDDA_VOLTAGE		VDDA_1V8		//模拟电压选择为3V3还是1V8,需要和实际硬件匹配
#define ADC_PGA_GAIN		8				//ADC模拟增益:(选择范围0~10),具体对应关系见相应DS说明
#define ADC_Volume			191				//ADC数字增益:(选择范围0~255),191:0DB,±0.5dB/Step
#define DAC_Volume			191				//DAC数字增益:(选择范围0~255),191:0DB,±0.5dB/Step 
#define Dmic_Selon 			0				//DMIC选择:默认选择关闭0,打开为1
#define ADC2DAC_Sel			0				//LOOP选择:内部ADC数据给到DAC自回环输出:默认选择关闭0,打开为1
#define DACHPModeOn			0				//输出负载开启HP驱动:默认选择关闭0,打开为1

#define	ES8312_MONONOUT		0
#define	ES8312_FM			0
/***************参数选择***************/

static int rt5616_init_reg[] = {
	0x08080002,	//0002 is the addr, 0808 is the value
	0x08080003,
	0x80000005,
	0x04C0000D,
	0x0808000F,
	0xAFAF0019,
	0x5C5C001C,
	0x0000001E,
	0x38200027,
	0x80800029,
	0x1250002A,
	0x0000003B,
	0x004F003C,
	0x2000003D,
	0x004F003E,
	0x40000045,
	0x0000004D,
	0x0000004E,
	0x0278004F,
	0x00000050,
	0x00000051,
	0x02780052,
	0xC8000053,
	0x98060061,
	0x88000062,
	0xF8FE0063,
	0x0A000064,
	0xCC000065,
	0x3F000066,
	0x003D006A,
	0x3600006C,
	0x80820070,
	0x00000073,
	0x0C000074,
	0x50000080,
	0x0E1C0081,
	0x08000082,
	0x0019008E,
	0x3100008F,
	0x2C000093,
	0x02000094,
	0x208000B0,
	0x000000B1,
	0x001100FA,
};

static uint16  es8311_init_reg[] = {
	0x0045,//45 is the addr, 0x00 is the value
	0x3001,
	0x1002,
	0x0002,
	0x1003,
	0x2416,
	0x2004,
	0x0005,
	0x2006,
	0x0007,
	0xff08,
	0x0f09,
	0x0f0a,
	0x000b,
	0x000c,
	0x1310,
	0x7f11,
	0x3f01,
	0x8000,
	0x010d,
	0x1014,
	0x2812,
	0x1013,
	0x020e,
	0x440f,
	0x0015,
	0x0a1b,
	0x6a1c,
	0x0837,
	0x0044,
	0xbf17,
	0xbf32,
};

#define RT5616_INIT_REG_LEN sizeof(rt5616_init_reg)/sizeof(rt5616_init_reg[0])
#define ES8311_INIT_REG_LEN sizeof(es8311_init_reg)/sizeof(es8311_init_reg[0])//xu

#define CODEC_IIC_NO 0	//EC600M iic_no 1/EC800M iic_no 0
#define I2C_SLAVE_ADDR  0x1B	//codec 5616

#define ES8311_REG_I2C_DEVICEID     0xFD
#define ES8311_I2C_SLAVE_ADDR       0x18



static short ql_rt5616_read_reg(unsigned char RegAddr, unsigned short *p_value)
{
	unsigned short status = 1;
	unsigned char temp_buf[2];
	unsigned char retry_count = 5;

	do 
    {
        status = ql_i2c_read(CODEC_IIC_NO, I2C_SLAVE_ADDR, RegAddr, temp_buf, 2);
		if (status != 0)
		{
            printf("Error:rt5616 [%dth] device[0x%x] addr[0x%x] failed\n", retry_count, I2C_SLAVE_ADDR, RegAddr);
        }	
		else
        {
			*p_value = (((unsigned short)temp_buf[0]) << 8) | temp_buf[1];
			break;
		}
	} while (--retry_count);

	return status;
}

static short ql_rt5616_write_reg(unsigned char RegAddr, unsigned short RegData)
{
    unsigned short status = 1;
    unsigned char param_data[3] = {0x00};
    unsigned char retry_count = 5;

    param_data[0] = (unsigned char)((RegData >> 8) & 0xFF);
    param_data[1] = (unsigned char)(RegData & 0xff);

    do 
    {
        status = ql_i2c_write(CODEC_IIC_NO, I2C_SLAVE_ADDR, RegAddr, param_data, 2);
        if (status != 0)
        {
            printf("Error:[%dth] device[0x%x] addr[0x%x] failed\n", retry_count, I2C_SLAVE_ADDR, RegAddr);
        }        
        else 
        {
            break;
        }
    } while (--retry_count);

    return status;
}

static void ql_rt5616_init(void)
{
	char i = 0;
	
	for (i = 0; i < RT5616_INIT_REG_LEN; i++)
	{
		ql_rt5616_write_reg(rt5616_init_reg[i]&0xFFFF, rt5616_init_reg[i]>>16);
	}
}

static void ql_rt5616_open(void)		//初始化5616型号codec
{
	unsigned short status = 0;
	unsigned short  val = 0;
	ql_i2c_init(CODEC_IIC_NO, 0);		//不同项目的使用的IIC_NO有区别，硬件上也不相同，请根据具体项目更改
	status = ql_rt5616_read_reg(0xFE, &val);
	if ((status == 0) && (0x10EC == val)) 
	{
		ql_rt5616_init();
		printf("quec_rt5616_init!!!\n");
	}

}
static void ql_rt5616_close(void)		//给5616款codec下电
{
	static unsigned int reg_5616_power[] = {0x00000061, 0x00000062, 0x00000063, 0x00000064, 0x00000065, 0x00000066};
	char i = 0;
	
	for (i = 0; i < sizeof(reg_5616_power)/sizeof(reg_5616_power[0]); i++)
	{
		ql_rt5616_write_reg(reg_5616_power[i]&0xFFFF, reg_5616_power[i]>>16);
	}
}




static short ql_es8311_read_reg(unsigned char RegAddr, unsigned char *p_value)
{
	unsigned short status = 1;
	unsigned char temp_buf = 0;
	unsigned char retry_count = 5;

	do 
    {
        status = ql_i2c_read(CODEC_IIC_NO, ES8311_I2C_SLAVE_ADDR, RegAddr, &temp_buf, 1);
		if (status != 0)
		{
            printf("Error:es8311 [%dth] device[0x%x] addr[0x%x] failed\n", retry_count, ES8311_I2C_SLAVE_ADDR, RegAddr);
        }	
		else
        {
			*p_value = temp_buf;
			break;
		}
	} while (--retry_count);

	return status;
}


static short ql_es8311_write_reg(unsigned char RegAddr, unsigned char RegData)
{
    unsigned short status = 1;
    unsigned char retry_count = 5;
	unsigned char temp_buf = RegData;

    do 
    {
        status = ql_i2c_write(CODEC_IIC_NO, ES8311_I2C_SLAVE_ADDR, RegAddr, &temp_buf, 1);
        if (status != 0)
        {
            printf("Error:[%dth] device[0x%x] addr[0x%x] failed\n", retry_count, ES8311_I2C_SLAVE_ADDR, RegAddr);
        }        
        else 
        {
            break;
        }
    } while (--retry_count);

    return status;
}

static void ql_es8311_init(void)
{
	ql_es8311_write_reg(0x45,0x00);	
	ql_es8311_write_reg(0x01,0x30);
	ql_es8311_write_reg(0x02,0x10);
	if(Ratio == PCM_FS_256)//Ratio=MCLK/LRCK=256：12M288-48K；4M096-16K; 2M048-8K
	{
		ql_es8311_write_reg(0x02,0x00);//MCLK DIV=1		
		ql_es8311_write_reg(0x03,0x10);	
		ql_es8311_write_reg(0x16,0x24);	
		ql_es8311_write_reg(0x04,0x20);	
		ql_es8311_write_reg(0x05,0x00);
		ql_es8311_write_reg(0x06,(SCLK_INV<<5) + SCLK_DIV -1);
		ql_es8311_write_reg(0x07,0x00);
		ql_es8311_write_reg(0x08,0xFF);
	}	
	if(Ratio == PCM_FS_128)// Ratio=MCLK/LRCK=128：6M144-48K；2M048-16K; 1M024-8K
	{
		ql_es8311_write_reg(0x02,0x08);		
		ql_es8311_write_reg(0x03,0x10);	
		ql_es8311_write_reg(0x16,0x24);	
		ql_es8311_write_reg(0x04,0x20);	
		ql_es8311_write_reg(0x05,0x00);
		ql_es8311_write_reg(0x06,(SCLK_INV<<5) + SCLK_DIV -1);
		ql_es8311_write_reg(0x07,0x00);
		ql_es8311_write_reg(0x08,0x7F);
	}			
	if(Ratio == PCM_FS_64)// Ratio=MCLK/LRCK=64：3M072-48K；1M024-16K; 512k-8K
	{
		/*当512K/8K时，DVDD必须接1V8，否则无法正常工作*/
		ql_es8311_write_reg(0x02,0x18);		
		ql_es8311_write_reg(0x03,0x20);	
		ql_es8311_write_reg(0x16,0x20);	
		ql_es8311_write_reg(0x04,0x20);	
		ql_es8311_write_reg(0x05,0x00);
		ql_es8311_write_reg(0x06,(SCLK_INV<<5) + SCLK_DIV -1);
		ql_es8311_write_reg(0x07,0x00);
		ql_es8311_write_reg(0x08,0x3F);
	}
	ql_es8311_write_reg(0x09,(DACChannelSel<<7) + Format + (Format_Len<<2));
	ql_es8311_write_reg(0x0A,Format + (Format_Len<<2));
	
	ql_es8311_write_reg(0x0B,0x00);	
	ql_es8311_write_reg(0x0C,0x00);	

	ql_es8311_write_reg(0x10,(0x1C*DACHPModeOn) + (0x60*VDDA_VOLTAGE) + 0x03);	
	
	ql_es8311_write_reg(0x11,0x7F);	
	
	ql_es8311_write_reg(0x00,0x80 + (MSMode_MasterSelOn<<6));//Slave  Mode	
	
	ql_es8311_write_reg(0x0D,0x01);	

	ql_es8311_write_reg(0x01,0x3F + (MCLK_SOURCE<<7));//做主情况下BCLK只能输出，不能选择引MCLK	
	
	ql_es8311_write_reg(0x14,(Dmic_Selon<<6) + (ADCChannelSel<<4) + ADC_PGA_GAIN);//选择CH1输入+30DB GAIN	

	ql_es8311_write_reg(0x12,0x28);	
	ql_es8311_write_reg(0x13,0x00 + (DACHPModeOn<<4));	
/*****************************************/
/*****************************************/		
	ql_es8311_write_reg(0x0E,0x02);	
	ql_es8311_write_reg(0x0F,0x44);	
	ql_es8311_write_reg(0x15,0x00);	
	ql_es8311_write_reg(0x1B,0x0A);	
	ql_es8311_write_reg(0x1C,0x6A);	
	ql_es8311_write_reg(0x37,0x08);
	ql_es8311_write_reg(0x44,(ADC2DAC_Sel <<7));	
	ql_es8311_write_reg(0x17,ADC_Volume);	
	ql_es8311_write_reg(0x32,DAC_Volume);	
}

static void ql_es8311_open(void)		//初始化8311型号codec
{
	unsigned short status = 0;
	unsigned char  val = 0;
	ql_i2c_init(CODEC_IIC_NO, 0);		//不同项目的使用的IIC_NO有区别，硬件上也不相同，请根据具体项目更改
	status = ql_es8311_read_reg(ES8311_REG_I2C_DEVICEID, &val);
	if ((status == 0) && (0x83 == val)) 
	{
		ql_es8311_init();
		printf("quec_es8311_init!!!\n");
	}

}

static ql_cb_on_speakerpa play_audio_pa_cb(unsigned int on){
	if(on){
		printf("PA on\r\n");
		ql_gpio_init(GPIO_PIN_NO_86, PIN_DIRECTION_OUT, PIN_PULL_PU, PIN_LEVEL_HIGH);
	}else{
		printf("PA off\r\n");
		ql_gpio_init(GPIO_PIN_NO_86, PIN_DIRECTION_OUT, PIN_PULL_PD, PIN_LEVEL_LOW);
	}
}
static char *tmp_buf = NULL;
static	audio_track_handle track_hd;
static	audio_record_handle record_hd;

static void ql_audio_track_callback(acm_audio_track_handle handle, acm_audio_track_event_t event)
{
	//printf("audio_track_callback event:%d\r\n", event);
}

static void ql_audio_record_callback(acm_audio_track_handle handle, acm_audio_record_event_t event)
{
	//printf("audio_record_callback event:%d\r\n", event);

}
static void quec_codec_test(void * argv)
{
	unsigned int record_len;
	unsigned int rate = 8000; 
	unsigned int channels = 1;
	int read_ret = -1, write_ret = -1;
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	
	printf("quec_codec_test start ...\r\n");

	int i = 0;
	ql_codecpcm_config pcm_config[] = {Ratio,PCM_CODEC_SAMPLE_8000,0};
	ql_set_volume(2);
	
	ql_bind_speakerpa_cb(play_audio_pa_cb);
	ql_codec_choose(AUD_EXTERNAL_CODEC, &pcm_config);	//模块内部配置后向外置codec输出pcm数据
	ql_es8311_open();									//初始化外置codec
	record_len = 320;
	tmp_buf = malloc(record_len);
	printf("choose external codec play audio ...\r\n");
	record_hd = ql_audio_record_open(rate, channels, 0x00, ql_audio_record_callback);
	track_hd = ql_audio_track_open(rate, channels, 0x00, ql_audio_track_callback);
	
	while (1) 
	{
		//printf("start record \n");
		read_ret = ql_audio_record_read(record_hd, tmp_buf, &record_len);                  
		printf("ql_pcm_read, pcm read:%d %d\r\n", read_ret,record_len);
		//ql_audio_record_close(record_hd, 0);
		//printf("start play\n");
		if(read_ret==0){
		write_ret = ql_audio_track_write(track_hd, tmp_buf, record_len);
		}
		 ql_rtos_task_sleep_ms(20);
	}

}
// application_init(quec_codec_test, "quec_codec_test", 3, 0);

