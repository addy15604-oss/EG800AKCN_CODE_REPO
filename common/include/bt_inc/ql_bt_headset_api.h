/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_HEADSET_API_H
#define _QL_BT_HEADSET_API_H

#include "ql_bt.h"


#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************
* Function: ql_bt_a2dp_connect
*
* Description:
*	Establishes an A2DP (Advanced Audio Distribution Profile) connection
*	with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target A2DP device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_a2dp_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_a2dp_disconnect
*
* Description:
*	Disconnects the current A2DP connection.
*
* Parameters:
*	None
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_a2dp_disconnect(void);

/*****************************************************************
* Function: ql_bt_a2dp_send_start
*
* Description:
*	Starts sending audio data using the A2DP connection.
*
* Parameters:
*	a2dp			[in]		Pointer to the A2DP structure containing the audio data.
*
* Return:
*	0							 Data streaming started successfully.
*	-1							 Failed to start streaming.
*
*****************************************************************/
int ql_bt_a2dp_send_start(struct appbt_a2dp *a2dp);

/*****************************************************************
* Function: ql_bt_a2dp_send_suspend
*
* Description:
*	Suspends the current audio streaming over the A2DP connection.
*
* Parameters:
*	None
*
* Return:
*	0							 Streaming suspended successfully.
*	-1							 Failed to suspend streaming.
*
*****************************************************************/
int ql_bt_a2dp_send_suspend(void);

/*****************************************************************
* Function: ql_bt_a2dp_send_pcm_data
*
* Description:
*	Sends PCM audio data over the established A2DP connection.
*
* Parameters:
*	data			[in]		Pointer to the PCM data to be sent.
*	length			[in]		Length of the PCM data.
*
* Return:
*	0							 Data sent successfully.
*	-1							 Failed to send data.
*
*****************************************************************/
int ql_bt_a2dp_send_pcm_data(unsigned char *data, int length);

/*****************************************************************
* Function: ql_bt_a2dp_set_volume
*
* Description:
*	Sets the volume for the A2DP connection.
*
* Parameters:
*	volume			[in]		Volume level to set (0-100).
*
* Return:
*	0							 Volume set successfully.
*	-1							 Failed to set volume.
*
*****************************************************************/
int ql_bt_a2dp_set_volume(unsigned char volume);

/*****************************************************************
* Function: ql_bt_a2dp_headset_absolute_volume_enable
*
* Description:
*	Enables or disables absolute volume control for A2DP headset.
*
* Parameters:
*	enable			[in]		1 to enable absolute volume control, 0 to disable.
*
* Return:
*	None
*
*****************************************************************/
void ql_bt_a2dp_headset_absolute_volume_enable(unsigned char enable);



