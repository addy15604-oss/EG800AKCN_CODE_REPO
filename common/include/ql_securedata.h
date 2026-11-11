/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#ifndef __QL_SECUREDATA_H__
#define __QL_SECUREDATA_H__



#include <stdint.h>

extern int32 ql_securedata_store(uint8_t index, uint8_t *pdata, uint32 len);
extern int32 ql_securedata_read(uint8_t index, uint8_t *pBuffer, uint32 len);


#endif
