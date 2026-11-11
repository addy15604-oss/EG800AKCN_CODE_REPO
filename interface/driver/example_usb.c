/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include "ql_application.h"
#include "ql_usb_descriptor.h"
#include "ql_boot.h"
#include "ql_uart.h"
#include "ql_rtos.h"
//#define CCID_SOFT_WARE
#define APP_DEBUG(fmt, args...)	printf(fmt, ##args)
#define USB_CDC_PID_TEST           1
#define USB_CDC_PID_VALUE          0x6001
#define USB_DESCRIPTOR_TYPE_STRING 3

UINT8   usb_connect = 0;
UINT8   quec_product[] = {0x10, 0x03, 'A', 0x00, 'n', 0x00, 'd', 0x00, 'r', 0x00, 'o', 0x00, 'i', 0x00, 'd', 0x00};
UINT16  len_product = sizeof(quec_product);
UINT8   protuctid = 2;
UINT8   *prost=quec_product;

//NOTE:The callback function must not invoke any “blocking” operating system calls.
void Ql_Usb_Detect_Handler(unsigned int states)
{   

	usb_connect = states; 

	if(states)
	{	
        printf("[OPEN]USB Connect\r\n");
	} 
	else
		printf("[OPEN]USB Disconnect\r\n");
}


static void quec_usb_test(void * argv)
{   
    INT     reg=0;
	INT		usb_connect_state = -1;	
	APP_DEBUG("<------example_usb.c------>\r\n");

	//注册USB连接与断开的回调函数
	ql_usbdect_register_cb(Ql_Usb_Detect_Handler);
	   
    while(1)
    {	
        if(usb_connect)
        { 
			usb_connect = NULL;
			if(ql_usb_get_type())
			{
				APP_DEBUG("PC usb!\r\n");
			}
			else
			{
				APP_DEBUG("Adaptor usb!\r\n");
			}
        }
		usb_connect_state = ql_usb_connect_state();
		APP_DEBUG("usb_connect_state = %d\r\n",usb_connect_state);
		ql_rtos_task_sleep_s(1); 
	}
}
//application_init(quec_usb_test, "quec_usb_test", 2, 0);
#ifdef CCID_SOFT_WARE
ql_queue_t ccid_queue ;
#define ABDATA_SIZE 64
#define PC_TO_RDR_ICCPOWERON                        0x62
#define PC_TO_RDR_ICCPOWEROFF                       0x63
#define PC_TO_RDR_GETPARAMETERS                     0x6C

#define BPROTOCOL_NUM_T0  0
#define BPROTOCOL_NUM_T1  1


//CCID----->PC
#define RDR_TO_PC_DATABLOCK                         0x80
#define RDR_TO_PC_SLOTSTATUS                        0x81
#define RDR_TO_PC_PARAMETERS                        0x82
#define RDR_TO_PC_ESCAPE                            0x83
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY         0x84

#define BM_ICC_PRESENT_ACTIVE        0x00
#define BM_ICC_PRESENT_INACTIVE      0x01
#define BM_ICC_NO_ICC_PRESENT        0x02

#define BM_COMMAND_STATUS_POS        0x06
#define BM_COMMAND_STATUS_NO_ERROR   (0x00 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_FAILED     (0x01 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_TIME_EXTN  (0x02 << BM_COMMAND_STATUS_POS)

#define   SLOTERROR_BAD_LENTGH                    0x01
#define   SLOTERROR_BAD_SLOT                      0x05
#define   SLOTERROR_BAD_POWERSELECT               0x07
#define   SLOTERROR_BAD_PROTOCOLNUM               0x07
#define   SLOTERROR_BAD_CLOCKCOMMAND              0x07
#define   SLOTERROR_BAD_ABRFU_3B                  0x07
#define   SLOTERROR_BAD_BMCHANGES                 0x07
#define   SLOTERROR_BAD_BFUNCTION_MECHANICAL      0x07
#define   SLOTERROR_BAD_ABRFU_2B                  0x08
#define   SLOTERROR_BAD_LEVELPARAMETER            0x08
#define   SLOTERROR_BAD_FIDI                      0x0A
#define   SLOTERROR_BAD_T01CONVCHECKSUM           0x0B
#define   SLOTERROR_BAD_GUARDTIME                 0x0C
#define   SLOTERROR_BAD_WAITINGINTEGER            0x0D
#define   SLOTERROR_BAD_CLOCKSTOP                 0x0E
#define   SLOTERROR_BAD_IFSC                      0x0F
#define   SLOTERROR_BAD_NAD                       0x10
#define   SLOTERROR_BAD_DWLENGTH                  0x08

