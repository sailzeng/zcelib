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
class async_luathead : public zce::async_object
{
    friend class async_luathead_mgr;

public:
    /*!
    * @brief      构造函数，
    * @param      async_mgr ,协程异步管理器的指针
    */
    async_luathead(zce::async_obj_mgr* async_mgr,
                   uint32_t reg_cmd);
protected:
    /*!
    * @brief      析构函数
    */
    ~async_luathead();

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
    virtual void on_run(bool first_run, bool& continue_run) override;

    /*!
    * @brief      异步对象超时处理
    *             继承zce::Async_Object的函数，
    */
    virtual void on_timeout(const zce::time_value& now_time,
                            bool& continued) override;

    virtual int luathread_run() = 0;

protected:

    ///
    zce::Lua_Tie* mgr_lua_tie_ = nullptr;

    ///Lua协程对象
    zce::Lua_Thread lua_thread_;
};

//====================================================================================

/*!
* @brief      协程对象主控管理类
*
*/
class async_luathead_mgr : public zce::async_obj_mgr
{
public:

    //
    async_luathead_mgr();
    virtual ~async_luathead_mgr();

    inline zce::Lua_Tie* mgr_lua_tie()
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