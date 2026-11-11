/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QL_DEV_H
#define _QL_DEV_H

typedef enum
{
	QL_DEV_SIM_0,
	QL_DEV_SIM_1,
	QL_NUM_OF_DEV_SIM,
}QL_Dev_SimID;

typedef enum 
{
	QL_DEV_SUCCESS,
	QL_DEV_GENERIC_FAILURE,
}QL_DEV_ERROR_CODE;

typedef enum
{
	QL_DEV_MODEM_MIN_FUN = 0 ,								//modem最小功能
	QL_DEV_MODEM_FULL_FUN,									//modem全功能
	QL_DEV_MODEM_DISABLE_RECEIVE_RF_CIRCUITS=3,				//modem关闭射频接收电路
	QL_DEV_MODEM_DISABLE_TRANSMIT_AND_RECEIVE_RF_CIRCUITS,	//modem关闭射频发射和接收电路
	QL_DEV_MODEM_DISABLE_SIM,								//modem关闭 SIM卡
	QL_DEV_MODEM_TURN_OFF_FULL_SECONDARY_RECEIVE,			//modem完全关闭辅助接收
}QL_DEV_MODEM_FUNCTION;

/*****************************************************************
* Function: ql_dev_get_model
*
* Description:
* 	获取设备型号。
* 
* Parameters:
* 	model	  		[out] 	设备型号，字符串。 
* 	model_len	  	[in] 	model 数组的长度。  
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_model(char*model,unsigned int model_len);



/*****************************************************************
* Function: ql_dev_get_sn
*
* Description:
* 	获取设备 SN。 
* 
* Parameters:
* 	sn	  		[out] 	设备 SN，字符串。 
* 	sn_len	  	[in] 	sn 数组的长度
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_sn(char*sn, unsigned int sn_len);

/*****************************************************************
* Function: ql_dev_set_modem_fun
*
* Description:
* 	设置设备的 modem 功能。
* 
* Parameters:
* 	function	  	[in] 	需要设置的 modem 功能，见 4.1 QL_DEV_MODEM_FUNCTION。 
* 	rst	  			[in] 	设置 modem 功能前，是否重启 modem。0：不重启；1：重启。 
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_set_modem_fun(QL_DEV_MODEM_FUNCTION function,int rst);

/*****************************************************************
* Function: ql_dev_get_modem_fun
*
* Description:
* 	获取设备的 modem 当前的功能。
* 
* Parameters:
* 	function	  		[out] 	返回设备的 modem 当前的功能，值含义见 4.1 QL_DEV_MODEM_FUNCTION。 
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_modem_fun(int* function);

/*****************************************************************
* Function: ql_dev_get_firmware_version
*
* Description:
* 	获取设备的固件版本。 
* 
* Parameters:
* 	version	  		[out] 	设备的固件版本，字符串。
* 	version_len	  	[in] 	version 数组的长度 
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_firmware_version(char *version,unsigned int version_len);

/*****************************************************************
* Function: ql_dev_get_firmware_version
*
* Description:
* 	获取客户设备的固件版本。 
* 
* Parameters:
* 	version	  		[out] 	客户设备的固件版本，字符串。
* 	version_len	  	[in] 	version 数组的长度 
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_cust_firmware_version(char *version,unsigned int version_len);

/*****************************************************************
* Function: ql_dev_get_imei
*
* Description:
* 	获取设备的 IMEI。
* 
* Parameters:
* 	imei	  		[out] 	设备的 IMSI，字符串。 
* 	imei_len	  	[in] 	imei 数组的长度。 
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_imei(char *imei,unsigned int imei_len);

/*****************************************************************
* Function: ql_dev_get_imei_by_simid
*
* Description:
* 	获取设备的 IMEI。
* 
* Parameters:
*   simid			[in] SIM卡号
* 	imei	  		[out] 	设备的 IMSI，字符串。 
* 	imei_len	  	[in] 	imei 数组的长度。 
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_imei_by_simid(QL_Dev_SimID simid,char *imei,unsigned int imei_len);


/*****************************************************************
* Function: ql_dev_get_cpu_uid
*
* Description:
* 	获取设备的cpu_uid。 
* 
* Parameters:
* 	uid_ret	  		[in] 用于保存设备的cpu_uid。
*
* Return:
* 	无。
*
*****************************************************************/
void ql_dev_get_cpu_uid(unsigned long long * uid_ret);



/*****************************************************************
* Function: ql_dev_get_product_id
*
* Description:
* 	获取设备的制造商 id。
* 
* Parameters:
* 	product_id	  		[out] 	返回设备的制造商 id。
* 	product_id_len	  	[in]    需要获取的制造商id的长度。此长度必须小于或者等于product_id所能存储的长度
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_product_id(char* product_id,unsigned int product_id_len);

/*****************************************************************
* Function: ql_dev_get_qverinfo
*
* Description:
* 	在APP测实现获取系统的Release_Version、DSP、RF版本号。 
* 
* Parameters:
* 	release_version		[in] 	用于存放系统的版本号。
* 	len	  　　	        [out] 	版本号数组的长度 
*
* Return:
* 	len	             成功，返回版本号总长度。
*	-1               失败。
*
*****************************************************************/
int  ql_dev_get_qverinfo(  char *release_version,unsigned int len);

/*****************************************************************
* Function: ql_dev_get_hw_version
*
* Description:
	恒科定制
* 	获取硬件版本号。 
* 
* Parameters:
* 	hw_version	  		[out] 	设备 hw_version，字符串。 
* 	hw_len	  			[in] 	hw_version 数组的长度
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
int ql_dev_get_hw_version(char* hw_version, unsigned int hw_len);

/*****************************************************************
* Function: ql_dev_get_sn_1
*
* Description:
* 	获取设备 SN。 
* 
* Parameters:
* 	sn	  		[out] 	sn1输出到的数组 
* 	sn_len	  	[in] 	需要获取的sn1号长度,长度不限,但最多返回30字节
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_get_sn_1(char*sn, unsigned int sn_len);

/*****************************************************************
* Function: ql_dev_get_sn_2
*
* Description:
	恒科定制
* 	获取设备 SN2。 
* 
* Parameters:
* 	sn2	  		[out] 	sn2输出到的数组
* 	sn_len	  	[in] 	需要获取的sn2号长度
*
* Return:
* 	当输入的sn_len <= 28时，返回sn_len
*	当输入的sn_len > 28时，返回-2
*	当输出的数组 == NULL || sn_len == 0时，返回-1
*
*****************************************************************/
int ql_dev_get_sn_2(char* sn2, unsigned int sn_len);

/*****************************************************************
* Function: ql_dev_set_imei
*
* Description:
* 	设置设备IMEI。 
* 
* Parameters:
* 	password	  		[in] 	操作密码，需填写正确的密码才允许设置imei
* 	imei	  			[in] 	需要设置的imei
* 	imei_len	  		[in] 	输入的imei长度
*
* Return:
* 	QL_DEV_SUCCESS			成功。
*	QL_DEV_GENERIC_FAILURE 	失败。
*
*****************************************************************/
QL_DEV_ERROR_CODE ql_dev_set_imei(char * password, char * imei,unsigned int imei_len);

#endif

