/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_fota.h"
#include "ql_power.h"
#include "ql_fs.h"
#include "ql_uart.h"
#include "ql_spi_nor.h"
#include "ql_spi.h"

#define fota_exam_log(fmt, ...) printf("[FOTA_EXAM][%s, %d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* 功能�? ********************************************************/
#define QUEC_IN_OR_EXT_FLASH_TEST  //模组内部flash和外挂flash同时支持ota测试

#ifndef QUEC_IN_OR_EXT_FLASH_TEST
// #define QUEC_EXTERNAL_NORFLASH_TEST		// 外挂NOR
// #define QUEC_EXTERNAL_NANDFLASH_TEST		// 外挂NAND
// #define QL_SECFOTA_ENABLE 				// 安全fota使能
#else
#define QUEC_EXTERNAL_NANDFLASH_TEST		// 外挂NAND
#endif
/* **************************************************************/

#ifdef QUEC_EXTERNAL_NANDFLASH_TEST
#define NAND_DEV_NAME "/spinand"
#define NAND_FOTA_PACK_PATH NAND_DEV_NAME "/fotafile.bin"
#endif


#ifdef QL_SECFOTA_ENABLE
void ql_secfota_callback(int err_code)
{
	fota_exam_log("ql_secfota_callback ret = %d", err_code);
}
#endif


#ifdef QUEC_EXTERNAL_NANDFLASH_TEST
static int cust_yaffs_mount(void)
{
	int ret = 0;
	ql_spi_nand_init(QL_SPI_NAND_DEV0, QL_SPI_NAND_PIN_33_36, QL_SPI_NAND_CLOCK_26M);

// 现在nand有两套接口，一个是ql nand，一个是ql yaffs
// ql nand是移远封装的接口，调用任意接口自动挂载，访问时不需要带设备�?+文件名，默认设备名是"spinand"
// ql yaffs是原生yaffs接口封装而来，需要手动挂载，访问必须带设备名，设备名自定�?
// 如果你想混用，那么挂载时无需手动挂载，需要触发一次自动挂载，且使用ql_yaffs访问文件时，必须带默认设备名"spinand"
#define QL_USE_QL_NAND_API 0
#if QL_USE_QL_NAND_API
	ret = ql_nand_fope("mount_temp.txt", "w+");
	if (ret)
	{
		printf("ql_nand_fope error\n");
		return -1;
	}
	ql_nand_fclose(ret);
	return 0;
#else
	ret = ql_yaffs_start_up();
	if (ret)
	{
		printf("ql_yaffs_start_up error, ret = %d\r\n", ret);
		return -1;
	}
	struct ql_yaffs_dev dev = {NAND_DEV_NAME, 0, 2048};
	ql_yaffs_create_dev(&dev);
	ret = ql_yaffs_mount(NAND_DEV_NAME);
	if (ret)
	{
		printf("ql_yaffs_mount error ret = %d, Try to format\r\n", ret);
		ql_yaffs_format(NAND_DEV_NAME, 1, 1, 0);
		ret = ql_yaffs_mount(NAND_DEV_NAME);
		if (ret)
		{
			printf("ql_yaffs_mount error, ret = %d\r\n", ret);
			return -1;
		}
	}
#endif
	return 0;
}
#endif

// minifota 单包升级例程
void ql_mergefile_minifota_test(void)
{
#define MG_MINIFOTA_PROFILE_IDX 1
#define MG_MINIFOTA_URL_TYPE QL_FOTA_DWNLD_MOD_HTTP
#define MG_MINIFOTA_PACK_RUL "http://112.31.84.164:8302/yang/old_new.bin"

	fota_exam_log("========== ql_mergefile_minifota_test ==========");

	int ret = ql_fota_mergefile_mini_system(MG_MINIFOTA_PROFILE_IDX,
											MG_MINIFOTA_URL_TYPE,
											MG_MINIFOTA_PACK_RUL,
											NULL, NULL, NULL);
	if (ret != 0)
	{
		fota_exam_log("ql_fota_mergefile_mini_system error, ret = %d", ret);
	}
}

