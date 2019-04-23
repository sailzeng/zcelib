
#include "zce_predefine.h"
#include "zce_async_fw_base.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_debugging.h"
#include "zce_timer_queue_base.h"

//=============================================================================================

//TIME ID
const int ZCE_Async_Object::ASYNCOBJ_ACTION_ID[] = { 10001, 20001 };

//构造函数
ZCE_Async_Object::ZCE_Async_Object(ZCE_Async_ObjectMgr *async_mgr,
                                   unsigned int create_cmd) :
    asyncobj_id_(ZCE_Async_ObjectMgr::INVALID_IDENTITY),
    async_mgr_(async_mgr),
    create_cmd_(create_cmd),
    timeout_id_(ZCE_Timer_Queue_Base::INVALID_TIMER_ID),
    process_errno_(0),
	trace_log_pri_(RS_INFO)
{
}

//析构函数
ZCE_Async_Object::~ZCE_Async_Object()
{
}

//初始化函数，在构造函数后调用，
int ZCE_Async_Object::initialize()
{
    return 0;
}

//结束销毁函数，在析构前的调用
void ZCE_Async_Object::finish()
{
    return;
}


//设置超时定时器
int ZCE_Async_Object::set_timeout(const ZCE_Time_Value &time_out)
{
    ZCE_Timer_Queue_Base *timer_queue = async_mgr_->timer_queue();
    ZCE_Time_Value delay_time(time_out);
    //注意使用的TIME ID
    timeout_id_ = timer_queue->schedule_timer(async_mgr_,
                                              this,
                                              delay_time);

    if (ZCE_Timer_Queue_Base::INVALID_TIMER_ID == timeout_id_)
    {
        return -1;
    }
    return 0;
}


//取消超时的定时器
void ZCE_Async_Object::cancel_timeout()
{
    if (ZCE_Timer_Queue_Base::INVALID_TIMER_ID != timeout_id_)
    {
        ZCE_Timer_Queue_Base *timer_queue = async_mgr_->timer_queue();
        timer_queue->cancel_timer(timeout_id_);
        timeout_id_ = ZCE_Timer_Queue_Base::INVALID_TIMER_ID;
    }
}



//异步对象开始
void ZCE_Async_Object::on_start()
{
}

//目前基类做的结束操作就是清理定时器
void ZCE_Async_Object::on_end()
{
    cancel_timeout();
}


//记录处理过程发生的错误
void ZCE_Async_Object::set_errorno(int error_no)
{
    process_errno_ = error_no;
}


//=============================================================================================
//异步对象管理器

//内部结构
ZCE_Async_ObjectMgr::ASYNC_OBJECT_RECORD::ASYNC_OBJECT_RECORD():
    create_num_(0),
    active_num_(0),
    end_num_(0),
    force_end_num_(0),
    timeout_num_(0),
    run_consume_ms_(0)
{
}

ZCE_Async_ObjectMgr::ASYNC_OBJECT_RECORD::~ASYNC_OBJECT_RECORD()
{
}


ZCE_Async_ObjectMgr::ZCE_Async_ObjectMgr() :
    ZCE_Timer_Handler(),
    id_builder_(1),
    pool_init_size_(0),
    pool_extend_size_(0)
{
}

ZCE_Async_ObjectMgr::~ZCE_Async_ObjectMgr()
{
}


//初始化，
int ZCE_Async_ObjectMgr::initialize(ZCE_Timer_Queue_Base *tq,
                                    size_t crtn_type_num,
                                    size_t running_number)
{
    timer_queue(tq);
    //对参数做调整
    if (crtn_type_num < DEFUALT_ASYNC_TYPE_NUM)
    {
        crtn_type_num = DEFUALT_ASYNC_TYPE_NUM;
    }
    if (running_number < DEFUALT_RUNNIG_ASYNC_SIZE)
    {
        running_number = DEFUALT_RUNNIG_ASYNC_SIZE;
    }

    regaysnc_pool_.rehash(crtn_type_num);
    running_aysncobj_.rehash(running_number);
    return 0;
}


