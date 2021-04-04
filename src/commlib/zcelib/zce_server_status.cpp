#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_server_status.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_ptr_guard.h"

ZCE_Server_Status *ZCE_Server_Status::instance_ = NULL;

/******************************************************************************************
class ZCE_STATUS_ITEM_ID
******************************************************************************************/

ZCE_STATUS_ITEM_ID::ZCE_STATUS_ITEM_ID(uint32_t statics_id,
                                       uint32_t classify_id,
                                       uint32_t subclassing_id) :
    statics_id_(statics_id),
    classify_id_(classify_id),
    subclassing_id_(subclassing_id)
{

}

ZCE_STATUS_ITEM_ID::ZCE_STATUS_ITEM_ID():
    statics_id_(0),
    classify_id_(0),
    subclassing_id_(0)
{
}

ZCE_STATUS_ITEM_ID::~ZCE_STATUS_ITEM_ID()
{

}

bool ZCE_STATUS_ITEM_ID::operator == (const ZCE_STATUS_ITEM_ID &others) const
{
    if (this->statics_id_ == others.statics_id_
        && this->classify_id_ == others.classify_id_
        && this->subclassing_id_ == others.subclassing_id_)
    {
        return true;
    }

    return false;
}

/******************************************************************************************
struct ZCE_STATUS_ITEM 状态计数器项
******************************************************************************************/
ZCE_STATUS_ITEM::ZCE_STATUS_ITEM():
    statics_type_(ZCE_STATUS_STATICS::PER_FIVE_MINTUES),
    counter_(0)
{
}

ZCE_STATUS_ITEM::ZCE_STATUS_ITEM(unsigned int statics_id,
                                 ZCE_STATUS_STATICS statics_type):
    item_id_(statics_id, 0, 0),
    statics_type_(statics_type),
    counter_(0)
{
}

ZCE_STATUS_ITEM::~ZCE_STATUS_ITEM()
{
}

/******************************************************************************************
class ZCE_STATUS_ITEM_WITHNAME 状态计数器+名字，用于配置，DUMP输出等
******************************************************************************************/
ZCE_STATUS_ITEM_WITHNAME::ZCE_STATUS_ITEM_WITHNAME(unsigned int statics_id,
                                                   ZCE_STATUS_STATICS statics_type,
                                                   const char *stat_name):
    statics_item_(statics_id, statics_type)
{
    strncpy(item_name_, stat_name, MAX_COUNTER_NAME_LEN);
    item_name_[MAX_COUNTER_NAME_LEN] = '\0';
}

ZCE_STATUS_ITEM_WITHNAME::ZCE_STATUS_ITEM_WITHNAME()
{
    item_name_[MAX_COUNTER_NAME_LEN]='\0';
}

ZCE_STATUS_ITEM_WITHNAME::~ZCE_STATUS_ITEM_WITHNAME()
{

}

/******************************************************************************************
ZCE_Server_Status
******************************************************************************************/

//构造函数,也给你单独使用的机会，所以不用protected
ZCE_Server_Status::ZCE_Server_Status():
    stat_lock_(NULL),
    stat_file_head_(NULL),
    status_stat_sandy_(NULL),
    status_copy_mandy_(NULL),
    multi_thread_guard_(false),
    initialized_(false)
{
}

//
ZCE_Server_Status::~ZCE_Server_Status()
{
    //清理掉各种指针
    if (stat_lock_)
    {
        delete stat_lock_;
        stat_lock_ = NULL;
    }

    if (status_stat_sandy_)
    {
        delete status_stat_sandy_;
        status_stat_sandy_ = NULL;
    }

    if (status_copy_mandy_)
    {
        delete status_copy_mandy_;
        status_copy_mandy_ = NULL;
    }

}

