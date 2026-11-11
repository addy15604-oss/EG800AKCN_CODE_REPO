
/**  
  @file
  ql_at.h

  @brief
  This file provides the definitions for at functions, and declares the 
  common functions.

*/
/*============================================================================
  Copyright (c) 2017 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------
11/03/2020  Nebula.li      create
=============================================================================*/

#ifndef __QL_AT_H__
#define __QL_AT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QL_AT_RESP_STATUS_ENUM{
	QL_AT_RESP_OK = 0,
	QL_AT_RESP_ERROR,
	QL_AT_RESP_CME_ERROR,
	QL_AT_RESP_CMS_ERROR,
	QL_AT_RESP_INVALID_PARAM,
	QL_AT_RESP_TIME_OUT,
	QL_AT_RESP_SYS_ERROR,
}QL_AT_RESP_STATUS_E;

typedef void(*ql_atcmd_asyc_cb)(QL_AT_RESP_STATUS_E errno,char * buffer,unsigned int len);
QL_AT_RESP_STATUS_E ql_atcmd_send_sync(char *cmd,char *resp,int resp_len,char *include_str,int time_out);
QL_AT_RESP_STATUS_E ql_atcmd_send_sync_by_simid (char *cmd,char *resp,int resp_len,char *include_str,int time_out, unsigned short simid);

/***********************************************************************
 *
 * Name:   ql_atcmd_asyc_cb    
 *
 * Description: 于ql_atcmd_async_init中注册的回调函数
 *
 * Parameters:  errno:	接口返回结果，参考枚举QL_AT_RESP_STATUS_E
 *              buffer:	AT指令执行返回结果，即urc内容
 *				len:	AT指令执行返回结果的长度
 *
 * Notes:    1.此回调在ql_atcmd_async_init中注册，在收到AT的返回结果后被执行，从
 *		 而上报给app端，此回调中请不要使用过大的数组等变量，因为这个回调的栈是在
 *		 ql_atcmd_async_init中第二个参数创建的一个AT结果处理task内，栈大小为指定
 *		 的stack_size，所以回调中请注意不要造成栈溢出;
 *		     2.此回调中不要执行过多耗时操作，以防止阻塞接收AT处理结果的流程,导致
 *		 下一次AT回调不及时等问题。
 *
 ***********************************************************************/
typedef void(*ql_atcmd_asyc_cb)(QL_AT_RESP_STATUS_E errno,char * buffer,unsigned int len);


/***********************************************************************
 *
 * Name:   ql_atcmd_send_async    
 *
 * Description: 发送AT指令，不会阻塞当前线程
 *
 * Parameters:  data：at字符串
 *              len： at字符串长度
 * Returns: 0: SUCCESS
 *
 * Notes:
 *
 ***********************************************************************/
QL_AT_RESP_STATUS_E ql_atcmd_send_async(char *data , unsigned int len);
/***********************************************************************
 *
 * Name:   ql_atcmd_send_async    
 *
 * Description: 删除异步at运行环境
 *
 * Parameters:  VOID
 *              
 * Returns: 0 :SUCCESS
 *
 * Notes:
 *
 ***********************************************************************/

int ql_atcmd_async_close(void);
/***********************************************************************
 *
 * Name:   ql_atcmd_send_async    
 *
 * Description: 初始化异步at
 *
 * Parameters: stack_ptr：  异步at运行栈起始地址可以为NULL;
 *             stack_size： 运行栈大小
 *             cb：用户回调函数
 * Returns:    0：success
 *
 * Notes:
 *
 ***********************************************************************/
int ql_atcmd_async_init(void *stack_ptr,unsigned int stack_size,ql_atcmd_asyc_cb cb);





int ql_atcmd_get_line_num(char *result);
int ql_atcmd_get_para_num_one_line(char *result,int line_index);
int ql_atcmd_get_para_value_str(char *result,int line_index,int para_index, char *para_value,int para_value_len);


#ifdef __cplusplus
}
#endif

#endif

