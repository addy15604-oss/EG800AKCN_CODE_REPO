/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_OBEX_API_H
#define _QL_BT_OBEX_API_H

#include "ql_bt.h"


#ifdef __cplusplus
extern "C" {
#endif

// OBEX profile

/*****************************************************************
* Function: ql_bt_obex_connect
*
* Description:
*	Establishes an OBEX (Object Exchange) connection with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_obex_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_obex_disconnect
*
* Description:
*	Disconnects an OBEX connection with the specified transaction ID.
*
* Parameters:
*	tid 			[in]		Transaction ID associated with the OBEX connection.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_obex_disconnect(int tid);

/*****************************************************************
* Function: ql_bt_obex_send_file
*
* Description:
*	Sends a file over an established OBEX connection.
*
* Parameters:
*	file			[in]		Pointer to the OBEX file transfer structure.
*
* Return:
*	0							 File sent successfully.
*	-1							 Failed to send file.
*
*****************************************************************/
int ql_bt_obex_send_file(struct bt_task_obex_file_send *file);

/*****************************************************************
* Function: ql_bt_obex_send_response
*
* Description:
*	Sends a response over the OBEX connection with the specified transaction ID.
*
* Parameters:
*	tid 			[in]		Transaction ID for the OBEX connection.
*	response_code	[in]		Response code to send (e.g., success, error).
*
* Return:
*	0							 Response sent successfully.
*	-1							 Failed to send response.
*
*****************************************************************/
int ql_bt_obex_send_response(int tid, unsigned char response_code);





#ifdef __cplusplus
} /*"C" */
#endif

#endif



