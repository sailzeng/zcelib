#include "soar/predefine.h"
#include "soar/zerg/frame_malloc.h"

//放入CPP?还是?
template <> ZergFrame_Mallocor<zce::Thread_Light_Mutex>*
ZergFrame_Mallocor<zce::Thread_Light_Mutex>::instance_ = NULL;

template <> ZergFrame_Mallocor<zce::Null_Mutex>*
ZergFrame_Mallocor<zce::Null_Mutex>::instance_ = NULL;