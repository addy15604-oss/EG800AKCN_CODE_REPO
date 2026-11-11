/*==========================================================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
===========================================================================================================*/

#include <stdio.h>
#include <string.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_type.h"
#include "ql_error.h"
#include "ql_func.h"


// 定义函数指针类型
typedef void (*ql_open_maping_init_func)(void);
typedef void (*cache_clean_memory_func)(void *addr, unsigned int size);

#define QUECTEL_APP1_ROM_STRAT   0x80615008      //app1 rom起始地址+8偏移地址

// 内核API映射函数启动地址
#define QUEC_STATIC_MAPPING_ADDR   0x7e47df14    //ql_static_mapping quec_staic_mapping 结构体地址;
#define VIRTUAL_FUNC_TBL_ADDR      0x7e47df24    //func_mapping virtual_func_tbl 结构体地址;
#define QL_OPEN_MAPING_INIT_ADDR   0x802ede2f    //ql_open_maping_init 函数地址
#define CACHE_CLEAN_MEMORY_ADDR    0x7e68d085    //CacheCleanMemory 函数地址

void  app1_start_test(void *APP1_ROM_STRAT)
{
    boot_para 	quec_boottime_para = {0};
    ql_open_entry* entry;
    unsigned int app_ro_addr;
    unsigned int tmp;
    app_ro_addr = (unsigned int)(APP1_ROM_STRAT);  
	entry = (ql_open_entry*) (APP1_ROM_STRAT);

    // 通过已知地址访问内核函数和变量
    ql_static_mapping *static_mapping = (ql_static_mapping *)QUEC_STATIC_MAPPING_ADDR;
    func_mapping *virtual_func_table = (func_mapping *)VIRTUAL_FUNC_TBL_ADDR;
    ql_open_maping_init_func open_maping_init =(ql_open_maping_init_func )QL_OPEN_MAPING_INIT_ADDR;
    cache_clean_memory_func cache_clean = (cache_clean_memory_func)CACHE_CLEAN_MEMORY_ADDR;
    
    // 初始化内核API映射表
	quec_boottime_para.static_maping = (ql_static_mapping *)QUEC_STATIC_MAPPING_ADDR;
	quec_boottime_para.kernel_maping= (func_mapping *)VIRTUAL_FUNC_TBL_ADDR;
    open_maping_init();

    printf("func:%s,line:%d,boot:   0x%08x \r\n",__func__,__LINE__, entry->boot);
    printf("func:%s,line:%d,kernel: 0x%08x \r\n",__func__,__LINE__, entry->kernel);
    printf("func:%s,line:%d,main:   0x%08x \r\n",__func__,__LINE__, entry->main);

    if(entry->data_load_addr && entry->data_ram_start && entry->data_ram_end) {
        UINT32 *src = (UINT32 *)(entry->data_load_addr);
        UINT32 *dst = (UINT32 *)entry->data_ram_start;
        UINT32 len = entry->data_ram_end - entry->data_ram_start;
        UINT32 i;
        int result;
        // 复制数据将flash RW段数据复制到RAM
        for(i = 0; i < len; i += 4) {
            *dst++ = *src++;
        }
        // 清除缓存
        cache_clean((void *)dst, len);

        // 重置指针
        src = (UINT32 *)(entry->data_load_addr);
        dst = (UINT32 *)entry->data_ram_start;

        tmp = memcmp((void *)src, (void *)dst, len/4);

        printf("memcmp result %s\r\n",(tmp==0)?"SUCCEED":"FAILED");

    }
    if(entry->boot)
    {
    	entry->boot(&quec_boottime_para);
    }
    if(entry->kernel)
    {
        entry->kernel(NULL);
    }
    if(entry->main)
    {
        (entry->main)(NULL);
    }
    ql_rtos_task_delete(NULL);
}

void app1_start_test_task(void)
{   
    printf("app1_start_task\r\n");
    app1_start_test(QUECTEL_APP1_ROM_STRAT);
}

//application_init(app1_start_task, "app1", 20, 0);