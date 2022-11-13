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
    virtual int timer_timeout(const zce::time_value& time,
                              int timer_id);
};
} //namespace ogre
