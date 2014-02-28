

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_
#define ZCE_LIB_ASYNC_FRAMEWORK_BASE_


#include "zce_boost_lord_rings.h"
#include "zce_os_adapt_coroutine.h"
#include "zce_timer_handler_base.h"

/*!
* @brief      异步对象的基类
*
*/
class ZCE_Async_Object :public ZCE_Timer_Handler
{
    friend class ZCE_Async_ObjectMgr;
public:

    ///协程的状态枚举
    enum RUNNING_STATE
    {
        STATE_RUNNIG = 1,
        STATE_TIMEOUT = 2,
        STATE_EXIT = 3,
        STATE_FORCE_EXIT = 4,
    };

public:
    ZCE_Async_Object(ZCE_Async_ObjectMgr *async_mgr);
protected:
    ~ZCE_Async_Object();

public:
    

    ///协程初始化的工作，在放入池子前执行一次，
    virtual int initialize();
    ///
    virtual ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr) = 0;

    ///异步对象开始
    virtual void on_start();

    ///异步对象运行
    virtual void on_run(bool &continue_run) = 0;

    ///异步对象运行结束，做结束，释放资源的事情
    virtual void on_finish();


protected:


    /*!
    @brief                   设置超时定时器,如果触发，回调函数on_timeout
    @param  sec              超时的秒
    @param  usec             超时的微秒数
    */
    int set_timeout_timer(int sec, int usec = 0);

    /*!
    @brief                   设置触发定时器,如果触发，回调函数on_timetouch
    @param  sec              超时的秒
    @param  usec             超时的微秒数
    */
    int set_timetouch_timer(int sec, int usec = 0);


    /*!
    * @brief      取消超时定时器
    */
    void cancel_timeout_timer();

    /*!
    * @brief      取消触发定时器
    */
    void cancel_touch_timer();
    
protected:

    ///异步对象ID
    unsigned int asyncobj_id_;

    ///管理者
    ZCE_Async_ObjectMgr *async_mgr_;

    ///对应激活的处理的命令
    unsigned int  active_cmd_;

    ///运行状态
    RUNNING_STATE  running_state_;

    //超时的定时器ID
    int timeout_id_;

    //触发的定时器ID
    int touchtimer_id_;
};


class ZCE_Timer_Queue;

/*!
* @brief      异步对象的管理器基类
*
*/
class ZCE_Async_ObjectMgr
{

protected:

    ///异步对象池子，
    typedef ZCE_LIB::lordrings<ZCE_Async_Object *>  ASYNC_OBJECT_POOL;

    ///异步对象记录
    struct ASYNC_OBJECT_RECORD
    {
        //异步对象池子，
        ASYNC_OBJECT_POOL coroutine_pool_;


        //下面是统计信息

        //创建的事务的数量
        uint64_t start_num_;

        //正常结束的数量
        uint64_t end_num_;

        ///强行结束的数量
        uint64_t force_end_num_;

        //运行过程发生超时的数量
        uint64_t timeout_num_;

        //销毁时状态异常的事务数量
        uint64_t exception_num_;

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
    * @brief      注册一类协程，其用reg_cmd对应，
    * @return     int 
    * @param      reg_cmd
    * @param      async_base
    * @param      init_clone_num
    */
    int register_asyncobj(unsigned int reg_cmd,
        ZCE_Async_Object* async_base,
        size_t init_clone_num);

    /*!
    * @brief      创建一个异步对象
    * @return     int
    * @param      cmd
    * @param      id
    */
    int create_asyncobj(unsigned int cmd, unsigned int *id);


    /*!
    * @brief      激活某个已经运行的异步对象
    * @return     int
    * @param      id 
    */
    int active_asyncobj(unsigned int id) = 0;


    /*!
    * @brief      取得定时器管理器，
    * @return     ZCE_Timer_Queue*
    */
    ZCE_Timer_Queue * get_timer_queue();
    
protected:

    /*!
    * @brief      从池子里面分配一个异步对象
    * @return     int
    * @param      cmd
    * @param      alloc_aysnc
    */
    int allocate_from_pool(unsigned int cmd, ZCE_Async_Object *&alloc_aysnc);

    /*!
    * @brief      释放一个异步对象到池子里面
    * @return     int
    * @param      free_async
    */
    int free_to_pool(ZCE_Async_Object *free_async);

    
    /*!
    * @brief      通过ID，寻找一个正在运行的异步对象
    * @return     int
    * @param[in]  id
    * @param[out] running_aysnc
    */
    int find_running_asyncobj(unsigned int id, ZCE_Async_Object *&running_aysnc);


    

protected:

    ///默认的异步对象类型数量
    static const size_t DEFUALT_ASYNC_TYPE_NUM = 1024;
    ///每类异步对象池子的初始化的数量
    static const size_t DEFUALT_INIT_POOL_SIZE = 2;
    ///默认同时运行的一部分对象的数量
    static const size_t DEFUALT_RUNNIG_ASYNC_SIZE = 1024;
    ///默认池子扩展的时候，扩展的异步对象的数量
    static const size_t POOL_EXTEND_ASYNC_NUM = 128;

public:
    ///无效的事务ID
    static const unsigned int INVALID_IDENTITY = 0;

protected:

    //事务ID发生器
    unsigned int id_builder_;

    //协程的池子，都是注册进来的
    ID_TO_REGASYNC_POOL_MAP aysncobj_pool_;

    ///正在运行的协程
    RUNNING_ASYNOBJ_MAP running_aysncobj_;

    ///定时器的管理器
    ZCE_Timer_Queue *timer_queue_;

};

#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_