//初始化的方法,通用的底层，
//Param1: char* statfilename MMAP影射的状态文件名称
//Param2: bool restore_mmap 是否用于恢复MMAP，如果是恢复，文件必须是存在的,
int ZCE_Server_Status::initialize(const char *stat_filename,
                                  bool restore_mmap,
                                  bool multi_thread)
{
    //映射文件名称不能为NULL，统计数量不能为0，统计初始化数组不能为NULL
    int ret = 0;

    //计算所需的空间
    size_t size_alloc = ARRYA_OF_SHM_STATUS::getallocsize(MAX_MONITOR_STAT_ITEM);

    //正常讲启动都会重新建立一个状态文件，
    //原来在restore_mmap == false是，会在open_mode后面并上一个O_TRUNC,但没有考虑到的是上报程序可能
    //一直打开了这个共享内存文件,
    //去掉这个步骤，重建数据依靠后面的MMAP数组部分，而不依赖重建文件
    int open_mode = O_CREAT | O_RDWR;

    //需要有一个拷贝的数据去，所以*2，还需要一个头
    ret = stat_file_.open(stat_filename,
                          sizeof(ZCE_STATUS_HEAD) + size_alloc * 2,
                          open_mode);

    if (ret != 0)
    {
        return ret;
    }

    stat_file_head_ = static_cast<ZCE_STATUS_HEAD *>(stat_file_.addr());

    //如果在这儿出现断言，就是你代码使用错误了.
    ZCE_ASSERT(NULL == status_stat_sandy_
               && NULL == status_copy_mandy_
               && NULL == stat_lock_ );

    // 统计数据区初始化
    char *stat_ptr = static_cast<char *>(stat_file_.addr()) + sizeof(ZCE_STATUS_HEAD);
    status_stat_sandy_ = ARRYA_OF_SHM_STATUS::initialize(MAX_MONITOR_STAT_ITEM,
                                                         stat_ptr,
                                                         restore_mmap);

    if (!status_stat_sandy_)
    {
        return -1;
    }

    // 拷贝数据区初始化
    char *copy_ptr = static_cast<char *>(stat_file_.addr()) + sizeof(ZCE_STATUS_HEAD) + size_alloc;
    status_copy_mandy_ = ARRYA_OF_SHM_STATUS::initialize(MAX_MONITOR_STAT_ITEM,
                                                         copy_ptr,
                                                         restore_mmap);

    if (!status_copy_mandy_)
    {
        return -1;
    }

    //修改线程锁保护的行为
    multi_thread_guard(multi_thread);

    //
    return 0;
}

//增加一些监控项目，有时候框架会有一部分监控项目，各自项目有有一些监控项目
void ZCE_Server_Status::add_status_item(size_t num_add_stat_item,
                                        const ZCE_STATUS_ITEM_WITHNAME item_ary[])
{
    // 扩大数组的空间, 多扩大几个空间也没有关系
    size_t num_old_stat_item = conf_stat_map_.size();
    conf_stat_map_.rehash(num_old_stat_item + num_add_stat_item + 128);

    for (size_t i = 0; i < num_add_stat_item; ++ i)
    {
        //能写auto正好
        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(item_ary[i].statics_item_.item_id_.statics_id_);
        if (iter == conf_stat_map_.end())
        {
            conf_stat_map_.insert(STATUS_WITHNAME_MAP::value_type(item_ary[i].statics_item_.item_id_.statics_id_,
                                                                  item_ary[i]) );
        }
        else
        {
            ZCE_LOG(RS_ERROR, "Add repeat ZCE_STATUS_ITEM_WITHNAME statics_id_[%u] name[%s],please check your code.",
                    item_ary[i].statics_item_.item_id_.statics_id_,
                    item_ary[i].item_name_);
            continue;
        }
    }
}

bool ZCE_Server_Status::is_exist_stat_id(unsigned int stat_id,
                                         ZCE_STATUS_ITEM_WITHNAME *status_item_withname) const
{
    //能写auto正好
    STATUS_WITHNAME_MAP::const_iterator iter = conf_stat_map_.find(stat_id);
    if (iter == conf_stat_map_.end())
    {
        return false;
    }
    *status_item_withname = iter->second;
    return true;
}

//修改是否需要多线程保护
void ZCE_Server_Status::multi_thread_guard(bool multi_thread)
{

    //如果旧有的锁仍然存在，先删除
    if (stat_lock_)
    {
        delete stat_lock_;
        stat_lock_ = NULL;
    }

    multi_thread_guard_ = multi_thread;

    //利用多态决定锁的行为
    if (multi_thread)
    {
        stat_lock_ = new ZCE_Thread_Light_Mutex();
    }
    else
    {
        stat_lock_ = new ZCE_Null_Mutex();
    }
}

