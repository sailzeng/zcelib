
#include "zerg_predefine.h"
#include "zerg_application.h"
#include "zerg_console_handler.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_ip_restrict.h"
#include "zerg_comm_manager.h"
#include "zerg_configure.h"
#include "zerg_stat_define.h"

/****************************************************************************************************
class  Zerg_Service_App
****************************************************************************************************/

//单子的static实例
Zerg_Service_App *Zerg_Service_App::instance_ = NULL;

//我又要偷偷藏着
Zerg_Service_App::Zerg_Service_App():
    zerg_comm_mgr_(NULL),
    conf_timestamp_(0)
{
}

Zerg_Service_App::~Zerg_Service_App()
{
}

//根据启动参数启动
int Zerg_Service_App::on_start(int argc, const char *argv[])
{
    int ret = 0;

    ret = Comm_Svrd_Appliction::on_start(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    //初始化统计模块
    //因为配置初始化时会从配置服务器拉取ip，触发统计，因此需要提前初始化
    ret = Comm_Stat_Monitor::instance()->initialize(app_base_name_.c_str(),
                                                    business_id_,
                                                    self_svc_id_,
                                                    0,
                                                    NULL,
                                                    false);
    if (ret != 0)
    {
        ZLOG_ERROR("zce_Server_Status init fail. ret=%d", ret);
        return ret;
    }


    size_t max_accept = 0, max_connect = 0, max_peer = 0;
    TCP_Svc_Handler::get_max_peer_num(max_accept, max_connect);
    max_peer = max_accept + max_connect + 16;

    //更新定时器
    //size_t old_node_num;
    //ZCE_Timer_Queue::instance()->extend_node(max_peer, old_node_num);

    //Reactor的修改一定要放在前面(读取配置后面)，至少吃了4次亏
    //居然在同一条河里淹死了好几次。最新的一次是20070929，
#ifdef ZCE_OS_WINDOWS
    //
    ZCE_Reactor::instance(new ZCE_Select_Reactor(max_peer));
    ZLOG_DEBUG("[zergsvr] ZCE_Reactor and zce_Select_Reactor initialized.");
#else
    ZCE_Reactor::instance(new ZCE_Epoll_Reactor(max_peer));
    ZLOG_INFO("[zergsvr] ZCE_Reactor and zce_Epoll_Reactor initialized.");
#endif


    //设置发送接收缓冲的数量
    ZBuffer_Storage::instance()->init_buflist_by_hdlnum(max_peer);

    //Zerg_Comm_Manager的配置部分可以动态重复加载
    Zerg_IPRestrict_Mgr::instance();
    //通信管理器初始化
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();


    //加载动态配置部分
    ret = reload_daynamic_config();

    if ( ret != 0 )
    {
        ZLOG_INFO("[zergsvr] ReloadDynamicConfig fail,ret = %d.", ret);
        return ret;
    }

    //
    ZLOG_INFO("[zergsvr] ReloadDynamicConfig Succ.Ooooo!Some people believe that God created the world,but.");

    //-----------------------------------------------------------------------------------------------
    //初始化静态数据
    ret = TCP_Svc_Handler::init_all_static_data();

    if ( ret != 0 )
    {
        return ret;
    }
    ret = UDP_Svc_Handler::init_all_static_data();
    if ( ret != 0 )
    {
        return ret;
    }

    //
    ret = zerg_comm_mgr_->init_allpeer();
    if (ret != 0)
    {
        return ret;
    }


    ZLOG_INFO("[zergsvr] init_instance Succ.Have Fun.!!!");
    //进程监控，这个最好，或者说必须放在程序初始化的最后，这样可以保证与分配的内存的初始化基本完成了,

    return 0;
}

////
//int Zerg_Service_App::reload()
//{
//    int ret = reload_daynamic_config();
//
//    if ( ret != 0 )
//    {
//        ZLOG_INFO("[zergsvr] reload config fail,ret = %d.", ret);
//        return ret;
//    }
//
//    TCP_Svc_Handler::adjust_svc_handler_pool();
//
//    return 0;
//}

//重新加载配置文件
int Zerg_Service_App::reload_daynamic_config()
{
    int ret = 0;

    //部分配置可以动态重复加载
    //看IP限制部分部分
    Zerg_Server_Config *zerg_config = dynamic_cast<Zerg_Server_Config *>(config_base_);
    // 配置重新加载
    ret = zerg_config->load_cfgfile();
    if (ret != 0)
    {
        ZLOG_ERROR("zerg reload config fail. ret=%d", ret);
        return ret;
    }


    ret = Zerg_IPRestrict_Mgr::instance()->get_config(zerg_config);

    if ( 0 != ret )
    {
        return ret;
    }

    //通信管理器读取配置文件
    ret = zerg_comm_mgr_->get_config(zerg_config);

    if (0 != ret )
    {
        return ret;
    }

    //控制台初始化
    //ret = Zerg_Console_Handler::instance()->initialize(*config_);
    //ZLOG_INFO("[zergsvr] Console initialize ret =%d .", ret);

    if (ret != 0 )
    {
        //控制台不启动，不终止程序运行,
        if ( SOAR_RET::ERROR_CONSOLE_IS_CLOSED  == ret)
        {
            ZLOG_INFO("[zergsvr] Console close .ret = %u", ret);
        }
        else
        {
            ZLOG_ERROR("[zergsvr] Console initialize fail.ret = %u", ret);
        }
    }

    //动态修改日志的级别
    set_log_priority(zerg_config->log_config_.log_level_);



    return 0;
}


int Zerg_Service_App::on_exit()
{
    ZLOG_INFO("[zergsvr] exit_instance Succ.Have Fun.!!!");

    //释放所有的静态资源，关闭所有的句柄
    TCP_Svc_Handler::uninit_all_staticdata();

    //清理管理器的实例
    Zerg_Comm_Manager::clean_instance();

    //清理单子
    Zerg_IPRestrict_Mgr::clean_instance();

    Comm_Stat_Monitor::clean_instance();

    //释放所有资源,会关闭所有的handle吗,ZCE_Reactor 会，ACE的Reactor看实现
    if (ZCE_Reactor::instance())
    {
        ZCE_Reactor::instance()->close();
    }

    //
    if (ZCE_Timer_Queue::instance())
    {
        ZCE_Timer_Queue::instance()->close();
    }

    ZBuffer_Storage::instance()->uninit_buffer_list();

    return 0;
}


//运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。最重要的函数,但是也最简单,
int Zerg_Service_App::on_run()
{

    //
    const size_t NORMAL_MAX_ONCE_SEND_FRAME = 4096;
    //
    const size_t SENDBUSY_MAX_ONCE_SEND_FRAME = 12288;

    //
    const size_t SEND_BUSY_JUDGE_STANDARD = NORMAL_MAX_ONCE_SEND_FRAME / 2;
    //
    const size_t SEND_IDLE_JUDGE_STANDARD = 128;

    //
    const time_t IDLE_REACTOR_WAIT_USEC = 1000;
    //
    const time_t NORMAL_REACTOR_WAIT_USEC = 100;
    //
    const time_t BUSY_REACTOR_WAIT_USEC = 20;
    //
    const time_t SEND_BUSY_REACTOR_WAIT_USEC = 0;

    //
    const size_t DEFAULT_IO_FIRST_RATIO = 32;

    size_t num_io_event = 0, num_send_frame = 0, want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;

    ZLOG_INFO("[zergsvr] Zerg_Service_App::run_instance start.");

    //microsecond
    ZCE_Reactor *preactor = ZCE_Reactor::instance();
    ZCE_Timer_Queue *p_timer_queue = ZCE_Timer_Queue::instance();

    ZCE_Time_Value run_interval(0, IDLE_REACTOR_WAIT_USEC);

    for (size_t i = 0; app_run_; ++i)
    {

        //如果比较忙，
        if ( num_send_frame >= SEND_BUSY_JUDGE_STANDARD )
        {
            //如果单单是发送比较忙，
            if (num_io_event == 0)
            {
                //取得发送数据数据
                run_interval.usec(SEND_BUSY_REACTOR_WAIT_USEC);
                want_send_frame = SENDBUSY_MAX_ONCE_SEND_FRAME;
            }
            else
            {
                run_interval.usec(BUSY_REACTOR_WAIT_USEC);
                want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;
            }
        }
        //如果比较优先
        else if ( num_send_frame < SEND_IDLE_JUDGE_STANDARD )
        {
            if (num_io_event == 0)
            {
                run_interval.usec(IDLE_REACTOR_WAIT_USEC);
                want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;
            }
            else
            {
                run_interval.usec(NORMAL_REACTOR_WAIT_USEC);
                want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;
            }
        }
        else
        {
            run_interval.usec(BUSY_REACTOR_WAIT_USEC);
            want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;
        }

        //
        preactor->handle_events(&run_interval, &num_io_event);

        //每次都在这儿初始化ZCE_Time_Value不好,其要调整.
        zerg_comm_mgr_->popall_sendpipe_write(want_send_frame, num_send_frame);

        //如果发送队列很忙，再进行一次发送
        if ((num_send_frame > SEND_BUSY_JUDGE_STANDARD && num_io_event == 0) || num_send_frame >= NORMAL_MAX_ONCE_SEND_FRAME )
        {
            zerg_comm_mgr_->popall_sendpipe_write(want_send_frame, num_send_frame);
        }

        //偶尔处理一下定时器
        if (i % DEFAULT_IO_FIRST_RATIO == 0)
        {
            p_timer_queue->expire();
        }
    }

    ZLOG_INFO("[zergsvr] Zerg_Service_App::run_instance end.");

    return 0;
}

bool Zerg_Service_App::if_proxy()
{
    unsigned short self_svc_type = self_svc_id_.services_type_;
    if (self_svc_type == SVC_PROXY_SERVER)
    {
        return true;
    }
    return false;
}

//得到唯一的单子实例
Zerg_Service_App *Zerg_Service_App::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Zerg_Service_App();
    }

    return instance_;
}

//清除单子实例
void Zerg_Service_App::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}
