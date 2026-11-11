/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QUEC_ZIP_COM_H_
#define _QUEC_ZIP_COM_H_

#include <stdio.h>
#include "ql_fs.h"
#include "linklist.h"

#define QUEC_ASR_OPEN_FILE_HANDLE


#define ZIP_STD

#ifdef ZIP_STD
#include <time.h>
#ifndef MAX_PATH
#define MAX_PATH 128
#endif
//typedef unsigned long DWORD;
typedef DWORD ZRESULT;
typedef char TCHAR;
typedef int HANDLE;
typedef time_t FILETIME;
// Basic data types
typedef unsigned char  Byte;  // 8 bits
typedef unsigned int   uInt;  // 16 bits or more
typedef unsigned long  uLong; // 32 bits or more
typedef void *voidpf;
typedef void     *voidp;
typedef long z_off_t;

typedef struct HZIP 
{ 
   int unused; 
}*HZIP; 

#endif

#ifdef ZIP_STD
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#ifdef _MSC_VER
#include <sys/utime.h> // microsoft puts it here
#else
#include <utime.h>
#endif
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
#include <direct.h>
#define lumkdir(t) (mkdir(t))
#else
#include <unistd.h>
#define lumkdir(t) (ql_mkdir(t,0))

#define lnandmkdir(t) (ql_nand_mkdir(t))

#endif
#include <sys/types.h>
#include <sys/stat.h>
//
typedef unsigned short WORD;
#define _tcslen strlen
#define _tcsicmp stricmp
#define _tcsncpy strncpy
#define _tcsstr strstr
#define INVALID_HANDLE_VALUE 0
#ifndef _T
#define _T(s) s
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000200
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif

#else
#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>
#include "zip.h"
#include <stdlib.h>
#include <string.h>
#endif

#ifdef UNICODE
#define _tsprintf swprintf
#else
#define _tsprintf sprintf
#endif


#define ZIP_HANDLE   1
#define ZIP_FILENAME 2
#define ZIP_MEMORY   3
#define ZIP_FOLDER   4

#define false	0
#define true	1

typedef enum
{
	QL_FILE_OK = 0,
	QL_FILE_FAILURE  = -1,
}ql_ret_value_e;

 typedef unsigned int File_Handle;
 
 typedef enum {
	 FS_TYPE_LITTLEFS,	   // LittleFS file system (corresponding to the ql_fopen series of interfaces)
	 FS_TYPE_NAND		  // NAND File System (corresponding to the ql_nand_fopen series of interfaces)
 } QL_File_Type;
 
 typedef union {
	 QFILE* fs_fp;	
	 File_Handle nand_hdl; 
 } QL_File_Handle;


 /* 80 bytes for filename max, 4 bytes for "UFS:" */
#define QUEC_FILENAME_AT_INPUT_MAX_LEN_NONE_UFS (80)
#define QUEC_FILE_PREFIX_LEN (4)
#define QUEC_FILE_FILENAME_MAX_LEN     	(QUEC_FILENAME_AT_INPUT_MAX_LEN_NONE_UFS+QUEC_FILE_PREFIX_LEN)
#define QUEC_FILE_MAX_PATH_LEN         	(QUEC_FILE_FILENAME_MAX_LEN * 2)


typedef unsigned int				uint;


#endif // quec_unzip_H

