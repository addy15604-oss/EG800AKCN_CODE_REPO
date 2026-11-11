/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/

/*===========================================================================
 * include files
 ===========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_application.h"
#include "ql_zip.h"
#include "ql_fs.h"
#include "ql_log.h"
#include "ql_rtos.h"
#include "ql_uart.h"
#include "ql_type.h"
#include "ql_func.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_ZIPDEMO_LOG(fmt, args...) printf("[%s %d]" fmt, __func__, __LINE__, ##args)

#define FILE_NAME1 "testFile1.txt"
#define FILE_NAME2 "testFile2.txt"
#define FILE_ZIP_NAME "test.zip"
#define FILE_NAME_ZIP1 "test01.txt"
#define FILE_NAME_ZIP2 "test02.txt"
#define U_DISK "U:"
#define U_FILE_UNDER_ROOT1 "" U_DISK "/" FILE_NAME1 ""
#define U_FILE_UNDER_ROOT2 "" U_DISK "/" FILE_NAME2 ""
#define U_FILE_UNDER_ZIP1 "" U_DISK "/" FILE_NAME_ZIP1 ""
#define U_FILE_UNDER_ZIP2 "" U_DISK "/" FILE_NAME_ZIP2 ""
#define U_FILE_UNDER_ZIP "" U_DISK "/" FILE_ZIP_NAME ""
#define DIR_NAME "testDir"
#define U_DIR_PATH "" U_DISK "/" DIR_NAME ""

#define ZIP_FILE_LEN (1024)
/*===========================================================================
 * Variate
 ===========================================================================*/
const char *unZipFilePath = U_FILE_UNDER_ZIP;

char *unZip_path = NULL;
const char *TestFilePath[2] = {U_FILE_UNDER_ROOT1, U_FILE_UNDER_ROOT2};
const char *TestFilezipname[2] = {U_FILE_UNDER_ZIP1, U_FILE_UNDER_ZIP2};

enum
{
    QL_SPI_DEV0 = 0,
    QL_SPI_DEV2 = 2
};
enum
{
    QL_PIN_16_19,
    QL_PIN_33_36
};
enum
{
    SPI_CLOCK_13M = 0x0,
    SPI_CLOCK_26M = 0x1,
    SPI_CLOCK_52M = 0x2,
};

typedef struct file
{
    unsigned int cnt;
    unsigned char buf[1024];
} file_contex;

/*===========================================================================
 * Functions
 ===========================================================================*/
