/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/server/status.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年3月7日
* @brief
*
*
* @details    服务器的统计类，
*             这个想法来自rong,sonicmao的C4A的服务器,向rong,sonicmao致敬.
*
*
* @note       修改记录，很简单的一个类，但改动很多
*     1.Date  :2011年10月30日
*     Author  :根据ZCELIB的代码对现有的代码进行优化
*     Modification  :
*     2.Date  :2012年1月15日
*     Author  :Sailzeng
*     Modification  :在N次反复，以及吐血的改了2次后，我开始倾向用一个最简单的方式解决问题，
*     3.Date  :2012年5月10日
*     Author  :Sailzeng
*     Modification  :很多啊同事还是吐槽需要设置classify，我想还是改了把。部分告别sandy和mandy
*     4.Date  :2013年10月1日
*     Author  :Sailzeng
*     Modification  :有一个防止统计ID重复的需求，将原来的ARRAY改造成了一个MAP映射，
*
*/

#pragma once

#include "zce/shared_mem/posix.h"
#include "zce/shm_container/vector.h"
#include "zce/lock/null_lock.h"
#include "zce/os_adapt/time.h"

namespace zce
{
//===========================================================================================
/*!
* @brief      状态统计的方式，比如5分钟记录一次的方式，一个小时记录的方式等
*
* @note
*/
enum class STATUS_STATICS
{
    //标识范围
    INVALID_TYPE = 0,

    //每5分钟进行一次统计，尽量用5分钟的统计方式
    PER_FIVE_MINTUES = 1,

    //每小时进行一次统计,:0会清空，这种统计方式主要用于1个小时变化数据，用于对比等，
    PER_HOUR = 2,
    //每天进行一次统计，0:0会清空，，这种统计方式主要用于1天变化数据
    PER_DAYS = 3,

    //绝对值
    ABSOLUTE_VALUE = 11,

    //原来还有一种每次启动时，数值是否清空的选项，算了，没必要保留了
};

//保证数据文件的一致性
#pragma pack(8)

//===========================================================================================
/*!
* @brief      统计项目ID,
*
* @note
*/
struct STATUS_ITEM_ID
{
public:

    STATUS_ITEM_ID(uint32_t statics_id,
                   uint32_t classify_id,
                   uint32_t subclassing_id);
    STATUS_ITEM_ID() = default;
    ~STATUS_ITEM_ID() = default;

    bool operator == (const STATUS_ITEM_ID& others) const;

public:

    ///统计ID
    uint32_t              statics_id_ = 0;
    ///分类ID，目前好像主要是业务ID,这个是可以变化的
    uint32_t              classify_id_ = 0;
    ///子分类ID，这个也是可以变化的，
    uint32_t              subclassing_id_ = 0;
};

//===========================================================================================
/*!
* @brief      ZCE_STATUS_ITEM_ID得到HASH因子的函数，
*
*/
struct HASH_ZCE_STATUS_ITEM_ID
{
public:
    //就把3个数值相+，这样冲突感觉还小一点，（左移反而感觉不好）
    size_t operator()(const STATUS_ITEM_ID& stat_item) const
    {
        return static_cast<size_t>(stat_item.statics_id_)
            + static_cast<size_t>(stat_item.classify_id_)
            + static_cast<size_t>(stat_item.subclassing_id_);
    }
};

//===========================================================================================
/*!
* @brief      状态计数器项,包括ID，统计方式，以及数值
*
*/
class ZCE_STATUS_ITEM
{
public:

    //构造函数和析构函数
    ZCE_STATUS_ITEM() = default;
    ZCE_STATUS_ITEM(unsigned int statics_id,
                    STATUS_STATICS statics_type);
    ~ZCE_STATUS_ITEM() = default;

public:

    ///ID标识
    STATUS_ITEM_ID   item_id_;

    ///可以重新计数
    STATUS_STATICS   statics_type_ = STATUS_STATICS::PER_FIVE_MINTUES;

    ///计数器
    uint64_t         counter_ = 0;
};

//===========================================================================================
/*!
* @brief      状态计数器+名字，用于配置，DUMP输出等
*
*/
class STATUS_ITEM_WITHNAME
{
public:

    //
    STATUS_ITEM_WITHNAME(unsigned int,
                         STATUS_STATICS,
                         const char*);
    STATUS_ITEM_WITHNAME();
    ~STATUS_ITEM_WITHNAME();

public:
    //统计项目名称的长度
    static const size_t       MAX_COUNTER_NAME_LEN = 127;

public:

    ZCE_STATUS_ITEM           statics_item_;

