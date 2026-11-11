/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#ifndef __QL_OTP_H
#define __QL_OTP_H


#define GEU_BASE                       	0xD4201000

//bank0
#define GEU_FUSE_ADDR_READ_USE1         (GEU_BASE + 0x0410)//bank0的bit96~bit127读地址
#define GEU_FUSE_ADDR_WRITE_USE1 		(GEU_BASE + 0x0044)//bank0的bit96~bit127写地址




//NOTES:If secboot is enabled, operation of bank1 and bank2 areas is prohibited 
//如果使能了secboot,禁止操作bank1和bank2区域
//bank1和bank2，读写地址，客户在使用的时候，保持一致即可
//bank1: 0x424~0x440 bit0~bit255
#define GEU_FUSE_VAL_ROOT_KEY1		(GEU_BASE + 0x0424)
#define GEU_FUSE_VAL_ROOT_KEY2		(GEU_BASE + 0x0428)
#define GEU_FUSE_VAL_ROOT_KEY3		(GEU_BASE + 0x042C)
#define GEU_FUSE_VAL_ROOT_KEY4		(GEU_BASE + 0x0430)
#define GEU_FUSE_VAL_ROOT_KEY5		(GEU_BASE + 0x0434)
#define GEU_FUSE_VAL_ROOT_KEY6		(GEU_BASE + 0x0438)
#define GEU_FUSE_VAL_ROOT_KEY7		(GEU_BASE + 0x043C)
#define GEU_FUSE_VAL_ROOT_KEY8		(GEU_BASE + 0x0440)

//bank2: 0x444~0x460 bit0~bit255
#define GEU_FUSE_VAL_OEM_HASH_KEY1  (GEU_BASE + 0x0444) 
#define GEU_FUSE_VAL_OEM_HASH_KEY2  (GEU_BASE + 0x0448) 
#define GEU_FUSE_VAL_OEM_HASH_KEY3  (GEU_BASE + 0x044C) 
#define GEU_FUSE_VAL_OEM_HASH_KEY4  (GEU_BASE + 0x0450) 
#define GEU_FUSE_VAL_OEM_HASH_KEY5  (GEU_BASE + 0x0454) 
#define GEU_FUSE_VAL_OEM_HASH_KEY6  (GEU_BASE + 0x0458) 
#define GEU_FUSE_VAL_OEM_HASH_KEY7  (GEU_BASE + 0x045C) 
#define GEU_FUSE_VAL_OEM_HASH_KEY8  (GEU_BASE + 0x0460) 


/*****************************************************************
* Function: ql_otp_read
*
* Description:
* 	Read one-time programmable area
* 
* Parameters:
* 	address	  		[in] 	read address 
* 	data	  		[out] 	Read the value in the address
* Return:
* 	SUCCESS			0
*	FAILURE			-1
*
*****************************************************************/
int ql_otp_read(unsigned int addre,void *data);
/*****************************************************************
* Function: ql_otp_write
*
* Description:
* 	Write one-time programmable area
* 
* Parameters:
* 	address	  		[in] 	write address
* 	data	  		[in] 	write the value,Write values can only be 0 write 1, not 1 write 0
* Return:
* 	SUCCESS			0
*	FAILURE			-1
*
*****************************************************************/
int ql_otp_write(unsigned int addre,unsigned int data);
/*****************************************************************
* Function: ql_otp_secboot_whether_enabled
*
* Description:
* 	judge secboot whether enable
* 
* Parameters:
* 	VOID
* Return:
*	return vlue   		meaning   
*	    0  		 		NO-FUSE-ONLY	 		NO-BOOT33-verify 	
*		1				FUSE-ONLY				NO-BOOT33-verify  	
*		2				NO-FUSE-ONLY			BOOT33-verify   		
*		3				FUSE-ONLY				BOOT33-verify   
*****************************************************************/
int ql_otp_secboot_whether_enabled(void);
#endif  /*__QL_OTP_H */
