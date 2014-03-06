

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_
#define ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_


#include "zce_async_fw_base.h"

//====================================================================================


/*!
* @brief      协程对象
*             
*/
class ZCE_Async_Coroutine :public ZCE_Async_Object
{
    friend class ZCE_Async_CoroutineMgr;

    //
    enum COROUTINE_STATE
    {
        COROUTINE_CONTINUE = 0x10001,
        COROUTINE_EXIT = 0x20001,
    };

public:
    ZCE_Async_Coroutine(ZCE_Async_CoroutineMgr *async_mgr);
protected:
    ~ZCE_Async_Coroutine();

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




    ///协程对象的运行函数
    void coroutine_do();
    
    ///切换回Main，协程还会继续运行
    void yeild_main_continue();

    ///切换回Main,协程退出
    void yeild_main_exit();

    ///切换回协程，也就是切换到他自己运行
    void yeild_coroutine();

    ///协程运行
    virtual int coroutine_run() = 0;


    /*!
    * @brief
    * @return     void
    * @param      continue_run
    */
    virtual void on_run(bool &continue_run);
    
protected:
    
    ///最小的堆栈
    static const size_t MIN_STACK_SIZE = 16 * 1024;
    ///默认堆栈
    static const size_t DEF_STACK_SIZE = 64 * 1024;
    ///最大的堆栈
    static const size_t MAX_STACK_SIZE = 256 * 1024;

public:

    ///static 函数，用于协程运行函数，调用协程对象的运行函数
    static void static_do(ZCE_Async_Coroutine *coroutine);

protected:
    
    ///协程对象
    coroutine_t      handle_;

    ///协程的堆栈大小，
    size_t           stack_size_;

    ///协程的状态
    COROUTINE_STATE  coroutine_state_;
};

//====================================================================================

/*!
* @brief      （协程）主控管理类
*             
*/
class ZCE_Async_CoroutineMgr :public ZCE_Async_ObjectMgr
{
public:

    //
    ZCE_Async_CoroutineMgr(ZCE_Timer_Queue *timer_queue);
    virtual ~ZCE_Async_CoroutineMgr();
    

};


#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_

