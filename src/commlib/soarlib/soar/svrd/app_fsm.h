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
    int app_start(int argc, const char* argv[]) override;

    //运行处理,
    int app_run() override;

    //退出处理
    int app_exit() override;

protected:

    virtual int register_trans_cmd() = 0;
};
