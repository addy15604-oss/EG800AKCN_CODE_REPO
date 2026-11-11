/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QL_FS_H
#define _QL_FS_H

#include "ql_type.h"
#include "lfs.h"

#define QFS_ID_MAX     3
#define NAND_FILE_NAME_SIZE 128

typedef unsigned int FILE_ID;

typedef struct
{
	int fs_id;
	lfs_file_t file;
} QFILE;

typedef struct
{
	int fs_id;
	lfs_dir_t dir;
} QDIR;

struct ql_yaffs_dev
{
    const char *name;
    u32 start_block;    
    u32 end_block;
};

typedef struct 
{   
    // Type of the file, either TYPE_REG:0 or TYPE_DIR:1
    UINT8 type;

    // Size of the file, only valid for REG files
    unsigned int size;

    // Name of the file stored as a null-terminated string
    char name[NAND_FILE_NAME_SIZE];
}NandFs_info;

#define U_DISK_SYM	'U'
#define B_DISK_SYM	'B'

#define PATH_MAX				260

typedef struct QL_TAG_TIME_INFO_STRUCT
{
        unsigned int    m_nMilliSec             : 10;   //Million second, 0-999
        unsigned int    m_nYear                 : 12;   //Year, 0-4095
        unsigned int    m_nReserved1    		  : 10;   //Reserved, Zero always
        unsigned int    m_nMonth                : 4;    //Month, 1-12
        unsigned int    m_nDate                 : 5;    //Day, 1-31
        unsigned int    m_nHour                 : 5;    //Hour, 0-23
        unsigned int    m_nMinute               : 6;    //Minute, 0-59
        unsigned int    m_nSecond               : 6;    //Second, 0-59
        unsigned int    m_nDayOfWeek    		  : 3;    //Day of week, 1-7
        unsigned int    m_nReserved2         	  : 3;    //Reserved, Zero always
}QL_TIMEINFO_T, *QL_LPTIMEINFO_T;

//	File Info defination
typedef	struct	QL_TAG_FS_FILE_INFO_STRUCT
{
	unsigned int		m_nSize;			//The size of the struct
	unsigned int		m_nMask;			//The op mask
	QL_TIMEINFO_T	m_tCreate;			//The create datetime
	QL_TIMEINFO_T	m_tModify;			//The modify datetime
	QL_TIMEINFO_T	m_tAccess;			//The access datetime
	unsigned int		m_nFileSize;		//The file size
	unsigned int		m_nAttr;			//The attribute
	unsigned short  	m_lpszFullName[PATH_MAX + 4];	//The full path
}QL_FSFILEINFO_T, *QL_LPFSFILEINFO_T;


typedef	struct	QL_TAG_FAT32FILEINFO_STRUCT
{
    QL_FSFILEINFO_T info;
    char name[100];
}QL_FAT32FILEINFO_T, *QL_LPFAT32FILEINFO_T;

typedef struct QL_FS_INFO_STRUCT
{
    long long iToTalSize;
	long long iUsedSize;
}QL_FS_INFO_T;

/*-----------------------------------------------------------------------------------------
 * Function: Format file system according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_format(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Mount file system according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_mount(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Unmount file system according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_unmount(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Get file system size according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_size(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Get file system used size according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_used_size(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Get file system free size according to disk character
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fs_free_size(char disk_sym);

/*-----------------------------------------------------------------------------------------
 * Function: Open file in the file system
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *    <mode>: 	[IN] Open mode .
 *		
 * Return:
 *    File stream.
-----------------------------------------------------------------------------------------*/
QFILE * ql_fopen(const char *fname, const char *mode);

/*-----------------------------------------------------------------------------------------
 * Function: Open file in the file system
 *
 * Parameter:
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *   LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fclose(QFILE * fp);

/*-----------------------------------------------------------------------------------------
 * Function: Synchronization File in the file system
 *
 * Parameter:
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fsync(QFILE * fp);

/*-----------------------------------------------------------------------------------------
 * Function: Read file
 *
 * Parameter:
 *    <buffer>: [OUT] File read buff.
 *    <size>: 	[IN] File read size.
 *    <num>:	[IN] File read format size.
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *    Real read size.
-----------------------------------------------------------------------------------------*/
int ql_fread(void * buffer, size_t size, size_t num, QFILE * fp);

