#pragma once

#include "zerg/comm_manager.h"

namespace zerg
{
class zerg_app : public soar::app_buspipe
{
public:
    //我又要偷偷藏着
    zerg_app();
    virtual ~zerg_app();

public:

    ///初始化,根据启动参数启动
    virtual int app_start(int argc, const char* argv[]);

    ///处理退出的清理工作
    virtual int app_exit();

    ///运行,运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。
    virtual int app_run();

public:

    ///通信管理器
    zerg::comm_manager* zerg_comm_mgr_;

    ///OP KEY文件的时间戳
    time_t             conf_timestamp_;
};
} // namespace zerg