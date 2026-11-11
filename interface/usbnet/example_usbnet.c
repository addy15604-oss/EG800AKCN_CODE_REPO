/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdarg.h>
#include "ql_type.h"
#include "ql_data_call.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include <stdio.h>
#include "sockets.h"
#include "ql_uart.h"
#include "ql_func.h"
#include "ql_usb_descriptor.h"


  
static int data_call_state=-1;

static void ql_nw_status_callback(int profile_idx, int nw_status)
{
	printf("[APP]ql_nw_status_callback: profile%d status=%d\r\n", profile_idx, nw_status);
	data_call_state=nw_status;
}

static void ql_data_call_cb(int profile_idx, int nw_status)
{
	data_call_state=nw_status;
	printf("[APP]ql_data_call_cb: profile(%d) status(%d) data_call_state(%d)\r\n", profile_idx, nw_status,data_call_state);
}

//网卡使用的通信接口规范是ECM，测试时Windows10/7需要安装对应的驱动
static void usbnet_test(void * argv)
{
	struct ql_data_call_info info = {0};
	char ip4_addr_str[16] = {0};
	int i=1,ret=0;

	printf("<-----------------example_usbnet.c---------------------->\r\n");
	
	for(int j=0;j<5;j++)//拨号5次
	{
			data_call_state=-1;
			//ql_wan_start(ql_nw_status_callback);//此处设置的回调，可用ql_set_data_call_asyn_mode接口的回调取代，但此接口仍保留可用
			ql_set_auto_connect(2, TRUE);
			if(i==1)
			{
				ql_set_data_call_asyn_mode(1, ql_data_call_cb);//异步拨号
				printf("Enable data call Asyn mode\r\n");
			}
			else if(i==2)
			{
				ql_set_data_call_asyn_mode(0, ql_data_call_cb);//同步拨号
				printf("Enable data call Sync mode\r\n");
			}
			ret=ql_start_data_call(2, 0,NULL, NULL, NULL, 0);
			/*
				参数1：拨号通道 1~8
				参数2: 0-ipv4 1-ipv6 2-ipv4v6
				参数3：APN接入网点
				参数4：用户名
				参数5：密码
				参数6：认证类型
			*/
			printf("ql_start_data_call  ret=%d\r\n", ret);
			
			if(data_call_state == -1)
			{
				ql_rtos_task_sleep_ms(5000);
				printf("loop data_call_state=%d\n", data_call_state);
			}
		
		if(data_call_state==1)//拨号拨通打印信息
		{
			printf("data_call success: %d\r\n", data_call_state);
			ql_get_data_call_info(2, 0, &info);
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
		ql_rtos_task_sleep_ms(5000);		
		
	}
	ret=ql_netdevctl_lwip_config(QL_NETIF_CONNECT,info.profile_idx);//绑定网卡 参数1：连接状态 参数2：拨号通道
	if(ret!=0)
	{
		printf("ql_netdevctl_lwip_config error  ret=%d %d\r\n", ret,info.profile_idx);
	}
	printf("ql_netdevctl_lwip_config success  ret=%d %d\r\n", ret,info.profile_idx);
}


//application_init(usbnet_test, "usbnet_test", 2, 0);



