

#include "ogre_predefine.h"
#include "ogre_app_handler.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_comm_manager.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_ip_restrict.h"
#include "ogre_application.h"

/****************************************************************************************************
class  Ogre4a_Service_App
****************************************************************************************************/

Ogre4a_Service_App *Ogre4a_Service_App::instance_ = NULL;

//我又要偷偷藏着
Ogre4a_Service_App::Ogre4a_Service_App():
    if_run_(true),
    timer_queue_(NULL)
{
}

Ogre4a_Service_App::~Ogre4a_Service_App()
{
    //关闭timer_queue_
    if (timer_queue_)
    {
        delete timer_queue_;
        timer_queue_ = NULL;
    }

    //    ogre_acceptor_ 会自己删除

}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月17日
Function        : Ogre4a_Service_App::InitInstance
Return          : int
Parameter List  :
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre4a_Service_App::init_instance()
{
    int ret = 0;

    //
    ret = Comm_Svrd_Appliction::init_instance();

    if (ret != 0)
    {
        return ret;
    }

    //得到文件配置参数
    ret = Ogre_Svr_Config::instance()->get_file_configure();

    if (ret != 0)
    {
        ZLOG_ERROR( "Get config fail，Please check config!\n");
        return ret;
    }

    size_t max_accept = 0, max_connect = 0, max_peer = 0;
    Ogre_TCP_Svc_Handler::get_maxpeer_num(max_accept, max_connect);
    ZLOG_INFO( "Ogre max accept number :%u,max connect number:%u.\n",
               max_accept, max_connect);
    max_peer = max_accept + max_connect + 16;

    //更新定时器
    size_t old_node_num;
    ZCE_Timer_Queue::instance()->extend_node(max_peer, old_node_num);

    //Reactor的修改一定要放在前面(读取配置后面)，至少吃了4次亏
    //居然在同一条河里淹死了好几次。最新的一次是20070929，
#ifdef ZCE_OS_WINDOWS
    //
    ZCE_Reactor::instance(new ZCE_Select_Reactor(max_peer));
    ZLOG_DEBUG("[zergsvr] ZCE_Reactor and ZCE_Select_Reactor initialized.");
#else
    ZCE_Reactor::instance(new ZCE_Epoll_Reactor(max_peer));
    ZLOG_INFO("[zergsvr] ZCE_Reactor and ZCE_Epoll_Reactor initialized.");
#endif

    //
    Ogre_App_Handler::instance();

    //在配置文件没有读取出来的时候,只显示调试信息

    //使用日志策略,得到配置文件,才能得到日志文件名称

    //先必须初始化Buffer Storage,设置为最大连接数的1/5,最少256个

    size_t szlist = (max_peer / 50 < 512 ) ? 512 : max_peer / 50;
    Ogre_Buffer_Storage::instance()->init_buffer_list(szlist);

    //Ogre_Comm_Manger 初始化
    ret = Ogre_Comm_Manger::instance()->init_comm_manger(Ogre_Svr_Config::instance()->cfg_ogre4a_);

    if (ret != 0 )
    {
        ZLOG_ERROR( "Ogre_Comm_Manger::instance()->init_comm_manger() fail !\n");
        return SOAR_RET::ERR_OGRE_INIT_COMM_MANAGER;
    }

    ZLOG_INFO( "InitInstance success.Have fun, my brother!!!\n");
    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月29日
Function        : Ogre4a_Service_App::ExitInstance
Return          : int
Parameter List  : NULL
Description     : 处理退出的清理工作
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre4a_Service_App::exit_instance()
{
    //
    Ogre_Comm_Manger::instance()->uninit_comm_manger();
    Ogre_Comm_Manger::clean_instance();

    //释放所有资源,会关闭所有的handle吗?看是什么Reactor
    ZCE_Reactor::instance()->close();
    Ogre_Buffer_Storage::instance()->uninit_buffer_list();

    //
    ZLOG_INFO( "ExitInstance Succ.Have Fun.!!!\n");

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月29日
Function        : Ogre4a_Service_App::run_instance
Return          : int
Parameter List  : NULL
Description     : 运行函数,不到万不得已,不会退出
Calls           :
Called By       :
Other           : 最重要的函数,但是也最简单
Modify Record   :
******************************************************************************************/
int Ogre4a_Service_App::run_instance()
{
    size_t numevent = 0;
    unsigned int idle = 0;
    size_t procframe = 0;
    ZLOG_INFO( "Ogre4a_Service_App::Run Start.\n");
    //microsecond
    const int INTERVAL_MACRO_SECOND = 10000;

    ZCE_Time_Value sleeptime(0, INTERVAL_MACRO_SECOND);
    ZCE_Time_Value interval(0, INTERVAL_MACRO_SECOND);

    for (; if_run_;)
    {
        //每次都在这儿初始化ZCE_Time_Value不好,其要调整.
        interval.usec(INTERVAL_MACRO_SECOND);
        ZCE_Reactor::instance()->handle_events(&interval, &numevent);

        //取得发送数据数据
        Ogre_Comm_Manger::instance()->get_all_senddata_to_write(procframe);
        //没有处理任何事件

        //如果有事件处理了就继续
        if (numevent > 0 || procframe > 0)
        {
            idle = 0;
            continue;
        }
        else
        {
            ++idle;
        }

        //你可以根据空闲状态处理一些自己的事情,
        if (idle > IDLE_SLEEP_INTERVAL )
        {
            ZCE_OS::sleep(sleeptime);
        }
    }

    return 0;
}

//设置是否继续运行
void Ogre4a_Service_App::set_run_sign(bool brun)
{
    if_run_ = brun;
}

//单子实例获得函数
Ogre4a_Service_App *Ogre4a_Service_App::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Ogre4a_Service_App();
    }

    return instance_;
}

//单子实例清理函数
void Ogre4a_Service_App::clean_instance()
{
    if (instance_ != NULL)
    {
        delete instance_;
        instance_ = NULL;
    }
}

