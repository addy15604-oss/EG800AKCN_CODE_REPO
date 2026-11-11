/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_COMM_API_H
#define _QL_BT_COMM_API_H

#include "ql_bt.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ql_bt_open_status)(int status, void *event_data);
typedef void (*ql_bt_scan_status)(struct bt_event_inquiry *inquiry,int result);
typedef void (*ql_bt_rssi_status)(signed char value);


/*****************************************************************
* Function: ql_bt_device_open
*
* Description:
*	Opens a Bluetooth device for use and applies the specified configuration.
*	This function is used to configure device-specific settings.
*
* Parameters:
*	cfg_confg		[in]		Pointer to the Bluetooth device configuration
*								structure.
*	cb				[in]		Callback function to receive the status of the
*								Bluetooth open operation.
*
* Return:
*	0							 Device opened successfully.
*	-1							 Failed to open the device.
*
*****************************************************************/
int ql_bt_device_open(struct bt_user_cfg *cfg_confg,ql_bt_open_status cb);


	
/*****************************************************************
* Function: ql_bt_close
*
* Description:
*	Closes the Bluetooth stack and releases any associated resources.
*	After calling this function, Bluetooth APIs will no longer be functional
*	until `ql_bt_open` is called again.
*
* Parameters:
*	None
*
* Return:
*	0							 Bluetooth stack closed successfully.
*	-1							 Failed to close the Bluetooth stack.
*
*****************************************************************/
int ql_bt_close(void);


/*****************************************************************
* Function: ql_bt_set_bt_address
*
* Description:
*	Sets the local Bluetooth device address. The new address will be used
*	for all Bluetooth operations.
*
* Parameters:
*	addr		   [in] 		Pointer to the Bluetooth address structure
*								containing the new address to be set.
*
* Return:
*	None
*
*****************************************************************/
void ql_bt_set_bt_address(struct bt_addr* addr);


/*****************************************************************
* Function: ql_bt_get_bt_address
*
* Description:
*	Retrieves the current local Bluetooth device address.
*
* Parameters:
*	addr		   [out]		Pointer to the Bluetooth address structure
*								where the current address will be stored.
*
* Return:
*	None
*
*****************************************************************/
void ql_bt_get_bt_address(struct bt_addr* addr);


/*****************************************************************
* Function: ql_bt_set_visiable
*
* Description:
*	Sets the visibility of the local Bluetooth device for discovery.
*
* Parameters:
*	visiable		[in]		Visibility state: 
*								1 for visible, 0 for non-visible.
*
* Return:
*	0							 Visibility set successfully.
*	-1							 Failed to set visibility.
*
*****************************************************************/
int ql_bt_set_visiable(int visiable);

