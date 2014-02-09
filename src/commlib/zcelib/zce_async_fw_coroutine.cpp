
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


///切换回协程，也就是切换到他自己运行
void ZCE_CRTNAsync_Coroutine::switch_to_coroutine()
{
    ZCE_OS::switch_to_coroutine(&handle_);
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

ZCE_CRTNAsync_Main::ZCE_CRTNAsync_Main():
    corout_id_builder_(1)
{
}

ZCE_CRTNAsync_Main::~ZCE_CRTNAsync_Main()
{

}


//初始化，
int ZCE_CRTNAsync_Main::initialize(size_t crtn_type_num,
    size_t running_number)
{
    //对参数做调整
    if (crtn_type_num < DEFUALT_CRTN_TYPE_NUM)
    {
        crtn_type_num = DEFUALT_CRTN_TYPE_NUM;
    }
    if (running_number < DEFUALT_RUNNIG_CRTN_SIZE)
    {
        running_number = DEFUALT_RUNNIG_CRTN_SIZE;
    }

    coroutine_pool_.rehash(crtn_type_num);
    running_coroutine_.rehash(running_number);
    return 0;
}

//注册一类协程，其用reg_cmd对应，
int ZCE_CRTNAsync_Main::register_cmd(unsigned int reg_cmd,
    ZCE_CRTNAsync_Coroutine* coroutine_base,
    size_t init_clone_num)
{
    //对参数做调整
    if (init_clone_num < DEFUALT_INIT_POOL_SIZE)
    {
        init_clone_num = DEFUALT_INIT_POOL_SIZE;
    }

    ID_TO_REGCOR_POOL_MAP::iterator iter_temp = coroutine_pool_.find(reg_cmd);
    if (iter_temp != coroutine_pool_.end())
    {
        return -1;
    }
    REG_COROUTINE_POOL *pool = new REG_COROUTINE_POOL();
    coroutine_pool_[reg_cmd] = pool;

    pool->push_back(coroutine_base);
    for (size_t i = 0; i < init_clone_num;i++)
    {
        pool->push_back(coroutine_base->clone());
    }

    return 0;
}







