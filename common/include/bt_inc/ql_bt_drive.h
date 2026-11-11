
#ifndef __QL_BT_DRIVE_H__
#define __QL_BT_DRIVE_H__



#ifdef __cplusplus
extern "C" {
#endif


typedef int ql_bt_err_t;
/* Definitions for error constants. */
#define QL_BT_OK          0       /*!< ql_bt_err_t value indicating success (no error) */
#define QL_BT_FAIL        -1      /*!< Generic ql_bt_err_t code indicating failure */

#define QL_BT_ERR_NO_MEM              0x101   /*!< Out of memory */
#define QL_BT_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
#define QL_BT_ERR_INVALID_STATE       0x103   /*!< Invalid state */
#define QL_BT_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
#define QL_BT_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
#define QL_BT_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
#define QL_BT_ERR_TIMEOUT             0x107   /*!< Operation timed out */
#define QL_BT_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
#define QL_BT_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
#define QL_BT_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
#define QL_BT_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
#define QL_BT_ERR_NOT_FINISHED        0x10C   /*!< Operation has not fully completed */
#define QL_BT_ERR_NOT_ALLOWED         0x10D   /*!< Operation is not allowed */



typedef void (*ql_bt_event_callback_t)(int status, void *event_data);




typedef enum ql_bt_event_type_e_
{
    QL_BTTASK_IND_TYPE_COMMON=0,
    QL_BTTASK_IND_TYPE_ACL,
	QL_BTTASK_IND_TYPE_SCO,
	QL_BTTASK_IND_TYPE_A2DP,
	QL_BTTASK_IND_TYPE_AVRCP,
	QL_BTTASK_IND_TYPE_HFP,
	QL_BTTASK_IND_TYPE_OBEX,
	QL_BTTASK_IND_TYPE_SPP,
	QL_BTTASK_IND_TYPE_LE,
	QL_BTTASK_IND_TYPE_A2DP_SINK,
	QL_BTTASK_IND_TYPE_HFP_HF,
	QL_BTTASK_IND_TYPE_HID,
	QL_BTTASK_IND_TYPE_PBAP,
	QL_BTTASK_IND_TYPE_MAP,
	QL_BTTASK_IND_TYPE_FTP,
}ql_bt_event_type_e;


typedef struct ql_bt_event_handlers_t_
{
    ql_bt_event_type_e     ql_bt_event_type;
    ql_bt_event_callback_t ql_bt_event_cb_func;
} ql_bt_event_handlers_t;


/*****************************************************************
* Function: ql_bt_register_event_handler
*
* Description:
*	Registers a callback function to handle specific Bluetooth events.
*
* Parameters:
*	event			[in]		Bluetooth event type to handle.
*	handler_func	[in]		Callback function to handle the specified event.
*
* Return:
*	0							 Event handler registered successfully.
*	-1							 Failed to register the event handler.
*
*****************************************************************/
ql_bt_err_t ql_bt_register_event_handler(ql_bt_event_type_e event, ql_bt_event_callback_t handler_func);

/*****************************************************************
* Function: ql_bt_unregister_event_handler
*
* Description:
*	Unregisters a previously registered event handler.
*
* Parameters:
*	event			[in]		Bluetooth event type to unregister.
*
* Return:
*	0							 Event handler unregistered successfully.
*	-1							 Failed to unregister the event handler.
*
*****************************************************************/
ql_bt_err_t ql_bt_unregister_event_handler(ql_bt_event_type_e event);



/*****************************************************************
* Function: ql_bt_get_event_handler_cb
*
* Description:
*   Retrieves the callback function registered for a specific 
*   Bluetooth event type.
*
* Parameters:
*   event           [in]        Bluetooth event type for which 
*                                the callback is to be retrieved.
*   handler_func    [out]       Pointer to store the retrieved 
*                                callback function.
*
* Return:
*   0                            Callback function retrieved 
*                                successfully.
*   -1                           Failed to retrieve the callback 
*                                function.
*
*****************************************************************/
ql_bt_err_t ql_bt_get_event_handler_cb(ql_bt_event_type_e event, ql_bt_event_callback_t *handler_func);

	

#ifdef __cplusplus
}    /* C */
#endif

#endif