static void ql_zip_demo_thread(void *param)
{
    int ret = 0;
    int run_num = 0;
    HZIP hz = 0;
    QFILE *fp1 = NULL;
    QFILE *fp2 = NULL;
    unsigned char *zip_fs_space = calloc(10, ZIP_FILE_LEN);
    if (NULL == zip_fs_space)
    {
        QL_ZIPDEMO_LOG("malloc err\r\n");
        return;
    }
    ql_rtos_task_sleep_s(5);
    QL_ZIPDEMO_LOG("========== zip demo start ==========\r\n");
    QL_ZIPDEMO_LOG("========== start create Test Files:[%s][%s]==========\r\n", TestFilePath[0], TestFilePath[1]);
    fp1 = ql_fopen(TestFilePath[0], "w+");
    fp2 = ql_fopen(TestFilePath[1], "w+");
    if ((fp1 == NULL) || (fp2 == NULL))
    {
        QL_ZIPDEMO_LOG("open file failed ! %p-%p\r\n", fp1, fp2);
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] fail==========\r\n", TestFilePath[0], TestFilePath[1]);
        goto main_exit;
    }
    else
    {
        QL_ZIPDEMO_LOG("open file:[%x][%x] success!\r\n", fp1, fp2);
    }
    memset(zip_fs_space, 'a', 10 * ZIP_FILE_LEN);
    ret = ql_fwrite(zip_fs_space, 10 * ZIP_FILE_LEN, 1, fp1);
    if (ret > 0)
    {
        QL_ZIPDEMO_LOG("file:test01.txt write :%d bytes data\r\n", ret);
    }
    else
    {
        ql_fclose(fp1);
        ql_fclose(fp2);
        QL_ZIPDEMO_LOG("file:test01.txt write Failed\r\n");
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] Failed==========\r\n", TestFilePath[0], TestFilePath[1]);
        goto main_exit;
    }
    memset(zip_fs_space, 'b', 5 * ZIP_FILE_LEN);
    ret = ql_fwrite(zip_fs_space, 5 * ZIP_FILE_LEN, 1, fp2);
    if (ret > 0)
    {
        QL_ZIPDEMO_LOG("file:test02.txt write :%d bytes data\r\n", ret);
    }
    else
    {
        ql_fclose(fp1);
        ql_fclose(fp2);
        QL_ZIPDEMO_LOG("file:test02.txt write Failed\r\n");
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] Failed==========\r\n", TestFilePath[0], TestFilePath[1]);
        goto main_exit;
    }
    ql_fclose(fp1);
    ql_fclose(fp2);
    QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] success==========\r\n", TestFilePath[0], TestFilePath[1]);
    ql_rtos_task_sleep_s(5);

    for (run_num = 0; run_num < 1; run_num++)
    {
        QL_ZIPDEMO_LOG("========== loop_test[%d] start==========\r\n", run_num + 1);
        QL_ZIPDEMO_LOG("========== start zip file[%d]==========\r\n", run_num + 1);

        hz = ql_fs_zip_create(unZipFilePath, 0, &ret);
        if (ret < 0)
        {
            QL_ZIPDEMO_LOG("create empty.zip:[%s] Failed,errcode=%d\r\n", unZipFilePath, ret);
            goto main_exit;
        }
        else
        {
            QL_ZIPDEMO_LOG("create empty.zip:[%s] Success\r\n", unZipFilePath);
        }

        ret = ql_fs_zip_add_file(hz, TestFilezipname[0], TestFilePath[0]);
        if (ret != QL_FILE_OK)
        {
            QL_ZIPDEMO_LOG("add [%s] to zip Failed\r\n", TestFilePath[0]);
            goto main_exit;
        }
        QL_ZIPDEMO_LOG("add [%s] to zip Success\r\n", TestFilePath[0]);

        ret = ql_fs_zip_add_file(hz, TestFilezipname[1], TestFilePath[1]);
        if (ret != QL_FILE_OK)
        {
            QL_ZIPDEMO_LOG("add [%s] to zip Failed\r\n", TestFilePath[1]);
            goto main_exit;
        }
        QL_ZIPDEMO_LOG("add [%s] to zip Success\r\n", TestFilePath[1]);

        ql_fs_zip_close(hz);
#if 0
          //?????
          ql_rtos_task_sleep_s(5);
          ql_unzip_mem_data_s *ql_unzip_tmp = (ql_unzip_mem_data_s *)calloc(1, sizeof(ql_unzip_mem_data_s));
          if (ql_unzip_tmp == NULL)
          {
                QL_ZIPDEMO_LOG("malloc err");
                goto main_exit;
          }
          
          ql_unzip_tmp->data = (void *)calloc(1, 15*ZIP_FILE_LEN);
          if (ql_unzip_tmp->data == NULL)
          {
                QL_ZIPDEMO_LOG("malloc err");
                free(ql_unzip_tmp);
                goto main_exit;
          }

          ql_unzip_tmp->total_len = 15*ZIP_FILE_LEN;
          ret = ql_fs_unzip_ex(unZipFilePath, QL_UNZIP_TO_MEMERY, ql_unzip_tmp);
          if(ret!=QL_FILE_OK)
          {
                QL_ZIPDEMO_LOG("unzip file:[%s] Failed",unZipFilePath);
                free(ql_unzip_tmp->data);
                free(ql_unzip_tmp);
                goto main_exit;
          }
          QL_ZIPDEMO_LOG("used_len=%d,item_num=%d\n",ql_unzip_tmp->used_len, ql_unzip_tmp->item_num);
          
          int j=0;
          int i = 0;
          char *tmep = NULL;
          for (j=0; j<ql_unzip_tmp->item_num; j++)
          {
              QL_ZIPDEMO_LOG("file path=%s\n", ql_unzip_tmp->item[j].file_path);
              QL_ZIPDEMO_LOG("file len=%d,type=%d\n", ql_unzip_tmp->item[j].item_len, ql_unzip_tmp->item[j].file_type);
              tmep  = (char *)ql_unzip_tmp->item[j].item_data;
              if (tmep && ql_unzip_tmp->item[j].file_type == QL_UNZIP_FILE)
              {
                    if (0 == strcmp(ql_unzip_tmp->item[j].file_path, TestFilePath[0]))
                    {
                        for (i=0; i<ql_unzip_tmp->item[j].item_len; i++)
                        {
                             if (tmep[i] != 'a')
                             {
                                  QL_ZIPDEMO_LOG("not match -----------");
                                  break;
                             }
                        }
                    }

                    if (0 == strcmp(ql_unzip_tmp->item[j].file_path, TestFilePath[1]))
                    {
                        for (i=0; i<ql_unzip_tmp->item[j].item_len; i++)
                        {
                             if (tmep[i] != 'b')
                             {
                                  QL_ZIPDEMO_LOG("not match -----------");
                                  break;
                             }
                        }
                    }    
              }           
          }

          free(ql_unzip_tmp->data);
          free(ql_unzip_tmp);
          QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n",unZipFilePath);
#else
        //???????
        QL_ZIPDEMO_LOG("========== zip file[%d] success==========\r\n", run_num + 1);
        ql_rtos_task_sleep_s(5);
        QL_ZIPDEMO_LOG("========== start unzip file[%d]==========\r\n", run_num + 1);

        ret = ql_fs_unzip(unZipFilePath, unZip_path, 1);
        if (ret != QL_FILE_OK)
        {
            QL_ZIPDEMO_LOG("unzip file:[%s] Failed\r\n", unZipFilePath);
            goto main_exit;
        }

        QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n", unZipFilePath);

#endif
        QL_ZIPDEMO_LOG("========== unzip file[%d] success==========\r\n", run_num + 1);

        ql_rtos_task_sleep_s(5);
    }