// minifota 双包升级例程
void ql_double_minifota_test(void)
{
#define DOUBLE_MINIFOTA_PROFILE_IDX 1
#define DOUBLE_MINIFOTA_URL_TYPE QL_FOTA_DWNLD_MOD_HTTP
#define DOUBLE_MINIFOTA_PACK_1_URL "http://112.31.84.164:8302/yang/old_new.bin"
#define DOUBLE_MINIFOTA_PACK_2_URL "http://112.31.84.164:8302/yang/old_new.bin"

	fota_exam_log("========== ql_double_minifota_test ==========");

#ifdef QL_SECFOTA_ENABLE
	ql_secfota_config(1, ql_secfota_callback);
#endif

	int ret = ql_fota_mini_system(DOUBLE_MINIFOTA_PROFILE_IDX,
								  DOUBLE_MINIFOTA_URL_TYPE,
								  DOUBLE_MINIFOTA_PACK_1_URL,
								  DOUBLE_MINIFOTA_PACK_2_URL,
								  NULL, NULL);
	if (ret != 0)
	{
		fota_exam_log("ql_fota_mini_system error, ret = %d", ret);
	}
}

void ql_fota_self_adapt_test(void)
{
	#define SELF_ADAPT_PROFILE_IDX		0
	#define SELF_ADAPT_PACK_URL			"http://112.31.84.164:8302/yang/old_new.bin"
	int ret = 0;
	fota_exam_log("========== !!!!!!!!!!!!!!!!!!!!99999999999 ==========");
    ret = ql_fota_start(SELF_ADAPT_PROFILE_IDX, SELF_ADAPT_PACK_URL);
	if(ret != 0){
		fota_exam_log("========== fota test end ==========");
	}
}

void ql_full_app_test(void)
{
	#define FULL_APP_PACK_URL 		"http://112.31.84.164:8300/august/app.bin"
	int ret = 0;
	//fota_exam_log("========== full fota test success !!!!!!!!!!!!!!!!!!!! ==========");
	fota_exam_log("full fota test star");
    ret = ql_fullfota_app_start(FULL_APP_PACK_URL);
	if(ret != 0){
		fota_exam_log("==========full fota test end ==========");
	}
}



static ql_flag_t fota_uart_flag;
#define QL_FOTA_UART_READ 0x1
void quec_fota_uart_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	static char read_flag = 1;
	if (read_flag)
	{
		read_flag = 0;
		ql_rtos_flag_release(fota_uart_flag, QL_FOTA_UART_READ, QL_FLAG_OR);
	}
}

