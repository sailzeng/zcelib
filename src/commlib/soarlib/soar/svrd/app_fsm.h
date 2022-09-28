#pragma once

#include "soar/svrd/app_bus.h"

/*!
* @brief      状态机的APP
*
* @note
*/
class svrd_fsm : public soar::app_buspipe
{
protected:

    //使用事务处理的Application
    svrd_fsm();
    virtual ~svrd_fsm();

public:

    //增加调用register_func_cmd
    virtual int app_start(int argc, const char* argv[]);

    //运行处理,
    virtual int app_run();

    //退出处理
    virtual int app_exit();

protected:

    virtual int register_trans_cmd() = 0;
};