main_exit:
    QL_ZIPDEMO_LOG("========== zip demo end ==========\r\n");
    ql_remove(TestFilePath[0]);
    ql_remove(TestFilePath[1]);
    if (zip_fs_space != NULL)
    {
        free(zip_fs_space);
        zip_fs_space = NULL;
    }
    ql_rtos_task_delete(NULL);
    return;
}

// application_init(ql_zip_demo_thread, "ql_zipdemo", 32, 0);

static void ql_unzip_demo_test(void *param)
{

    ql_rtos_task_sleep_s(5);
    QL_ZIPDEMO_LOG("========== start unzip file==========\r\n");

    int ret = ql_fs_unzip(unZipFilePath, unZip_path, 1);
    if (ret != QL_FILE_OK)
    {
        QL_ZIPDEMO_LOG("unzip file:[%s] Failed\r\n", unZipFilePath);
        goto main_exit;
    }
    QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n", unZipFilePath);
main_exit:
    QL_ZIPDEMO_LOG("========== zip demo end ==========\r\n");
    ql_rtos_task_delete(NULL);
    return;
}

#define NAND_DEV "NAND:/"
#define U_DEV "U:"
#define TEST_TXT_NAME "test.txt"
#define TEST_ZIP_NAME "test.zip"

#define LITFS_ZIP_PATH "" U_DEV "/" TEST_ZIP_NAME ""

