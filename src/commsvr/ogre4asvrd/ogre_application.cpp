#include "ogre_predefine.h"
#include "ogre_app_timer.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_comm_manager.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_ip_restrict.h"
#include "ogre_application.h"

/****************************************************************************************************
class  Ogre_Service_App
****************************************************************************************************/

//我又要偷偷藏着
Ogre_Service_App::Ogre_Service_App()
{
}

Ogre_Service_App::~Ogre_Service_App()
{
}

//
int Ogre_Service_App::app_start(int argc,const char *argv[])
{
    int ret = 0;
    ZCE_TRACE_FUNC_RETURN(RS_INFO,&ret);
    //
    ret = Svrd_Appliction::app_start(argc,argv);

    if (ret != 0)
    {
        return ret;
    }

    Ogre_Server_Config *config = reinterpret_cast<Ogre_Server_Config *>(config_base_);

    size_t max_accept = 0,max_connect = 0,max_peer = 0;
    Ogre_TCP_Svc_Handler::get_maxpeer_num(max_accept,max_connect);
    ZCE_LOG(RS_INFO,"Ogre max accept number :%u,max connect number:%u.\n",
            max_accept,max_connect);
    max_peer = max_accept + max_connect + 16;

    //在配置文件没有读取出来的时候,只显示调试信息

    //使用日志策略,得到配置文件,才能得到日志文件名称

    //先必须初始化Buffer Storage,设置为最大连接数的1/5,最少512个
    size_t size_list = (max_peer / 32 < 512)?512:max_peer / 32;
    Ogre_Buffer_Storage::instance()->init_buffer_list(size_list);

    //Ogre_Comm_Manger 初始化
    ret = Ogre_Comm_Manger::instance()->get_config(config);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR,"Ogre_Comm_Manger::instance()->init_comm_manger() fail !\n");
        return SOAR_RET::ERR_OGRE_INIT_COMM_MANAGER;
    }

    ZCE_LOG(RS_INFO,"%s success.Have fun, my brother!!!\n",__ZCE_FUNC__);
    return 0;
}

//处理退出的清理工作
int Ogre_Service_App::app_exit()
{
    //
    Ogre_Comm_Manger::instance()->uninit_comm_manger();
    Ogre_Comm_Manger::clean_instance();

    Ogre_Buffer_Storage::instance()->uninit_buffer_list();

    //基类的退出
    Svrd_Appliction::app_exit();

    //
    ZCE_LOG(RS_INFO,"%s Succ.Have Fun.!!!\n",__ZCE_FUNC__);

    return 0;
}

//运行函数,不到万不得已,不会退出.
//最重要的函数, 但是也最简单
int Ogre_Service_App::app_run()
{
    //空闲N次后,SLEEP的时间间隔
    const unsigned int  IDLE_SLEEP_INTERVAL = 512;

    size_t numevent = 0;
    unsigned int idle = 0;
    size_t procframe = 0;
    ZCE_LOG(RS_INFO,"Ogre_Service_App::Run Start.\n");
    //microsecond
    const int INTERVAL_MACRO_SECOND = 10000;

    zce::Time_Value sleeptime(0,INTERVAL_MACRO_SECOND);
    zce::Time_Value interval(0,INTERVAL_MACRO_SECOND);

    for (; app_run_;)
    {
        //每次都在这儿初始化zce::Time_Value不好,其要调整.
        interval.usec(INTERVAL_MACRO_SECOND);
        ZCE_Reactor::instance()->handle_events(&interval,&numevent);

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
        if (idle > IDLE_SLEEP_INTERVAL)
        {
            zce::sleep(sleeptime);
        }
    }

    return 0;
}