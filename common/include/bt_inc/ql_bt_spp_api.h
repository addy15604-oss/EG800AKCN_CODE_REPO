/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_SPP_API_H
#define _QL_BT_SPP_API_H

#include "ql_bt.h"


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************
* Function: ql_bt_spp_connect
*
* Description:
*	Establishes an SPP (Serial Port Profile) connection with
*	the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_spp_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_spp_disconnect
*
* Description:
*	Disconnects the current SPP connection.
*
* Parameters:
*	spp_port		[in]		The SPP port associated with the connection.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_spp_disconnect(unsigned short spp_port);

/*****************************************************************
* Function: ql_bt_spp_send_data
*
* Description:
*	Sends data over the established SPP connection.
*
* Parameters:
*	data			[in]		Pointer to the data to be sent.
*	length			[in]		Length of the data to be sent.
*	spp_port		[in]		The SPP port associated with the connection.
*
* Return:
*	0							 Data sent successfully.
*	-1							 Failed to send data.
*
*****************************************************************/
int ql_bt_spp_send_data(unsigned char* data, unsigned short length, unsigned short spp_port);






#ifdef __cplusplus
} /*"C" */
#endif

#endif