typedef unsigned int File_Handle;
typedef unsigned int size_t;
void nand_zip_test(void *argv)
{

    int ret = 0;
    File_Handle filep;
    size_t status = 0;
    HZIP hzs;

    ql_rtos_task_sleep_s(5);
    ql_spi_nand_init(QL_SPI_DEV0, QL_PIN_33_36, SPI_CLOCK_26M);
    
    //写入文件1
    filep = ql_nand_fopen("mount_temp.txt", "w+");
    if (filep < 0)
    {
        printf("nand mount Failed\r\n");
        return;
    }
    char mount_buf[50]="mount_temp_the_test_file";

    status = ql_nand_fwrite(mount_buf, 1, strlen(mount_buf), filep);
    if (status != strlen(mount_buf))
    {
        printf("fx_file_write is failede.!!\n");
        return;
    }

    status = ql_nand_fsync(filep);
    if (status != 0)
    {
        printf("ql_nand_fsync is failede.!!\n");
        return;
    }
    printf("ql_nand_fwrite is ok.!!\r\n");
    ql_nand_fclose(filep);

    //写入文件2
    char bufs_test[50] = "quec_zips2_test_in_windows";
    char reads_buf[56] = {0};
    filep = ql_nand_fopen(TEST_TXT_NAME, "w+");
    status = ql_nand_fwrite(bufs_test, 1, strlen(bufs_test), filep);
    if (status != strlen(bufs_test))
    {
        printf("fx_file_write is failede.!!\n");
        return;
    }
    status = ql_nand_fsync(filep);
    if (status != 0)
    {
        printf("ql_nand_fsync is failede.!!\n");
        return;
    }
    printf("ql_nand_fwrite is ok.!!\r\n");
    ql_nand_fclose(filep);

    // 构建nand文件路径，用于压缩解压
    char unZipFilePath[50] = {0};
    sprintf(unZipFilePath, "%s%s", NAND_DEV, TEST_TXT_NAME);
    char zipFilePath[50] = {0};
    sprintf(zipFilePath, "%s%s", NAND_DEV, TEST_ZIP_NAME);

    // 创建一个zip文件头
    hzs = ql_fs_zip_create(zipFilePath, 0, &ret);
    if (ret < 0)
    {
        printf("create empty.zip:[%s] Failed,errcode=%d\r\n", zipFilePath, ret);
        ql_fs_zip_close(hzs);
        return;
    }
    else
    {
        printf(("create empty.zip:[%s] Success\r\n", zipFilePath));
    }
    
    //添加文件1
    ret = ql_fs_zip_add_file(hzs, TEST_TXT_NAME, unZipFilePath);
    if (ret != QL_FILE_OK)
    {
        printf("add [%s] to zip Failed\r\n", TEST_TXT_NAME);
        ql_fs_zip_close(hzs);
        return;
    }
     
    ///添加文件2
    char unZipFilePath2[50] = {0};
    sprintf(unZipFilePath2, "%s%s", NAND_DEV, "mount_temp.txt");

    ret = ql_fs_zip_add_file(hzs, "mount_temp.txt", unZipFilePath2);
    if (ret != QL_FILE_OK)
    {
        printf("add [mount_temp.txt] to zip Failed\r\n");
        ql_fs_zip_close(hzs);
        return;
    }

    printf("add to zip Success\r\n");
    ql_fs_zip_close(hzs);
    // printf("add to zip Success\r\n");
    printf("zip_size_file:[%d]\r\n", sizeof(zipFilePath));

    //先移除文件
    ret = ql_nand_fremove(TEST_TXT_NAME);
    ret = ql_nand_fremove("mount_temp.txt");

    printf("ql_nand_fsize :ret=%d\r\n", ql_nand_fsize(TEST_ZIP_NAME));
    
    //开始解压数据
#if 1
    ql_rtos_task_sleep_s(5);
    printf("========== start unzip file[%d]==========\r\n");

    char *unZip_path = "NAND:/";
    ret = ql_fs_unzip(zipFilePath, NULL, 1);

    if (ret != QL_FILE_OK)
    {
        printf("unzip file:[%s] Failed\r\n", zipFilePath);
        return;
    }
    printf("unzip file:[%s] Success\r\n", zipFilePath);
#endif
    //读取解压的文件
#if 1
    printf("========== start read file1==========\r\n");
    filep = ql_nand_fopen(TEST_TXT_NAME, "r");
    if (filep < 0)
    {
        printf("nand mount Failed:%d\r\n",__LINE__);
        return;
    }
    status = ql_nand_fread(reads_buf, 1, sizeof(reads_buf), filep);
    
    printf("status:%d\r\n",status);

    ql_nand_fclose(filep);
    printf("reads_buf1:%s\r\n", reads_buf);
   

    printf("========== read file2_start==========\r\n");

    memset(reads_buf, 0, sizeof(reads_buf));

    filep = ql_nand_fopen("mount_temp.txt", "r");
    if (filep < 0)
    {
        printf("nand mount Failed:%d\r\n",__LINE__);
        return;
    }
    status = ql_nand_fread(reads_buf, 1, sizeof(reads_buf), filep);

    printf("status:%d\r\n", status);

    ql_nand_fclose(filep);

    printf("reads_buf2:%s\r\n", reads_buf);
    printf("read from zip Success\r\n");

    printf("========== end read file==========\r\n");

#endif
    //把压缩包数据写到littfs系统中，方便导入到windows中进行解压
#if 0
    
    printf("========== start read test_zip==========\r\n");

    char zip_buf[169]={0};

    filep=ql_nand_fopen("test.zip", "rb");

    if(filep == NULL){
        printf("fx_file_open is failede.!!\r\n");
        return;
    }
    status = ql_nand_fread(zip_buf,1,sizeof(zip_buf),filep);

    if(status != 168){
        printf("fx_file_read is failede.!!\n");
        ql_nand_fclose(filep);
        return;
    }
    printf("read from zip Success\r\n");
    
    printf("status:%d\r\n",status);

    ql_nand_fclose(filep);
    
   // printf("zip_data:");

    // for(int i=0;i<sizeof(zip_buf);i++){
    //     printf("%x",zip_buf[i]);
    // }
    // printf("\r\n");
 
    QFILE *littfs_filep = NULL;
    
    littfs_filep = ql_fopen(LITFS_ZIP_PATH, "wb");

    if(littfs_filep == NULL){
        printf("ql_fopen is failede.!!\r\n");
        return;
    }
     
    ret = ql_fwrite(zip_buf,sizeof(zip_buf),1,littfs_filep);
    
    if(ret>0){
        printf("ql_fwrite is ok.!!\r\n");
        ql_fclose(littfs_filep);
    }else{
        printf("ql_fwrite is failede.!!\r\n");
        ql_fclose(littfs_filep);
        return;
    }
    ql_fclose(littfs_filep);

#endif
}



