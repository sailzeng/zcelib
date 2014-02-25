
#include "zce_predefine.h"
#include "zce_async_fw_base.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_log_debug.h"

//------------------------------------------------------------------------------------
ZCE_Async_Object::ZCE_Async_Object(ZCE_Async_ObjectMgr *async_mgr) :
    identity_(INVALID_IDENTITY),
    async_mgr_(async_mgr),
    active_cmd_(0),
    running_state_(STATE_RUNNIG)
{
}

ZCE_Async_Object::~ZCE_Async_Object()
{
}




//------------------------------------------------------------------------------------

ZCE_Async_ObjectMgr::ZCE_Async_ObjectMgr(ZCE_Timer_Queue *timer_queue) :
id_builder_(1),
timer_queue_(timer_queue)
{
}

ZCE_Async_ObjectMgr::~ZCE_Async_ObjectMgr()
{

    //将内存池子里面的数据全部清理掉。好高兴，因为我释放了内存，从Inmoreliu那儿得到了一顿饭。
    ID_TO_REGCOR_POOL_MAP::iterator pooliter = reg_coroutine_.begin();
    ID_TO_REGCOR_POOL_MAP::iterator poolenditer = reg_coroutine_.end();

    for (; pooliter != poolenditer; ++pooliter)
    {
        unsigned int regframe_cmd = pooliter->first;
        ASYNC_OBJECT_RECORD &pool_reg = (pooliter->second);
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
            ZCE_Async_Object *corout_base = NULL;
            pool_reg.coroutine_pool_.pop_front(corout_base);

            delete corout_base;
            corout_base = NULL;
        }
    }
}


//初始化，
int ZCE_Async_ObjectMgr::initialize(size_t crtn_type_num,
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
int ZCE_Async_ObjectMgr::register_coroutine(unsigned int reg_cmd,
    ZCE_Async_Object* coroutine_base,
    size_t init_clone_num)
{
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
    ASYNC_OBJECT_RECORD record;
    reg_coroutine_[reg_cmd] = record;
    ASYNC_OBJECT_RECORD &ref_rec = reg_coroutine_[reg_cmd];

    ref_rec.coroutine_pool_.push_back(coroutine_base);
    for (size_t i = 0; i < init_clone_num; i++)
    {
        ZCE_Async_Object *crtn = coroutine_base->clone(this);
        crtn->active_cmd_ = reg_cmd;
        ref_rec.coroutine_pool_.push_back(crtn);
    }

    return 0;
}





///从池子里面分配一个
int ZCE_Async_ObjectMgr::allocate_from_pool(unsigned int cmd, ZCE_Async_Object *&crt_crtn)
{

    ID_TO_REGCOR_POOL_MAP::iterator mapiter = reg_coroutine_.find(cmd);

    if (mapiter == reg_coroutine_.end())
    {
        return -1;
    }

    ASYNC_OBJECT_RECORD &reg_crtn = reg_coroutine_[cmd];

    //还有最后一个
    if (reg_crtn.coroutine_pool_.size() == 1)
    {
        ZLOG_INFO("[ZCELIB] Before extend pool.");
        //取一个模型
        ZCE_Async_Object *model_trans = NULL;
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
            ZCE_Async_Object *cloned_base = model_trans->clone(this);
            reg_crtn.coroutine_pool_.push_back(cloned_base);
        }

        //将模型放到第N个
        reg_crtn.coroutine_pool_.push_back(model_trans);
        ZLOG_INFO("[ZCELIB] After Extend trans.");
    }

    //取得一个事务
    reg_crtn.coroutine_pool_.pop_front(crt_crtn);
    //初始化丫的
    crt_crtn->initialize();

    return 0;
}

///归还给池子里面
int ZCE_Async_ObjectMgr::free_to_pool(ZCE_Async_Object *free_crtn)
{

    ID_TO_REGCOR_POOL_MAP::iterator mapiter = reg_coroutine_.find(free_crtn->active_cmd_);

    if (mapiter == reg_coroutine_.end())
    {
        return -1;
    }


    //
    ASYNC_OBJECT_RECORD &reg_record = mapiter->second;
    ZLOG_DEBUG("[framework] Return clone frame command %u,Pool size=%u .",
        free_crtn->active_cmd_,
        reg_record.coroutine_pool_.size());


    //用于资源的回收
    free_crtn->finish();

    //
    reg_record.coroutine_pool_.push_back(free_crtn);
    return 0;
}











