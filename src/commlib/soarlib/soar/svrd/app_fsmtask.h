#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_
#define SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_

#include "soar/svrd/app_bus.h"

#define REGISTER_NOTIFYTRANS_CMD(fsmtask_fsm_class, cmd_word ) \
    { \
        int __ret_register_ = FSMTask_Manger::instance()->register_trans_cmd(cmd_word, \
                                                                             new fsmtask_fsm_class(zce::Timer_Queue::instance(),\
                                                                             FSMTask_Manger::instance())); \
        if (__ret_register_ !=  0) \
        { \
            return __ret_register_; \
        } \
    }

class soar::FSMTask_TaskBase;

class Comm_SvrdApp_FSMTask : public soar::App_BusPipe
{
protected:

    //使用事务处理的Application
    Comm_SvrdApp_FSMTask();
    virtual ~Comm_SvrdApp_FSMTask();

public:

    //增加调用register_func_cmd
    virtual int app_start(int argc, const char* argv[]);

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
    virtual int register_notify_task(soar::FSMTask_TaskBase*& clone_task,
                                     size_t& task_num,
                                     size_t& task_stack_size) = 0;
};

#endif //#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_
