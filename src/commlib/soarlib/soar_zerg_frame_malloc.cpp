


#include "soar_predefine.h"
#include "soar_zerg_frame_malloc.h"

//放入CPP?还是?
template <> AppFrame_Mallocor_Mgr<ZCE_Thread_Light_Mutex> *
AppFrame_Mallocor_Mgr<ZCE_Thread_Light_Mutex>::instance_ = NULL;

template <> AppFrame_Mallocor_Mgr<ZCE_Null_Mutex> *
AppFrame_Mallocor_Mgr<ZCE_Null_Mutex>::instance_ = NULL;

