#include "soar/predefine.h"
#include "soar/zerg/frame_malloc.h"

//放入CPP?还是?
template <> zergframe_mallocor<zce::thread_light_mutex>*
zergframe_mallocor<zce::thread_light_mutex>::instance_ = NULL;

template <> zergframe_mallocor<zce::null_lock>*
zergframe_mallocor<zce::null_lock>::instance_ = NULL;