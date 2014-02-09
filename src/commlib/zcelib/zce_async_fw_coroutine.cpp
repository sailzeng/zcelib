
#include "zce_predefine.h"
#include "zce_async_fw_coroutine.h"



//------------------------------------------------------------------------------------
ZCE_CRTNAsync_Coroutine::ZCE_CRTNAsync_Coroutine()
{
}

ZCE_CRTNAsync_Coroutine::~ZCE_CRTNAsync_Coroutine()
{
}


///取得协程的句柄
coroutine_t *ZCE_CRTNAsync_Coroutine::get_handle()
{
    return &handle_;
}


//切换回Main
void ZCE_CRTNAsync_Coroutine::switch_to_main()
{
    ZCE_OS::switch_to_main(&handle_);
}


///
void ZCE_CRTNAsync_Coroutine::set_state(ZCE_CRTNAsync_Coroutine::STATE_COROUTINE state)
{
    state_ = state;
}


//协程对象的运行函数
void ZCE_CRTNAsync_Coroutine::coroutine_do()
{
    //
    while (state_ != STATE_EXIT)
    {
        //
        coroutine_start();
        coroutine_run();
        coroutine_end();
        
        //
        switch_to_main();
    }
}

///static 函数，用于协程运行函数，调用协程对象的运行函数
void ZCE_CRTNAsync_Coroutine::coroutine_do(ZCE_CRTNAsync_Coroutine *coroutine)
{
    coroutine->coroutine_do();
}


//------------------------------------------------------------------------------------

ZCE_CRTNAsync_Main::ZCE_CRTNAsync_Main()
{
}


ZCE_CRTNAsync_Main::~ZCE_CRTNAsync_Main()
{

}







