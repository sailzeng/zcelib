#include "zce/predefine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/async/async_base.h"

//=============================================================================================

namespace zce
{
//TIME ID
const int async_object::ASYNCOBJ_ACTION_ID[] = { 10001,20001 };

//构造函数
async_object::async_object(async_obj_mgr* async_mgr,
                           uint32_t create_cmd) :
    async_mgr_(async_mgr),
    create_cmd_(create_cmd)
{
}

//析构函数
async_object::~async_object()
{
}

//初始化函数，在构造函数后调用，
int async_object::initialize()
{
    return 0;
}

//结束销毁函数，在析构前的调用
void async_object::terminate()
{
    return;
}

//设置超时定时器
int async_object::set_timeout(const zce::time_value& time_out)
{
    timer_queue* timer_queue = async_mgr_->timer_queue();
    zce::time_value delay_time(time_out);
    //注意使用的TIME ID
    int ret = timer_queue->schedule_timer(async_mgr_,
                                          timeout_id_,
                                          delay_time);

    if (0 != ret)
    {
        return -1;
    }
    return 0;
}

//取消超时的定时器
void async_object::cancel_timeout()
{
    if (timer_queue::INVALID_TIMER_ID != timeout_id_)
    {
        zce::timer_queue* timer_queue = async_mgr_->timer_queue();
        timer_queue->cancel_timer(timeout_id_);
        timeout_id_ = zce::timer_queue::INVALID_TIMER_ID;
    }
}

//目前基类做的结束操作就是清理定时器
void async_object::on_end()
{
    cancel_timeout();
}

//记录处理过程发生的错误
void async_object::set_errorno(int error_no)
{
    running_errno_ = error_no;
}

//=============================================================================================
//异步对象管理器

//内部结构

async_obj_mgr::async_obj_mgr() :
    timer_handler()
{
}

async_obj_mgr::~async_obj_mgr()
{
}

//初始化，
int async_obj_mgr::initialize(zce::timer_queue* tq,
                              size_t crtn_type_num,
                              size_t running_number)
{
    timer_queue(tq);

    regaysnc_pool_.rehash(crtn_type_num);
    running_aysncobj_.rehash(running_number);

    return 0;
}

//
void async_obj_mgr::terminate()
{
    RUNNING_ASYNOBJ_MAP::iterator run_iter = running_aysncobj_.begin();
    RUNNING_ASYNOBJ_MAP::iterator run_end = running_aysncobj_.end();
    for (; run_iter != run_end; ++run_iter)
    {
        async_object* async_obj = run_iter->second;

        //统计强制退出的数量
        ASYNC_RECORD_POOL::iterator iter_temp = regaysnc_pool_.find(async_obj->create_cmd_);
        if (iter_temp == regaysnc_pool_.end())
        {
            ZCE_ASSERT(false);
            continue;
        }
        ASYNC_OBJECT_RECORD& reg_async = iter_temp->second;
        ++reg_async.force_end_num_;

        async_obj->on_end();
        free_to_pool(async_obj);
    }

    //将内存池子里面的数据全部清理掉。好高兴，因为我释放了内存，从Inmoreliu那儿得到了一顿饭。
    ASYNC_RECORD_POOL::iterator pooliter = regaysnc_pool_.begin();
    ASYNC_RECORD_POOL::iterator poolenditer = regaysnc_pool_.end();

    for (; pooliter != poolenditer; ++pooliter)
    {
        unsigned int regframe_cmd = pooliter->first;
        ASYNC_OBJECT_RECORD& pool_reg = (pooliter->second);
        //记录信息数据
        ZCE_LOG(RS_INFO, "[ZCELIB] Register command[%u] size of pool[%u] capacity of pool[%u].",
                regframe_cmd,
                pool_reg.aysncobj_pool_.size(),
                pool_reg.aysncobj_pool_.capacity()
        );

        //出现了问题，
        if (pool_reg.aysncobj_pool_.size() != pool_reg.aysncobj_pool_.capacity())
        {
            ZCE_LOG(RS_ERROR, "[ZCELIB] Plase notice!! size[%u] != capacity[%u]"
                    " may be exist memory leak.",
                    pool_reg.aysncobj_pool_.size(),
                    pool_reg.aysncobj_pool_.capacity());
        }

        //是否池子
        size_t pool_reg_trans_len = pool_reg.aysncobj_pool_.size();
        for (size_t i = 0; i < pool_reg_trans_len; ++i)
        {
            async_object* corout_base = nullptr;
            pool_reg.aysncobj_pool_.pop_front(corout_base);
            corout_base->terminate();
            delete corout_base;
            corout_base = nullptr;
        }
    }
    return;
}

//注册一类异步对象，其用reg_cmd对应，
int async_obj_mgr::register_asyncobj(uint32_t create_cmd,
                                     async_object* async_base)
{
    //这两个值必须是重新设置过的
    ZCE_ASSERT(pool_init_size_ > 0 && pool_extend_size_ > 0);

    //
    if (async_obj_mgr::INVALID_COMMAND == create_cmd)
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Register command[%u] error.",
                create_cmd);
        return -1;
    }

    //一个命令字CMD只能注册一次
    ASYNC_RECORD_POOL::iterator iter_temp = regaysnc_pool_.find(create_cmd);
    if (iter_temp != regaysnc_pool_.end())
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Register command[%u] repeated error.",
                create_cmd);
        return -1;
    }
    ASYNC_OBJECT_RECORD record;
    regaysnc_pool_[create_cmd] = record;
    ASYNC_OBJECT_RECORD& ref_rec = regaysnc_pool_[create_cmd];

    async_base->initialize();
    ref_rec.aysncobj_pool_.initialize(pool_init_size_ + 1);
    ref_rec.aysncobj_pool_.push_back(async_base);
    for (size_t i = 0; i < pool_init_size_; i++)
    {
        async_object* crtn = async_base->clone(this, create_cmd);
        crtn->initialize();
        ref_rec.aysncobj_pool_.push_back(crtn);
    }
    return 0;
}

