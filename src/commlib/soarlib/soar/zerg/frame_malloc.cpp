#include "soar/predefine.h"
#include "soar/zerg/frame_malloc.h"

//放入CPP?还是?
template <> ZergFrame_Mallocor<zce::thread_light_mutex>*
ZergFrame_Mallocor<zce::thread_light_mutex>::instance_ = NULL;

template <> ZergFrame_Mallocor<zce::null_lock>*
ZergFrame_Mallocor<zce::null_lock>::instance_ = NULL;