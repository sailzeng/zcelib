#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_
#define SOARING_LIB_SVRD_APP_TRNAS_H_

#include "soar/svrd/app_bus.h"


/*!
* @brief      状态机的APP
*
* @note
*/
class SvrdApp_FSM : public soar::App_BusPipe
{
protected:

    //使用事务处理的Application
    SvrdApp_FSM();
    virtual ~SvrdApp_FSM();

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

#endif //#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_