//判断某个命令是否是注册（创建）异步对象命令
bool async_obj_mgr::is_register_cmd(uint32_t cmd)
{
    ASYNC_RECORD_POOL::iterator mapiter = regaysnc_pool_.find(cmd);
    if (mapiter == regaysnc_pool_.end())
    {
        return false;
    }
    return true;
}

//从池子里面分配一个
int async_obj_mgr::allocate_from_pool(uint32_t create_cmd,
                                      ASYNC_OBJECT_RECORD*& async_rec,
                                      async_object*& crt_async)
{
    ASYNC_RECORD_POOL::iterator mapiter = regaysnc_pool_.find(create_cmd);
    if (mapiter == regaysnc_pool_.end())
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB] Allocate async object command[%u] error.",
                create_cmd);
        return -1;
    }

    ASYNC_OBJECT_RECORD& reg_async = mapiter->second;

    //还有最后一个,扩展，
    if (reg_async.aysncobj_pool_.size() == 1)
    {
        ZCE_LOG(RS_INFO, "[ZCELIB] Before extend pool.");
        //取一个模型
        async_object* model_trans = nullptr;
        reg_async.aysncobj_pool_.pop_front(model_trans);

        size_t capacity_of_pool = reg_async.aysncobj_pool_.capacity();
        reg_async.aysncobj_pool_.resize(capacity_of_pool + pool_extend_size_);

        ZCE_LOG(RS_INFO, "[ZCELIB] Async object pool size[%u],  command[%u],"
                " capacity[%u] , resize[%u] .",
                reg_async.aysncobj_pool_.size(),
                create_cmd,
                capacity_of_pool,
                capacity_of_pool + pool_extend_size_);

        //用模型克隆N-1个Trans
        for (size_t i = 0; i < pool_extend_size_; ++i)
        {
            async_object* cloned_base = model_trans->clone(this, create_cmd);
            reg_async.aysncobj_pool_.push_back(cloned_base);
        }

        //将模型放到第N个
        reg_async.aysncobj_pool_.push_back(model_trans);
        ZCE_LOG(RS_INFO, "[ZCELIB] After Extend Async object .");
    }

    //取得一个事务
    reg_async.aysncobj_pool_.pop_front(crt_async);
    async_rec = &reg_async;

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Allocate async object command[%u],"
            "after alloc pool size[%u] .",
            create_cmd,
            reg_async.aysncobj_pool_.size());

    return 0;
}

///归还给池子里面，释放一个异步对象到池子里面
int async_obj_mgr::free_to_pool(async_object* free_crtn)
{
    ASYNC_RECORD_POOL::iterator mapiter = regaysnc_pool_.find(free_crtn->create_cmd_);
    if (mapiter == regaysnc_pool_.end())
    {
        return -1;
    }

    //
    ASYNC_OBJECT_RECORD& reg_record = mapiter->second;
    reg_record.aysncobj_pool_.push_back(free_crtn);

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Return clone frame command [%u],Pool size [%u].",
            free_crtn->create_cmd_,
            reg_record.aysncobj_pool_.size());

    return 0;
}

//创建异步对象
int async_obj_mgr::create_asyncobj(uint32_t cmd,
                                   uint32_t& id,
                                   bool& continued)
{
    int ret = 0;
    async_object* crt_async = nullptr;
    ASYNC_OBJECT_RECORD* async_rec = nullptr;
    continued = false;
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
    id = id_builder_;
    crt_async->asyncobj_id_ = id_builder_;

    ++async_rec->create_num_;

    //启动丫的
    crt_async->on_run(true, continued);
    //如果运行一下就退出了,直接结束回收
    if (continued == false)
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

    ZCE_LOG(RS_DEBUG, "[ZCELIB] Async object create. command [%u] create, "
            "id [%u],and continue run [%s].",
            cmd,
            id_builder_,
            continued ? "TRUE" : "FALSE");
    return 0;
}