#define CCID_NUM_OF_SLOTS       1
#define   SLOT_NO_ERROR         0x81
#define   SLOT_NO_ERROR2        0x00

#define CCID_RESP_HDR_SIZE                10
#define RDR_TO_PC_DATABLOCK                         0x80

/* defines for the CCID_CMD Layers */

#define LEN_RDR_TO_PC_SLOTSTATUS 10
#define CCID_MSG_HDR_SIZE                 10
#define CCID_RESP_HDR_SIZE                10
//default para value for T0/1
#define   DEFAULT_FIDI              0x11
#define   DEFAULT_T01CONVCHECKSUM   0x10
#define   DEFAULT_EXTRA_GUARDTIME   0x00
#define   DEFAULT_WAITINGINTEGER    0x41
#define   DEFAULT_CLOCKSTOP         0x00
#define   DEFAULT_IFSC              0x20
#define   DEFAULT_NAD               0x00

struct ccid_bulk_out_header
{
    unsigned char bMessageType;
    unsigned int dwLength;
    unsigned char bSlot;
    unsigned char bSeq;
    unsigned char bSpecific_0;
    unsigned char bSpecific_1;
    unsigned char bSpecific_2;
    unsigned char APDU[ABDATA_SIZE];
} __attribute__((packed));
struct ccid_bulk_in_header
{
    unsigned char bMessageType;
    unsigned int dwLength;
    unsigned char bSlot;
    unsigned char bSeq;
    unsigned char bStatus;
    unsigned char bError;
    unsigned char bSpecific;
    unsigned char abData[ABDATA_SIZE];
    unsigned char bSizeToSend;
} __attribute__((packed));
typedef struct
{
    uint8_t bmFindexDindex;
    uint8_t bmTCCKST0;
    uint8_t bGuardTimeT0;
    uint8_t bWaitingIntegerT0;
    uint8_t bClockStop;
} ProtocolT0_t;


typedef struct
{
    uint8_t bmFindexDindex;
    uint8_t bmTCCKST1;
    uint8_t bGuardTimeT1;
    uint8_t bWaitingIntegerT1;
    uint8_t bClockStop;
    uint8_t bifsc;
    uint8_t bNadVal;
} ProtocolT1_t;

typedef enum
{
    CHK_PARAM_SLOT = 1,
    CHK_PARAM_DWLENGTH = (1<<1),
    CHK_PARAM_abRFU2 = (1<<2),
    CHK_PARAM_abRFU3 = (1<<3),
} QL_CMD_PARAM_CHECK_E;
	