//
void ZCE_Async_ObjectMgr::finish()
{
    RUNNING_ASYNOBJ_MAP::iterator run_iter = running_aysncobj_.begin();
    RUNNING_ASYNOBJ_MAP::iterator run_end = running_aysncobj_.end();
    for (; run_iter != run_end; ++run_iter)
    {
        ZCE_Async_Object *async_obj = run_iter->second;

        //统计强制退出的数量
        ID_TO_REGASYNC_POOL_MAP::iterator iter_temp = regaysnc_pool_.find(async_obj->create_cmd_);
        if (iter_temp == regaysnc_pool_.end())
        {
            ZCE_ASSERT(false);
            continue;
        }
        ASYNC_OBJECT_RECORD &reg_async = iter_temp->second;
        ++reg_async.force_end_num_;

        async_obj->on_end();
        free_to_pool(async_obj);
    }

    //将内存池子里面的数据全部清理掉。好高兴，因为我释放了内存，从Inmoreliu那儿得到了一顿饭。
    ID_TO_REGASYNC_POOL_MAP::iterator pooliter = regaysnc_pool_.begin();
    ID_TO_REGASYNC_POOL_MAP::iterator poolenditer = regaysnc_pool_.end();

    for (; pooliter != poolenditer; ++pooliter)
    {
        unsigned int regframe_cmd = pooliter->first;
        ASYNC_OBJECT_RECORD &pool_reg = (pooliter->second);
        //记录信息数据
        ZCE_LOG(RS_INFO, "[ZCELIB] Register command[%u] size of pool[%u] capacity of pool[%u].",
                regframe_cmd,
                pool_reg.aysncobj_pool_.size(),
                pool_reg.aysncobj_pool_.capacity()
               );

        //出现了问题，
        if (pool_reg.aysncobj_pool_.size() != pool_reg.aysncobj_pool_.capacity())
        {
            ZCE_LOG(RS_ERROR, "[ZCELIB] Plase notice!! size[%u] != capacity[%u] may be exist memory leak.",
                    pool_reg.aysncobj_pool_.size(),
                    pool_reg.aysncobj_pool_.capacity());
        }

        //是否池子
        size_t pool_reg_trans_len = pool_reg.aysncobj_pool_.size();
        for (size_t i = 0; i < pool_reg_trans_len; ++i)
        {
            ZCE_Async_Object *corout_base = NULL;
            pool_reg.aysncobj_pool_.pop_front(corout_base);
            corout_base->finish();
            delete corout_base;
            corout_base = NULL;

        }
    }
    return;
}

//注册一类协程，其用reg_cmd对应，
int ZCE_Async_ObjectMgr::register_asyncobj(unsigned int create_cmd,
                                           ZCE_Async_Object *coroutine_base)
{

    //这两个值必须是重新设置过的
    ZCE_ASSERT(pool_init_size_ > 0 && pool_extend_size_ > 0);

    //
    if (ZCE_Async_ObjectMgr::INVALID_COMMAND == create_cmd)
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Register command[%u] error.",
                create_cmd);
        return -1;
    }

    //一个命令字CMD只能注册一次
    ID_TO_REGASYNC_POOL_MAP::iterator iter_temp = regaysnc_pool_.find(create_cmd);
    if (iter_temp != regaysnc_pool_.end())
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Register command[%u] repeated error.",
                create_cmd);
        return -1;
    }
    ASYNC_OBJECT_RECORD record;
    regaysnc_pool_[create_cmd] = record;
    ASYNC_OBJECT_RECORD &ref_rec = regaysnc_pool_[create_cmd];

    coroutine_base->initialize();
    ref_rec.aysncobj_pool_.initialize(pool_init_size_ + 1);
    ref_rec.aysncobj_pool_.push_back(coroutine_base);
    for (size_t i = 0; i < pool_init_size_; i++)
    {
        ZCE_Async_Object *crtn = coroutine_base->clone(this, create_cmd);
        crtn->initialize();
        ref_rec.aysncobj_pool_.push_back(crtn);
    }
    return 0;
}

//判断某个命令是否是注册（创建）异步对象命令
bool ZCE_Async_ObjectMgr::is_register_cmd(unsigned int cmd)
{
    ID_TO_REGASYNC_POOL_MAP::iterator mapiter = regaysnc_pool_.find(cmd);
    if (mapiter == regaysnc_pool_.end())
    {
        return false;
    }
    return true;
}

