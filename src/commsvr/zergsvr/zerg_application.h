
#ifndef ZERG_SERVER_APPLICATION_H_
#define ZERG_SERVER_APPLICATION_H_

#include "zerg_tcp_ctrl_handler.h"
#include "zerg_app_handler.h"

class Zerg_Comm_Manager;
/****************************************************************************************************
class  Zerg_Service_App
****************************************************************************************************/
class Zerg_Service_App : public Comm_Svrd_Appliction
{

protected:

    //
    static const size_t NORMAL_MAX_ONCE_SEND_FRAME   = 4096;
    //
    static const size_t SENDBUSY_MAX_ONCE_SEND_FRAME = 12288;

    //
    static const size_t SEND_BUSY_JUDGE_STANDARD     = NORMAL_MAX_ONCE_SEND_FRAME / 2;
    //
    static const size_t SEND_IDLE_JUDGE_STANDARD     = 128;

    //
    static const time_t IDLE_REACTOR_WAIT_USEC       = 1000;
    //
    static const time_t NORMAL_REACTOR_WAIT_USEC     = 100;
    //
    static const time_t BUSY_REACTOR_WAIT_USEC       = 20;
    //
    static const time_t SEND_BUSY_REACTOR_WAIT_USEC  = 0;

    //
    static const size_t DEFAULT_IO_FIRST_RATIO       = 32;

protected:
    //我又要偷偷藏着
    Zerg_Service_App();
    ~Zerg_Service_App();

    //定义而不实现
    Zerg_Service_App(const Zerg_Service_App *);
    const Zerg_Service_App &operator =(const Zerg_Service_App &);

public:

    //ZEN的TimeQueue，这个最后要自己释放
    ZCE_Timer_Queue         *timer_queue_;

    Zerg_Comm_Manager       *zerg_comm_mgr_;

    //OP KEY文件的时间戳
    time_t                   conf_timestamp_;

protected:
    //单子的static实例
    static Zerg_Service_App *instance_;

public:

    //初始化
    int init_instance();
    //退出实例化
    int exit_instance();
    //运行
    int run_instance();

    bool if_proxy();

private:
    int load_app_conf()
    {
        // 框架 已经加载了zerg的配置
        return 0;
    }
    int init()
    {
        // 框架 已经加载了zerg的配置
        //zce_UNUSED_ARG(config);
        return 0;
    }
    void exit() {}
    int reload();

    int reload_daynamic_config();

public:

    //单子实例函数
    static Zerg_Service_App *instance();
    //清理单子实例
    static void clean_instance();

    conf_zerg::ZERG_CONFIG *config_;

};

//

#endif //_ZERG_SERVER_APPLICATION_H_

