/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   fsm.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年4月21日
* @brief      这儿更多的是一个示例。
*
*
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_FSM_
#define ZCE_LIB_ASYNC_FRAMEWORK_FSM_

#include "zce/async/async_base.h"

//====================================================================================

namespace zce
{
/*!
* @brief      状态机对象
*/
class Async_FSM : public Async_Object
{
    friend class Async_FSMMgr;

public:
    /*!
    * @brief      构造函数，
    * @param      async_mgr ,协程异步管理器的指针
    */
    Async_FSM(zce::Async_Obj_Mgr* async_mgr,
              uint32_t create_cmd);
protected:
    /*!
    * @brief      析构函数
    */
    ~Async_FSM();

    /*!
    * @brief      状态机运行处理，继承zce::Async_Object的函数，此函数仅仅用于作为例子进行参考，
    * @param[in]  outer_data 传入参数，外部调用数据
    * @param[out] continue_run 返回参数，返回当前的协程是否要继续运行下去
    */
    virtual void on_run(bool& running) = 0;

    /*!
    * @brief      状态机对象超时处理，默认continue_run返回false，让系统回收，
    * @param[in]  now_time  发生超时的时间，
    * @param[out] continue_run 异步对象是否继续运行,
    */
    virtual void on_timeout(const zce::Time_Value& now_time,
                            bool& running) override;

    /*!
    * @brief      设置的状态机阶段，
    * @param      stage
    */
    void set_stage(int stage);

    /*!
    * @brief      取得的状态机阶段
    * @return     int
    */
    int get_stage() const;

protected:

    ///状态机的阶段
    int               fsm_stage_;
};

//====================================================================================

/*!
* @brief      状态机主控管理类
*
*/
class Async_FSMMgr : public zce::Async_Obj_Mgr
{
public:

    //
    Async_FSMMgr();
    virtual ~Async_FSMMgr();

protected:

    ///默认状态机对象池子的初始化的数量,状态机不太消耗内存，可以追求更好的性能
    static const size_t FSM_POOL_INIT_SIZE = 4;
    ///默认状态机扩展的时候，扩展的异步对象的数量
    static const size_t FSM_POOL_EXTEND_SIZE = 256;
};
}

#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_FSM_