//在sandy数据区里面，找数据项目
//这个函数里面不要加锁（在上层加），因为这个函数是一个公用函数，可能会……
int ZCE_Server_Status::find_insert_idx(uint32_t statics_id,
                                       uint32_t classify_id,
                                       uint32_t subclassing_id,
                                       size_t *sandy_idx)
{
    *sandy_idx = static_cast<size_t>(-1);

    ZCE_STATUS_ITEM_ID stat_item_id(statics_id, classify_id, subclassing_id);
    STATID_TO_INDEX_MAP::iterator iter_tmp = statid_to_index_.find(stat_item_id);

    if ( iter_tmp != statid_to_index_.end() )
    {
        *sandy_idx = iter_tmp->second;
        return 0;
    }

    //如果原来没这个统计项目,看看是否需要要添加，检查statics_id_
    STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(statics_id);
    if (iter == conf_stat_map_.end())
    {
        ZCE_LOG(RS_ERROR, "Use one statics_id_,it don't config [%u] ,please check your code.",
                statics_id);
        return -1;
    }


    //如果已经满了，也算了
    if (status_stat_sandy_->full())
    {
        ZCE_LOG(RS_ERROR, "Statics array is full,please extend in start. ary size is [%lu]",
                status_stat_sandy_->size());
        return -1;
    }

    size_t idx = status_stat_sandy_->size();

    ZCE_STATUS_ITEM status_item;
    status_item.item_id_ = stat_item_id;
    status_item.counter_ = 0;

    //定义过这个统计项目，只是没有没有添加
    status_stat_sandy_->push_back(status_item);

    //在MAP中间增加这个索引
    statid_to_index_[stat_item_id] = idx;

    *sandy_idx = idx;
    return 0;
}

//根据一个已经存在的文件进行初始化,用于恢复数据区,文件必须已经存在，
int ZCE_Server_Status::initialize(const char *stat_filename, bool multi_thread)
{
    ZCE_ASSERT(stat_filename != NULL );

    int ret = 0;
    //
    ret = initialize(stat_filename, true, multi_thread);

    if ( 0 != ret)
    {
        return ret;
    }

    return 0;
}

//创建一个已经存在的文件进行初始化,用于恢复数据区,如果文件必须已经存在，会重新创建
//Param1: char* statfilename MMAP影射的状态文件名称
//Param2: size_t num_stat_ary     状态计数器的个数,
//Param3: ZCE_STATUS_ITEM item_ary[] 状态技术器的
//Param4: multi_thread 多线程环境
int ZCE_Server_Status::initialize(const char *stat_filename,
                                  size_t num_stat_item,
                                  const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                                  bool multi_thread)
{
    // 允许参数num_stat_item == 0, 因为有些app是不需要app层的监控的
    ZCE_ASSERT(stat_filename != NULL);
    //
    int ret = 0;
    ret = initialize(stat_filename, false, multi_thread);

    if ( 0 != ret)
    {
        return ret;
    }

    add_status_item(num_stat_item, item_ary);

    //多增加一些空间，目标是减小冲突，空间换时间
    statid_to_index_.rehash( static_cast<size_t>(MAX_MONITOR_STAT_ITEM * 1.2 ));

    //
    stat_file_head_->monitor_start_time_ = static_cast<uint64_t>(time(NULL));
    stat_file_head_->copy_time_ = static_cast<uint64_t>(time(NULL));
    stat_file_head_->active_time_ = static_cast<uint64_t>(time(NULL));

    initialized_ = true;

    return 0;
}

//相对值修改mandy或者sandy统计计数，使用统计ID和分类ID作为key,接口使用方便一点，你不用记录很多对应关系,但速度慢一点,
int ZCE_Server_Status::increase_by_statid(uint32_t statics_id,
                                          uint32_t classify_id,
                                          uint32_t subclassing_id,
                                          int64_t incre_value)
{
    if (!initialized_)
    {
        // 未初始化
        ZCE_ASSERT(0);
        return -1;
    }

    int ret = 0;
    size_t sandy_idx = static_cast<size_t>(-1);

    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    ret = find_insert_idx(statics_id, classify_id, subclassing_id, &sandy_idx);

    if ( 0 != ret )
    {
        return ret;
    }

    //增加统计数值
    (status_stat_sandy_->begin() + sandy_idx)->counter_ += incre_value;
    return 0;
}

//绝对值修改监控统计项目，
int ZCE_Server_Status::set_by_statid(uint32_t statics_id,
                                     uint32_t classify_id,
                                     uint32_t subclassing_id,
                                     uint64_t set_value)
{
    if (!initialized_)
    {
        // 未初始化
        ZCE_ASSERT(0);
        return -1;
    }

    int ret = 0;
    size_t sandy_idx = static_cast<size_t>(-1);

    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    ret = find_insert_idx(statics_id, classify_id, subclassing_id, &sandy_idx);

    if ( 0 != ret )
    {
        return ret;
    }

    //设置统计数值
    (status_stat_sandy_->begin() + sandy_idx)->counter_ = set_value;
    return 0;
}

