#pragma once

#include "zce/async/async_base.h"
#include "zce/script/lua_tie.h"

//====================================================================================
namespace zce
{
/*!
* @brief      协程对象
*             注意，为了避免一些无意义的暴漏，我这儿选择的继承方式是private
*/
class Async_LuaThead : public zce::Async_Object
{
    friend class Async_LuaTheadMgr;

    //
    enum class COROUTINE_STATE
    {
        //
        INVALID = 0x0,
        //携程传递给管理器的状态值，
        CONTINUE = 0x10001,
        EXIT = 0x10002,

        //超时后，管理器通知携程的状态值
        TIMEOUT = 0x20002,
    };

public:
    /*!
    * @brief      构造函数，
    * @param      async_mgr ,协程异步管理器的指针
    */
    Async_LuaThead(zce::Async_Obj_Mgr* async_mgr, unsigned int reg_cmd);
protected:
    /*!
    * @brief      析构函数
    */
    ~Async_LuaThead();

public:

    /*!
    * @brief      初始化函数，在构造函数后调用，在放入池子前执行一次，
    * @return     int 0标识成功
    */
    virtual int initialize() override;

    /*!
    * @brief      结束销毁函数，在析构前的调用
    * @return     int
    */
    virtual void terminate() override;

protected:

    /*!
    * @brief      LUA Thread 的运行
    *             继承zce::Async_Object的函数，
    */
    virtual void on_run(const void* recv_data,
                        size_t data_len,
                        bool& running) override;

    /*!
    * @brief      异步对象超时处理
    *             继承zce::Async_Object的函数，
    */
    virtual void on_timeout(const zce::Time_Value& now_time,
                            bool& running) override;

protected:

    ///
    zce::Lua_Tie  *mgr_lua_tie_ = nullptr;

    ///Lua协程对象
    zce::Lua_Thread lua_thread_;

    ///协程的状态
    COROUTINE_STATE  coroutine_state_ = COROUTINE_STATE::INVALID;
};

//====================================================================================

/*!
* @brief      协程对象主控管理类
*
*/
class Async_LuaTheadMgr : public zce::Async_Obj_Mgr
{
public:

    //
    Async_LuaTheadMgr();
    virtual ~Async_LuaTheadMgr();

    inline zce::Lua_Tie  *mgr_lua_tie()
    {
        return &mgr_lua_tie_;
    }
protected:

    ///默认异步对象池子的初始化的数量
    static const size_t COROUTINE_POOL_INIT_SIZE = 4;
    ///默认池子扩展的时候，扩展的异步对象的数量
    static const size_t COROUTINE_POOL_EXTEND_SIZE = 128;

    //管理器的lua state
    zce::Lua_Tie  mgr_lua_tie_;
};
}