//解压从windows接受的压缩包
ql_sem_t semaRef;
int read_len = 0;
char r_data[1024] = {0};
void quec_uart_rx_callback(QL_UART_PORT_NUMBER_E port, void *para)
{

    read_len = ql_uart_read(port, r_data, sizeof(r_data));
    ql_rtos_semaphore_release(semaRef);
}
void quec_main_uart_rx(void)
{

    int ret = -1;

    ql_uart_config_t dcb;
    ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART);

    ret = ql_uart_open(QL_MAIN_UART_PORT, QL_UART_BAUD_115200, QL_FC_NONE);

    ql_uart_get_dcbconfig(QL_MAIN_UART_PORT, &dcb);

    dcb.baudrate = QL_UART_BAUD_115200;
    ql_uart_set_dcbconfig(QL_MAIN_UART_PORT, &dcb);

    ql_uart_register_cb(QL_MAIN_UART_PORT, quec_uart_rx_callback); // use callback to read uart data

    // 清除可能已存在的信号量计数
    while(ql_rtos_semaphore_wait(semaRef, 0) == 0);

    ql_rtos_semaphore_wait(semaRef, QL_WAIT_FOREVER);

    printf("data_len:%d\r\n", read_len);

    // printf("data:\r\n");
    // for (int i = 0; i < read_len; i++)
    // {
    //     printf("%x", r_data[i]);
    // }

    // printf("\r\n");
 
}

