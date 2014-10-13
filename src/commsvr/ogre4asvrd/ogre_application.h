

#ifndef OGRE_SERVER_APPLICATION_H_
#define OGRE_SERVER_APPLICATION_H_

/****************************************************************************************************
class  Ogre4a_Service_App
****************************************************************************************************/
class Ogre4a_Service_App : public Comm_Svrd_Appliction
{

protected:
    //我又要偷偷藏着
    Ogre4a_Service_App();
    ~Ogre4a_Service_App();

    //定义而不实现
    Ogre4a_Service_App(const Ogre4a_Service_App &);
    const Ogre4a_Service_App &operator =(const Ogre4a_Service_App &);

public:
    //设置是否继续运行
    void set_run_sign(bool brun);

    //初始化
    int init_instance();
    //退出实例化
    int exit_instance();
    //运行
    int run_instance();

    virtual int init()
    {
        return 0;
    }

    virtual int re_read_load_cfg()
    {
        return 0;
    }

public:
    //单子实例获得函数
    static Ogre4a_Service_App *instance();
    //
    static void clean_instance();

public:
    //是否允许
    bool                     if_run_;

    //ZEN LIB的TimeQueue，这个最后要自己释放
    ZCE_Timer_Queue          *timer_queue_;

protected:

    //空闲N次后,SLEEP的时间间隔
    static const unsigned int  IDLE_SLEEP_INTERVAL = 512;

    //针对EPOLL的IO处理优先级，相对定时器
    static const int           EPOOL_IO_FIRST_PROCESS = 128;

    //单子实例指针
    static Ogre4a_Service_App   *instance_;

};

#endif //_OGRE_SERVER_APPLICATION_H_

