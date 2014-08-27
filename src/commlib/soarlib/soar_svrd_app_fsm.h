
#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_
#define SOARING_LIB_SVRD_APP_TRNAS_H_

#include "soar_svrd_app_base.h"

#define REGISTER_TRANSACTION_CMD(trans_class, cmd_word ) \
    { \
        int __ret_register_ = Transaction_Manager::instance()->register_trans_cmd(cmd_word, \
                                                                                  new trans_class(ZCE_Timer_Queue::instance(),Transaction_Manager::instance())); \
        if (__ret_register_ !=  SOAR_RET::SOAR_RET_SUCC) \
        { \
            return __ret_register_; \
        } \
    }

class Comm_SvrdApp_Transaction : public Comm_Svrd_Appliction
{

protected:

    //使用事务处理的Application
    Comm_SvrdApp_Transaction();
    ~Comm_SvrdApp_Transaction();

public:

    //增加调用register_func_cmd
    virtual int init_instance();

    //运行处理,
    virtual int run_instance();

    //退出处理
    virtual int exit_instance();

protected:

    // 处理想要处理的数据
    virtual int proc(size_t &proc_data_num)
    {
        proc_data_num = 0;
        return 0;
    }

    virtual int register_trans_cmd() = 0;

};

#endif //#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_