    //计数器名称
    char                      item_name_[MAX_COUNTER_NAME_LEN + 1];
};

//用于帮助你定义 ZCE_STATUS_ITEM_WITHNAME数组
#ifndef DEF_STATUS_ITEM
#define DEF_STATUS_ITEM(_statics_id,_statics_type) zce::STATUS_ITEM_WITHNAME(_statics_id,_statics_type,(#_statics_id))
#endif

//===========================================================================================

/*!
* @brief      存放统计信息的状态文件的头部。记录监控时间等时间戳
*
*/
struct ZCE_STATUS_HEAD
{
    //监控开始时间
    uint64_t monitor_start_time_;

    //监控数据复制的时间戳
    uint64_t copy_time_;

    //向monitor上报的时间
    uint64_t report_monitor_time_;

    //激活时间长度，目前没有记录
    uint64_t active_time_;
};

#pragma pack()

//===========================================================================================

/*!
* @brief      使用Posix MMAP,记录保存服务器的一些计数器,状态,
*
* @note       本来使用的是锁模式，但发现使用这个东西会导致满世界的问题扩大化,
*/
class server_status : public zce::non_copyable
{
protected:

    ///存放统计数据的共享内存数组，
    typedef zce::shm_vector<ZCE_STATUS_ITEM>     ARRYA_OF_SHM_STATUS;
    ///统计ID到数组的下标的hash map
    typedef std::unordered_map<STATUS_ITEM_ID, size_t, HASH_ZCE_STATUS_ITEM_ID>     STATID_TO_INDEX_MAP;
    ///statics_id_做key的ZCE_STATUS_ITEM_WITHNAME的结构
    typedef std::unordered_map<uint32_t, STATUS_ITEM_WITHNAME>    STATUS_WITHNAME_MAP;

    ///统计数据的数组，用于dump输出的数据结构
    typedef std::vector<STATUS_ITEM_WITHNAME>  ARRAY_OF_STATUS_WITHNAME;

public:

    ///构造函数,也给你单独使用的机会，所以不用protected
    server_status();
    ///析构函数
    virtual ~server_status();

protected:

    /*!
    * @brief      初始化的方法,通用的底层，
    * @return     int
    * @param[in]  stat_filename MMAP影射的状态文件名称
    * @param[in]  restore_mmap  是否用于恢复MMAP，如果是恢复，文件必须是存在的,
    * @param[in]  multi_thread  是否是多线程环境使用
    */
    int initialize(const char* stat_filename,
                   bool restore_mmap,
                   bool multi_thread);

    /*!
    * @brief      在数据区里面，找数据项目
    * @return     int 如果成功找到返回0
    * @param[in]  statics_id  统计ID
    * @param[in]  classify_id 分类ID
    * @param[in]  subclassing_id 子分类ID
    * @param[out] idx 查询到数据项目存放索引
    */
    int find_insert_idx(uint32_t statics_id,
                        uint32_t classify_id,
                        uint32_t subclassing_id,
                        size_t* idx);

public:

    /*!
    * @brief      根据一个已经存在的文件进行初始化,用于恢复数据区,文件必须已经存在，
    *             一般查询状态工具使用这个函数。
    * @return     int
    * @param[in]  stat_filename MMAP影射的状态文件名称
    * @param[in]  multi_thread  是否多线程
    */
    int initialize(const char* stat_filename,
                   bool multi_thread);

    /*!
    * @brief      创建一个已经存在的文件进行初始化,用于恢复数据区,如果文件必须已经存在，会重新创建
    *             需要记录状态的服务器使用这个函数
    * @return     int
    * @param[in]  stat_filename MMAP影射的状态文件名称
    * @param[in]  num_stat_item item_ary的数量，
    * @param[in]  item_ary      统计项目，
    * @param[in]  multi_thread  是否多线程
    * @note       注意统计项目数量不是监控ID的数量，监控ID数量默认是固定的 @ref MAX_MONITOR_STAT_ITEM
    */
    int initialize(const char* stat_filename,
                   size_t num_stat_item,
                   const STATUS_ITEM_WITHNAME item_ary[],
                   bool multi_thread);

    /*!
    * @brief      增加一些监控项目，如果有2段的初始化（比如框架一次，业务一次）时使用
    * @return     void
    * @param      num_stat_item item_ary的数量
    * @param      item_ary      增加的统计项目
    */
    void add_status_item(size_t num_stat_item,
                         const STATUS_ITEM_WITHNAME item_ary[]);

    ///初始化以后，修改是否需要多线程保护
    void multi_thread_guard(bool multi_thread);

    ///监控项是否已经存在
    bool is_exist_stat_id(unsigned int stat_id,
                          STATUS_ITEM_WITHNAME* status_item_withname) const;

    /*!
    * @brief      使用统计ID和分类ID作为key,对统计值增加1
    * @return     int
    * @param[in]  statics_id  统计ID
    * @param[in]  classify_id 分类ID
    * @param[in]  subclassing_id 子分类ID
    */
    inline int add_one(uint32_t statics_id,
                       uint32_t classify_id,
                       uint32_t subclassing_id)
    {
        return add_number(statics_id, classify_id, subclassing_id, 1);
    }

