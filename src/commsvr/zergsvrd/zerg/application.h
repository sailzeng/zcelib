#pragma once

#include "zerg/tcp_ctrl_handler.h"

class Zerg_Comm_Manager;

/*!
* @brief
*
* @note
*/
class Zerg_Service_App: public soar::Svrd_Appliction
{
public:
    //我又要偷偷藏着
    Zerg_Service_App();
    virtual ~Zerg_Service_App();

public:

    ///初始化,根据启动参数启动
    virtual int app_start(int argc,const char *argv[]);

    ///处理退出的清理工作
    virtual int app_exit();

    ///运行,运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。
    virtual int app_run();

public:

    ///通信管理器
    Zerg_Comm_Manager *zerg_comm_mgr_;

    ///OP KEY文件的时间戳
    time_t             conf_timestamp_;
};