//从池子里面分配一个
int ZCE_Async_ObjectMgr::allocate_from_pool(unsigned int create_cmd,
                                            ASYNC_OBJECT_RECORD *&async_rec,
                                            ZCE_Async_Object *&crt_async)
{

    ID_TO_REGASYNC_POOL_MAP::iterator mapiter = regaysnc_pool_.find(create_cmd);
    if (mapiter == regaysnc_pool_.end())
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Allocate async object command[%u] error.", create_cmd);
        return -1;
    }

    ASYNC_OBJECT_RECORD &reg_async = mapiter->second;

    //还有最后一个,扩展，
    if (reg_async.aysncobj_pool_.size() == 1)
    {
        ZCE_LOG(RS_INFO, "[ZCELIB] Before extend pool.");
        //取一个模型
        ZCE_Async_Object *model_trans = NULL;
        reg_async.aysncobj_pool_.pop_front(model_trans);

        size_t capacity_of_pool = reg_async.aysncobj_pool_.capacity();
        reg_async.aysncobj_pool_.resize(capacity_of_pool + pool_extend_size_);

        ZCE_LOG(RS_INFO, "[ZCELIB] Async object pool size[%u],  command[%u], capacity[%u] , resize[%u] .",
                reg_async.aysncobj_pool_.size(),
                create_cmd,
                capacity_of_pool,
                capacity_of_pool + pool_extend_size_);

        //用模型克隆N-1个Trans
        for (size_t i = 0; i < pool_extend_size_; ++i)
        {
            ZCE_Async_Object *cloned_base = model_trans->clone(this, create_cmd);
            reg_async.aysncobj_pool_.push_back(cloned_base);
        }

        //将模型放到第N个
        reg_async.aysncobj_pool_.push_back(model_trans);
        ZCE_LOG(RS_INFO, "[ZCELIB] After Extend Async object .");
    }

    //取得一个事务
    reg_async.aysncobj_pool_.pop_front(crt_async);
    async_rec = &reg_async;

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Allocate async object command[%u],after alloc pool size[%u] .",
            create_cmd,
            reg_async.aysncobj_pool_.size());

    return 0;
}

///归还给池子里面，释放一个异步对象到池子里面
int ZCE_Async_ObjectMgr::free_to_pool(ZCE_Async_Object *free_crtn)
{

    ID_TO_REGASYNC_POOL_MAP::iterator mapiter = regaysnc_pool_.find(free_crtn->create_cmd_);

    if (mapiter == regaysnc_pool_.end())
    {
        return -1;
    }

    //
    ASYNC_OBJECT_RECORD &reg_record = mapiter->second;
    reg_record.aysncobj_pool_.push_back(free_crtn);

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Return clone frame command [%u],Pool size [%u].",
            free_crtn->create_cmd_,
            reg_record.aysncobj_pool_.size());

    return 0;
}

//创建异步对象
int ZCE_Async_ObjectMgr::create_asyncobj(unsigned int cmd, void *outer_data, unsigned int *id)
{
    int ret = 0;
    ZCE_Async_Object *crt_async = NULL;
    ASYNC_OBJECT_RECORD *async_rec = NULL;

    //从池子里面找一个异步对象
    ret = allocate_from_pool(cmd, async_rec, crt_async);
    if (ret != 0)
    {
        return ret;
    }
    ++id_builder_;
    if (id_builder_ == INVALID_IDENTITY)
    {
        ++id_builder_;
    }
    *id = id_builder_;
    crt_async->asyncobj_id_ = id_builder_;

    
    ++async_rec->create_num_;
    crt_async->on_start();

    //启动丫的
    bool continue_run = false;
    crt_async->on_run(outer_data, continue_run);

    //如果运行一下就退出了,直接结束回收
    if (continue_run == false)
    {
        ++async_rec->end_num_;
        crt_async->on_end();
        free_to_pool(crt_async);
    }
    else
    {
        //第一次用auto，感觉豪爽！！！
        auto iter = running_aysncobj_.insert(std::make_pair(id_builder_, crt_async));
        //如果没有插入成功,理论上没有可能出现这种情况，不可能出现这么多异步对象
        if (iter.second == false)
        {
            ZCE_ASSERT_ALL(false);
            return -1;
        }
    }

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Async object create. command [%u] create, id [%u],and continue run [%s].",
            cmd,
            id_builder_,
            continue_run ? "TRUE" : "FALSE");

    return 0;
}