static void nand_unzip_test(void *argv)
{

    printf("======nand_unzip_test_start=======\r\n");

    int ret = 0;
    File_Handle filep,filep1;
    size_t status = 0;
    HZIP hzs;

    char from_win_buf[50]={0};

    ql_rtos_semaphore_create(&semaRef, 0);

    quec_main_uart_rx();//接受从windows下压缩包的数据
   
    ql_spi_nand_init(QL_SPI_DEV0, QL_PIN_33_36, SPI_CLOCK_26M);

    filep = ql_nand_fopen("zip_test2.zip", "wb");

    if (filep < 0)
    {
        printf("ql_nand_fopen is failede.!!\r\n");
        return;
    }
    status = ql_nand_fwrite(r_data, 1, sizeof(r_data), filep);

    if (status > 0)
    {
        printf("ql_nand_fwrite is ok.!!\r\n");
        ql_nand_fclose(filep);
    }
    else
    {
        printf("ql_nand_fwrite is failede.!!\r\n");
        ql_nand_fclose(filep);
        return;
    }
    

    char zipFilePath[128] = {0};

    sprintf(zipFilePath, "%s%s", NAND_DEV,"zip_test2.zip" );
    
    ret = ql_fs_unzip(zipFilePath, NULL, 1);

    if (ret != QL_FILE_OK)
    {
        printf("unzip file:[%s] Failed\r\n", zipFilePath);
        return;
    }
    printf("unzip file:[%s] Success\r\n", zipFilePath);

    filep = ql_nand_fopen("zip_test.txt", "r");

    if(filep < 0){
        printf("ql_nand_fopen is failede.!!\r\n");
        ql_nand_fclose(filep);
        return;
    } 
   
    //读文件1
    status = ql_nand_fread(from_win_buf, 1, sizeof(from_win_buf), filep);

    if (status > 0)
    {
        printf("ql_nand_fread is ok.!! %d\r\n",__LINE__);
        ql_nand_fclose(filep);
    }
    else
    {
        printf("ql_nand_fread is failede.!!\r\n");
        ql_nand_fclose(filep);
        return;
    }

    printf("status:%d\r\n",status);
    printf("from_win_buf:%s\r\n", from_win_buf);
    
    //读文件2
    memset(from_win_buf, 0, sizeof(from_win_buf));

    filep1 = ql_nand_fopen("zip_test2.txt", "r");

    if(filep1 < 0){
        printf("ql_nand_fopen is failede.!!\r\n");
        ql_nand_fclose(filep1);
        return;
    }
  
    status = ql_nand_fread(from_win_buf, 1, sizeof(from_win_buf), filep1);

    if (status > 0)
    {
        printf("ql_nand_fread is ok.!!\r\n");
        ql_nand_fclose(filep1);
    }
    else
    {
        printf("ql_nand_fread is failede.!!\r\n");
        ql_nand_fclose(filep1);
        return;
    }

    printf("from_win_buf:%s\r\n", from_win_buf);

    printf("======nand_unzip_test_end=======\r\n");
}





#define FILE_ZIP_NAME1 "test3.zip"
#define FILE_NAME1 "ZIPTEST.TXT"
#define U_DISK1 "U:"
#define FILE_NAME_ZIP1 "ZIPTEST1.txt"
#define FILE_ZIP_DIR_FILE1 "" U_DISK1 "/" FILE_NAME_ZIP1 ""
#define FILE_ZIP_DS_FILE1 "" U_DISK1 "/" FILE_NAME1 ""
#define FILE_ZIP_PATH1 "" U_DISK1 "/" FILE_ZIP_NAME1 ""
const char *unZipFilePaths1 = FILE_ZIP_PATH1;

