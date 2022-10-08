#include "ogre/predefine.h"
#include "ogre/app_timer.h"
#include "ogre/buf_storage.h"
#include "ogre/configure.h"
#include "ogre/comm_manager.h"
#include "ogre/svc_tcp.h"
#include "ogre/ip_restrict.h"
#include "ogre/application.h"

namespace ogre
{
//我又要偷偷藏着
application::application()
{
}

application::~application()
{
}

//
int application::app_start(int argc, const char* argv[])
{
    int ret = 0;
    ZCE_TRACE_FUNC_RETURN(RS_INFO, &ret);
    //
    ret = soar::app_buspipe::app_start(argc, argv);

    if (ret != 0)
    {
        return ret;
    }

    configure* config = reinterpret_cast<configure*>(config_base_);

    size_t max_accept = 0, max_connect = 0, max_peer = 0;
    svc_tcp::get_maxpeer_num(max_accept, max_connect);
    ZCE_LOG(RS_INFO, "Ogre max accept number :%u,max connect number:%u.\n",
            max_accept, max_connect);
    max_peer = max_accept + max_connect + 16;

    //在配置文件没有读取出来的时候,只显示调试信息

    //使用日志策略,得到配置文件,才能得到日志文件名称

    //先必须初始化Buffer Storage,设置为最大连接数的1/5,最少512个
    size_t size_list = (max_peer / 32 < 512) ? 512 : max_peer / 32;
    buffer_storage::instance()->init_buffer_list(size_list);

    //comm_manager 初始化
    ret = comm_manager::instance()->get_config(config);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "comm_manager::instance()->init_comm_manger() fail !\n");
        return SOAR_RET::ERR_OGRE_INIT_COMM_MANAGER;
    }

    ZCE_LOG(RS_INFO, "%s success.Have fun, my brother!!!\n", __ZCE_FUNC__);
    return 0;
}

//处理退出的清理工作
int application::app_exit()
{
    //
    comm_manager::instance()->uninit_comm_manger();
    comm_manager::clear_inst();

    buffer_storage::instance()->uninit_buffer_list();

    //基类的退出
    app_exit();

    //
    ZCE_LOG(RS_INFO, "%s Succ.Have Fun.!!!\n", __ZCE_FUNC__);

    return 0;
}

//运行函数,不到万不得已,不会退出.
//最重要的函数, 但是也最简单
int application::app_run()
{
    //空闲N次后,SLEEP的时间间隔
    const unsigned int  IDLE_SLEEP_INTERVAL = 512;

    size_t numevent = 0;
    unsigned int idle = 0;
    size_t procframe = 0;
    ZCE_LOG(RS_INFO, "application::Run Start.\n");
    //microsecond
    const int INTERVAL_MACRO_SECOND = 10000;

    zce::time_value sleeptime(0, INTERVAL_MACRO_SECOND);
    zce::time_value interval(0, INTERVAL_MACRO_SECOND);

    for (; app_run_;)
    {
        //每次都在这儿初始化zce::Time_Value不好,其要调整.
        interval.usec(INTERVAL_MACRO_SECOND);
        zce::reactor::instance()->handle_events(&interval, &numevent);

        //取得发送数据数据
        comm_manager::instance()->get_all_senddata_to_write(procframe);
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
}// namespace ogre