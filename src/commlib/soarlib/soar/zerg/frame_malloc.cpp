#include "soar/predefine.h"
#include "soar/zerg/frame_malloc.h"

//放入CPP?还是?
template <> ZergFrame_Mallocor<ZCE_Thread_Light_Mutex>*
ZergFrame_Mallocor<ZCE_Thread_Light_Mutex>::instance_ = NULL;

template <> ZergFrame_Mallocor<ZCE_Null_Mutex>*
ZergFrame_Mallocor<ZCE_Null_Mutex>::instance_ = NULL;