//通过ID，寻找一个正在运行的异步对象
int ZCE_Async_ObjectMgr::find_running_asyncobj(unsigned int id,
                                               ZCE_Async_Object *&running_aysnc)
{
    running_aysnc = NULL;
    auto iter = running_aysncobj_.find(id);
    if (running_aysncobj_.end() == iter)
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Can't find async object id [%u] .",
                id);
        return -1;
    }
    running_aysnc = iter->second;
    return 0;
}

//激活某个已经运行的异步对象
int ZCE_Async_ObjectMgr::active_asyncobj(unsigned int id, void *outer_data)
{
    int ret = 0;
    ZCE_Async_Object *async_obj = NULL;
    ret = find_running_asyncobj(id, async_obj);
    if (ret != 0)
    {
        return ret;
    }

    //增加记录统计数据
    ID_TO_REGASYNC_POOL_MAP::iterator mapiter = regaysnc_pool_.find(async_obj->create_cmd_);
    if (mapiter == regaysnc_pool_.end())
    {
        //到这儿应该是框架代码错误了，一个异步对象的创建命令字没有注册？
        ZCE_LOG(RS_ERROR, "[ZCELIB] Can't find async object id [%u] create cmd [%u].",
                id,
                async_obj->create_cmd_);
        return -1;
    }
    ASYNC_OBJECT_RECORD &async_rec = mapiter->second;

    //激活同时取消定时器
    async_obj->cancel_timeout();

    bool continue_run = false;
    async_obj->on_run(outer_data,continue_run);
    ++async_rec.active_num_;

    //如果不继续运行了，
    if (continue_run == false)
    {
        ++async_rec.end_num_;
        async_obj->on_end();
        free_to_pool(async_obj);
    }
    ZCE_LOG(RS_DEBUG, "[ZCELIB] Async object active. command [%u] create, id [%u],and continue run [%s].",
            async_obj->create_cmd_,
            id,
            continue_run ? "TRUE" : "FALSE");

    return 0;
}

//超时处理
int ZCE_Async_ObjectMgr::timer_timeout(const ZCE_Time_Value &now_time,
                                       const void *act)
{
    ZCE_Async_Object *async_obj = (ZCE_Async_Object *)(act);
    //增加记录统计数据
    ID_TO_REGASYNC_POOL_MAP::iterator mapiter = regaysnc_pool_.find(async_obj->create_cmd_);
    if (mapiter == regaysnc_pool_.end())
    {
        return -1;
    }
    ASYNC_OBJECT_RECORD &async_rec = mapiter->second;

    //处理前取消定时器
    async_obj->cancel_timeout();

    bool continue_run = false;
    async_obj->on_timeout(now_time, continue_run);
    ++async_rec.timeout_num_;

    //如果不继续运行了，
    if (continue_run == false)
    {
        ++async_rec.end_num_;
        async_obj->on_end();
        free_to_pool(async_obj);
    }

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Async object timeout. command [%u] create, id [%u],and continue run [%s].",
            async_obj->create_cmd_,
            async_obj->asyncobj_id_,
            continue_run ? "TRUE" : "FALSE");

    return 0;
}


//打印管理器的基本信息，运行状态
void ZCE_Async_ObjectMgr::dump_info(ZCE_LOG_PRIORITY log_priority) const
{
    //
    ZCE_LOG(log_priority, "Register create cmd size[%lu].active running async object size[%lu].",
            regaysnc_pool_.size(),
            running_aysncobj_.size());
    //打印每个创建命令对于异步对象池子的数据
    auto iter =  regaysnc_pool_.begin();
    for (; iter != regaysnc_pool_.end(); ++iter)
    {
        const ASYNC_OBJECT_RECORD &async_rec = iter->second;
        unsigned int create_cmd = iter->first;
        ZCE_LOG(log_priority, "[ZCELIB] Async object create_cmd [%u]."
                "Pool capacity[%lu] size[%lu] create_num[%llu],active_num[%llu]"
                "end_num [%llu] force_end_num[%llu] timeout_num_[%llu].",
                create_cmd,
                async_rec.aysncobj_pool_.capacity(),
                async_rec.aysncobj_pool_.size(),
                async_rec.create_num_,
                async_rec.active_num_,
                async_rec.end_num_,
                async_rec.force_end_num_,
                async_rec.timeout_num_);
    }
}