const char *zipFilePath1 = FILE_ZIP_DS_FILE1;

const char *zipNAME1 = FILE_ZIP_DIR_FILE1;

char *unZip_paths1 = NULL;
void littfs_zip_test(void *argv)
{

    int ret = 0;

    QFILE *filep = NULL;

    HZIP hzs;
    // ql_rtos_task_sleep_s(5);
    // ql_spi_nand_init(QL_SPI_DEV0, QL_PIN_33_36,SPI_CLOCK_26M);
    // ret = ql_nand_mkdir("quec_zips1");
    // printf("ql_nand_mkdir %d\n",ret);

    char bufs_test[50] = "quec_zips2_test_in_windows";

    char read_buf[50] = {0};

    filep = ql_fopen(FILE_ZIP_DS_FILE1, "wb+");

    if (filep == NULL)
    {
        printf("fx_file_open is failede.!!\n");
        return;
    }

    ret = ql_fwrite(bufs_test, strlen(bufs_test), 1, filep);

    if (ret > 0)
    {
        printf("fx_file_write is success.!!\n");
    }
    else
    {
        printf("fx_file_write is failede.!!\n");
        ql_fclose(filep);

        return;
    }

    ql_fclose(filep);

    ql_rtos_task_sleep_s(5);

    // 创建一个zip文件头
    hzs = ql_fs_zip_create(unZipFilePaths1, 0, &ret);

    if (ret < 0)
    {
        printf("create empty.zip:[%s] Failed,errcode=%d\r\n", unZipFilePaths1, ret);
        printf("------------------\n");
        ql_fs_zip_close(hzs);
        return;
    }
    else
    {
        printf(("create empty.zip:[%s] Success\r\n", unZipFilePaths1));
    }

    ret = ql_fs_zip_add_file(hzs, zipNAME1, zipFilePath1);

    if (ret != QL_FILE_OK)
    {
        printf("add [%s] to zip Failed\r\n", zipFilePath1);
        ql_fs_zip_close(hzs);
        return;
    }
    else
    {
        printf("add [%s] to zip Success\r\n", zipFilePath1);
    }

    ql_fs_zip_close(hzs);

    // printf("add to zip Success\r\n");
    ql_rtos_task_sleep_s(5);
    printf("========== start unzip file[%d]==========\r\n");

    ret = ql_fs_unzip(unZipFilePaths1, unZip_paths1, 1);

    if (ret != QL_FILE_OK)
    {
        printf("unzip file:[%s] Failed\r\n", unZipFilePaths1);
        return;
    }
    printf("unzip file:[%s] Success\r\n", unZipFilePaths1);

#if 0
    printf("========== start read zip file==========\r\n");

    filep =ql_fopen(unZipFilePaths1, "rb");

    if(filep==NULL){
        printf("littfss_fx_file_open is failede.!!\n");
        return;
    }

    ret=ql_fread(read_buf, sizeof(read_buf), 1, filep);

    ql_fclose(filep);

    printf("read file size is %d\r\n",ret);

    printf("data:");
    for(int i=0;i<sizeof(read_buf);i++){
        printf("%x",read_buf[i]);
    }
    printf("\r\n");
#endif
}

// application_init(ql_unzip_demo_test, "ql_zipdemo", 32, 0);

//application_init(nand_zip_test, "nand_zip_test", 10, 0);

//application_init(littfs_zip_test, "littfs_zip_test", 10, 0);

// application_init(ql_zip_demo_thread,"ql_zip_demo_thread",10,0);

//application_init(nand_unzip_test, "nand_unzip_test", 10, 0);

// application_init(test_seek, "test_seek", 10, 0);