//根据统计ID和分类ID作为key，得到统计数值
uint64_t ZCE_Server_Status::get_counter(uint32_t statics_id,
                                        uint32_t classify_id,
                                        uint32_t subclassing_id)
{
    ZCE_STATUS_ITEM_ID stat_item_id(statics_id, classify_id, subclassing_id);
    STATID_TO_INDEX_MAP::iterator iter_tmp = statid_to_index_.find(stat_item_id);

    if ( iter_tmp != statid_to_index_.end() )
    {
        ZCE_Lock_Ptr_Guard guard(stat_lock_);
        size_t index = iter_tmp->second;
        return (status_stat_sandy_->begin()  + index)->counter_;
    }
    else
    {
        return 0;
    }
}

//取得计数器的个数
size_t ZCE_Server_Status::num_of_counter()
{
    //不会改变的数值，不加锁
    return status_stat_sandy_->size();
}

//获取copy_time
uint64_t ZCE_Server_Status::get_copy_time()
{
    return stat_file_head_->copy_time_;
}

//清理过期的数据，在你的定时器触发时调用（当然前面最好应该上报），用于将一些数据清0，
//理论上每5分钟调用一次就OK
void ZCE_Server_Status::check_overtime(time_t now_time)
{
    ZCE_ASSERT(initialized_);

    //
    ZCE_STATUS_STATICS clear_type = ZCE_STATUS_STATICS::INVALID_TYPE;
    time_t last_clear_time = static_cast<time_t>(stat_file_head_->active_time_);

    //看时间周期发生了什么变化没有
    if (last_clear_time / FIVE_MINTUE_SECONDS !=
        now_time / FIVE_MINTUE_SECONDS)
    {
        clear_type = ZCE_STATUS_STATICS::PER_FIVE_MINTUES;

        //如果5分钟都没有变化，小时不会变化
        if (last_clear_time / zce::ONE_HOUR_SECONDS !=
            now_time / zce::ONE_HOUR_SECONDS)
        {
            clear_type = ZCE_STATUS_STATICS::PER_HOUR;

            //如果小时都没有变化，天不会变化
            if (last_clear_time / zce::ONE_DAY_SECONDS !=
                now_time / zce::ONE_DAY_SECONDS)
            {
                clear_type = ZCE_STATUS_STATICS::PER_DAYS;
            }
        }
    }

    size_t num_of_counter = status_stat_sandy_->size();

    //如果没有处理
    if (clear_type == ZCE_STATUS_STATICS::INVALID_TYPE)
    {
        return;
    }

    //先复制到备份内存
    copy_stat_counter();

    //清理过期的统计数据
    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    for (size_t i = 0; i < num_of_counter; ++i)
    {
        ZCE_STATUS_ITEM *cur_item =  status_stat_sandy_->begin() + i;

        if (clear_type >= cur_item->statics_type_)
        {
            //由于zergsvr不能按照这种方式上报自己的监控数据
            //所以取消下面的上报功能，统一由外部工具来上报copy内存区的数据
            cur_item->counter_ = 0;
        }
    }

    //记录清理时间
    stat_file_head_->active_time_ = static_cast<uint64_t>(now_time);
}