    /*!
    * @brief      使用统计ID和分类ID作为key,相对值修改监控统计值
    * @return     int
    * @param[in]  statics_id  统计ID
    * @param[in]  classify_id 分类ID
    * @param[in]  subclassing_id 子分类ID，查询会使用统计ID，分类ID，子分类ID一起查询
    * @param[in]  incre_value 修改的相对值，符号整数，可加可减
    */
    int add_number(uint32_t statics_id,
                   uint32_t classify_id,
                   uint32_t subclassing_id,
                   int64_t incre_value);

    /*!
    * @brief      使用统计ID和分类ID作为key,绝对值修改监控统计项目计数器
    * @return     int
    * @param[in]  statics_id  统计ID
    * @param[in]  classify_id 分类ID
    * @param[in]  subclassing_id 子分类ID
    * @param[in]  set_value 修改的统计值
    */
    int set_counter(uint32_t statics_id,
                    uint32_t classify_id,
                    uint32_t subclassing_id,
                    uint64_t set_value);

    /*!
    * @brief      根据统计ID和分类ID作为key，得到统计数值
    * @return     uint64_t 返回的统计值
    * @param[in]  statics_id  统计ID
    * @param[in]  classify_id 分类ID
    * @param[in]  subclassing_id 子分类ID
    */
    uint64_t get_counter(uint32_t statics_id,
                         uint32_t classify_id,
                         uint32_t subclassing_id);

    //取得计数器的个数
    size_t num_of_counter();

    //获取copy_time
    uint64_t get_copy_time();

    //清理过期的数据，在你的定时器触发时调用（当然前面最好应该上报），用于将一些数据清0，
    //理论上每5分钟调用一次就OK
    void check_overtime(time_t now_time);

    ///备份计数器信息
    void copy_stat_counter();

    ///由于将内部数据全部取出，用于你外部打包之类
    void dump_all(ARRAY_OF_STATUS_WITHNAME& array_status,
                  bool dump_copy = false);

    ///Dump所有的数据
    void dump_status_info(std::ostringstream& strstream,
                          bool dump_copy = false);

    ///Dump所有的数据
    void dump_status_info(zce::LOG_PRIORITY log_priority,
                          bool dump_copy = false);

    ///得到文件的头部信息
    void get_stat_head(ZCE_STATUS_HEAD* stat_head);

    ///记录监控的上报时间
    void report_monitor_time(uint64_t report_time = static_cast<uint64_t>(time(nullptr)));

    //单子的函数群，不是我不知道可以用BOOST的模板使用单子，是这样更加直接清爽，容易扩张修改一些
    //我不会为了单子考虑所谓的保护问题，你自己保证你的初始化函数不会重入

public:

    //得到单子实例
    static server_status* instance();
    //单子实例赋值
    static void instance(server_status*);
    //清理单子实例
    static void clear_inst();

protected:

    //最多的监控项目ID,
    static constexpr size_t MAX_MONITOR_STAT_ITEM = 64 * 1024;

    //五分钟的描述
    static constexpr time_t FIVE_MINTUE_SECONDS = 300;
    //一小时的秒数
    static constexpr time_t ONE_HOURS_SECONDS = 3600;
    //一天的时间
    static constexpr time_t ONE_DAY_SECONDS = 86400;

    //统计项目数值增加1
    static constexpr uint64_t INCREASE_VALUE_ONCE = 1;

protected:

    //单子实例
    static server_status* instance_;

protected:

    //多态的锁,
    zce::lock_base* stat_lock_;

    //MMAP内存影射的数据文件
    zce::shm_posix        stat_file_;

    //内存文件头
    ZCE_STATUS_HEAD* stat_file_head_;

    //mandy和sandy是原来代码中间为了区分用数组定位，hash定位两个数据区的东东，
    //后来代码全部改为了用hansh定位，不好意思，两个小MM，怪蜀黍就是步放过你们，
    //http://t.qq.com/angelbaby22

    // 存放状态计数器的数组
    ARRYA_OF_SHM_STATUS* status_stat_sandy_;

    //状态计数器的一份拷贝(备份)，数据放在共享内存中，我们可以用考虑用mandy读取数据，所以可以讲每个
    ARRYA_OF_SHM_STATUS* status_copy_mandy_;

    //记录配置的的统计数据SET，用于记录配置的统计项目，也用于防止重复插入和dump 输出时有名称信息
    STATUS_WITHNAME_MAP  conf_stat_map_;

    //STAT_ID to idx索引的MAP
    STATID_TO_INDEX_MAP  statid_to_index_;

    //是否进行多线程保护
    bool                 multi_thread_guard_;

    // 是否已经初始化
    bool                 initialized_;
};
}