static unsigned int ql_ccid_check_cmd_params(unsigned int check_type,struct ccid_bulk_out_header DATA,unsigned char* bStatus)
{
	*bStatus = BM_ICC_PRESENT_ACTIVE | BM_COMMAND_STATUS_NO_ERROR ;
    if (check_type & CHK_PARAM_SLOT)
    {
         /* slot num should < CCID_NUMBER_OF_SLOTs */
        if(DATA.bSlot >= CCID_NUM_OF_SLOTS)
        {
            *bStatus  = BM_COMMAND_STATUS_FAILED|BM_ICC_NO_ICC_PRESENT;
            printf("slot error");
            return SLOTERROR_BAD_SLOT;
        }
    }

    /* some cmd has no abData field DwLength should be 0 */
    if (check_type & CHK_PARAM_DWLENGTH)
    {
        if (DATA.dwLength != 0)
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("len error");
            return SLOTERROR_BAD_LENTGH;
        }
    }

        /* some cmd's abRFU field should be 0 */
    if (check_type & CHK_PARAM_abRFU2)
    {
        /* 2B abRFU Reserved for Future Use*/
        if ((DATA.bSpecific_1 != 0) ||
            (DATA.bSpecific_2 != 0))
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("abrfu2b error");
            return SLOTERROR_BAD_ABRFU_2B;
        }
    }

    /* some cmd's abRFU field should be 0 */
    if (check_type & CHK_PARAM_abRFU3)
    {
        /* 3B abRFU Reserved for Future Use*/
        if ((DATA.bSpecific_0 != 0) ||
            (DATA.bSpecific_1 != 0) ||
            (DATA.bSpecific_2 != 0))
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("abrfu3b error");
            return SLOTERROR_BAD_ABRFU_3B;
        }
    }

    return SLOT_NO_ERROR;
}
static int ql_reader2pc_data_block(unsigned char error_code,unsigned char bStatus,struct ccid_bulk_out_header data ,void *buff,unsigned short len)
{
    unsigned short length = CCID_RESP_HDR_SIZE;
	struct ccid_bulk_in_header  bulk_in_data; 
    bulk_in_data.bMessageType = RDR_TO_PC_DATABLOCK;
    bulk_in_data.bError = error_code;
    bulk_in_data.bSpecific = 0;
    bulk_in_data.dwLength = len;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    memcpy(&bulk_in_data.abData,buff,len);
    length +=bulk_in_data.dwLength;
    return ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,length);
}
static int ql_reader2pc_slot_status(uint8_t error_code,unsigned char bStatus,struct ccid_bulk_out_header data)
{
	struct ccid_bulk_in_header  bulk_in_data; 
    bulk_in_data.bMessageType = RDR_TO_PC_SLOTSTATUS;
    bulk_in_data.dwLength = 0;
    bulk_in_data.bError = error_code;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    bulk_in_data.bSpecific = 0;/* bClockStatus = 00h Clock running
                                    01h Clock stopped in state L
                                    02h Clock stopped in state H
                                    03h Clock stopped in an unknown state
                                    All other values are RFU. */
    return ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,LEN_RDR_TO_PC_SLOTSTATUS);
}
static void reader2pc_params(uint8 errorCode,unsigned char bStatus,struct ccid_bulk_out_header data,ProtocolT1_t t1_param)
{
	struct ccid_bulk_in_header	bulk_in_data; 
    uint16_t length = CCID_RESP_HDR_SIZE;
    bulk_in_data.bMessageType = RDR_TO_PC_PARAMETERS;
    bulk_in_data.bError = errorCode;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    if(errorCode == SLOT_NO_ERROR)
    {
        bulk_in_data.dwLength = sizeof(ProtocolT1_t);
        length += sizeof(ProtocolT1_t);
    }
    else
    {
        bulk_in_data.dwLength = 0;
    }
    bulk_in_data.bSpecific = BPROTOCOL_NUM_T1;
    memcpy(bulk_in_data.abData,&t1_param,sizeof(ProtocolT1_t));
    ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,length);
}


