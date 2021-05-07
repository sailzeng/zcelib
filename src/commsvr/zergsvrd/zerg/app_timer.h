#pragma once

namespace zerg
{
/*!
* @brief      ZERG服务的定时器，定时器用于各种处理
*
* @note
*/
class  App_Timer: public  Server_Timer_Base
{
public:
    //
    App_Timer();
protected:
    //自己管理自己的清理
    ~App_Timer();

    ///定时器触发
    virtual int timer_timeout(const zce::Time_Value &time,const void *arg);

public:

    ///ZERG服务器定时器ID,
    static const  int  ZERG_TIMER_ID[];

protected:

    //定时器触发的随机偏移, 避免所有服务器同时触发
    unsigned int random_reload_point_;
};
}
