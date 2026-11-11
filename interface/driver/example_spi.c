/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include <string.h>

#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_spi.h"


int  compare_count = 0 ;  //用于测试SPI回环,回环检测失败 数值加1

/*
各个型号SPI初始化所对应的PORT口号,2025-3-4-nick
1602:SPI0(GPIO33~36)SPI_PORT0;SPI1(GPIO4~7)SPI_PORT1;SPI2(GPIO12~15)SPI_PORT2
1605:SPI0(GPIO33~36)SPI_PORT0;SPI1(GPIO4~7)SPI_PORT1;SPI2(GPIO12~15)SPI_PORT2
1606:SPI0(GPIO16~19)SPI_PORT0;SPI1(GPIO4~7)SPI_PORT2;SPI2(GPIO33~36)SPI_PORT1//1606情况特殊
1607:SPI0(GPIO33~36)SPI_PORT0;SPI1(GPIO4~7)SPI_PORT2;//1607只有两组SPI
1609:SPI0(GPIO33~36)SPI_PORT0;SPI2(GPIO12~15)SPI_PORT2//1609只有两组SPI
*/
/* Fias-2022/04/08: (spi ql_spi_write_read)*/
/*Note: For this test you need to connect TX and RX of SPI*/
static void quec_spi_test(void * argv)
{
	printf("spi ql_spi_write_read test \r\n");

	int ret = -1;
	unsigned char buf_in[256] = {0};
	unsigned char buf_out[128] = "=========This is the spi DMA test !!===========\r\n";

	ql_spi_init(SPI_PORT2, SPI_MODE0,SPI_CLK_26MHZ);
	//ql_rtos_task_sleep_s(1);

	while(1)
	{	
		printf("ql_spi_buf_out:  %s \r\n ",buf_out);	
		ret= ql_spi_write_read(SPI_PORT2, buf_in, buf_out, sizeof(buf_out));
		if(ret != 0)
		{
			printf("ql_spi_read error, ret = %d \r\n",ret);
			break;
		}
		
		printf("ql_spi_buf_in:  %s \r\n",buf_in);

		if(0 != memcmp(buf_out,buf_in,strlen(buf_out)))
		{
			  compare_count ++	;
			  printf("compare_count:  %d \r\n",compare_count);
			  
		}
		
		 memset(buf_in,0,sizeof(buf_in));

		 ql_rtos_task_sleep_ms(500);
		
	}
}


static void quec_spi_pio_width_test(void * argv)
{
	printf("spi ql_spi_pio_width_write_read test \r\n");

	int ret = -1, i = 0, bit_width = 9;
	unsigned int buf_in[4] = {0};
    unsigned int buf_out[4] = {0};

	buf_out[0] = 0x01AA;
	buf_out[1] = 0x0155;
	buf_out[2] = 0x00F1;
	buf_out[3] = 0x001F;

	ql_rtos_task_sleep_s(2);

	ret = ql_spi_init(SPI_PORT0, SPI_MODE0,SPI_CLK_3_25MHZ);

    printf("spi init ret = 0x%x, sizeof(buf_out)/sizeof(unsigned int)=%d\n", ret, sizeof(buf_out)/sizeof(unsigned int));

	while(1)
	{	

		printf("ql_spi_buf_out:  ");			
		for (i = 0; i < sizeof(buf_out)/sizeof(unsigned int); i++)
		{
			printf("0X%x ",buf_out[i]);	
		}
		printf("\r\n ");

		ret= ql_spi_pio_width_write_read(SPI_PORT0, buf_in, buf_out, sizeof(buf_out)/sizeof(unsigned int), bit_width);
		if(ret != 0)
		{
			printf("ql_spi_pio_width_write_read error, ret = %d \r\n",ret);
			break;
		}
		
		printf("ql_spi_buf_in:  ");			
		for (i = 0; i < sizeof(buf_in)/sizeof(unsigned int); i++)
		{
			printf("0X%x ",buf_in[i]);	
		}
		printf("\r\n ");

        ql_rtos_task_sleep_ms(500);
		
	}
}

//application_init(quec_spi_test, "quec_spi_test", 4, 20);
//application_init(quec_spi_pio_width_test, "quec_spi_pio_width_test", 4, 20);