/*****************************************************************
* Function: ql_bt_bonding
*
* Description:
*	Initiates a bonding (pairing) process with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device to pair with.
*
* Return:
*	0							 Bonding initiated successfully.
*	-1							 Failed to initiate bonding.
*
*****************************************************************/
int ql_bt_bonding(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_del_bond
*
* Description:
*    Deletes an existing bonding record for the specified device.
*    This function locates the device record by its address, clears 
*    the record, updates the stored records, and removes the bond 
*    association from the system.
*
* Parameters:
*    addr            [in]    Address of the device to unbond.
*
* Return:
*    0                       Bonding record deleted successfully.
*   -1                       Failed to find the bonding record.
*
*****************************************************************/
int ql_bt_del_bond(struct bt_addr addr);


/*****************************************************************
* Function: ql_bt_accept_bonding
*
* Description:
*	Accepts or rejects a bonding request from another device.
*
* Parameters:
*	addr			[in]		Address of the device requesting bonding.
*	accept_reject	[in]		1 to accept, 0 to reject the bonding request.
*
* Return:
*	0							 Operation completed successfully.
*	-1							 Failed to process the bonding request.
*
*****************************************************************/
int ql_bt_accept_bonding(struct bt_addr addr, int accept_reject);

/*****************************************************************
* Function: ql_bt_add_device_record
*
* Description:
*	Adds a device record to the Bluetooth device list.
*
* Parameters:
*	device			[in]		Pointer to the device record to be added.
*
* Return:
*	None
*
*****************************************************************/
void ql_bt_add_device_record(struct bt_device_record *device);

/*****************************************************************
* Function: ql_bt_get_device_records
*
* Description:
*	Retrieves all stored device records.
*
* Parameters:
*	info			[out]		Pointer to the array of device records.
*
* Return:
*	Number of device records retrieved, or -1 on failure.
*
*****************************************************************/
int ql_bt_get_device_records(struct bt_device_record **info);


/*****************************************************************
* Function: ql_bt_inquiry_ex
*
* Description:
*	Starts a Bluetooth device discovery process with a callback for scan status.
*
* Parameters:
*	inquiry_length	[in]		Duration of the inquiry in seconds.
*	num_responses	[in]		Maximum number of devices to discover.
*	cb				[in]		Callback function to receive scan status updates.
*
* Return:
*	0							 Inquiry started successfully.
*	-1							 Failed to start inquiry.
*
*****************************************************************/
int ql_bt_inquiry_ex(unsigned char inquiry_length, unsigned char num_responses, ql_bt_scan_status cb);

/*****************************************************************
* Function: ql_bt_inquiry_cancel
*
* Description:
*	Cancels an ongoing Bluetooth device inquiry process.
*
* Parameters:
*	None
*
* Return:
*	0							 Inquiry cancelled successfully.
*	-1							 Failed to cancel inquiry.
*
*****************************************************************/
int ql_bt_inquiry_cancel(void);


/*****************************************************************
* Function: ql_read_rssi_ex
*
* Description:
*	Reads the RSSI of a connected device and provides the result via callback.
*
* Parameters:
*	handle			[in]		Connection handle of the device.
*	cb				[in]		Callback function to receive RSSI status.
*
* Return:
*	0							 RSSI request sent successfully.
*	-1							 Failed to send RSSI request.
*
*****************************************************************/
int ql_read_rssi_ex(unsigned short handle, ql_bt_rssi_status cb);


/*****************************************************************
* Function: ql_bt_change_local_name
*
* Description:
*	Changes the local Bluetooth device name.
*
* Parameters:
*	name			[in]		Pointer to the new name string.
*
* Return:
*	0							 Name changed successfully.
*	-1							 Failed to change the name.
*
*****************************************************************/
int ql_bt_change_local_name(const char *name);

/*****************************************************************
* Function: ql_bt_pin_reply
*
* Description:
*	Sends a PIN code reply to a pairing request from a remote device.
*
* Parameters:
*	addr			[in]		Address of the remote device requesting pairing.
*	pin 			[in]		Pointer to the PIN code string.
*
* Return:
*	0							 PIN reply sent successfully.
*	-1							 Failed to send the PIN reply.
*
*****************************************************************/
int ql_bt_pin_reply(struct bt_addr addr, char *pin);

/*****************************************************************
* Function: ql_bt_pin_negative_reply
*
* Description:
*	Sends a negative reply to reject a pairing request from a remote device.
*
* Parameters:
*	addr			[in]		Address of the remote device requesting pairing.
*
* Return:
*	0							 Negative reply sent successfully.
*	-1							 Failed to send the negative reply.
*
*****************************************************************/
int ql_bt_pin_negative_reply(struct bt_addr addr);


/*****************************************************************
* Function: ql_bt_set_sniff_mode
*
* Description:
*	Enables or disables sniff mode for low-power operation.
*
* Parameters:
*	enable			[in]		1 to enable sniff mode, 0 to disable.
*
* Return:
*	0							 Sniff mode configuration successful.
*	-1							 Failed to configure sniff mode.
*
*****************************************************************/
int ql_bt_set_sniff_mode(unsigned char enable);


/*****************************************************************
* Function: ql_bt_bt_dut
*
* Description:
*	Puts the Bluetooth module into Device Under Test (DUT) mode.
*
* Parameters:
*	None
*
* Return:
*	0							 DUT mode activated successfully.
*	-1							 Failed to activate DUT mode.
*
*****************************************************************/
int ql_bt_bt_dut(void);


/*****************************************************************
* Function: ql_bt_acl_connect
*
* Description:
*	Establishes an ACL (Asynchronous Connection-Less) connection
*	with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*
* Return:
*	0							 Connection initiated successfully.
*	-1							 Failed to initiate connection.
*
*****************************************************************/
int ql_bt_acl_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_cancel_acl_connect
*
* Description:
*	Cancels an ongoing ACL connection attempt with the specified device.
*
* Parameters:
*	addr			[in]		Address of the target device.
*
* Return:
*	0							 Connection cancellation successful.
*	-1							 Failed to cancel the connection.
*
*****************************************************************/
int ql_bt_cancel_acl_connect(struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_acl_disconnect
*
* Description:
*	Disconnects an established ACL connection.
*
* Parameters:
*	handle			[in]		Connection handle of the ACL link.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_acl_disconnect(unsigned short int handle);







#ifdef __cplusplus
} /*"C" */
#endif

#endif