/*****************************************************************
* Function: ql_bt_avrcp_connect
*
* Description:
*	Establishes an AVRCP (Audio/Video Remote Control Profile) connection
*	with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target AVRCP device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_avrcp_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_avrcp_disconnect
*
* Description:
*	Disconnects the current AVRCP connection.
*
* Parameters:
*	None
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_avrcp_disconnect(void);


//HFP profile

/*****************************************************************
* Function: ql_bt_hfp_connect
*
* Description:
*	Establishes an HFP (Hands-Free Profile) connection with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*	call_status 	[in]		Initial call status (e.g., ongoing or idle).
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_hfp_connect(struct bt_addr addr, int call_status);

/*****************************************************************
* Function: ql_bt_hfp_disconnect
*
* Description:
*	Disconnects the current HFP connection.
*
* Parameters:
*	None
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_hfp_disconnect(void);

/*****************************************************************
* Function: ql_bt_hfp_accept_connection
*
* Description:
*	Accepts an incoming HFP connection from a remote device.
*
* Parameters:
*	call_status 	[in]		Call status after accepting the connection.
*	addr			[in]		Address of the remote device requesting connection.
*
* Return:
*	0							 Connection accepted successfully.
*	-1							 Failed to accept connection.
*
*****************************************************************/
int ql_bt_hfp_accept_connection(int call_status, struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_hfp_reject_connection
*
* Description:
*	Rejects an incoming HFP connection from a remote device.
*
* Parameters:
*	addr			[in]		Address of the remote device requesting connection.
*
* Return:
*	0							 Connection rejected successfully.
*	-1							 Failed to reject connection.
*
*****************************************************************/
int ql_bt_hfp_reject_connection(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_hfp_update_callstatus
*
* Description:
*	Updates the current call status (e.g., call in progress, on hold, etc.).
*
* Parameters:
*	index			[in]		CIEV index specifying the type of call status.
*	value			[in]		Value of the call status (e.g., 0 = no call, 1 = call active).
*	delay			[in]		Delay before updating the status, in milliseconds.
*
* Return:
*	0							 Call status updated successfully.
*	-1							 Failed to update call status.
*
*****************************************************************/
int ql_bt_hfp_update_callstatus(enum ciev_index index, int value, int delay);

/*****************************************************************
* Function: ql_bt_hfp_update_phone_number
*
* Description:
*	Updates the phone number during an HFP call.
*
* Parameters:
*	num 			[in]		Phone number to update.
*	type			[in]		Type of number (e.g., mobile, home, etc.).
*
* Return:
*	0							 Phone number updated successfully.
*	-1							 Failed to update phone number.
*
*****************************************************************/
int ql_bt_hfp_update_phone_number(const char *num, int type);

/*****************************************************************
* Function: ql_bt_hfp_update_callhold
*
* Description:
*	Updates the call hold status (e.g., hold or resume).
*
* Parameters:
*	status			[in]		Call hold status (e.g., 1 for hold, 0 for resume).
*
* Return:
*	0							 Call hold status updated successfully.
*	-1							 Failed to update call hold status.
*
*****************************************************************/
int ql_bt_hfp_update_callhold(int status);

/*****************************************************************
* Function: ql_bt_hfp_callhold
*
* Description:
*	Places a call on hold or rejects an incoming call.
*
* Parameters:
*	reject			[in]		1 to reject the call, 0 to hold the call.
*
* Return:
*	0							 Call hold or rejection successful.
*	-1							 Failed to hold or reject call.
*
*****************************************************************/
int ql_bt_hfp_callhold(int reject);

/*****************************************************************
* Function: ql_bt_hfp_voice_recogonition
*
* Description:
*	Enables or disables voice recognition in HFP.
*
* Parameters:
*	enable			[in]		1 to enable voice recognition, 0 to disable.
*
* Return:
*	None
*
*****************************************************************/
int ql_bt_hfp_voice_recogonition(int enable);

/*****************************************************************
* Function: ql_bt_hfp_set_speaker_gain
*
* Description:
*	Sets the gain (volume) level of the speaker during a call.
*
* Parameters:
*	gain			[in]		Gain value to set (e.g., from 0 to 100).
*
* Return:
*	0							 Speaker gain set successfully.
*	-1							 Failed to set speaker gain.
*
*****************************************************************/
int ql_bt_hfp_set_speaker_gain(int gain);

/*****************************************************************
* Function: ql_bt_hfp_atchld
*
* Description:
*	Executes the AT+CHLD command to manage call hold and multiparty calls.
*
* Parameters:
*	msg 			[in]		Pointer to the task event structure for CHLD command.
*
* Return:
*	None
*
*****************************************************************/
void ql_bt_hfp_atchld(struct bt_task_event *msg);

/*****************************************************************
* Function: ql_bt_switch_voice_to_phone
*
* Description:
*	Switches the voice call from the headset to the phone.
*
* Parameters:
*	None
*
* Return:
*	0							 Voice switched successfully.
*	-1							 Failed to switch voice to phone.
*
*****************************************************************/
int ql_bt_switch_voice_to_phone(void);

/*****************************************************************
* Function: ql_bt_switch_voice_to_headset
*
* Description:
*	Switches the voice call from the phone to the headset.
*
* Parameters:
*	None
*
* Return:
*	0							 Voice switched successfully.
*	-1							 Failed to switch voice to headset.
*
*****************************************************************/
int ql_bt_switch_voice_to_headset(void);

/*****************************************************************
* Function: ql_bt_hfp_set_microphone_gain
*
* Description:
*	Sets the gain (volume) level of the microphone during a call.
*
* Parameters:
*	gain			[in]		Gain value to set (e.g., from 0 to 100).
*
* Return:
*	0							 Microphone gain set successfully.
*	-1							 Failed to set microphone gain.
*
*****************************************************************/
int ql_bt_hfp_set_microphone_gain(int gain);

// SCO profile

/*****************************************************************
* Function: ql_bt_sco_connect
*
* Description:
*	Establishes an SCO (Synchronous Connection-Oriented) connection with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_sco_connect(struct bt_addr addr);


/*****************************************************************
* Function: ql_bt_sco_disconnect
*
* Description:
*	Disconnects an SCO connection with the specified handle.
*
* Parameters:
*	handle			[in]		Connection handle for the SCO link.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_sco_disconnect(unsigned short int handle);



// headset profile


/*****************************************************************
* Function: ql_bt_connect_headset
*
* Description:
*	Establishes a connection to a Bluetooth headset with the specified device address.
*
* Parameters:
*	addr			[in]		Address of the headset device.
*	call_status 	[in]		Call status (e.g., active or idle).
*
* Return:
*	0							 Connection successful.
*	-1							 Failed to connect.
*
*****************************************************************/
int ql_bt_connect_headset(struct bt_addr addr, int call_status);


/*****************************************************************
* Function: ql_bt_disconnect_headset
*
* Description:
*	Disconnects the Bluetooth headset connection.
*
* Parameters:
*	addr			[in]		Pointer to the address of the headset device.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_disconnect_headset(struct bt_addr *addr);



#ifdef __cplusplus
} /*"C" */
#endif

#endif



