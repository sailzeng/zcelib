#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_
#define SOARING_LIB_SVRD_APP_TRNAS_H_

#include "soar/svrd/app_bus.h"

#define REGISTER_TRANSACTION_CMD(trans_class, cmd_word ) \
    { \
        int __ret_register_ = Transaction_Manager::instance()->register_trans_cmd(cmd_word, \
                                                                                  new trans_class(ZCE_Timer_Queue::instance(),Transaction_Manager::instance())); \
        if (__ret_register_ !=  0) \
        { \
            return __ret_register_; \
        } \
    }

/*!
* @brief      状态机的APP
*
* @note
*/
class Comm_SvrdApp_FSM : public soar::App_Buspipe
{
protected:

    //使用事务处理的Application
    Comm_SvrdApp_FSM();
    virtual ~Comm_SvrdApp_FSM();

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
