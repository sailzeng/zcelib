
#include "zce_predefine.h"
#include "zce_async_fw_coroutine.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_debugging.h"



//========================================================================================

ZCE_Async_Coroutine::ZCE_Async_Coroutine(ZCE_Async_ObjectMgr *async_mgr,unsigned int reg_cmd) :
    ZCE_Async_Object(async_mgr, reg_cmd)
{
    //堆栈大小默认选择最小的，
}

ZCE_Async_Coroutine::~ZCE_Async_Coroutine()
{
}


//初始化协程的对象
int ZCE_Async_Coroutine::initialize()
{
    ZCE_Async_Object::initialize();
    int ret = 0;
    ret = ZCE_LIB::make_coroutine(&handle_,
                                  stack_size_,
                                  true,
                                  (ZCE_COROUTINE_3PARA)ZCE_Async_Coroutine::static_do,
                                  (void *)this,
                                  NULL,
                                  NULL
                                 );
    if (ret != 0)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::make_coroutine return fail.", ret);
        return ret;
    }
    return 0;
}

//清理协程对象
void ZCE_Async_Coroutine::finish()
{
    ZCE_Async_Object::finish();
    ZCE_LIB::delete_coroutine(&handle_);
    return;
}

//调用协程
void ZCE_Async_Coroutine::on_run(void *outer_data, bool &continue_run)
{
    outer_data_ = outer_data;
    continue_run = false;

    yeild_coroutine();

    outer_data_ = NULL;

    //根据调用返回的函数记录的状态值得到当前的状态
    if (coroutine_state_ == COROUTINE_CONTINUE)
    {
        continue_run = true;
    }
    else if (coroutine_state_ == COROUTINE_EXIT)
    {
        continue_run = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}


//调用协程
void ZCE_Async_Coroutine::on_timeout(const ZCE_Time_Value & /*now_time*/ ,
                                     bool &continue_run)
{
    continue_run = false;
    coroutine_state_ = COROUTINE_TIMEOUT;
    yeild_coroutine();

    //根据调用返回的函数记录的状态值得到当前的状态
    if (coroutine_state_ == COROUTINE_CONTINUE)
    {
        continue_run = true;
    }
    else if (coroutine_state_ == COROUTINE_EXIT)
    {
        continue_run = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}

//切换回Main，协程还会继续运行
void ZCE_Async_Coroutine::yeild_main_continue()
{
    coroutine_state_ = COROUTINE_CONTINUE;
    ZCE_LIB::yeild_main(&handle_);
}

//切换回Main,协程退出
void ZCE_Async_Coroutine::yeild_main_exit()
{
    coroutine_state_ = COROUTINE_EXIT;
    ZCE_LIB::yeild_main(&handle_);
}


//切换回协程，也就是切换到他自己运行
void ZCE_Async_Coroutine::yeild_coroutine()
{
    ZCE_LIB::yeild_coroutine(&handle_);
}



//协程对象的运行函数
void ZCE_Async_Coroutine::coroutine_do()
{
    //如果需要协程
    for (;;)
    {
        //
        coroutine_run();

        //运行完毕后，返回主程序
        yeild_main_exit();
    }
}

///static 函数，用于协程运行函数，调用协程对象的运行函数
void ZCE_Async_Coroutine::static_do(ZCE_Async_Coroutine *coroutine)
{
    coroutine->coroutine_do();
}



//等待time_out 时间后超时，设置定时器后，切换协程到main
int ZCE_Async_Coroutine::waitfor_timeout(const ZCE_Time_Value &time_out)
{
    int ret = 0;
    ret = set_timeout(time_out);
    if (0 != ret)
    {
        return ret;
    }
    yeild_main_continue();
    return 0;
}



//=====================================================================================

//携程主控管理类
ZCE_Async_CoroutineMgr::ZCE_Async_CoroutineMgr() :
    ZCE_Async_ObjectMgr()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

ZCE_Async_CoroutineMgr::~ZCE_Async_CoroutineMgr()
{
}






