/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce/async/fw_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Saturday, March 01, 2014
* @brief      一套很简单的异步框架的基类，已经扩展出了FSM和协程的两个子类，
*             Lua的协程如果我想封装应该也不难，Lua代码部分我已经没有障碍。
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


#include "zce/util/lord_rings.h"
#include "zce/os_adapt/coroutine.h"
#include "zce/timer/handler_base.h"
#include "zce/logger/logging.h"
#include "zce/timer/queue_base.h"

namespace zce
{

class Async_Obj_Mgr;

/*!
* @brief      异步对象的基类
*
*/
class Async_Object
{
    friend class Async_Obj_Mgr;

public:
    Async_Object(Async_Obj_Mgr* async_mgr,
                 uint32_t create_cmd);
protected:
    virtual ~Async_Object();

public:


    /*!
    * @brief      初始化函数，在构造函数后调用，在放入池子前执行一次，
    *             为什么不妨到构造函数，因为担心有些初始化会有不成功的情况，
    * @return     int 0标识成功
    */
    virtual int initialize();


    /*!
    * @brief      结束销毁函数，在析构前的调用，其实就是对应上面
    * @return     int
    */
    virtual void finish();

    /*!
    * @brief      克隆自己
    * @return     zce::Async_Object*
    * @param      async_mgr
    */
    virtual Async_Object* clone(Async_Obj_Mgr* async_mgr,
                                uint32_t reg_cmd) = 0;

    /*!
    * @brief      异步对象开始,可以用来做每次重新进行初始化时候的事情
    */
    virtual void on_start();

    /*!
    * @brief      异步对象运行
    * @param[out] continue_run 异步对象是否继续运行，如果不继续(返回false)，
    */
    virtual void on_run(const void* outer_data,bool& continue_run) = 0;


    /*!
    * @brief      异步对象超时处理
    * @param[in]  now_time  发生超时的时间，
    * @param[out] continue_run 异步对象是否继续运行,
    */
    virtual void on_timeout(const ZCE_Time_Value& now_time,
                            bool& continue_run) = 0;

    /*!
    * @brief      异步对象运行结束，做结束，释放资源的事情
    *             目前这个类做的事情主要是清理定时器
    */
    virtual void on_end();


    /*!
    * @brief      记录处理过程发生的错误
    * @param      error_no
    */
    void set_errorno(int error_no);

    /*!
    * @brief      保存接收到外部数据的指针，
    */
    inline void receive_data(const void* recv_data)
    {
        receive_data_ = recv_data;
    }

protected:

    /*!
    * @brief                   设置超时定时器,如果触发，回调函数
    * @param  time_out         超时的时间，相对时间，
    */
    int set_timeout(const ZCE_Time_Value& time_out);


    /*!
    * @brief      取消超时定时器
    */
    void cancel_timeout();



protected:

    ///TIME ID
    static const int ASYNCOBJ_ACTION_ID[2];

protected:

    ///异步对象ID
    unsigned int asyncobj_id_ = 0;

    ///管理者
    Async_Obj_Mgr* async_mgr_ = NULL;

    ///对应激活的处理的命令
    uint32_t  create_cmd_ = 0;

    ///超时的定时器ID
    int timeout_id_ = Timer_Queue_Base::INVALID_TIMER_ID;

    /// 异步对象处理的错误码，统计事物错误的时候使用
    int running_errno_ = 0;

    ///日志跟踪的优先级
    zce::LOG_PRIORITY trace_log_pri_ = RS_INFO;

    ///传递的外部数据，
    const void* receive_data_ = NULL;
};


//=======================================================================================

class Timer_Queue_Base;

/*!
* @brief      异步对象的管理器基类
*
*/
class Async_Obj_Mgr: public zce::Timer_Handler
{

protected:

    ///异步对象池子，
    typedef zce::lordrings<Async_Object*>  ASYNC_OBJECT_POOL;
    ///异步对象记录
    struct ASYNC_OBJECT_RECORD
    {
        //异步对象池子，
        ASYNC_OBJECT_POOL aysncobj_pool_;

        //下面是统计信息

        //创建的事务的数量
        uint64_t create_num_ = 0;

        //销毁时状态异常的事务数量
        uint64_t active_num_ = 0;

        //正常结束的数量
        uint64_t end_num_ = 0;

        ///强行结束的数量
        uint64_t force_end_num_ = 0;

        //运行过程发生超时的数量
        uint64_t timeout_num_ = 0;

