

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_
#define ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_


#include "zce_boost_lord_rings.h"
#include "zce_os_adapt_coroutine.h"

//------------------------------------------------------------------------------------

class ZCE_CRTNAsync_Coroutine
{

public:

    enum STATE_COROUTINE
    {
        STATE_RUNNIG  = 1,
        STATE_ENDRUN,
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

    ///
    void set_state(ZCE_CRTNAsync_Coroutine::STATE_COROUTINE state);

    ///协程启动，做初始化工作
    virtual int coroutine_start();

    ///协程运行
    virtual int coroutine_run() = 0;

    ///协程结束，做结束，释放资源的事情
    virtual int coroutine_end();
    
public:

    ///static 函数，用于协程运行函数，调用协程对象的运行函数
    static void coroutine_do(ZCE_CRTNAsync_Coroutine *);

protected:

    ///
    coroutine_t      handle_;

    ///
    STATE_COROUTINE  state_;

};

//------------------------------------------------------------------------------------

class ZCE_CRTNAsync_Main
{

protected:
    
    //
    typedef ZCE_LIB::lordrings<ZCE_CRTNAsync_Coroutine *>                REG_COROUTINE_POOL;
    //
    typedef std::unordered_map<unsigned int, REG_COROUTINE_POOL* >       ID_TO_REGCOR_POOL_MAP;

    //
    typedef std::unordered_map<unsigned int, ZCE_CRTNAsync_Coroutine * > CMD_TO_COROUTINE_MAP;

public:

    ZCE_CRTNAsync_Main();
    virtual ~ZCE_CRTNAsync_Main();


    ///
    int initialize(size_t type_num,
        size_t coroutine_num);

    ///
    int register_cmd(unsigned int reg_cmd,
        ZCE_CRTNAsync_Coroutine* coroutine_base,
        size_t init_coroutine_num);


    ///
    void switch_to_coroutine();

protected:
    
    //事务ID发生器
    unsigned int           corout_id_builder_;
    
    //
    CMD_TO_COROUTINE_MAP   id_to_coroutine_;

    //注册进来的协程
    ID_TO_REGCOR_POOL_MAP  cmd_clone_corout_;

    ///正在运行的协程
    ID_TO_REGCOR_POOL_MAP  running_coroutine_;


};


#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_

