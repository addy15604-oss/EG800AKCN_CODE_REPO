/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_adc.h"
#include "ql_power.h"

#define test_log(fmt, args...)	printf(fmt, ##args)

/**********
pm802(guilin:adc0、adc1、vbat): 600S、600N_LD、100N_AB
pm813(ningbo:adc0、adc1、vbat): 100Y
pm803(adc0、vbat):               600N_LC、100N_AA
*************/
#if 1
static void quec_adc_test(void * argv)
{
	unsigned short adc_vol = 0;
	unsigned char at_channel = 0;
	unsigned short bat_vol = 0;
	
	//ql_adc_init();  //PM802、PM813 需要初始化
	
	while(1)
	{
		at_channel = 0;
		
		//ql_adc_onff(1);   //PM802 需要开关，否则会有误差
		ql_rtos_task_sleep_s(10);
		//ql_adc_onff(0);
			
		//ql_adc_read(0, &adc_vol);      //PM802、PM803、PM813都支持adc0
        //adc_vol = adc_vol*49/10;
		test_log(" adc0_vol: %d mV ",adc_vol);

		//ql_adc_read(1, &adc_vol);     //PM802、PM813支持adc1, PM803没有adc1
		test_log(" adc1_vol: %d mV ",adc_vol);
        
		//bat_vol = ql_get_battery_vol(); //PM802、PM803、PM813都支持
		test_log(" vbat_vol: %d mV \r\n",bat_vol);
		ql_rtos_task_sleep_s(1);
	}
}
#else
static void quec_adc_test(void * argv)
{
	unsigned char at_channel = 0,times=10,i=0;
	unsigned short adc_vol = 0;
    unsigned short batvol_sample[10]={0};

    unsigned short vbat_vol = 0;
    unsigned short vbat_sample[10]={0};
    
	ql_adc_init();
	while(1)
	{
	    times = 10;i=0;
	    while(times--)
	    {
	        ql_adc_onff(1);
            ql_rtos_task_sleep_ms(5);
            ql_adc_onff(0);
            
		    //adc_vol = ql_get_battery_vol();
            ql_adc_read(at_channel, &adc_vol);
            batvol_sample[i]=adc_vol;

            vbat_vol = ql_get_battery_vol();
            vbat_sample[i] = vbat_vol;
            
            i++;
        }
        {  //过滤和平均adc0
            unsigned short max=batvol_sample[0], min=batvol_sample[0];
            unsigned short j=0, vol_all=0;
            for(j=1; j<10; j++){    //取最大最小??
                if(max < batvol_sample[j])
                    max=batvol_sample[j];

                if(min > batvol_sample[j])
                    min=batvol_sample[j];
            }
            
            for(j=0; j<10; j++){
                vol_all += batvol_sample[j];
            }

            vol_all = vol_all - min - max;  //去掉最大最小??
            adc_vol = vol_all/8;
        }   
        {   //过滤和平均vbat
            unsigned short max=vbat_sample[0], min=vbat_sample[0];
            unsigned short j=0, vol_all=0;
            for(j=1; j<10; j++){    //取最大最小??
                if(max < vbat_sample[j])
                    max=vbat_sample[j];

                if(min > vbat_sample[j])
                    min=vbat_sample[j];
            }
            
            for(j=0; j<10; j++){
                vol_all += vbat_sample[j];
            }

            vol_all = vol_all - min - max;  //去掉最大最小??
            vbat_vol = vol_all/8;
        }   
        
		test_log(" adc0=%d mV  vbat=%d mV\n", adc_vol, vbat_vol);
		ql_rtos_task_sleep_s(1);
        
	}

}
#endif

//application_init(quec_adc_test, "quec_adc_test", 2, 0);

