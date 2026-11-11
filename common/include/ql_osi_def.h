#ifndef OSI_DEF_H
#define OSI_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************  task priority defination   ******************************/
typedef enum
{
	APP_PRIORITY_IDLE = 1, // reserved
    APP_PRIORITY_LOW = 4,
    APP_PRIORITY_BELOW_NORMAL = 8,
    APP_PRIORITY_NORMAL = 12,
    APP_PRIORITY_ABOVE_NORMAL = 16,
    APP_PRIORITY_HIGH = 25,
    APP_PRIORITY_REALTIME = 30  
}APP_ThreadPriority_e;



/****************************  error code about osi    ***************************/
typedef enum
{
	QL_OSI_SUCCESS             =     0,
	QL_OSI_ERR_INVALID_PARAM   =     1,
    QL_OSI_ERR_INVALID_STATE   =     2,
}osi_errcode_e;

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */