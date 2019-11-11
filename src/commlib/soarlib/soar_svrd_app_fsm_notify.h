
#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_
#define SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_

#include "soar_svrd_app_base.h"

#define REGISTER_NOTIFYTRANS_CMD(notify_trans_class, cmd_word ) \
    { \
        int __ret_register_ = MT_NOTIFY_TRANS_MANGER::instance()->register_trans_cmd(cmd_word, \
                                                                                     new notify_trans_class(ZCE_Timer_Queue::instance(),MT_NOTIFY_TRANS_MANGER::instance())); \
        if (__ret_register_ !=  0) \
        { \
            return __ret_register_; \
        } \
    }

class NotifyTrans_TaskBase;

class Comm_SvrdApp_FSM_Notify : public Soar_Svrd_Application
{

protected:

    //使用事务处理的Application
    Comm_SvrdApp_FSM_Notify();
    virtual ~Comm_SvrdApp_FSM_Notify();

public:

    //增加调用register_func_cmd
    virtual int app_start(int argc, const char *argv[]);

    //运行处理,
    virtual int app_run();

    //退出处理
    virtual int app_exit();

protected:

    //注册要事务处理的命令，
    //可以使用宏REGISTER_NOTIFYTRANS_CMD注册命令
    virtual int register_notifytrans_cmd() = 0;

    //注册notify的处理线程,你把这几个参数赋值了就OK
    //clone_task，麻烦你new一个给我
    //task_num 不要用太多的线程，合理就OK，除非有大量的阻塞
    //task_stack_size 如果线程数量较多，切记控制堆栈大小
    virtual int register_notify_task(NotifyTrans_TaskBase *&clone_task,
                                     size_t &task_num,
                                     size_t &task_stack_size) = 0;

};

#endif //#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_

