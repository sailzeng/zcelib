#pragma once

namespace zerg
{
/*!
* @brief      ZERG服务的定时器，定时器用于各种处理
*
* @note
*/
class  app_timer : public  soar::server_timer
{
public:
    //
    app_timer();
protected:
    //自己管理自己的清理
    ~app_timer();

    ///定时器触发
    virtual int timer_timeout(const zce::time_value& time,
                              int timer_id);
};
}
