/*---------------------------------------------------------------------------
Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
Quectel Wireless Solution Proprietary and Confidential.
---------------------------------------------------------------------------*/
#ifndef _QUEC_CAMERA_H
#define _QUEC_CAMERA_H

typedef enum
{
   DEFULT_MODE,
   MIRROR_MODE,
   AUTO_MODE,
}QL_CAM_DECODE_MODE;

extern void Ql_Cam_Identify(unsigned char op);
extern void ql_set_decode_mirror(QL_CAM_DECODE_MODE mode);


#define CAM_SUPPORT_MAX_DECODE_LEN 2500


typedef struct
{
	unsigned char  Type;
	int            DecodeLen;
	int            result;
	unsigned char  DataBuf[CAM_SUPPORT_MAX_DECODE_LEN];
} CAL_IDENTITY_RESULT_STRUCT,*P_CAL_IDENTITY_RESULT_STRUCT;

typedef void (*Callback_preview)(P_CAL_IDENTITY_RESULT_STRUCT outdata);


typedef enum
{
   CAM_DECODE_SUCESS    = 0,
   CAM_DECODE_FAIL      = -1,
   CAM_DECODE_INIT_FAIL = -2,
   CAM_DECODE_NOT_OPEN  = -3,
} ENUM_CAM_DECODE_RESULT;


typedef struct

{
	unsigned short         camwidth;     
	unsigned short         camheight;
	unsigned char          dectaskprio;  
	unsigned char          decbufcnt;    
	unsigned int  *        decbufaddr;  
	Callback_preview       decodecb;    
	unsigned char 		   preview;      
	unsigned char          pretaskprio;  
	unsigned char          prebufcnt;   
	unsigned int *         prebufaddr;  
	unsigned char  *       lcdmemeryaddr;
	unsigned short         lcdprewidth;  
	unsigned short         lcdpreheight; 
} CAM_DECODE_CONFIG_CTX;


ENUM_CAM_DECODE_RESULT ql_qr_camera_open(CAM_DECODE_CONFIG_CTX * camconfig);

ENUM_CAM_DECODE_RESULT ql_qr_camera_close (void);














#endif


