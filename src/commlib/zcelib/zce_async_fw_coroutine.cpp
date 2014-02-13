
#include "zce_predefine.h"
#include "zce_async_fw_coroutine.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_log_debug.h"



//------------------------------------------------------------------------------------
ZCE_CRTNAsync_Coroutine::ZCE_CRTNAsync_Coroutine():
    command_(0),
    coroutine_id_(0),
    state_()
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
void ZCE_CRTNAsync_Coroutine::yeild_main()
{
    ZCE_OS::yeild_main(&handle_);
}


///切换回协程，也就是切换到他自己运行
void ZCE_CRTNAsync_Coroutine::yeild_coroutine()
{
    ZCE_OS::yeild_coroutine(&handle_);
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
        yeild_main();
    }
}

///static 函数，用于协程运行函数，调用协程对象的运行函数
void ZCE_CRTNAsync_Coroutine::static_do(ZCE_CRTNAsync_Coroutine *coroutine)
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
    
    //将内存池子里面的数据全部清理掉。好高兴，因为我释放了内存，从Inmoreliu那儿得到了一顿饭。
    ID_TO_REGCOR_POOL_MAP::iterator pooliter = reg_coroutine_.begin();
    ID_TO_REGCOR_POOL_MAP::iterator poolenditer = reg_coroutine_.end();

    for (; pooliter != poolenditer; ++pooliter)
    {
        unsigned int regframe_cmd = pooliter->first;
        COROUTINE_RECORD &pool_reg = (pooliter->second);
        //记录信息数据
        ZLOG_INFO("[ZCELIB] Register command:%u size of pool:%u capacity of pool:%u.",
            regframe_cmd,
            pool_reg.coroutine_pool_.size(),
            pool_reg.coroutine_pool_.capacity()
            );

        //出现了问题，
        if (pool_reg.coroutine_pool_.size() != pool_reg.coroutine_pool_.capacity())
        {
            ZLOG_ERROR("[ZCELIB] Plase notice!! size[%u] != capacity[%u] may be exist memory leak.",
                pool_reg.coroutine_pool_.size(),
                pool_reg.coroutine_pool_.capacity());
        }

        //是否池子
        size_t pool_reg_trans_len = pool_reg.coroutine_pool_.size();

        for (size_t i = 0; i < pool_reg_trans_len; ++i)
        {
            ZCE_CRTNAsync_Coroutine *corout_base = NULL;
            pool_reg.coroutine_pool_.pop_front(corout_base);

            delete corout_base;
            corout_base = NULL;
        }
    }
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

    reg_coroutine_.rehash(crtn_type_num);
    running_coroutine_.rehash(running_number);
    return 0;
}

//注册一类协程，其用reg_cmd对应，
int ZCE_CRTNAsync_Main::register_coroutine(unsigned int reg_cmd,
    ZCE_CRTNAsync_Coroutine* coroutine_base,
    size_t init_clone_num,
    size_t stack_size)
{
    int ret = 0;
    //对参数做调整
    if (init_clone_num < DEFUALT_INIT_POOL_SIZE)
    {
        init_clone_num = DEFUALT_INIT_POOL_SIZE;
    }

    ID_TO_REGCOR_POOL_MAP::iterator iter_temp = reg_coroutine_.find(reg_cmd);
    if (iter_temp != reg_coroutine_.end())
    {
        return -1;
    }
    COROUTINE_RECORD record;
    reg_coroutine_[reg_cmd] = record;
    COROUTINE_RECORD &ref_rec = reg_coroutine_[reg_cmd];

    ref_rec.coroutine_pool_.push_back(coroutine_base);
    for (size_t i = 0; i < init_clone_num;i++)
    {
        ZCE_CRTNAsync_Coroutine *crtn = coroutine_base->clone();
        crtn->set_command(reg_cmd);
        ret = ZCE_OS::make_coroutine(crtn->get_handle(),
            stack_size,
            true,
            (ZCE_COROUTINE_3PARA)ZCE_CRTNAsync_Coroutine::static_do,
            (void *)crtn,
            NULL,
            NULL
            );
        if (ret != 0)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_OS::make_coroutine return fail.", ret);
            return ret;
        }
        ref_rec.coroutine_pool_.push_back(crtn);
    }

    return 0;
}





///从池子里面分配一个
int ZCE_CRTNAsync_Main::allocate_from_pool(unsigned int cmd, ZCE_CRTNAsync_Coroutine *&crt_crtn)
{

    ID_TO_REGCOR_POOL_MAP::iterator mapiter = reg_coroutine_.find(cmd);

    if (mapiter == reg_coroutine_.end())
    {
        return -1;
    }

    COROUTINE_RECORD &reg_crtn = reg_coroutine_[cmd];

    //还有最后一个
    if (reg_crtn.coroutine_pool_.size() == 1)
    {
        ZLOG_INFO("[ZCELIB] Before extend pool.");
        //取一个模型
        ZCE_CRTNAsync_Coroutine *model_trans = NULL;
        reg_crtn.coroutine_pool_.pop_front(model_trans);

        size_t capacity_of_pool = reg_crtn.coroutine_pool_.capacity();
        reg_crtn.coroutine_pool_.resize(capacity_of_pool + POOL_EXTEND_COROUTINE_NUM);

        ZLOG_INFO("[ZCELIB] Coroutine pool Size=%u,  command %u, capacity = %u , resize =%u .",
            reg_crtn.coroutine_pool_.size(),
            cmd,
            capacity_of_pool,
            capacity_of_pool + POOL_EXTEND_COROUTINE_NUM);

        //用模型克隆N-1个Trans
        for (size_t i = 0; i < POOL_EXTEND_COROUTINE_NUM; ++i)
        {
            ZCE_CRTNAsync_Coroutine *cloned_base = model_trans->clone();
            reg_crtn.coroutine_pool_.push_back(cloned_base);
        }

        //将模型放到第N个
        reg_crtn.coroutine_pool_.push_back(model_trans);
        ZLOG_INFO("[ZCELIB] After Extend trans.");
    }

    //取得一个事务
    reg_crtn.coroutine_pool_.pop_front(crt_crtn);
    //初始化丫的
    crt_crtn->coroutine_start();

    return 0;
}

///
int ZCE_CRTNAsync_Main::free_to_pool(ZCE_CRTNAsync_Coroutine *free_crtn)
{

    ID_TO_REGCOR_POOL_MAP::iterator mapiter = reg_coroutine_.find(free_crtn->command_);

    if (mapiter == reg_coroutine_.end())
    {
        return -1;
    }


    //
    COROUTINE_RECORD &reg_record = mapiter->second;
    ZLOG_DEBUG("[framework] Return clone frame command %u,Pool size=%u .",
        free_crtn->command_,
        reg_record.coroutine_pool_.size());


    //用于资源的回收
    free_crtn->coroutine_end();

    //
    reg_record.coroutine_pool_.push_back(free_crtn);
    return 0;
}




///激活一个协程
int ZCE_CRTNAsync_Main::active_coroutine(unsigned int cmd, unsigned int *id)
{

    return 0;
}

///切换到ID对应的那个线程
int ZCE_CRTNAsync_Main::yeild_coroutine(unsigned int id)
{

    return 0;
}











