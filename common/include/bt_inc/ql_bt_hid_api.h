/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_HID_API_H
#define _QL_BT_HID_API_H


#include "ql_bt.h"

#ifdef __cplusplus
extern "C" {
#endif


//hid profile

/*****************************************************************
* Function: ql_bt_hid_connect
*
* Description:
*	Establishes a HID (Human Interface Device) profile connection
*	with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target HID device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_hid_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_hid_disconnect
*
* Description:
*	Disconnects the current HID profile connection.
*
* Parameters:
*	None
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_hid_disconnect(void);




#ifdef __cplusplus
} /*"C" */
#endif

#endif