static void quec_fota_uart_test(void * argv)
{
	int ret = -1, event = 0;
	int write_bytes = 0;
	unsigned char data[] = "hello, this is file fota test!!!, please Transfer dfota.bin\n";
	unsigned char read_buf[4096] = {0};
	ql_uart_config_t dcb;
	int count = 0, res = 0, file_len = 0;
	QFILE *fp = NULL;
	qlFotaImgProcCtxPtr ctx = NULL;
	int file_size = 0;
#if defined(QUEC_EXTERNAL_NORFLASH_TEST)
	unsigned char *id;
	int port_index = EXTERNAL_NORFLASH_PORT33_36;
	int clk = _APBC_SSP_FNCLKSEL_1_625MHZ_;

	printf("[FS] ========== exflash init  \r\n");
	ql_spi_nor_init(port_index, clk);

	id = ql_spi_nor_read_id(port_index);
	printf("[NORFLASH]=====norflash ID:0x%02x 0x%02x 0x%02x\r\n", id[0], id[1], id[2]);

	ret = qextfs_init('C', "external_fs", 1, port_index, 0, 0x100000);
	printf("[FS] ========== exfs init : %d	\r\n", ret);

	ql_fota_set_package_path("C:/FotaFile.bin");
	ql_remove("C:/FotaFile.bin");
#elif defined(QUEC_EXTERNAL_NANDFLASH_TEST) 
	ret = cust_yaffs_mount();
	if(ret)
	{
		printf("cust_yaffs_mount fail\r\n");
		return;
	}
	printf("cust_yaffs_mount success\r\n");
	char fota_pkg_dir[40] = {0};
	sprintf(fota_pkg_dir, "%s%s", QL_FOTA_IN_NAND_TAG, NAND_FOTA_PACK_PATH);	//nand tag
	ql_fota_set_package_path(fota_pkg_dir);
#endif

	ret = ql_uart_open(QL_MAIN_UART_PORT, QL_UART_BAUD_115200, QL_FC_HW);
	if (ret)
	{
		printf("open uart[%d] failed! \n", QL_MAIN_UART_PORT);
		return;
	}

	ql_rtos_flag_create(&fota_uart_flag);
	/*****************************************/
	ql_uart_get_dcbconfig(QL_MAIN_UART_PORT, &dcb);
	printf("[%s][%d]baudrate:%d, data_bit:%d, stop_bit:%d, parity_bit:%d, flow_ctrl:%d \r\n", __func__, __LINE__, dcb.baudrate, dcb.data_bit, dcb.stop_bit, dcb.parity_bit, dcb.flow_ctrl);
	dcb.baudrate = QL_UART_BAUD_115200;
	ql_uart_set_dcbconfig(QL_MAIN_UART_PORT, &dcb);
	/*****************************************/

	ql_uart_register_cb(QL_MAIN_UART_PORT, quec_fota_uart_callback); // use callback to read uart data

	ql_uart_write(QL_MAIN_UART_PORT, data, sizeof(data));
	ql_rtos_flag_wait(fota_uart_flag, QL_FOTA_UART_READ, QL_FLAG_OR, &event, QL_WAIT_FOREVER);

#ifdef QL_SECFOTA_ENABLE
	ql_secfota_config(1, ql_secfota_callback);
#endif

	ctx = ql_fota_init();
	if (!ctx)
	{
		printf("*** fota init fail ***");
		return;
	}

	count = 0;
	while ((ret >= 0) && (count < 60))
	{
		ret = ql_uart_read(QL_MAIN_UART_PORT, read_buf, sizeof(read_buf));
		if (ret <= 0)
		{
			count++;
			ql_rtos_task_sleep_ms(5);
			continue;
		}
		else
		{
			count = 0;
			file_len += ret;
			res = ql_fota_image_write(ctx, (void *)read_buf, ret);
			if (res < 0)
			{
				printf("ql_fwrite error:%d \n", res);
			}
		}
	}

	if (count >= 60)
	{
		printf("Transfer end:%d ,file_len:%d\n", ret, file_len);
	}
	ql_uart_close(QL_MAIN_UART_PORT);

	ret = ql_fota_image_flush(ctx);
	if (ret)
	{
		printf("*** fota image flush fail ***");
		return;
	}
	printf("ql_fota_image_flush end\n");
	ret = ql_fota_image_verify(ctx);
	if (ret)
	{
		printf("*** fota image verify fail ***");
		return;
	}
	printf("ql_fota_image_verify success! \n");

#ifdef QUEC_EXTERNAL_NANDFLASH_TEST
	ql_yaffs_sync(NAND_DEV_NAME);
#endif

	printf("ready to power reset!!\n");
	ql_power_reset();
}


#ifdef QUEC_IN_OR_EXT_FLASH_TEST

