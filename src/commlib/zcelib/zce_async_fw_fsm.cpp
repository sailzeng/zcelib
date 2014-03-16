
#include "zce_predefine.h"
#include "zce_async_fw_fsm.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_log_debug.h"

//=====================================================================================



//=====================================================================================

//状态机主控管理类
ZCE_Async_FSMMgr::ZCE_Async_FSMMgr(ZCE_Timer_Queue *timer_queue) :
ZCE_Async_ObjectMgr(timer_queue)
{
    pool_init_size_ = FSM_POOL_INIT_SIZE;
    pool_extend_size_ = FSM_POOL_INIT_SIZE;
}

ZCE_Async_FSMMgr::~ZCE_Async_FSMMgr()
{
}

 



