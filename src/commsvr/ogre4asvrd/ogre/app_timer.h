#pragma once

namespace ogre
{
class  app_timer : public soar::server_timer
{
public:
    //
    app_timer();
protected:
    ~app_timer();

    //
    virtual int timer_timeout(const zce::time_value& time, const void* arg);

public:
    //定时器ID,避免New传递,回收
    static const  int      OGRE_APP_TIME_ID[];
};
} //namespace ogre
