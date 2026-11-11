/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#ifndef _QL_FLASHRW_H_
#define _QL_FLASHRW_H_


#define FLASH_RW_ERROR     -1
/*****************************************************************
* Function: ql_norflash_get_addr
*
* Description:
* 	Get partition address
* 
* Parameters:
* 	patition_name	[in] 	partition name
*   start_addr      [out]   partition addr
*   src             [out]   partition size
*
* Return:
* 	NULL
*	
*
*****************************************************************/
extern void ql_norflash_get_addr(unsigned char *patition_name, unsigned int *start_addr, unsigned int *size);

/*****************************************************************
* Function: ql_norflash_do_read
*
* Description:
* 	Read a section of a partition
* 
* Parameters:
* 	patition_name	[in] 	partition name
*   start_addr      [in]    The address from which data is to be read
*   buf_addr        [out]   Read the data storage address
*	size            [in]    Read the data size
* Return:
* 	NULL
*	
*
*****************************************************************/
extern unsigned int ql_norflash_do_read(unsigned char *patition_name, unsigned int addr, unsigned int buf_addr, unsigned int size);

/*****************************************************************
* Function: ql_norflash_do_write
*
* Description:
* 	write a section of a partition
* 
* Parameters:
* 	patition_name	[in] 	partition name
*   start_addr      [in]    The address from which data is to be write
*   buf_addr        [in]    write the data address
*	size            [in]    write the data size
* Return:
* 	NULL
*	
*
*****************************************************************/
extern unsigned int ql_norflash_do_write(unsigned char *patition_name, unsigned int addr, unsigned int buf_addr, unsigned int size);

/*****************************************************************
* Function: ql_norflash_do_erase
*
* Description:
* 	erase a section of a partition
* 
* Parameters:
* 	patition_name	[in] 	partition name
*   start_addr      [in]    The address from which data is to be erase
*   size            [in]    write the data size
*
* Return:
* 	NULL
*	
*
*****************************************************************/
extern int ql_norflash_do_erase(unsigned char *patition_name, unsigned int addr, unsigned int size);

/*****************************************************************
* Function: ql_norflash_read_id
*
* Description:
* 	get flash id
* 
* Parameters:
* 	NULL
* 
*
*
* Return:
* 	NULL
*	
*
*****************************************************************/
extern int ql_norflash_read_id(void);
#endif
