/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BLE_API_H
#define _QL_BLE_API_H


#include "ql_bt.h"



#ifdef __cplusplus
extern "C" {
#endif




/*****************************************************************
* Function: ql_bt_le_connect
*
* Description:
*   Establishes a LE (Low Energy) connection with the specified device.
*   The `type` parameter defines the address type (e.g., public or random).
*
* Parameters:
*   addr            [in]        Address of the target LE device.
*   type            [in]        Address type of the target device (e.g., public, random).
*
* Return:
*   0                            Connection successful.
*   -1                           Failed to connect.
*
*****************************************************************/
int ql_bt_le_connect(struct bt_addr addr, int type);

/*****************************************************************
* Function: ql_bt_le_disconnect
*
* Description:
*	Disconnects the LE connection identified by the given connection handle.
*
* Parameters:
*	con_handle		[in]		Connection handle of the LE connection.
*
* Return:
*	0							 Disconnection successful.
*	-1							 Failed to disconnect.
*
*****************************************************************/
int ql_bt_le_disconnect(unsigned short con_handle);


/*****************************************************************
* Function: ql_le_get_pair_info
*
* Description:
*	Retrieves the pairing information for a specified index.
*
* Parameters:
*	index			[in]		Index for retrieving pairing information.
*
* Return:
*	0							 Pairing information retrieved successfully.
*	-1							 Failed to retrieve pairing information.
*
*****************************************************************/
int ql_le_get_pair_info(int index);


/*****************************************************************
* Function: ql_le_remove_pair_info
*
* Description:
*	Removes pairing information for a specified index.
*
* Parameters:
*	index			[in]		Index for removing pairing information.
*
* Return:
*	0							 Pairing information removed successfully.
*	-1							 Failed to remove pairing information.
*
*****************************************************************/
int ql_le_remove_pair_info(int index);


/*****************************************************************
* Function: ql_bt_le_set_white_list
*
* Description:
*   Adds a device to the LE white list. The `type` parameter defines
*   the address type of the device (e.g., public or random).
*
* Parameters:
*   addr            [in]        Address of the device to add to the white list.
*   type            [in]        Address type of the device (e.g., public, random).
*
* Return:
*   0                            Device added to the white list successfully.
*   -1                           Failed to add device to the white list.
*
*****************************************************************/
int ql_bt_le_set_white_list(struct bt_addr addr, unsigned char type);

/*****************************************************************
* Function: ql_bt_le_remove_white_list
*
* Description:
*   Removes a device from the LE white list. The `type` parameter
*   defines the address type of the device (e.g., public or random).
*   After removal, the device will no longer be included in the
*   whitelist and cannot participate in whitelist-restricted 
*   Bluetooth operations.
*
* Parameters:
*   addr            [in]        Address of the device to remove from the white list.
*   type            [in]        Address type of the device (e.g., public, random).
*
* Return:
*   0                            Device removed from the white list successfully.
*   -1                           Failed to remove device from the white list.
*
*****************************************************************/
int ql_bt_le_remove_white_list(struct bt_addr addr, unsigned char type);


/*****************************************************************
* Function: ql_bt_le_clear_white_list
*
* Description:
*	Clears all devices from the LE white list.
*
* Parameters:
*	None
*
* Return:
*	0							 White list cleared successfully.
*	-1							 Failed to clear the white list.
*
*****************************************************************/
int ql_bt_le_clear_white_list();

/*****************************************************************
* Function: ql_bt_le_set_random_address
*
* Description:
*	Sets the random address for the LE device.
*
* Parameters:
*	addr			[in]		The random address to be set for the device.
*
* Return:
*	0							 Random address set successfully.
*	-1							 Failed to set random address.
*
*****************************************************************/
int ql_bt_le_set_random_address(struct bt_addr addr);

/*****************************************************************
* Function: ql_le_set_adv_mode
*
* Description:
*	Sets the advertising mode for LE (Low Energy) advertising.
*	The mode is defined by the following enum:
*	- `BT_LE_ADV_LEGACY_MODE`: Legacy advertising mode.
*	- `BT_LE_EXTENDED_ADV_LEGACY_MODE`: Extended legacy advertising mode.
*	- `BT_LE_EXTENDED_ADV_AUX_MODE`: Extended auxiliary advertising mode.
*
* Parameters:
*	le_adv_mode 	[in]		Advertising mode to set (one of the enum values).
*								- `BT_LE_ADV_LEGACY_MODE`: Legacy advertising mode.
*								- `BT_LE_EXTENDED_ADV_LEGACY_MODE`: Extended legacy advertising mode.
*								- `BT_LE_EXTENDED_ADV_AUX_MODE`: Extended auxiliary advertising mode.
*
* Return:
*	None
*
*****************************************************************/
void ql_le_set_adv_mode(uint8_t le_adv_mode);

/*****************************************************************
* Function: ql_bt_le_set_adv_parameters
*
* Description:
*	Sets the advertising parameters for LE (Low Energy) advertising.
*
* Parameters:
*	para			[in]		Pointer to the LE advertising parameters structure.
*
* Return:
*	0							 Parameters set successfully.
*	-1							 Failed to set advertising parameters.
*
*****************************************************************/
int ql_bt_le_set_adv_parameters(struct ql_bt_task_le_adv_parameters *para);





/*****************************************************************
* Function: ql_bt_le_create_adv_data
*
* Description:
*	Creates the advertising data for LE (Low Energy) advertising.
*
* Parameters:
*	gap 			[in]		GAP (Generic Access Profile) type.
*	buffer			[in]		Pointer to the buffer where the advertising data is stored.
*	buffer_size 	[in]		Size of the buffer.
*	data			[in]		Pointer to the actual data to be used in advertising.
*	size			[in]		Size of the advertising data.
*
* Return:
*	0							 Advertising data created successfully.
*	-1							 Failed to create advertising data.
*
*****************************************************************/
int ql_bt_le_create_adv_data(int gap, unsigned char *buffer, int buffer_size, unsigned char *data, int size);


/*****************************************************************
* Function: ql_bt_le_set_adv_data
*
* Description:
*	Sets the advertising data for LE (Low Energy) advertising.
*
* Parameters:
*	data			[in]		Pointer to the advertising data.
*	length			[in]		Length of the advertising data.
*
* Return:
*	0							 Advertising data set successfully.
*	-1							 Failed to set advertising data.
*
*****************************************************************/
int ql_bt_le_set_adv_data(unsigned char *data, unsigned char length);



/*****************************************************************
* Function: ql_le_set_scan_response_data
*
* Description:
*	Sets the scan response data for LE (Low Energy) advertising.
*
* Parameters:
*	data			[in]		Pointer to the scan response data.
*	length			[in]		Length of the scan response data.
*
* Return:
*	0							 Scan response data set successfully.
*	-1							 Failed to set scan response data.
*
*****************************************************************/
int ql_le_set_scan_response_data(unsigned char *data, unsigned char length);


/*****************************************************************
* Function: ql_bt_le_set_ext_adv_parameters
*
* Description:
*	Sets the extended advertising parameters for LE (Low Energy).
*
* Parameters:
*	para			[in]		Pointer to the extended advertising parameters structure.
*
* Return:
*	0							 Extended advertising parameters set successfully.
*	-1							 Failed to set extended advertising parameters.
*
*****************************************************************/
int ql_bt_le_set_ext_adv_parameters(struct bt_task_le_ext_adv_parameters *para);



/*****************************************************************
* Function: ql_bt_le_set_adv_set_random_address
*
* Description:
*	Sets a random address for a specific LE advertising set.
*
* Parameters:
*	handle			[in]		Handle of the advertising set.
*	addr			[in]		Random address to be set for the advertising set.
*
* Return:
*	0							 Random address set successfully.
*	-1							 Failed to set random address.
*
*****************************************************************/
int ql_bt_le_set_adv_set_random_address(unsigned char handle, struct bt_addr addr);

/*****************************************************************
* Function: ql_bt_le_set_ext_adv_data
*
* Description:
*	Sets the extended advertising data for LE (Low Energy) advertising.
*
* Parameters:
*	data			[in]		Pointer to the advertising data.
*	length			[in]		Length of the advertising data.
*	handle			[in]		Handle for the advertising set.
*	operation		[in]		Operation to perform (e.g., insert or update).
*	fragment		[in]		Fragmentation behavior for the data.
*
* Return:
*	0							 Advertising data set successfully.
*	-1							 Failed to set extended advertising data.
*
*****************************************************************/
int ql_bt_le_set_ext_adv_data(unsigned char *data, unsigned char length, unsigned char handle, unsigned char operation, unsigned char fragment);


/*****************************************************************
* Function: ql_bt_le_set_ext_scan_response
*
* Description:
*	Sets the extended scan response data for LE (Low Energy) advertising.
*
* Parameters:
*	data			[in]		Pointer to the scan response data.
*	length			[in]		Length of the scan response data.
*	handle			[in]		Handle for the advertising set.
*	operation		[in]		Operation to perform (e.g., insert or update).
*	fragment		[in]		Fragmentation behavior for the data.
*
* Return:
*	0							 Scan response data set successfully.
*	-1							 Failed to set extended scan response data.
*
*****************************************************************/
int ql_bt_le_set_ext_scan_response(unsigned char *data, unsigned char length, unsigned char handle, unsigned char operation, unsigned char fragment);



/*****************************************************************
* Function: ql_bt_le_set_adv_enable
*
* Description:
*	Enables or disables LE (Low Energy) advertising.
*
* Parameters:
*	enable			[in]		1 to enable advertising, 0 to disable advertising.
*
* Return:
*	0							 Advertising enabled or disabled successfully.
*	-1							 Failed to enable or disable advertising.
*
*****************************************************************/
int ql_bt_le_set_adv_enable(int enable);

/*****************************************************************
* Function: ql_ble_set_adv_cfg_enable
*
* Description:
*	Enables or disables the advertising configuration for LE (Low Energy) advertising.
*
* Parameters:
*	params			[in]		Pointer to the advertising parameters structure.
*	enable			[in]		1 to enable advertising configuration, 0 to disable.
*
* Return:
*	0							 Configuration enabled or disabled successfully.
*	-1							 Failed to enable or disable advertising configuration.
*
*****************************************************************/
int ql_ble_set_adv_cfg_enable(struct ql_bt_adv_params* params, int enable);


/*****************************************************************
* Function: ql_bt_le_set_pair_enable
*
* Description:
*	Enables or disables LE (Low Energy) pairing.
*
* Parameters:
*	set_enable		[in]		1 to enable pairing, 0 to disable pairing.
*
* Return:
*	0							 Pairing enabled or disabled successfully.
*	-1							 Failed to enable or disable pairing.
*
*****************************************************************/
int ql_bt_le_set_pair_enable(unsigned char set_enable);


/*****************************************************************
* Function: ql_bt_le_set_default_phy
*
* Description:
*	Sets the default PHY (physical layer) for LE (Low Energy) connections.
*
* Parameters:
*	all_phys		[in]		PHY for all connections (bitmask of supported PHY types).
*	tx_phys 		[in]		PHY to be used for transmitting (e.g., 1M, 2M, Coded).
*	rx_phys 		[in]		PHY to be used for receiving (e.g., 1M, 2M, Coded).
*
* Return:
*	0							 PHY settings applied successfully.
*	-1							 Failed to set PHY.
*
*****************************************************************/
int ql_bt_le_set_default_phy(unsigned char all_phys, unsigned char tx_phys, unsigned char rx_phys);


/*****************************************************************
* Function: ql_bt_le_register_att_service
*
* Description:
*	Registers an ATT (Attribute Protocol) service with read and write callback functions.
*
* Parameters:
*	profile_data	[in]		Pointer to the profile data for the ATT service.
*	profile_data_len [in]		Length of the profile data.
*	read_callback	[in]		Callback function for handling read requests.
*	write_callback	[in]		Callback function for handling write requests.
*
* Return:
*	0							 Service registered successfully.
*	-1							 Failed to register service.
*
*****************************************************************/
int ql_bt_le_register_att_service(unsigned char *profile_data, unsigned short profile_data_len, att_read_callback_t read_callback, att_write_callback_t write_callback);


/*****************************************************************
* Function: ql_le_send_service_change
*
* Description:
*	Sends a service change notification to the connected device.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*
* Return:
*	None
*
*****************************************************************/
void ql_le_send_service_change(unsigned short conn_handle);

/*****************************************************************
* Function: ql_appbt_le_notify
*
* Description:
*	Sends a notification to the client for a given LE (Low Energy) characteristic.
*
* Parameters:
*	conn_handle 	[in]		Connection handle of the LE device.
*	att_handle		[in]		Attribute handle for the characteristic.
*	data			[in]		Pointer to the data to be notified.
*	size			[in]		Size of the data to be notified.
*
* Return:
*	0							 Notification sent successfully.
*	-1							 Failed to send notification.
*
*****************************************************************/
int ql_appbt_le_notify(unsigned short conn_handle, unsigned short att_handle, unsigned char *data, int size);


/*****************************************************************
* Function: ql_appbt_le_indicate
*
* Description:
*	Sends an indication to the client for a given LE (Low Energy) characteristic.
*
* Parameters:
*	conn_handle 	[in]		Connection handle of the LE device.
*	att_handle		[in]		Attribute handle for the characteristic.
*	data			[in]		Pointer to the data to be indicated.
*	size			[in]		Size of the data to be indicated.
*
* Return:
*	0							 Indication sent successfully.
*	-1							 Failed to send indication.
*
*****************************************************************/
int ql_appbt_le_indicate(unsigned short conn_handle, unsigned short att_handle, unsigned char *data, int size);


/*****************************************************************
* Function: ql_le_custom_mtu_request
*
* Description:
*	Sends a custom MTU (Maximum Transmission Unit) request to the peer device.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*	mtu 			[in]		The requested MTU size.
*
* Return:
*	0							 MTU request sent successfully.
*	-1							 Failed to send MTU request.
*
*****************************************************************/
int ql_le_custom_mtu_request(unsigned short conn_handle, unsigned short mtu);


/*****************************************************************
* Function: ql_bt_le_scan
*
* Description:
*	Starts LE (Low Energy) scanning for nearby devices with the specified parameters.
*
* Parameters:
*	type			[in]		Scan type (e.g., passive or active).
*	interval		[in]		Scan interval (in units of 0.625 ms).
*	window			[in]		Scan window (in units of 0.625 ms).
*	own_address_type [in]		Address type for the local device.
*	handle			[in]		Callback function to handle scan events.
*
* Return:
*	0							 Scanning started successfully.
*	-1							 Failed to start scanning.
*
*****************************************************************/
int ql_bt_le_scan(unsigned char type, unsigned short int interval, unsigned short int window, unsigned char own_address_type, le_scan_event_handle_t handle);


/*****************************************************************
* Function: ql_bt_le_scan_with_filter
*
* Description:
*	Starts LE scanning for nearby devices with the specified parameters and a filter.
*
* Parameters:
*	type			[in]		Scan type (e.g., passive or active).
*	interval		[in]		Scan interval (in units of 0.625 ms).
*	window			[in]		Scan window (in units of 0.625 ms).
*	own_address_type [in]		Address type for the local device.
*	filter			[in]		Scan filter for device discovery (e.g., specific devices).
*	handle			[in]		Callback function to handle scan events.
*
* Return:
*	0							 Scanning with filter started successfully.
*	-1							 Failed to start scanning with filter.
*
*****************************************************************/
int ql_bt_le_scan_with_filter(unsigned char type, unsigned short int interval, unsigned short int window, unsigned char own_address_type, unsigned char filter, le_scan_event_handle_t handle);


/*****************************************************************
* Function: ql_bt_le_scan_stop
*
* Description:
*	Stops the ongoing LE (Low Energy) scanning process.
*
* Parameters:
*	None
*
* Return:
*	0							 Scanning stopped successfully.
*	-1							 Failed to stop scanning.
*
*****************************************************************/
int ql_bt_le_scan_stop(void);

/*****************************************************************
* Function: ql_le_custom_gatt_scan_start
*
* Description:
*	Starts a custom GATT (Generic Attribute Profile) scan for a specific LE (Low Energy) connection.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*
* Return:
*	0							 Scan started successfully.
*	-1							 Failed to start scan.
*
*****************************************************************/
int ql_le_custom_gatt_scan_start(unsigned short conn_handle);


/*****************************************************************
* Function: ql_le_custom_write_command
*
* Description:
*	Sends a write command to a specific attribute handle on the LE connection.
*	The write command does not require an acknowledgment.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*	att_handle		[in]		Attribute handle to write to.
*	data			[in]		Pointer to the data to be written.
*	size			[in]		Size of the data.
*
* Return:
*	0							 Write command sent successfully.
*	-1							 Failed to send write command.
*
*****************************************************************/
int ql_le_custom_write_command(unsigned short conn_handle, unsigned short att_handle, unsigned char *data, unsigned short size);


/*****************************************************************
* Function: ql_le_custom_write_request
*
* Description:
*	Sends a write request to a specific attribute handle on the LE connection.
*	This request requires an acknowledgment from the peer device.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*	att_handle		[in]		Attribute handle to write to.
*	data			[in]		Pointer to the data to be written.
*	size			[in]		Size of the data.
*
* Return:
*	0							 Write request sent successfully.
*	-1							 Failed to send write request.
*
*****************************************************************/
int ql_le_custom_write_request(unsigned short conn_handle, unsigned short att_handle, unsigned char *data, unsigned short size);


/*****************************************************************
* Function: ql_le_custom_read_request
*
* Description:
*	Sends a read request for a specific attribute handle on the LE connection.
*	The request will retrieve the attribute value from the peer device.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*	att_handle		[in]		Attribute handle to read from.
*
* Return:
*	0							 Read request sent successfully.
*	-1							 Failed to send read request.
*
*****************************************************************/
int ql_le_custom_read_request(unsigned short conn_handle, unsigned short att_handle);


/*****************************************************************
* Function: ql_le_custom_connection_parameter_update_request
*
* Description:
*	Sends a connection parameter update request for an LE connection.
*	This request allows updating parameters like connection interval, slave latency, and supervision timeout.
*
* Parameters:
*	conn_handle 	[in]		Connection handle for the LE connection.
*	conn_interval	[in]		New connection interval to request.
*	slave_latency	[in]		New slave latency to request.
*	supervision_timeout [in]	New supervision timeout to request.
*
* Return:
*	0							 Connection parameter update request sent successfully.
*	-1							 Failed to send connection parameter update request.
*
*****************************************************************/
int ql_le_custom_connection_parameter_update_request(unsigned short conn_handle, unsigned short conn_interval, unsigned short slave_latency, unsigned short supervision_timeout);



#ifdef __cplusplus
} /*"C" */
#endif

#endif


