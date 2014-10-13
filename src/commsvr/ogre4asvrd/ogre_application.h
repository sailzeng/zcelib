

#ifndef OGRE_SERVER_APPLICATION_H_
#define OGRE_SERVER_APPLICATION_H_

/****************************************************************************************************
class  Ogre_Service_App
****************************************************************************************************/
class Ogre_Service_App : public Soar_Svrd_Appliction
{

public:

    //我又要偷偷藏着
    Ogre_Service_App();
    virtual ~Ogre_Service_App();

public:

    ///初始化,根据启动参数启动
    virtual int on_start(int argc, const char *argv[]);

    ///处理退出的清理工作
    virtual int on_exit();

    ///运行,运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。
    virtual int on_run();

protected:

    //空闲N次后,SLEEP的时间间隔
    static const unsigned int  IDLE_SLEEP_INTERVAL = 512;

    //针对EPOLL的IO处理优先级，相对定时器
    static const int           EPOOL_IO_FIRST_PROCESS = 128;


};

#endif //_OGRE_SERVER_APPLICATION_H_

