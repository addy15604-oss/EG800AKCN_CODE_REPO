/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QL_IIC_H
#define _QL_IIC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	STANDARD_MODE = 0,	/*100Kbps*/
	FAST_MODE = 1,		/*400Kbps*/
	HS_MODE = 2,		/*3.4 Mbps slave/3.3 Mbps master,standard mode when not doing a high speed transfer*/
	HS_MODE_FAST = 3,	/*3.4 Mbps slave/3.3 Mbps master,fast mode when not doing a high speed transfer*/
} QL_I2C_MODE_E;


/*****************************************************************
* Function:     ql_i2c_init 
* 
* Description:
*               This function initialize the configurations for an IIC channel.
*               including the specified pins for IIC, IIC type, and IIC channel No.
*
* Parameters:
*               i2c_no:
*                   i2c channel No, 与gpio对应关系如下，不同项目开放的资源存在差异，请按照下面对应关系选择.
*
*               	   GPIO_NUM			 I2C_NO
*						49-50	  ---->		0
*						10-11	  ---->		1
*						12-13	  ---->		2
*						25-26	  ---->		4
*               
*               fastMode:
*                   i2c clock speed,refer to struct QL_I2C_MODE_E.
*                   Now only support STANDARD_MODE and FAST_MODE.
* Return:        
*               0 , the function execution succeed.
*               -1, the parameter error.
*****************************************************************/
int ql_i2c_init(unsigned char i2c_no, unsigned int fastMode);

/*****************************************************************
* Function:     ql_i2c_write 
* Description:
*               This function initialize the configurations for an IIC channel.
*               including the specified pins for IIC, IIC type, and IIC channel No.
*
* Parameters:
*               i2c_no  : i2c channel No, the range is 0 or 2,EC100Y set 0,EC600S set 1.
*               slaveaddress: slave address
*				addr	: Address of the register that stores data
*				data	: write data buffer
*				datalen : data length
* Return:        
*               0 , the function execution succeed.
*               -1, the parameter error.
*****************************************************************/
int ql_i2c_write(unsigned char i2c_no, unsigned short slaveaddress, unsigned short addr, unsigned char *data, unsigned int datalen);

/*****************************************************************
* Function:     ql_i2c_write 
* Description:
*               This function initialize the configurations for an IIC channel.
*               including the specified pins for IIC, IIC type, and IIC channel No.
*
* Parameters:
*               i2c_no  : i2c channel No, the range is 0 or 2,EC100Y set 0,EC600S set 1.
*               slaveaddress: slave address
*				addr	: Register address to read data from
*				data	: read data buffer
*				datalen : data length
* Return:        
*               0 , the function execution succeed.
*               -1, the parameter error.
*****************************************************************/
int ql_i2c_read(unsigned char i2c_no, unsigned short slaveaddress, unsigned short addr, unsigned char *data, unsigned int datalen);

/*****************************************************************
* Function:     ql_i2c_read_with_delay 
* Description:
*               This function reads data from an I2C device with a configurable delay
*               after writing the register address and before sending the read request.
*               The function first writes the register address to the device, then waits
*               for the specified delay time, and finally reads the data from the register.
*
* Parameters:
*               i2c_no  : i2c channel No, the range is 0 or 2,EC100Y set 0,EC600S set 1.
*               slaveaddress: slave address
*				addr	: Register address to read data from
*				data	: read data buffer
*				datalen : data length
*               delay_ms: After writing the register address, delay for the specified number of milliseconds before sending the read request
* Return:        
*               0 , the function execution succeed.
*               -1, the parameter error.
*****************************************************************/
int ql_i2c_read_with_delay(unsigned char i2c_no, unsigned short slaveaddress, unsigned short addr, unsigned char *data, unsigned int datalen, unsigned int delay_ms);

#ifdef __cplusplus
} /*"C" */
#endif



#endif
