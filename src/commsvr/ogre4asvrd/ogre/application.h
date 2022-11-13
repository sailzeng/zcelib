#pragma once

namespace ogre
{
class application : public soar::app_buspipe
{
public:

    //我又要偷偷藏着
    application();
    virtual ~application();

public:

    ///初始化,根据启动参数启动
    virtual int app_start(int argc, const char* argv[]);

    ///处理退出的清理工作
    virtual int app_exit();

    ///运行,运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。
    virtual int app_run();
};
}
