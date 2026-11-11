/*============================================================================
  Copyright (c) 2022 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/


#ifndef _QL_BT_OLD_API_H
#define _QL_BT_OLD_API_H



#ifdef __cplusplus
extern "C" {
#endif


int ql_bt_open(bt_event_handle_t handler, struct bt_user_init_cfg *init_cfg, struct bt_user_cfg *cfg_confg);

int ql_bt_unbond(struct bt_addr addr);

void ql_bt_update_device_record(void);

int ql_bt_inquiry(unsigned char inquiry_length, unsigned char num_responses);

int ql_read_rssi(unsigned short handle);

void ql_get_bt_connect_status(quec_cb_bt_connect_status cb);

void ql_get_scan_bt_status(quec_cb_bt_scan_status cb);

void ql_get_bt_open_status(quec_cb_bt_open_status cb);

int ql_bt_get_open_status(void);

void ql_bt_sort_device_record(struct bt_addr addr);

void ql_bt_a2dp_connected_handler(unsigned char *addr);

void ql_bt_a2dp_disconnected_handler(void);

void ql_bt_handle_a2dp_rejected(void);

int ql_bt_a2dp_headset_set_volume(unsigned char volume);

void ql_bt_a2dp_start_handler(void);

void ql_bt_a2dp_remote_sbc_capabilities_handler(unsigned int a2dp_capabilites);

int ql_bt_a2dp_send_sbc_data(unsigned int length, unsigned char* data,
									unsigned int timestamp, unsigned short int seq_num,
									 unsigned char payload_type,
                                    unsigned char frames);

int ql_bt_a2dp_SBC_test_send_message(unsigned int msg_id);



int ql_bt_send_bttask_command(struct bt_task_command * command);

void ql_bt_mgr_handler(struct bt_task_event * msg);

void ql_bt_free(void *p);

void ql_bt_close_set(void);


int ql_bt_le_unregister_service(void);


int ql_bt_le_set_gap_name(char *gap_name, int name_len);

int ql_bt_le_set_ext_adv_enable(unsigned char enable, unsigned char number,unsigned char handle[], unsigned short duration[], unsigned char max_event[]);


int ql_bt_le_read_by_group_type_request(unsigned short start, unsigned short end, unsigned short uuid);

int ql_bt_le_get_device_records(ble_device_info_record **bleInfo);

int ql_le_find_information_request	  (unsigned short start, unsigned short end);

int ql_le_disconnect_with_handle(uint16_t con_handle);

void ql_ble_update_device_record(void);

int ql_bt_le_read_by_type_request(unsigned short start, unsigned short end, unsigned short uuid);

int ql_bt_le_mtu_request(unsigned short acl_handle);





#ifdef __cplusplus
} /*"C" */
#endif

#endif