        //运行总消耗时间
        uint64_t run_consume_ms_ = 0;
    };

    
    //异步对象记录池子（包括异步对象和记录信息）
    typedef std::unordered_map<uint32_t,ASYNC_OBJECT_RECORD> ASYNC_RECORD_POOL;
    //
    typedef std::unordered_map<uint32_t,zce::Async_Object* > RUNNING_ASYNOBJ_MAP;

public:

    ///异步对象管理器的构造函数
    Async_Obj_Mgr();
    virtual ~Async_Obj_Mgr();

    /*!
    * @brief      初始化，控制各种池子，容器的大小
    * @return     int
    * @param      crtn_type_num
    * @param      running_number
    * @param      init_lock_pool 初始化
    */
    int initialize(zce::Timer_Queue_Base* tq,
                   size_t crtn_type_num,
                   size_t running_number);


    /*!
    * @brief      结束所有的协程处理，回收释放资源，
    *             打印统计信息，检查是否有泄漏等
    */
    void finish();


    /*!
    * @brief      注册一类协程，其用reg_cmd对应，
    * @return     int
    * @param      create_cmd
    * @param      async_base
    * @param      init_clone_num
    */
    int register_asyncobj(uint32_t create_cmd,
                          zce::Async_Object* async_base);

    /*!
    * @brief      判断某个命令是否是注册（创建）异步对象命令
    * @return     bool
    * @param      cmd
    */
    bool is_register_cmd(uint32_t cmd);

    /*!
    * @brief      创建一个异步对象
    * @return     int
    * @param      cmd         创建的命令，如果是注册命令，会创建一个异步对象进行处理
    * @param      outer_data  外部数据，带给异步对象，给他处理
    * @param      id          返回参数，内部创建异步对象的ID，
    */
    int create_asyncobj(uint32_t cmd,void* outer_data,unsigned int* id);


    /*!
    * @brief      激活某个已经运行的异步对象,
    * @return     int
    * @param      outer_data
    * @param      id
    */
    int active_asyncobj(unsigned int id,void* outer_data);

    /*!
    * @brief      打印管理器的基本信息，运行状态
    * @param      log_priority   日志的优先级，以此优先级进行输出
    */
    void dump_info(zce::LOG_PRIORITY log_priority) const;

protected:

    /*!
    * @brief      从池子里面分配一个异步对象
    * @return     int
    * @param      cmd
    * @param      async_rec
    * @param      alloc_aysnc
    */
    int allocate_from_pool(uint32_t cmd,
                           ASYNC_OBJECT_RECORD*& async_rec,
                           zce::Async_Object*& alloc_aysnc);

    /*!
    * @brief      释放一个异步对象到池子里面
    * @return     int
    * @param      free_async
    */
    int free_to_pool(zce::Async_Object* free_async);


    /*!
    * @brief      通过ID，寻找一个正在运行的异步对象
    * @return     int  返回0表示成功
    * @param[in]  id   运行的异步对象的标识ID
    * @param[out] running_aysnc 查询到的异步对象
    */
    int find_running_asyncobj(unsigned int id,zce::Async_Object*& running_aysnc);



    /*!
    * @brief      定时器触发的处理函数
    * @return     int
    * @param      now_time
    * @param      act
    */
    int timer_timeout(const ZCE_Time_Value& now_time,
                      const void* act);



protected:
    ///默认的异步对象类型数量
    static const size_t DEFUALT_ASYNC_TYPE_NUM = 2048;
    ///默认同时运行的一部分对象的数量
    static const size_t DEFUALT_RUNNIG_ASYNC_SIZE = 256 * 1024;
public:
    ///无效的事务ID
    static const unsigned int INVALID_IDENTITY = 0;
    ///无效的的命令
    static const unsigned int INVALID_COMMAND = 0;

protected:

    //事务ID发生器
    unsigned int id_builder_ = 1;

    //异步对象的池子，都是注册进来的
    ASYNC_RECORD_POOL regaysnc_pool_;

    ///正在运行的协程
    RUNNING_ASYNOBJ_MAP running_aysncobj_;

    ///异步对象池子的初始化大小，
    size_t  pool_init_size_ = DEFUALT_ASYNC_TYPE_NUM;

    ///异步对象池子的每次扩大的数量
    size_t  pool_extend_size_ = DEFUALT_RUNNIG_ASYNC_SIZE;


};

}

#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_

