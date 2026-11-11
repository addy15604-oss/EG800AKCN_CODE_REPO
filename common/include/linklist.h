/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/


/**
 ******************************************************************************
 * @file    linklist.h
 * @author  Chavis.Chen
 * @version V1.0.0
 * @date    26-Sep-2018
 * @brief   This file contains the common definitions, macros and functions to
 *          be shared throughout the project.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 QUECTEL Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */


#ifndef _LINKLIST_H
#define _LINKLIST_H

#include "ql_type.h"

typedef enum
{
    DIR_FIFO,
    DIR_LIFO
} push_dir_t;

#define DIR_DEFAULT DIR_FIFO

typedef uint8 data_t;

typedef uint32 datasize_t;

typedef uint32 node_cnt_t;

typedef int32 linklist_id_t;

#pragma pack(push)
#pragma pack(1)
typedef struct node
{
	struct node * next;
    datasize_t size;
    data_t data[1];
} linklist;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    node_cnt_t node_cnt;
    linklist * header;
    linklist * tail;
    linklist * curt;
} linklist_t;
#pragma pack(pop)

typedef linklist_t * linklist_handler_t;

extern bool 		bLinklistLogEn;

extern QuecOSStatus linklist_init(linklist_handler_t * linklist_handler);
extern QuecOSStatus linklist_push(linklist_handler_t * linklist_handler, push_dir_t dir, data_t * data, datasize_t size);
extern QuecOSStatus linklist_pop(linklist_handler_t * linklist_handler, data_t * * data_pp, datasize_t * data_len);
extern QuecOSStatus linklist_get_node_cnt(linklist_handler_t * linklist_handler, node_cnt_t * nodeCnt);
extern QuecOSStatus linklist_node_find(linklist_handler_t * linklist_handler, uint32 i, linklist * * dest_node);
extern QuecOSStatus linklist_node_delete(linklist_handler_t * linklist_handler, uint32 i);
extern QuecOSStatus linklist_clean(linklist_handler_t * linklist_handler);
extern QuecOSStatus linklist_deinit(linklist_handler_t * linklist_handler);
extern void 		linklist_mem_free(void * mem_ptr);

#endif
