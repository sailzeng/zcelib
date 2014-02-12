

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_
#define ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_


#include "zce_boost_lord_rings.h"
#include "zce_os_adapt_coroutine.h"

//------------------------------------------------------------------------------------


/*!
* @brief      协程对象
*             
*/
class ZCE_CRTNAsync_Coroutine
{
    friend class ZCE_CRTNAsync_Main;
public:

    ///协程的状态枚举
    enum STATE_COROUTINE
    {
        STATE_RUNNIG  = 1,
        STATE_END,
        STATE_TIMEOUT,
        STATE_EXIT,
    };

public:
    ZCE_CRTNAsync_Coroutine();
protected:
    ~ZCE_CRTNAsync_Coroutine();

public:

    ///取得协程的句柄
    coroutine_t *get_handle();

    ///协程对象的运行函数
    void coroutine_do();
    
    ///切换回Main
    void switch_to_main();

    ///切换回协程，也就是切换到他自己运行
    void switch_to_coroutine();

    ///设置协程的状态
    void set_state(ZCE_CRTNAsync_Coroutine::STATE_COROUTINE state);

    ///协程启动，做初始化工作
    virtual int coroutine_start();

    ///协程运行
    virtual int coroutine_run() = 0;

    ///协程结束，做结束，释放资源的事情
    virtual int coroutine_end();
    
    ///
    ZCE_CRTNAsync_Coroutine *clone();

    ///
    inline void set_command(unsigned int cmd);
    ///

public:

    ///static 函数，用于协程运行函数，调用协程对象的运行函数
    static void static_do(ZCE_CRTNAsync_Coroutine *);

protected:

    ///
    coroutine_t      handle_;

    ///对应处理的命令
    unsigned int     command_;

    ///
    unsigned int     coroutine_id_;

    ///
    STATE_COROUTINE  state_;
    

};

//------------------------------------------------------------------------------------


/*!
* @brief      （协程）主控管理类
*             
*/
class ZCE_CRTNAsync_Main
{

protected:
    
    //
    typedef ZCE_LIB::lordrings<ZCE_CRTNAsync_Coroutine *>                REG_COROUTINE_POOL;
    //
    typedef std::unordered_map<unsigned int, REG_COROUTINE_POOL>         ID_TO_REGCOR_POOL_MAP;

    //
    typedef std::unordered_map<unsigned int, ZCE_CRTNAsync_Coroutine * > CMD_TO_COROUTINE_MAP;

public:

    //
    ZCE_CRTNAsync_Main();
    virtual ~ZCE_CRTNAsync_Main();


    /*!
    * @brief      初始化，控制各种池子，容器的大小
    * @return     int
    * @param      crtn_type_num
    * @param      running_number
    * @note       
    */
    int initialize(size_t crtn_type_num = DEFUALT_CRTN_TYPE_NUM,
        size_t running_number = DEFUALT_RUNNIG_CRTN_SIZE);

    ///注册一类协程，其用reg_cmd对应，
    int register_coroutine(unsigned int reg_cmd,
        ZCE_CRTNAsync_Coroutine* coroutine_base,
        size_t init_clone_num,
        size_t stack_size);

    ///激活一个协程
    int active_coroutine(unsigned int cmd,unsigned int *id);

    ///切换到ID对应的那个线程
    int switch_to_coroutine(unsigned int id);

protected:

    ///分配一个协程
    int allocate_coroutine(unsigned int cmd);

    ///释放一个协程
    int free_coroutine(unsigned int cmd);
protected:


protected:

    ///
    static const size_t DEFUALT_CRTN_TYPE_NUM = 512;
    ///
    static const size_t DEFUALT_INIT_POOL_SIZE = 8;
    ///
    static const size_t DEFUALT_RUNNIG_CRTN_SIZE = 1024;
    ///
    static const size_t DEFUALT_POOL_ADD_CRTN_SIZE = 256;

protected:
    
    //事务ID发生器
    unsigned int           corout_id_builder_;

    //协程的池子，都是注册进来的
    ID_TO_REGCOR_POOL_MAP  coroutine_pool_;

    ///正在运行的协程
    CMD_TO_COROUTINE_MAP   running_coroutine_;


};


#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_

