/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/

#ifndef _QL_WTD_H
#define _QL_WTD_H


#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
 *
 * Name:   ql_wtd_timeoutperiod_set	
 *
 * Description: 设置看门狗超时时间,同时设置active模式和sleep模式
 *              注：进入sleep模式后确保无任何唤醒源，只要有唤醒源唤醒，看门狗计数便会重置
 * Parameters:value 0~15			   
 *				0:1s      8:96s
 *   			1:2s	  9:128s	
 *              2:4s     10:192s
 *              3:8s     11:256s
 *              4:16s    12:384s    
 *              5:32s    13:512s    
 *              6:48s    14:768s    
 *              7:64s    15:1024s                    
 * Returns: 	 1 ：设置成功
 *				 0 ：设置失败
 * Notes:
 *
 ***********************************************************************/

int ql_wtd_timeoutperiod_set(unsigned char value);
/***********************************************************************
 *
 * Name:   ql_wtd_faultwake_enable	
 *
 * Description: 配置PMIC中fault_wu_en、fault_wu，用于故障事件的处理、PMIC唤醒事件等
 *              用户无需关系，参考example_wtd.c使用即可
 * Parameters:	enable		   
 *						0 :禁用faultwake,
 *   					1 :使能faultwake
 * Returns: 	 1 ：设置成功
 *			     0 ：设置失败
 * Notes:
 *
 ***********************************************************************/

int ql_wtd_faultwake_enable(unsigned char enable);
/***********************************************************************
 *
 * Name:   ql_wtd_enable	
 *
 * Description: 使能/禁止硬件看门狗功能
 *
 * Parameters:	enable		   
 *						0 ：禁用硬件看门狗,
 *   					1 ：使能硬件看门狗
 * Returns: 	 1 ：设置成功
 *			     0 ：设置失败
 * Notes:
 *
 ***********************************************************************/

int ql_wtd_enable(unsigned char enable);
/***********************************************************************
 *
 * Name:   ql_wtd_feed	
 *
 * Description: 喂狗
 *
 * Parameters:	none	   
 *						
 *   			
 * Returns: 	 1 ：设置成功
 *			     0 ：设置失败
 * Notes:
 *
 ***********************************************************************/

int ql_wtd_feed(void);


#ifdef __cplusplus
} /*"C" */
#endif

#endif