//通过ID，寻找一个正在运行的异步对象
int async_obj_mgr::find_running_asyncobj(uint32_t id,
                                         async_object*& running_aysnc)
{
    running_aysnc = nullptr;
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
int async_obj_mgr::active_asyncobj(uint32_t id,
                                   bool& running)
{
    int ret = 0;
    async_object* async_obj = nullptr;
    running = false;
    ret = find_running_asyncobj(id, async_obj);
    if (ret != 0)
    {
        return ret;
    }

    //增加记录统计数据
    ASYNC_RECORD_POOL::iterator mapiter = regaysnc_pool_.find(async_obj->create_cmd_);
    if (mapiter == regaysnc_pool_.end())
    {
        //到这儿应该是框架代码错误了，一个异步对象的创建命令字没有注册？
        ZCE_LOG(RS_ERROR, "[ZCELIB] Can't find async object id [%u] create cmd [%u].",
                id,
                async_obj->create_cmd_);
        return -1;
    }

    ASYNC_OBJECT_RECORD& async_rec = mapiter->second;

    //激活同时取消定时器
    async_obj->cancel_timeout();

    async_obj->on_run(false, running);
    ++async_rec.active_num_;

    //如果不继续运行了，
    if (running == false)
    {
        ++async_rec.end_num_;
        async_obj->on_end();
        free_to_pool(async_obj);
    }
    ZCE_LOG(RS_DEBUG, "[ZCELIB] Async object active. command [%u] create, id [%u],and continue run [%s].",
            async_obj->create_cmd_,
            id,
            running ? "TRUE" : "FALSE");

    return 0;
}

//超时处理
int async_obj_mgr::timer_timeout(const zce::time_value& now_time,
                                 int timer_id)
{
    auto iter = timer_to_async_map_.find(timer_id);
    if (iter == timer_to_async_map_.end())
    {
        return -1;
    }
    auto async_obj = iter->second;
    //增加记录统计数据
    ASYNC_RECORD_POOL::iterator mapiter = regaysnc_pool_.find(async_obj->create_cmd_);
    if (mapiter == regaysnc_pool_.end())
    {
        return -1;
    }
    ASYNC_OBJECT_RECORD& async_rec = mapiter->second;

    //处理前取消定时器
    cancel_timer(async_obj);

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

//给一个async_object设置一个定时器
int async_obj_mgr::schedule_timer(zce::async_object* aysnc_obj,
                                  const zce::time_value& delay_time)
{
    int ret = 0;
    ZCE_ASSERT_ALL(aysnc_obj->timeout_id_ == -1);
    if (aysnc_obj->timeout_id_ != -1)
    {
        return -1;
    }
    ret = timer_queue_->schedule_timer(this,
                                       aysnc_obj->timeout_id_,
                                       delay_time);
    if (ret != 0)
    {
        return ret;
    }
    timer_to_async_map_[aysnc_obj->timeout_id_] = aysnc_obj;
    return 0;
}

//! 取消定时器
int async_obj_mgr::cancel_timer(zce::async_object* aysnc_obj)
{
    ZCE_ASSERT_ALL(aysnc_obj->timeout_id_ != -1);
    if (aysnc_obj->timeout_id_ == -1)
    {
        return -1;
    }
    int ret = timer_queue_->cancel_timer(aysnc_obj->timeout_id_);
    auto iter = timer_to_async_map_.find(aysnc_obj->timeout_id_);
    aysnc_obj->timeout_id_ = -1;
    if (ret != 0)
    {
        return ret;
    }
    if (iter == timer_to_async_map_.end())
    {
        return -1;
    }
    return 0;
}

//去的当前的负载情况
void async_obj_mgr::load_foctor(size_t& load_cur, size_t& load_max)
{
    load_cur = running_aysncobj_.size();
    if (max_load_async_ < load_cur)
    {
        max_load_async_ = load_cur;
    }
    load_max = max_load_async_;
}

//打印管理器的基本信息，运行状态
void async_obj_mgr::dump_info(zce::LOG_PRIORITY log_priority) const
{
    //
    ZCE_LOG(log_priority, "Register create cmd size[%lu].active running async object size[%lu].",
            regaysnc_pool_.size(),
            running_aysncobj_.size());
    //打印每个创建命令对于异步对象池子的数据
    auto iter = regaysnc_pool_.begin();
    for (; iter != regaysnc_pool_.end(); ++iter)
    {
        const ASYNC_OBJECT_RECORD& async_rec = iter->second;
        uint32_t create_cmd = iter->first;
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
}