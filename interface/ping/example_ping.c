/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_type.h"
#include "ql_application.h"
#include "ql_data_call.h"
#include "ql_ping.h"
#include "ql_uart.h"
#include <stdarg.h>
#include "sockets.h"

#define PING_CID 1
#define PING_HOST "www.baidu.com"
#define PING_HOSTV6          "www.taobao.com"
#define PING_TO_MS 4000
#define PING_DATA_SIZE 32
#define PING_NUM 4
typedef enum 
{
	QUEC_START_DATACALL,
	QUEC_START_QPING_V4,
	QUEC_STOP_DATACALL,
	QUEC_START_QPING_V6,//鍙戣捣ipV6鏁版嵁鎷ㄥ彿
}QPING_ACTION_E;

typedef struct
{
	int flag;
	QPING_ACTION_E msg;
}QPING_ACTION_T;

ql_sem_t g_ql_ping_sem = NULL;
static ql_queue_t rtos_test_queue = NULL;
#define MSG_MAX_NUM 10
#define APP_DEBUG printf	

#if 1
#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart1[SERIAL_RX_BUFFER_LEN];

static void quec_control_uart_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	int ret1 = 0,ret2=0;
	char buf[50] = {0};
	
	unsigned int  totalBytes = ql_uart_read(port, m_RxBuf_Uart1, sizeof(m_RxBuf_Uart1));
	if (totalBytes <= 0)
	{
		printf("<-- No data in UART buffer! -->\r\n");
		return;
	}
	
	if(0 == memcmp(m_RxBuf_Uart1,"QUEC_START_QPING_V6",19)) //ipv6 ping
	{
		APP_DEBUG("<-%s -->\r\n",m_RxBuf_Uart1);
		QPING_ACTION_T quecdatacall_msg = {0};
		quecdatacall_msg.msg = QUEC_START_QPING_V6;

		ql_rtos_queue_release(rtos_test_queue, sizeof(QPING_ACTION_T), (u8*)(&quecdatacall_msg), 0);
		
		goto end;
	}
	
	if(0 == memcmp(m_RxBuf_Uart1,"QUEC_START_QPING_V4",19)) //ipv4 ping
	{
		APP_DEBUG("<-%s -->\r\n",m_RxBuf_Uart1);
		QPING_ACTION_T quecdatacall_msg = {0};
		quecdatacall_msg.msg = QUEC_START_QPING_V4;

		ql_rtos_queue_release(rtos_test_queue, sizeof(QPING_ACTION_T), (u8*)(&quecdatacall_msg), 0);
		
		goto end;
	}
	
end:
	ql_uart_write(port,m_RxBuf_Uart1,totalBytes);
	memset(m_RxBuf_Uart1,0,SERIAL_RX_BUFFER_LEN);
	return;
}

static unsigned char ControlPortInitFlag = 0;
static QL_UART_PORT_NUMBER_E ControlPort = QL_MAIN_UART_PORT;
static unsigned char Ql_Control_Port_Open(QL_UART_PORT_NUMBER_E port)
{
	int ires;
	
	ires = ql_uart_open(port, QL_UART_BAUD_115200, QL_FC_NONE);
	if (ires) 
	{
		printf("ql_open cdc fail\r\n");
		return 0xff;
	}
	
    ql_uart_register_cb(port, quec_control_uart_callback);	//use callback to read uart data
	ControlPortInitFlag = 1;
	ControlPort = port;
	
	return 0;
}           

static void myprintk(const char *str, ...)
{
	va_list varg;
	int retv;
	static char sbuffer[2200];  
	int i,j,len;
    int port_no = ControlPort;
	unsigned char ret;
	if(!ControlPortInitFlag)
		Ql_Control_Port_Open(ControlPort);
    memset(sbuffer, 0, sizeof(sbuffer));
    va_start( varg, str );                  
    retv=vsprintf(sbuffer, str,  varg); 
    va_end( varg );
    len = strlen(sbuffer);

	ql_uart_write(port_no,sbuffer, len);
}
#endif
static int data_call_state=-1;

static void ql_nw_status_callback(int profile_idx, int nw_status)
{
	printf("profile(%d) status: %d\r\n", profile_idx, nw_status);
}
static void ql_data_call_cb(int profile_idx, int nw_status)
{
	data_call_state=nw_status;
	
	printf("[APP]ql_data_call_cb: profile(%d) status(%d) data_call_state(%d)\r\n", profile_idx, nw_status,data_call_state);
}

static int datacall_satrt(void)
{
	int ret = 0;
	
	printf("wait for network register done\r\n");

	if(ql_network_register_wait(120) != 0)
	{
		printf("*** network register fail ***\r\n");
		ret = -1;
	}
	else
	{
		printf("network register ok ...\r\n");
	}

	return ret;
}