static void quec_ccid_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	int read_len = 0;
    char a[16]={0};
	struct ccid_bulk_out_header r_data = {0};
    read_len = ql_uart_read(port, &r_data, sizeof(r_data));
    ql_rtos_queue_release(ccid_queue, sizeof(r_data),&r_data, QL_NO_WAIT);
}
static void quec_ccid_startup(void * argv)
{
	/*此函数启动阶段早，禁止加打印和复杂任务*/
	ql_cdc_switch_ccid_desc(1);
    ql_rtos_queue_create(&ccid_queue, sizeof(struct ccid_bulk_out_header), 64);
	ql_uart_open(QL_USB_CDC_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
    ql_uart_register_cb(QL_USB_CDC_PORT, quec_ccid_callback);	//use callback to read uart data
}

static void quec_ccid_test(void * argv)
{
  	int ret = -1;
	int write_bytes = 0;
	int read_len = 0;
	struct ccid_bulk_out_header r_data = {0};
	unsigned char bStatus = 0 ,errorCode=0 ;
	unsigned char resp_buff[13] = {0x3b,0x88,0x80,0x01,0x86,0x88,0x54,0x69,0x61,0x6e,0x59,0x75,0x19};
	ProtocolT1_t t1_param={.bmFindexDindex = DEFAULT_FIDI,
							.bmTCCKST1 = DEFAULT_T01CONVCHECKSUM,
							.bGuardTimeT1 =  DEFAULT_EXTRA_GUARDTIME,
						    .bWaitingIntegerT1 =  DEFAULT_WAITINGINTEGER,
						    .bClockStop =  DEFAULT_CLOCKSTOP,
						    .bifsc = DEFAULT_IFSC,
						    .bNadVal = DEFAULT_NAD
							};

    
	//ql_rtos_task_sleep_ms(100);
    
	while (1)
	{  
		ql_rtos_queue_wait(ccid_queue, &r_data, sizeof(r_data), QL_WAIT_FOREVER);
		switch(r_data.bMessageType)
		{
			case PC_TO_RDR_ICCPOWERON:
			    errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				printf("power on");
			    ql_reader2pc_data_block(errorCode,bStatus,r_data,(unsigned char *)resp_buff,13);
			    break;

			case PC_TO_RDR_ICCPOWEROFF:
			    errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				bStatus = BM_COMMAND_STATUS_NO_ERROR|BM_ICC_PRESENT_ACTIVE;
				printf("power off");
			    ql_reader2pc_slot_status(errorCode,bStatus,r_data);
			    break;
			case PC_TO_RDR_GETPARAMETERS:
				errorCode = errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				bStatus = BM_COMMAND_STATUS_NO_ERROR|BM_ICC_PRESENT_ACTIVE;
				reader2pc_params(errorCode,bStatus,r_data,t1_param);
				break;
		}
		
		
	}    
}
//user_boot_init(quec_ccid_startup, 0);
//application_init(quec_ccid_test, "quec_ccid_test", 4, 0);
#endif
//#define USBHID_SUPPORT
#ifdef USBHID_SUPPORT
#define QL_KEYBOARD_REPORT_SIZE  8    //don't change it, one time report size must max is 8 byte, because it's hid keyboard report descript;
#define QL_CUST_HID_REPORT_SIZE  64   //don't change it, one time report size must max is 64 byte because usb interruct ep max len;
#define hid_max(a, b) (((a) > (b)) ? (a) : (b))
#define hid_min(a, b) (((a) < (b)) ? (a) : (b))
// keyboard test value array
UINT8 key_value[8][QL_KEYBOARD_REPORT_SIZE] = 
{
	{0x00, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},// a b c ...
	{0x02, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},// shitf + a b c  ... = ABC...
	{0x02, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},// A
	{0x00, 0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x00},
	{0x01, 0x00, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00},//ctrl + z


	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//end 
};
// key value send need send emty data sync state.
UINT8 key_empty_data[QL_KEYBOARD_REPORT_SIZE] = 
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct
{
	unsigned char ascii;
	unsigned char key;
	unsigned char shift;
} ql_asc_trans_key;

//more infomation see HID Usage Tables FOR Universal Serial Bus (USB)-10 Keyboard/Keypad Page (0x07) and ascii table.
ql_asc_trans_key ql_key_map[]=
{
	//a - z 
	{0x61,0x04,0x00},{0x62,0x05,0x00},{0x63,0x06,0x00},{0x64,0x07,0x00},
	{0x65,0x08,0x00},{0x66,0x09,0x00},{0x0A,0x67,0x00},{0x68,0x0B,0x00},
	{0x69,0x0C,0x00},{0x6A,0x0D,0x00},{0x6B,0x0E,0x00},{0x6C,0x0F,0x00},
	{0x6D,0x10,0x00},{0x6E,0x11,0x00},{0x6F,0x12,0x00},{0x70,0x13,0x00},
	{0x71,0x14,0x00},{0x72,0x15,0x00},{0x73,0x16,0x00},{0x74,0x17,0x00},
	{0x75,0x18,0x00},{0x76,0x19,0x00},{0x77,0x1A,0x00},{0x78,0x1B,0x00},
	{0x79,0x1C,0x00},{0x7A,0x1D,0x00},
	//A -- Z shift+
	{0x41,0x04,0x01},{0x42,0x05,0x01},{0x43,0x06,0x01},{0x44,0x07,0x01},
	{0x45,0x08,0x01},{0x46,0x09,0x01},{0x4A,0x67,0x01},{0x48,0x0B,0x01},
	{0x49,0x0C,0x01},{0x4A,0x0D,0x01},{0x4B,0x0E,0x01},{0x4C,0x0F,0x01},
	{0x4D,0x10,0x01},{0x4E,0x11,0x01},{0x4F,0x12,0x01},{0x50,0x13,0x01},
	{0x51,0x14,0x01},{0x52,0x15,0x01},{0x53,0x16,0x01},{0x54,0x17,0x01},
	{0x55,0x18,0x01},{0x56,0x19,0x01},{0x57,0x1A,0x01},{0x58,0x1B,0x01},
	{0x59,0x1C,0x01},{0x5A,0x1D,0x01},
	//0 1 2 3 - 9
	{0x30,0x27,0x00},{0x31,0x1E,0x00},{0x32,0x1F,0x00},{0x33,0x20,0x00}, 
	{0x34,0x21,0x00},{0x35,0x22,0x00},{0x36,0x23,0x00},{0x37,0x24,0x00}, 
	{0x38,0x25,0x00}, {0x39,0x26,0x00}, 
	//shift+
	{0x29,0x27,0x01}, // )
	{0x21,0x1E,0x01}, // !
	{0x40,0x1F,0x01}, // @
	{0x23,0x20,0x01}, // #
	{0x24,0x21,0x01}, // $
	{0x25,0x22,0x01}, // %
	{0x5E,0x23,0x01}, // ^
	{0x26,0x24,0x01}, // &
	{0x2A,0x25,0x01}, // *
	{0x28,0x26,0x01}, // (
	{0x20,0x2C,0x00}, // space 
	{0x0D,0x28,0x00}, // enter

	// Add more if you need

	{0x00,0x00,0x00}// map end
};

ql_asc_trans_key* quec_key_find_by_assii(ql_asc_trans_key* map, unsigned char ascii)
{
	ql_asc_trans_key* map_ptr = NULL;
	map_ptr = map;
	do
	{
		if(map_ptr->ascii == ascii){
			return map_ptr;
		}
		map_ptr++;
	} while(map_ptr->ascii != 0x00 && map_ptr->key != 0x00);
	
	return NULL;
}
void quec_usb_hid_sta_keyboard_test()
{
	int i = 0;
	ql_asc_trans_key *trans_key = NULL;
	unsigned char key_array[QL_KEYBOARD_REPORT_SIZE] = {0};
	char test_str[]="abcdefghijklmnopqrstuvwxyz 1234567890 !@#$%^&*() ABCDEFGHIJKLMNOPQRSTUVWXYZ 1!@2#3DeStIn\r\n";
	int ret = 0;
	printf("usb_hid_sta_keyboard_test in");
	// way 1:ascii report test
	for(i = 0;i < strlen(test_str);i++ ){
		trans_key = quec_key_find_by_assii(&ql_key_map, test_str[i]);
		if(trans_key != NULL){
			//printf("trans_key:0x%x shift:0x%x\r\n", trans_key->key, trans_key->shift);
			if(trans_key->shift){
				key_array[0] = 0x02;// shift down
				//NOTE most PC keyboard language input mode change is with Shift, please keep it colse.
			}

			// only report one byte once time.
			key_array[2] = trans_key->key;
			ret = ql_usb_hid_send_data(key_array, QL_KEYBOARD_REPORT_SIZE);
			if(ret != USB_DEVICE_RC_OK) {
				printf("key_array send error %d",ret);
			}
			ret = ql_usb_hid_send_data(key_empty_data, QL_KEYBOARD_REPORT_SIZE);
			if(ret != USB_DEVICE_RC_OK) {
				printf("key_empty_data send error %d",ret);
			}
			memset(key_array, 0, sizeof(key_array));
			ql_rtos_task_sleep_ms(20);//host interval
		}
	}

	// way 2:raw key report test
    ret = ql_usb_hid_send_data(key_value[0], QL_KEYBOARD_REPORT_SIZE);
    if(ret != USB_DEVICE_RC_OK) {
        printf("key_value send error %d",ret);
    }
    ret = ql_usb_hid_send_data(key_empty_data, QL_KEYBOARD_REPORT_SIZE);
    if(ret != USB_DEVICE_RC_OK) {
        printf("key_empty_data send error %d",ret);
    }
	
	printf("usb_hid_sta_keyboard_test leave.");
	
}
static void quec_usb_hid_test(void * argv)
{
    ql_rtos_task_sleep_s(10);
    printf("<------example_usb_hid.c------>\r\n");
    while(1)
    {
		quec_usb_hid_sta_keyboard_test();
		ql_rtos_task_sleep_s(5);
    }
}
static void quec_hid_startup(void * argv)
{
	/*此函数启动阶段早，禁止加打印和复杂任务*/
	ql_usb_enable_hid(1);
}
//application_init(quec_usb_hid_test, "quec_usb_hid_test", 4, 0);
//user_boot_init(quec_hid_startup, 0);
#endif

static void usb_vbus_wakeup_test(void)
{
	printf("usb_vbus_wakeup_test\r\n");
	ql_usb_vbus_wakeup_set_enable(0);	// disable vbus wakeup
    ql_autosleep_enable(1);
}

// application_init(usb_vbus_wakeup_test, "usb_vbus_wakeup_test", 4, 0);