//由于将内部数据全部取出，用于你外部打包之类
//由于是一个较少调用的函数，我降低了他的性能，保证记录使用的内存空间更小
void ZCE_Server_Status::dump_all(ARRAY_OF_STATUS_WITHNAME &array_status, bool dump_copy)
{
    ZCE_Lock_Ptr_Guard guard(stat_lock_);


    //两个数据区大小应该一样
    size_t num_of_counter = status_stat_sandy_->size();
    array_status.resize(num_of_counter);


    for (size_t i = 0; i < num_of_counter; ++i)
    {
        if (false == dump_copy)
        {
            array_status[i].statics_item_ = (*status_stat_sandy_)[i];
        }
        else
        {
            array_status[i].statics_item_ = (*status_copy_mandy_)[i];
        }


        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(array_status[i].statics_item_.item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(array_status[i].item_name_,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //因为前面的代码都做过检查，所以，这儿理论不会出现找不到的可能，除非代码错误。
            ZCE_ASSERT(false);
        }
    }
}

//备份计数器信息
void ZCE_Server_Status::copy_stat_counter()
{
    //将备份数据数据去赋值
    size_t num_of_counter = status_stat_sandy_->size();
    status_copy_mandy_->resize(num_of_counter);
    ZCE_STATUS_ITEM *stat_sandy_begin = status_stat_sandy_->begin();
    ZCE_STATUS_ITEM *stat_mandy_begin = status_copy_mandy_->begin();
    std::copy_n(stat_sandy_begin,num_of_counter,stat_mandy_begin);

    //刷新备份时间
    ZCE_ASSERT(stat_file_head_ != NULL);
    stat_file_head_->copy_time_ = static_cast<uint32_t>(time(NULL));
}

//Dump所有的数据
void ZCE_Server_Status::dump_status_info(std::ostringstream &strstream, bool dump_copy)
{
    size_t num_of_counter = 0;
    strstream << "Statistics Number:" << static_cast<unsigned int>(num_of_counter) << std::endl;

    ZCE_STATUS_ITEM *stat_process_iter = NULL;

    //根据确定是从哪个数据区读取数据
    if (dump_copy)
    {
        num_of_counter = status_stat_sandy_->size();
        stat_process_iter = status_stat_sandy_->begin();
    }
    else
    {
        num_of_counter = status_copy_mandy_->size();
        stat_process_iter = status_copy_mandy_->begin();
    }

    //记录监控配置的名字的变量
    char statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN + 1];
    statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN] = '\0';

    ZCE_Lock_Ptr_Guard guard(stat_lock_);


    for (size_t i = 0; i < num_of_counter; ++i)
    {

        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find((stat_process_iter + i)->item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(statics_item_name,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //因为前面的代码都做过检查，所以，这儿理论不会出现找不到的可能，除非代码错误。
            ZCE_ASSERT(false);
        }

        strstream << std::setw(6) << i << ".";
        strstream << "statics id<" << std::setw(10) << (stat_process_iter + i)->item_id_.statics_id_ << ">";
        strstream << "game app id<" << std::setw(10) << (stat_process_iter + i)->item_id_.classify_id_ << ">";
        strstream << "classify id<" << std::setw(10) << (stat_process_iter + i)->item_id_.subclassing_id_ << ">";
        strstream << std::setw(ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN) << statics_item_name << ":" << (stat_process_iter + i)->counter_ << std::endl;
    }
}

//Dump所有的数据
void ZCE_Server_Status::dump_status_info(zce::LOG_PRIORITY log_priority, bool dump_copy)
{
    size_t num_of_counter = 0;
    ZCE_STATUS_ITEM *stat_process_iter = NULL;


    //记录监控配置的名字的变量
    char statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN + 1];
    statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN] = '\0';

    //根据确定是从哪个数据区读取数据
    if (dump_copy)
    {
        num_of_counter = status_stat_sandy_->size();
        stat_process_iter = status_stat_sandy_->begin();
    }
    else
    {
        num_of_counter = status_copy_mandy_->size();
        stat_process_iter = status_copy_mandy_->begin();
    }

    ZCE_LOG(RS_INFO, "Statistics Number: %u", num_of_counter);
    ZCE_LOG(RS_INFO, "index.<statics id,classify id> name                            :number");

    ZCE_STATUS_ITEM_WITHNAME tmp_check;
    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    for (size_t i = 0; i < num_of_counter; ++i)
    {
        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find((stat_process_iter + i)->item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(statics_item_name,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //因为前面的代码都做过检查，所以，这儿理论不会出现找不到的可能，除非代码错误。
            ZCE_ASSERT(false);
        }

        ZCE_LOG(log_priority, "%5u.<%10u, %10u, %10u> %32s : %llu ",
                i,
                (stat_process_iter + i)->item_id_.statics_id_,
                (stat_process_iter + i)->item_id_.classify_id_,
                (stat_process_iter + i)->item_id_.subclassing_id_,
                statics_item_name,
                (stat_process_iter + i)->counter_
               );
    }
}

//得到文件的头部信息
void ZCE_Server_Status::get_stat_head(ZCE_STATUS_HEAD *stat_head )
{
    *stat_head = *stat_file_head_;
}

//记录监控的上报时间
void ZCE_Server_Status::report_monitor_time(uint64_t report_time)
{
    stat_file_head_->report_monitor_time_ = report_time;
}

//得到唯一的单子实例
ZCE_Server_Status *ZCE_Server_Status::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZCE_Server_Status();
    }

    return instance_;
}

//赋值唯一的单子实例
void ZCE_Server_Status::instance(ZCE_Server_Status *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//清除单子实例
void ZCE_Server_Status::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