/*-----------------------------------------------------------------------------------------
 * Function: Write file
 *
 * Parameter:
 *    <buffer>: [IN] File write buff.
 *    <size>: 	[IN] File write size.
 *    <num>:	[IN] File write format size.
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_fwrite(void * buffer, size_t size, size_t num, QFILE * fp);

/*-----------------------------------------------------------------------------------------
 * Function: Write one character into file
 *
 * Parameter:
 *    <chr>: [IN] File write character.
 *    <fp>:  [IN] File stream.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_fputc(int chr, QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Write string into file
 *
 * Parameter:
 *    <chr>: [IN] File write string.
 *    <fp>:  [IN] File stream.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_fputs(const char *str, QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Format write string into file
 *
 * Parameter:
 *    <fp>:     [IN] File stream.
 *    <format>: [IN] File format.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_fprintf(QFILE *fp, const char *format, ...);

/*-----------------------------------------------------------------------------------------
 * Function: Read one character from file
 *
 * Parameter:
 *    <fp>:  [IN] File stream.
 *		
 * Return:
 *    Read character.
-----------------------------------------------------------------------------------------*/
int ql_fgetc(QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Read string from file
 *
 * Parameter:
 *    <fpstr>:  [OUT] Read buff.
 *    <n>:      [IN] Read lengths.
 *    <fp>:     [IN] File stream.
 *		
 * Return:
 *    Read string.
-----------------------------------------------------------------------------------------*/
char *ql_fgets(char *str, int n, QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Format read string from file
 *
 * Parameter:
 *    <fp>:     [IN] File stream.
 *    <format>: [IN] File format.
 *		
 * Return:
 *    Real read size.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fscanf(QFILE *fp, const char *format, ...);


/*-----------------------------------------------------------------------------------------
 * Function: Relocate file pointer
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *    <offset>: 	[IN] off set.
 *    <origin>:     [IN] Starting position.
 *		
 * Return:
 *     LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_fseek(QFILE *fp, long offset, int origin);

/*-----------------------------------------------------------------------------------------
 * Function: File truncate
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *    <length>:    [IN] truncate size.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_ftruncate(QFILE *fp, u32 length);

/*-----------------------------------------------------------------------------------------
 * Function: Get the current location of the file pointer
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *		
 * Return:
 *    Current location of the file pointer.
-----------------------------------------------------------------------------------------*/
long ql_ftell(QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Set file location to given stream
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *		
 * Return:
 *    
-----------------------------------------------------------------------------------------*/
int ql_frewind(QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Get file size
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *		
 * Return:
 *    File size.
-----------------------------------------------------------------------------------------*/
int ql_fsize(QFILE *fp);

/*-----------------------------------------------------------------------------------------
 * Function: Create directory
 *
 * Parameter:
 *    <path>: 	[IN] Directory path.
 *    <mode>: 	[IN] create mode.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_mkdir(const char *path, u32 mode);

/*-----------------------------------------------------------------------------------------
 * Function: Open directory
 *
 * Parameter:
 *    <path>: 	[IN] Directory path.
 *    <mode>: 	[IN] create mode.
 *		
 * Return:
 *    Directory stream.
-----------------------------------------------------------------------------------------*/
QDIR * ql_opendir(const char *path);

/*-----------------------------------------------------------------------------------------
 * Function: Open directory
 *
 * Parameter:
 *    <dp>: 	[IN] Directory stream.
 *		
 * Return:
 *    Directory stream.
-----------------------------------------------------------------------------------------*/
int ql_closedir(QDIR *dp);

/*-----------------------------------------------------------------------------------------
 * Function: Read directory
 *
 * Parameter:
 *    <dp>: 	[IN] Directory stream.
 *    <info>: 	[IN] File info structure ..
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_readdir(QDIR *dp, struct lfs_info *info);

/*-----------------------------------------------------------------------------------------
 * Function: Relocate directory pointer
 *
 * Parameter:
 *    <dp>: 	[IN] Directory stream.
 *    <loc>: 	[IN] Relocate position.
 *		
 * Return:
 *    
-----------------------------------------------------------------------------------------*/
int ql_seekdir(QDIR *dp, long int loc);

/*-----------------------------------------------------------------------------------------
 * Function: Get the current location of the directory pointer
 *
 * Parameter:
 *    <dp>: 	[IN] Directory stream.
 *		
 * Return:
 *    Current location of the directory pointer.
-----------------------------------------------------------------------------------------*/
long ql_telldir(QDIR *dp);

/*-----------------------------------------------------------------------------------------
 * Function: Set directory location to given stream
 *
 * Parameter:
 *    <dp>: 	[IN] Directory stream.
 *		
 * Return:
 *    
-----------------------------------------------------------------------------------------*/
int ql_rewinddir(QDIR *dp);

/*-----------------------------------------------------------------------------------------
 * Function: Remove files in the file system
 *
 * Parameter:
 *    <fname>: 	[IN] File name.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_remove(const char *fname);

/*-----------------------------------------------------------------------------------------
 * Function: Rename files 
 *
 * Parameter:
 *    <oldpath>: 	[IN] File oldpath.
 *    <newpath>: 	[IN] File newpath.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_rename(const char *oldpath, const char *newpath);

/*-----------------------------------------------------------------------------------------
 * Function: Copy files 
 *
 * Parameter:
 *    <oldpath>: 	[IN] File old path.
 *    <newpath>: 	[IN] File new path.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_copy(const char *oldpath, const char *newpath);

/*-----------------------------------------------------------------------------------------
 * Function: Judge whether the file exists
 *
 * Parameter:
 *    <path>: 	[IN] File path.
 *    <mode>: 	[IN] File mode.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_access(const char *path, u32 mode);

/*-----------------------------------------------------------------------------------------
 * Function: Find info about a directory
 *
 * Parameter:
 *    <path>: 	[IN] File path.
 *    <info>: 	[IN] File info structure.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_stat(const char *path, struct lfs_info *info);
/***********************************************************************
 *
 * Name:		ql_FDI_fopen
 *
 * Description: used in an implementation-specified matter to open or create a file and
*    associate it with a stream.
 *
 * Parameters:
 *  char                 *filename_ptr      [IN]    const character string for file name specifier.
*  char			*mode                [IN] 	const character string for type specification;
 *
 * Returns:
 *	the stream index used for the opened file.
 *    If an error is detected, FDI_fopen returns 0.
 *
 * Notes:
 *
 ***********************************************************************/
FILE_ID ql_FDI_fopen(const char *filename_ptr, const char *mode);
/***********************************************************************
 *
 * Name:		ql_FDI_fclose
 *
 * Description: used to close a file stream.
 *
 * Parameters:
 *  FILE_ID                 stream      [IN]    stream index for the file to be closed.
 *
 * Returns:
 *		0:   pass
 * 	 	EOF: fail
 * Notes:
 *
 ***********************************************************************/
int ql_FDI_fclose(FILE_ID stream);
/***********************************************************************
 *
 * Name:		ql_FDI_fread
 *
 * Description: read file.
 *
 * Parameters:
 *	 void		*buffer_ptr   	[OUT]	pointer to buffer to place data read
 *	size_t		element_size 	[IN] 		size of element referenced by buffer pointer
 *	size_t		count    		[IN]		number of elements to be read
 *	FILE_ID 		stream		[IN]		stream index for the file to be closed.
 *
 * Returns:
 *		number of elements succesfully read
 * Notes:
 *
 ***********************************************************************/
size_t ql_FDI_fread(void *buffer_ptr, size_t element_size, size_t count, FILE_ID stream);
/***********************************************************************
 *
 * Name:		ql_FDI_fwrite
 *
 * Description: write file.
 *
 * Parameters:
 *	 void		*buffer_ptr   	[IN]		pointer to buffer to be written
 *	size_t		element_size 	[IN] 		size of element referenced by buffer pointer
 *	size_t		count    		[IN]		number of elements to be read
 *	FILE_ID 		stream		[IN]		stream index for the file to be closed.
 *
 * Returns:
 *		number of elements succesfully write
 * Notes:
 *
 ***********************************************************************/
size_t ql_FDI_fwrite(const void *buffer_ptr,size_t element_size,size_t count,FILE_ID stream);
/***********************************************************************
 *
 * Name:		ql_FDI_fseek
 *
 * Description: sets the file position indicator of the file specified  by stream.
 *				The new position, measured in bytes from the beginning of the file
 *				is obtained by adding offset to the position specified by wherefrom.
 *
 * Parameters:
 *    FILE_ID                 stream      [IN]    stream index for the file
 *	long				offset 	[IN] 		the offset to obtain to the position indicator (in bytes)
 *	int				wherefrom    		[IN]		the position to add offset(SEEK_SET, SEEK_CUR, SEEK_END)

 *
 * Returns:
  *		0:  pass
 * 	 	EOF: fail
 * Notes:
 *
 ***********************************************************************/

int ql_FDI_fseek(FILE_ID stream, long offset, int wherefrom);

/***********************************************************************
 *
 * Name:		ql_FDI_GetFileSize
 *
 * Description:  get file size
 *
 * Parameters:
 *  int                 fd      [IN]    file handler
 *
 * Returns:
 * 	 	file length
 * Notes:
 *
 ***********************************************************************/
unsigned int ql_FDI_GetFileSize(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: Open file in the nand flash
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *    <mode>: 	[IN] Open mode .
 *		
 * Return:
 *    File stream.
-----------------------------------------------------------------------------------------*/
FILE_ID ql_nand_fopen(const char *filename_ptr, const char *mode);

/*-----------------------------------------------------------------------------------------
 * Function: Open file in the nand flash
 *
 * Parameter:
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *   LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fclose(FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Read file in the nand flash
 *
 * Parameter:
 *    <buff>: [OUT] File read buff.
 *    <size>: 	[IN] File read size.
 *    <count>:	[IN] File read format size.
 *    <stream>: 	[IN] File stream.
 *		
 * Return:
 *    Real read size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fread(void *buff, size_t element_size, size_t count, FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Write file in the nand flash
 *
 * Parameter:
 *    <buff>: [IN] File write buff.
 *    <size>: 	[IN] File write size.
 *    <count>:	[IN] File write format size.
 *    <stream>: 	[IN] File stream.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fwrite(const void *buff, size_t element_size, size_t count, FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Relocate file pointer in the nand flash
 *
 * Parameter:
 *    <stream>:         [IN] File stream.
 *    <offset>: 	[IN] off set.
 *    <wherefrom>:     [IN] Starting position.
 *		
 * Return:
 *     LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fseek(FILE_ID stream, long offset, int wherefrom);

/*-----------------------------------------------------------------------------------------
 * Function: Remove files in the nand flash
 *
 * Parameter:
 *    <filename_ptr>: 	[IN] File name.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fremove(const char *filename_ptr);

//20201013 august end


//20201221 august add to SPI nand flash
/*-----------------------------------------------------------------------------------------
 * Function: Rename the nand file
 *
 * Parameter:
 *    <oldpath>: 	[IN] File oldpath.
 *    <newpath>: 	[IN] File newpath.
 *		
 * Return:
 *    0: no error.
 *   -1:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_frename(char *name, char *new_name);

/*-----------------------------------------------------------------------------------------
  * Function: Get nand file size
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *		
 * Return:
 *    File size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fsize(char *file_name);

/*-----------------------------------------------------------------------------------------
 * Function: Create directory
 *
 * Parameter:
 *    <path>: 	[IN] Directory path.
 *		
 * Return:
 *    0: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int	ql_nand_mkdir(char *dir_name);

/*-----------------------------------------------------------------------------------------
 * Function: Judge whether the file exists
 *
 * Parameter:
 *    <path>: 	[IN] File name.
 *		
 * Return:
 *    1: exists.
 *    0: no exists
-----------------------------------------------------------------------------------------*/
int ql_nand_access(char *file_name);
//20201221 august end

/*-----------------------------------------------------------------------------------------
 * Function: Get the remaining space of the file system
 *
 * Parameter:
 *		
 * Return:
 *   	Free space size(For reference only)
-----------------------------------------------------------------------------------------*/
int ql_nand_get_free_size(void);

/*-----------------------------------------------------------------------------------------
 * Function: Format the file system
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
void ql_nand_format(void);

/*-----------------------------------------------------------------------------------------
 * Function: qfs_init 挂载文件系统到指定分区
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int qfs_init(char disk_sym, char *patition_name, unsigned char is_format);
/*-----------------------------------------------------------------------------------------
 * @brief   此函数用于将某个分区（不局限于fs分区）挂载/格式化为文件系统，并绑定盘符
 * @param[in] disk_sym  		输入参数1的描述
 * @param[in] patition_name  	需要格式化/挂载的分区
 * @param[in] is_format 		是否格式化
 * @param[in] spi_port 			spi端口
 * @param[in] startaddr 		分区的相对起始地址
 * @param[in] size 				需要格式化的大小，最小应为0X8000（32K）
 * @return     	挂载成功/失败 = 0/1 
 * 				如果某个分区已被初始化，则返回失败
 * 				如果某个盘符已被使用，则返回失败
 * @warning		分区名称应存在，起始地址、大小应落在配置文件配置的分区区域(可用aboot查看)
-----------------------------------------------------------------------------------------*/
int qextfs_init(char disk_sym, char *patition_name, unsigned char is_format, unsigned char spi_port, unsigned int startaddr, unsigned int size);

/*-----------------------------------------------------------------------------------------
 * Function: ql_load_per_size_set  配置lfs 单次读写数据的size，用于小数据读写时，更快的读写完成
 *
 * Parameter: read_size : 读lfs数据，单次读取的字节数 ，必须4字节对齐
 *            prog_size : 写lfs数据，单次写入的字节数 ，必须4字节对齐
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_set_flash_atomic_size(unsigned int read_size,unsigned int prog_size);
/*-----------------------------------------------------------------------------------------
 * Function: Open file in the nand flash
 *
 * Parameter:
 *    <fs_id>: 	[IN] Disk character.
 *    <mode>: 	[IN] Open mode .
 *		
 * Return:
 *    File stream.
-----------------------------------------------------------------------------------------*/
FILE_ID ql_nand_fopen(const char *filename_ptr, const char *mode);

/*-----------------------------------------------------------------------------------------
 * Function: Open file in the nand flash
 *
 * Parameter:
 *    <fp>: 	[IN] File stream.
 *		
 * Return:
 *   LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fclose(FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Read file in the nand flash
 *
 * Parameter:
 *    <buff>: [OUT] File read buff.
 *    <size>: 	[IN] File read size.
 *    <count>:	[IN] File read format size.
 *    <stream>: 	[IN] File stream.
 *		
 * Return:
 *    Real read size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fread(void *buff, size_t element_size, size_t count, FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Write file in the nand flash
 *
 * Parameter:
 *    <buff>: [IN] File write buff.
 *    <size>: 	[IN] File write size.
 *    <count>:	[IN] File write format size.
 *    <stream>: 	[IN] File stream.
 *		
 * Return:
 *    Real write size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fwrite(const void *buff, size_t element_size, size_t count, FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Relocate file pointer in the nand flash
 *
 * Parameter:
 *    <stream>:         [IN] File stream.
 *    <offset>: 	[IN] off set.
 *    <wherefrom>:     [IN] Starting position.
 *		
 * Return:
 *     LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fseek(FILE_ID stream, long offset, int wherefrom);

/*-----------------------------------------------------------------------------------------
 * Function: Relocate file pointer in the nand flash
 *
 * Parameter:
 *    <stream>:         [IN] File stream.
 *    <offset>: 	[IN] off set.
 *    <wherefrom>:     [IN] Starting position.
 *		
 * Return:
 *    the current location of the file pointer,
 *	  which returns -1 on an error call 
 *   
-----------------------------------------------------------------------------------------*/

int ql_nand_lseek(FILE_ID stream, long offset, int wherefrom);

/*-----------------------------------------------------------------------------------------
 * Function: Gets the number of bytes offset from the current position of
 * the file pointer to the beginning of the file
 * Parameter:
 *    <stream>:         [IN] File stream.
 *		
 * Return:
 *     offset bytes, which returns -1 on an error call
 *    
-----------------------------------------------------------------------------------------*/

int ql_nand_ftell(FILE_ID stream);

/*-----------------------------------------------------------------------------------------
 * Function: Remove files in the nand flash
 *
 * Parameter:
 *    <filename_ptr>: 	[IN] File name.
 *		
 * Return:
 *    LFS_ERR_OK: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_fremove(const char *filename_ptr);

//20201013 august end


//20201221 august add to SPI nand flash
/*-----------------------------------------------------------------------------------------
 * Function: Rename the nand file
 *
 * Parameter:
 *    <oldpath>: 	[IN] File oldpath.
 *    <newpath>: 	[IN] File newpath.
 *		
 * Return:
 *    0: no error.
 *   -1:  error
-----------------------------------------------------------------------------------------*/
int ql_nand_frename(char *name, char *new_name);

/*-----------------------------------------------------------------------------------------
  * Function: Get nand file size
 *
 * Parameter:
 *    <fp>:         [IN] File stream.
 *		
 * Return:
 *    File size.
-----------------------------------------------------------------------------------------*/
int ql_nand_fsize(char *file_name);

/*-----------------------------------------------------------------------------------------
 * Function: Create directory
 *
 * Parameter:
 *    <path>: 	[IN] Directory path.
 *		
 * Return:
 *    0: no error.
 *    other:  error
-----------------------------------------------------------------------------------------*/
int	ql_nand_mkdir(char *dir_name);

/*-----------------------------------------------------------------------------------------
 * Function: Judge whether the file exists
 *
 * Parameter:
 *    <path>: 	[IN] File name.
 *		
 * Return:
 *    1: exists.
 *    0: no exists
-----------------------------------------------------------------------------------------*/
int ql_nand_access(char *file_name);
//20201221 august end

/*-----------------------------------------------------------------------------------------
 * Function: Get the remaining space of the file system
 *
 * Parameter:
 *		
 * Return:
 *   	Free space size(For reference only)
-----------------------------------------------------------------------------------------*/
int ql_nand_get_free_size(void);

/*-----------------------------------------------------------------------------------------
 * Function: Format the file system
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
void ql_nand_format(void);
/*-----------------------------------------------------------------------------------------
 * Function: Format the file system 初始化nand spi
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_spi_nand_init(int spi_port, int spi_pin,int spi_clk);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_removedir 初始化nand spi
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int	ql_nand_removedir(char *dir_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_closedir 初始化nand spi
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int	ql_nand_closedir(FILE_ID hDirID);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_readddir 初始化nand spi
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int	ql_nand_readddir(FILE_ID hDirID, NandFs_info *fileinfo_ptr);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_opendir 初始化nand spi
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
FILE_ID	ql_nand_opendir(char *dir_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_lstat 获取路径是文件还是文件夹
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_nand_lstat(char *path,NandFs_info *fileinfo_ptr);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_fgets 获取文件一行字符
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
char *ql_nand_fgets(char *str, int n, FILE_ID stream);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_fsync 修改都被写入到永久存储设备
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_nand_fsync(FILE_ID stream);
 /*-----------------------------------------------------------------------------------------
 * Function: ql_nand_yaffs_sync 保存yaffs checkpoint接口
 *
 * Parameter:
 *		
 * Return:
 *   	 
  -----------------------------------------------------------------------------------------*/
int ql_nand_yaffs_sync(void);
 /*-----------------------------------------------------------------------------------------
  * Function: ql_nand_fgetc 获取一个字符
  *
  * Parameter:
  * 	 
  * Return:
  * 	 
 -----------------------------------------------------------------------------------------*/
int ql_nand_fgetc(FILE_ID stream);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_feof 文件是否已经到达了文件结束
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_nand_feof(FILE_ID stream,char *file_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_nand_frewind 置重置到文件的起始位置
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int ql_nand_frewind(FILE_ID stream);
/*-----------------------------------------------------------------------------------------
 * Function: qfs_init 挂载文件系统到指定分区
 *
 * Parameter:
 * 	 
 * Return:
 * 	 
-----------------------------------------------------------------------------------------*/

int qfs_init(char disk_sym, char *patition_name, unsigned char is_format);
/*-----------------------------------------------------------------------------------------
 * Function: qextfs_init 挂载文件系统到外部flash
 *
 * Parameter:
 *		
 * Return:
 *   	
-----------------------------------------------------------------------------------------*/
int qextfs_init(char disk_sym, char *patition_name, unsigned char is_format, unsigned char spi_port, unsigned int startaddr, unsigned int size);

/*-----------------------------------------------------------------------------------------
 * Function: ql_fs_get_bk_erase_count 获取文件系统每个block的擦写次数,
 *			 发送AT+QERASECOUNT = 1，重启后，可使用该api查询block擦写次数
 * Parameter:
 *      flash_addr[in]: 非0x80000000 开始的文件系统flash地址，例如：0x00654000
 *		
 * Return:
 *   	flash_addr所在block的擦写次数
 *      -1:返回失败，检查flash_addr是否在customer_fs范围内
-----------------------------------------------------------------------------------------*/
int ql_fs_get_bk_erase_count(int flash_addr);



/****************************************yaffs原生接口,支持分区************************************************/
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_start_up YAFFS文件系统的初始化
 *
 * Parameter:
 *		
 * Return:
 *      返回负数:表示初始化失败
 *   	
-----------------------------------------------------------------------------------------*/
int ql_yaffs_start_up(void);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_create_dev YAFFS文件系统不同分区的创建
 *
 * Parameter:
 *		dev_config:分区配置参数
 *                 
 * Return:
 *      返回负数:表示创建失败
 *   	
-----------------------------------------------------------------------------------------*/
int ql_yaffs_create_dev(struct ql_yaffs_dev *dev_config);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_mount YAFFS文件系统挂载，支持不同分区
 *
 * Parameter:
 *		path:分区路径
 *                 
 * Return:
 *      0:表示挂载成功
 *      返回负数:表示挂载失败
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_mount(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_unmount YAFFS文件系统的卸载，支持不同分区
 *
 * Parameter:
 *		path:分区路径
 *                 
 * Return:
 *      0:表示卸载成功
 *      返回负数:表示卸载失败
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_unmount(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_open 文件的打开或者创建
 *
 * Parameter:
 *		path:文件路径
 *		                
 * Return:
 *      如果成功，返回一个非负的文件描述符
 *      如果失败，返回一个负数
 *     
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_open(const char *filename, const char *mode);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_close 已打开文件的关闭
 *
 * Parameter:
 *		fd:文件描述符    
 *      
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_close(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_read 已打开文件的读取
 *
 * Parameter:
 *      buff:指向用于存储读取数据的缓冲区的指针
 *		element_size:每个元素的大小，以字节为单位
 *      count:要读取的元素个数
 *      fd:文件描述符，指示要从中读取数据的文件
 *      
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_read(void *buff, size_t element_size, size_t count, int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_write 已打开文件的写入
 *
 * Parameter:
 *      buff:指向用于存储写入数据的缓冲区的指针
 *		element_size:每个元素的大小，以字节为单位
 *      count:要写入的元素个数
 *      fd:文件描述符，指示要写入数据的文件
 *		                
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_write(const void *buff, size_t element_size, size_t count, int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_lseek 改变文件的当前读/写位置（偏移量）
 *
 * Parameter:
 *      fd:文件描述符，指示要操作的文件
 *      offset:偏移量，指定相对于 whence 参数的偏移量
 *      whence:指定偏移量的起始位置
 *		                
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_lseek(int fd, long offset, int whence);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_unlink 文件或目录的删除
 *
 * Parameter:
 *	    path:删除文件的路径	 
 *  
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_unlink(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_fsize 文件大小的获取
 *
 * Parameter:
 *	    path:要获取大小的文件路径	 
 *  
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_fsize(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_rename 文件的重命名
 *
 * Parameter:
 *	    name:旧的文件路径或目录路径
 *      new_name:新的文件路径或目录路径
 *
 * Return:
 *      返回负数:表示错误 
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_rename(const char *name, const char *new_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_mkdir 目录的创建
 *
 * Parameter:
 *      dir_name:要创建新目录的路径
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_mkdir(const char *dir_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_opendir 目录的打开
 *
 * Parameter:
 *      dir_name:要打开目录的路径     
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_opendir(const char *dir_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_closedir 目录的关闭
 *
 * Parameter:
 *      dir_name:要关闭目录的路径 
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int	ql_yaffs_closedir(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_rmdir 目录的删除
 *
 * Parameter:
 *      dir_name:要删除目录的路径 
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_rmdir(const char *dir_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_access 检查指定路径上的文件或目录是否可以访问
 *
 * Parameter:
 *      path:要检查的文件或目录的路径名
 *		                
 * Return:
 *      返回负数:表示错误 
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_access(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_freespace YAFFS文件系统剩余空间的查询
 *
 * Parameter:
 *		path:指定要查询空闲空间的文件系统路径，通常是文件系统的挂载点或根目录，支持不同分区
 *
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_freespace(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_fsync 将YAFFS文件系统缓冲区的数据写入NANDFlash
 *
 * Parameter:
 *      fb:文件描述符
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_fsync(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_sync YAFFS文件系统checkpoint的保存
 *
 * Parameter:
 *      path:文件系统路径，支持不同分区
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_sync(const char *path);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_format YAFFS文件系统的格式化，支持不同分区
 *
 * Parameter:
 *      path:要格式化的YAFFS 文件系统
 *		unmount_flag:卸载标志 0：不卸载，1：尝试卸载
 *      force_unmount_flag:强制卸载标志 0：不强制卸载，1：强制卸载
 *      remount_flag:重新挂载标志 0：不重新挂载，1：重新挂载
 *
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_format(const char *path, int unmount_flag, int force_unmount_flag, int remount_flag);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_ftell 文件当前偏移位置的获取
 *
 * Parameter:
 *		fd:文件描述符
 *
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_ftell(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_fgetc 从打开的文件中获取一个字符
 *
 * Parameter:
 *      fd:文件描述符       
 *
 * Return:
 *      获取的字符
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_fgetc(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_fgets 从打开的文件中获取一行字符串
 *
 * Parameter:
 *      str:指向读取数据缓存buf指针
 *      n:读取数据的个数
 *      fd:文件描述符
 *		                
 * Return:
 *      返回NULL，表示获取字符串失败
 *
-----------------------------------------------------------------------------------------*/
char *ql_yaffs_fgets(char *str, int n, int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_feof 检查是否已偏移到文件末尾
 *
 * Parameter:
 *      fd:文件描述符
 *      file_name:文件路径
 *		                
 * Return:
 *      1:表示已偏移到文件末尾
 *      0:表示未偏移到文件末尾
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_feof(int fd, const char *file_name);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_frewind 重置文件到起始位置
 *
 * Parameter:
 *      fd:文件描述符
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_frewind(int fd);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_readddir 通过文件描述符获取文件大小和类型
 *
 * Parameter:
 *      fd:文件描述符
 *      fileinfo_ptr:指向存储文件状态数据结构的指针
 *		                
 * Return:
 *      返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int	ql_yaffs_readddir(int fd, NandFs_info *fileinfo_ptr);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffs_lstat 通过文件路径获取文件大小和类型
 *
 * Parameter:
 *      path:文件路径
 *      fileinfo_ptr:指向存储文件状态数据结构的指针
 *
 * Return:
 *     返回负数:表示错误
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffs_lstat(char *path, NandFs_info *fileinfo_ptr);
/*-----------------------------------------------------------------------------------------
 * Function: ql_yaffsfs_getlasterror 通过yaffs的错误码
 *
 * Parameter:
 *      NULL
 *
 * Return:
 *     返回负数:yaffs的错误码
 *
-----------------------------------------------------------------------------------------*/
int ql_yaffsfs_getlasterror(void);


/****************************************yaffs原生接口,支持分区************************************************/
#endif


