/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_async_fw_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Saturday, March 01, 2014
* @brief
*
*
* @details
*
*
*
* @note
*
*/


#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_
#define ZCE_LIB_ASYNC_FRAMEWORK_BASE_


#include "zce_boost_lord_rings.h"
#include "zce_os_adapt_coroutine.h"
#include "zce_timer_handler_base.h"
#include "zce_trace_log_debug.h"

/*!
* @brief      异步对象的基类
*
*/
class ZCE_Async_Object
{
    friend class ZCE_Async_ObjectMgr;

public:
    ZCE_Async_Object(ZCE_Async_ObjectMgr *async_mgr);
protected:
    ~ZCE_Async_Object();

public:


    /*!
    * @brief      初始化函数，在构造函数后调用，在放入池子前执行一次，
    * @return     int 0标识成功
    */
    virtual int initialize(unsigned int reg_cmd);


    /*!
    * @brief      结束销毁函数，在析构前的调用
    * @return     int
    */
    virtual int finish();

    /*!
    * @brief      克隆自己
    * @return     ZCE_Async_Object*
    * @param      async_mgr
    */
    virtual ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr) = 0;

    /*!
    * @brief      异步对象开始
    */
    virtual void on_start();

    /*!
    * @brief      异步对象运行
    * @param[out] continue_run 异步对象是否继续运行，如果不继续(返回false)，
    */
    virtual void on_run(bool &continue_run) = 0;


    /*!
    * @brief      异步对象超时处理
    * @param[in]  now_time  发生超时的时间，
    * @param[out] continue_run 异步对象是否继续运行,
    */
    virtual void on_timeout(const ZCE_Time_Value &now_time,
                            bool &continue_run) = 0;

    /*!
    * @brief      异步对象运行结束，做结束，释放资源的事情
    *             目前这个类做的事情主要是清理定时器
    */
    virtual void on_end();


protected:


    /*!
    * @brief                   设置超时定时器,如果触发，回调函数
    * @param  time_out         超时的时间，相对时间，
    */
    int set_timeout(const ZCE_Time_Value &time_out);


    /*!
    * @brief      取消超时定时器
    */
    void cancel_timeout();


protected:

    ///TIME ID
    static const int ASYNCOBJ_ACTION_ID[2];

protected:

    ///异步对象ID
    unsigned int asyncobj_id_;

    ///管理者
    ZCE_Async_ObjectMgr *async_mgr_;

    ///对应激活的处理的命令
    unsigned int  create_cmd_;

    ///超时的定时器ID
    int timeout_id_;

};


//=======================================================================================

class ZCE_Timer_Queue;

/*!
* @brief      异步对象的管理器基类
*
*/
class ZCE_Async_ObjectMgr : public ZCE_Timer_Handler
{

protected:

    ///异步对象池子，
    typedef ZCE_LIB::lordrings<ZCE_Async_Object *>  ASYNC_OBJECT_POOL;

    ///异步对象记录
    struct ASYNC_OBJECT_RECORD
    {
        ASYNC_OBJECT_RECORD();
        ~ASYNC_OBJECT_RECORD();

        //异步对象池子，
        ASYNC_OBJECT_POOL aysncobj_pool_;


        //下面是统计信息

        //创建的事务的数量
        uint64_t create_num_;

        //销毁时状态异常的事务数量
        uint64_t active_num_;

        //正常结束的数量
        uint64_t end_num_;

        ///强行结束的数量
        uint64_t force_end_num_;

        //运行过程发生超时的数量
        uint64_t timeout_num_;


        //运行总消耗时间
        uint64_t run_consume_ms_;
    };

    //
    typedef std::unordered_map<unsigned int, ASYNC_OBJECT_RECORD> ID_TO_REGASYNC_POOL_MAP;
    //
    typedef std::unordered_map<unsigned int, ZCE_Async_Object * > RUNNING_ASYNOBJ_MAP;

public:

    ///异步对象管理器的构造函数
    ZCE_Async_ObjectMgr(ZCE_Timer_Queue *timer_queue);
    virtual ~ZCE_Async_ObjectMgr();

    /*!
    * @brief      初始化，控制各种池子，容器的大小
    * @return     int
    * @param      crtn_type_num
    * @param      running_number
    */
    int initialize(size_t crtn_type_num = DEFUALT_ASYNC_TYPE_NUM,
                   size_t running_number = DEFUALT_RUNNIG_ASYNC_SIZE);


    /*!
    * @brief      结束所有的协程处理，回收释放资源，
    *             打印统计信息，检查是否有泄漏等
    */
    void finish();


    /*!
    * @brief      注册一类协程，其用reg_cmd对应，
    * @return     int
    * @param      reg_cmd
    * @param      async_base
    * @param      init_clone_num
    */
    int register_asyncobj(unsigned int reg_cmd,
                          ZCE_Async_Object *async_base);

    /*!
    * @brief      创建一个异步对象
    * @return     int
    * @param      cmd
    * @param      id
    */
    int create_asyncobj(unsigned int cmd, unsigned int *id);


    /*!
    * @brief      激活某个已经运行的异步对象,
    * @return     int
    * @param      id
    */
    int active_asyncobj(unsigned int id);


    
    /*!
    * @brief      打印管理器的基本信息，运行状态
    * @param      log_priority   日志的优先级，以此优先级进行输出
    */
    void dump_info(ZCE_LOG_PRIORITY log_priority) const;

protected:

    /*!
    * @brief      从池子里面分配一个异步对象
    * @return     int
    * @param      cmd
    * @param      async_rec
    * @param      alloc_aysnc
    */
    int allocate_from_pool(unsigned int cmd,
                           ASYNC_OBJECT_RECORD *&async_rec,
                           ZCE_Async_Object *&alloc_aysnc);

    /*!
    * @brief      释放一个异步对象到池子里面
    * @return     int
    * @param      free_async
    */
    int free_to_pool(ZCE_Async_Object *free_async);


    /*!
    * @brief      通过ID，寻找一个正在运行的异步对象
    * @return     int  返回0表示成功
    * @param[in]  id   运行的异步对象的标识ID
    * @param[out] running_aysnc 查询到的异步对象
    */
    int find_running_asyncobj(unsigned int id, ZCE_Async_Object *&running_aysnc);



    /*!
    * @brief      定时器触发的处理函数
    * @return     int
    * @param      now_time
    * @param      act
    */
    int handle_timeout(const ZCE_Time_Value &now_time,
                       const void *act);

protected:

    ///默认的异步对象类型数量
    static const size_t DEFUALT_ASYNC_TYPE_NUM = 2048;
    ///默认同时运行的一部分对象的数量
    static const size_t DEFUALT_RUNNIG_ASYNC_SIZE = 2048;



public:
    ///无效的事务ID
    static const unsigned int INVALID_IDENTITY = 0;
    ///无效的的命令
    static const unsigned int INVALID_COMMAND = 0;

protected:

    //事务ID发生器
    unsigned int id_builder_;

    //协程的池子，都是注册进来的
    ID_TO_REGASYNC_POOL_MAP regaysnc_pool_;

    ///正在运行的协程
    RUNNING_ASYNOBJ_MAP running_aysncobj_;

    ///异步对象池子的初始化大小，
    size_t  pool_init_size_;

    ///异步对象池子的每次扩大的数量
    size_t  pool_extend_size_;
};

#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_