static void ql_ping_cb(ql_ping_state_e state, int err, ql_ping_stat_t * statistic)
{
	printf("ping cb stat %d  \r\n",state);
	if(state == QL_PING_INPROGRESS)
	{
		if(err == 0)
			printf("ping:\tip: %s\tdatasize: %3d\trtt: %4d\tttl: %4d\r\n",statistic->ipSrc, statistic->dataSize, statistic->rtt, statistic->ttl);
		else
			printf("ping:\terror: %d\r\n", err);
	}
	else
	{
		printf("ping:\tsend: %d\trecv: %d\tlost: %d\tminRtt: %d\tmaxRtt: %d\r\n", statistic->sentNum, statistic->rcvdNum, statistic->lostNum, statistic->minRtt, statistic->maxRtt);
	}
	if(statistic->sentNum >= PING_NUM|| statistic->rcvdNum >= PING_NUM)
	{
		
		printf("ping cb send count %d recv count %d  \r\n",statistic->sentNum,statistic->rcvdNum);
		ql_rtos_semaphore_release(g_ql_ping_sem);
	
}
}

static void ping_test(void * argv)
{
	struct ql_data_call_info info = {0};
	char ip4_addr_str[16] = {0};
	char ip6_addr_str[64] = {0};
	int ret = 0;
	ql_ping_ctx_t *ctx = NULL;
	ql_ping_cfg_t cfg = {
		.pdpCid  = PING_CID,
		.host    = PING_HOST,
		.timeout = PING_TO_MS,
		.dataSize= PING_DATA_SIZE,
		.pingNum = PING_NUM
	};
		
	ql_rtos_queue_create(&rtos_test_queue, sizeof(QPING_ACTION_T), MSG_MAX_NUM);
	printf("<-----------------example_ping.c---------------------->\r\n");
	Ql_Control_Port_Open(QL_MAIN_UART_PORT);
	myprintk("HELLO QUECTEL\r\n");

	QPING_ACTION_T msg;
	
	if(datacall_satrt())
	{
		printf("*** data call fail ***\r\n");
		return;
	}
	while(1)
	{
		ql_rtos_queue_wait(rtos_test_queue, (u8*)(&msg), sizeof(msg), 0xFFFFFFFF);  // wait for the message from main task
		switch(msg.msg)
		{
			case QUEC_START_QPING_V6:
				//同步模式的IPV6数据拨号
				cfg.host =  PING_HOSTV6;
				ql_set_data_call_asyn_mode(0, ql_data_call_cb);
				ret = ql_start_data_call(1, 1, NULL, NULL, NULL, 0);
				if(ret)
				{
					printf("*** ql_start_data_call fail ***\r\n");
				}
				
				while(data_call_state != 1)
				{
					ql_rtos_task_sleep_ms(2000);
					printf("loop data_call_state=%d\n", data_call_state);
				}			
				
				ql_get_data_call_info(1, 1, &info);
				
				printf("[ST-INFO] profile_idx: %d\r\n", info.profile_idx);
				printf("[ST-INFO] ip_version: %d\r\n", info.ip_version);
				printf("[ST-INFO] v6.state: %d\r\n", info.v6.state);
				printf("[ST-INFO] v6.reconnect: %d\r\n", info.v6.reconnect);
				inet_ntop(AF_INET6, &info.v6.addr.ip, ip6_addr_str, sizeof(ip6_addr_str));
				printf("[ST-INFO] v6.addr.ip: %s\r\n", ip6_addr_str);
				inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip6_addr_str, sizeof(ip6_addr_str));
				printf("[ST-INFO] v6.addr.pri_dns: %s\r\n", ip6_addr_str);
				inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip6_addr_str, sizeof(ip6_addr_str));
				printf("[ST-INFO] v6.addr.sec_dns: %s\r\n", ip6_addr_str);
				break;
				
			case QUEC_START_QPING_V4:
				cfg.host =  PING_HOST;
								//杩涜IPV4 鏁版嵁鎷ㄥ彿
				ret = ql_start_data_call(1, 0, NULL, NULL, NULL, 0);
				if(ret)
				{
					printf("*** ql_start_data_call fail ***\r\n");
				}
				
				while(data_call_state != 1)
				{
					ql_rtos_task_sleep_ms(2000);
					printf("loop data_call_state=%d\n", data_call_state);
				}			
				
				printf("data_call_state Success=%d\n\n", data_call_state);

				ql_get_data_call_info(1, 0, &info);
				printf("info.profile_idx: %d\r\n", info.profile_idx);
				printf("info.ip_version: %d\r\n", info.ip_version);
				printf("info.v4.state: %d\r\n", info.v4.state);
				printf("info.v4.reconnect: %d\r\n", info.v4.reconnect);

				inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
				printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);

				inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
				printf("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

				inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
				printf("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
				break;
		}

		//寮�濮媝ing
		ctx = ql_ping_init(cfg, ql_ping_cb);
		printf("*** ql_ping_cb %p ***\r\n",ql_ping_cb);
		
		if(ctx == NULL)
		{
			printf("*** ping init fail ***\r\n");
			return;
		}

		ql_rtos_semaphore_create(&g_ql_ping_sem,0);
		printf("*** begin ping ***\r\n");
		ql_ping(ctx);
		ql_rtos_semaphore_wait(g_ql_ping_sem,QL_WAIT_FOREVER);
		printf("*** end ping ***\r\n");
		ql_ping_deinit(ctx);
		ql_rtos_semaphore_delete(g_ql_ping_sem);
	}
}

//application_init(ping_test, "ping_test", 4, 0);