static unsigned char use_nandflash_ota_flag = 0;
void quec_usbcdc_fota_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	int read_len = 0;
	char r_data[1024] = {0};
	read_len = ql_uart_read(port, r_data, sizeof(r_data));
	printf("uart[%d] callback read data, data:%s, read_len:%d \n", port, r_data, read_len);
	if(strcmp(r_data,"use_in") == 0){
		printf("use quec inside flash for full fota app\r\n");
		ql_fota_fixed_internal_flash_upgrade(TRUE);
		ql_fota_set_package_path("U:/FotaFile.bin");
		use_nandflash_ota_flag = 0;

	}else if(strcmp(r_data,"use_ext") == 0){ 

		printf("use quec outside flash for full fota\r\n");
		ql_fota_fixed_internal_flash_upgrade(FALSE);
		ql_fota_set_package_path("QOTANAD@/spinand/fotafile.bin");
		use_nandflash_ota_flag = 1;
	}
}
static void quec_fota_uart_double_test(void * argv)
{
	int ret = -1, event = 0;
	int write_bytes = 0;
	unsigned char data[] = "hello, this is file fota test!!!, please Transfer dfota.bin\n";
	unsigned char read_buf[4096] = {0};
	ql_uart_config_t dcb;
	int count = 0, res = 0, file_len = 0;
	QFILE *fp = NULL;
	qlFotaImgProcCtxPtr ctx = NULL;
	int file_size = 0;


	ret = cust_yaffs_mount();
	if(ret)
	{
		printf("cust_yaffs_mount fail\r\n");
		return;
	}
	printf("cust_yaffs_mount success\r\n");


	ql_uart_open(QL_USB_CDC_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
    ql_uart_register_cb(QL_USB_CDC_PORT, quec_usbcdc_fota_callback);	//use callback to read uart data


	ret = ql_uart_open(QL_MAIN_UART_PORT, QL_UART_BAUD_115200, QL_FC_HW);
	if (ret)
	{
		printf("open uart[%d] failed! \n", QL_MAIN_UART_PORT);
		return;
	}

	ql_rtos_flag_create(&fota_uart_flag);
	/*****************************************/
	ql_uart_get_dcbconfig(QL_MAIN_UART_PORT, &dcb);
	printf("[%s][%d]baudrate:%d, data_bit:%d, stop_bit:%d, parity_bit:%d, flow_ctrl:%d \r\n", __func__, __LINE__, dcb.baudrate, dcb.data_bit, dcb.stop_bit, dcb.parity_bit, dcb.flow_ctrl);
	dcb.baudrate = QL_UART_BAUD_115200;
	ql_uart_set_dcbconfig(QL_MAIN_UART_PORT, &dcb);
	/*****************************************/

	ql_uart_register_cb(QL_MAIN_UART_PORT, quec_fota_uart_callback); // use callback to read uart data

	ql_uart_write(QL_MAIN_UART_PORT, data, sizeof(data));
	ql_rtos_flag_wait(fota_uart_flag, QL_FOTA_UART_READ, QL_FLAG_OR, &event, QL_WAIT_FOREVER);


	ctx = ql_fota_init();
	if (!ctx)
	{
		printf("*** fota init fail ***");
		return;
	}

	count = 0;
	while ((ret >= 0) && (count < 400))
	{
		ret = ql_uart_read(QL_MAIN_UART_PORT, read_buf, sizeof(read_buf));
		if (ret <= 0)
		{
			count++;
			ql_rtos_task_sleep_ms(5);
			continue;
		}
		else
		{
			count = 0;
			file_len += ret;
			res = ql_fota_image_write(ctx, (void *)read_buf, ret);
			if (res < 0)
			{
				printf("ql_fwrite error:%d \n", res);
			}
		}
	}
	
	if (count >= 400)
	{
		printf("Transfer end:%d ,file_len:%d\n", ret, file_len);
	}
	ql_uart_close(QL_MAIN_UART_PORT);

	ret = ql_fota_image_flush(ctx);
	if (ret)
	{
		printf("*** fota image flush fail ***");
		return;
	}
	printf("ql_fota_image_flush end\n");
	ret = ql_fota_image_verify(ctx);
	if (ret)
	{
		printf("*** fota image verify fail ***");
		return;
	}
	printf("ql_fota_image_verify success! \n");

if(use_nandflash_ota_flag)
	ql_yaffs_sync("/spinand");

	printf("ready to power reset!!\n");
	ql_power_reset();
}


application_init(quec_fota_uart_double_test, "quec_fota_uart_double_test", 50, 10);//串口FBF升级,双flash

#endif


// application_init(ql_mergefile_minifota_test, "ql_mergefile_minifota_test", 10, 2);	//单包升级
// application_init(ql_double_minifota_test, "quec_fota_test", 6, 10);					//双包升级
// application_init(ql_fota_self_adapt_test, "ql_fota_self_adapt_test", 10, 2);			//自适应包单包MINI和FULL升级
// application_init(ql_full_app_test, "ql_full_app_test", 2, 10);						//全量升级
// application_init(quec_fota_uart_test, "quec_fota_uart_test", 50, 10);				//串口FBF升级


