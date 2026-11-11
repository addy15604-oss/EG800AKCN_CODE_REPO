/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

	
#ifndef _QL_KEYPAD_H
#define _QL_KEYPAD_H
	

	
#ifdef __cplusplus
	extern "C" {
#endif

#define QL_KEYPAD_MATRIX_LENGTH 8

typedef struct ql_event_data_struct {
	int event_id;
	int param_01;
	int param_02;
	int param_03;
	int key_v;
} ql_event_data_t;

typedef enum QL_KPAD_MKOUT_ENUM{
	QL_KP_OUT0 = 0,
	QL_KP_OUT1,
	QL_KP_OUT2,
	QL_KP_OUT3,
	QL_KP_OUT4,
	QL_KP_OUT5,
	QL_KP_OUT6,
	QL_KP_OUT7,
	QL_KP_OUT_NO_VALID
}QL_KPAD_MKOUT_E;

typedef enum QL_KPAD_MKIN_ENUM{
	QL_KP_IN0 = 0,
	QL_KP_IN1,
	QL_KP_IN2,
	QL_KP_IN3,
	QL_KP_IN4,
	QL_KP_IN5,
	QL_KP_IN6,
	QL_KP_IN7,
	QL_KP_IN_NO_VALID
}QL_KPAD_MKIN_E;

typedef unsigned int (*ql_event_cb)(ql_event_data_t);
/***********************************************************************
 *
 * Name:  ql_keypad_Register
 *
 * Description: 注册keypad中断回调函数
 *
 * Parameters:  返回keypad参数
 *              
 * Returns:     0：send_success
 *              other: faile
 * Notes:
 *
 ***********************************************************************/
int ql_keypad_Register(ql_event_cb event_cb);
/***********************************************************************
 *
 * Name:  ql_lcd_init 
 *
 * Description: 使能keypad
 *
 * Parameters:  1使能0去使能
 *              
 * Returns:     null
 *              
 * Notes:
 *
 ***********************************************************************/
void ql_keypad_MultikeyEn(unsigned char enable);
/***********************************************************************
 *
 * Name:  ql_lcd_init 
 *
 * Description: 初始化keypad参数
 *
 * Parameters:  multi_key默认为0 key_hold_dura防抖时间 row按键行 col按键列
 *              
 * Returns:     0：send_success
 *              other: faile
 * Notes:
 *
 ***********************************************************************/
int ql_keypad_Config(unsigned char multi_key, unsigned char key_hold_dura, QL_KPAD_MKOUT_E row[QL_KEYPAD_MATRIX_LENGTH], QL_KPAD_MKIN_E col[QL_KEYPAD_MATRIX_LENGTH]);


#ifdef __cplusplus
} /*"C" */
#endif

#endif


