/******************************************************************************************
Copyright           : 2000-2004, Tencent Technology (Shenzhen) Company Limited.
FileName            : zerg_application.cpp
Author              : Sail(ZENGXING)
Version             :
Date Of Creation    : 2005年11月17日
Description         :

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
******************************************************************************************/

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
    timer_queue_(NULL),
    zerg_comm_mgr_(NULL),
    conf_timestamp_(0)
{
    config_ = new conf_zerg::ZERG_CONFIG();
}

Zerg_Service_App::~Zerg_Service_App()
{
    if (config_)
    {
        delete config_;
        config_ = NULL;
    }

    //关闭timer_queue_
    if (timer_queue_)
    {
        delete timer_queue_;
        timer_queue_ = NULL;
    }

    //zerg_acceptor_ 会自己删除?
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月17日
Function        : Zerg_Service_App::init_instance
Return          : int
Parameter List  :
  Param1: int argc
  Param2: char* argv[]
Description     : 根据启动参数启动
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int
Zerg_Service_App::init_instance()
{
    int ret = 0;

    ret = Comm_Svrd_Appliction::init_instance();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    //初始化统计模块
    //因为配置初始化时会从配置服务器拉取ip，触发统计，因此需要提前初始化
    ret = Comm_Stat_Monitor::instance()->initialize(true,
                                                    Comm_Svrd_Config::instance()->self_svr_id_,
                                                    0,
                                                    NULL,
                                                    false);

    if (ret != 0)
    {
        ZLOG_ERROR("zce_Server_Status init fail. ret=%d", ret);
        return ret;
    }

    config_ = &(Comm_Svrd_Config::instance()->zerg_config_);
    ret = Zerg_Server_Config::instance()->init(config_);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
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
    ZCE_Reactor::instance(new zce_Select_Reactor(max_peer));
    ZLOG_DEBUG("[zergsvr] ZCE_Reactor and zce_Select_Reactor initialized.");
#else
    ZCE_Reactor::instance(new zce_Epoll_Reactor(max_peer));
    ZLOG_INFO("[zergsvr] ZCE_Reactor and zce_Epoll_Reactor initialized.");
#endif

    //得到系统时钟等,
    new Zerg_App_Timer_Handler(ZCE_Timer_Queue::instance());

    //设置发送接收缓冲的数量
    ZBuffer_Storage::instance()->init_buflist_by_hdlnum(max_peer);

    //Zerg_Comm_Manager的配置部分可以动态重复加载
    Zerg_IPRestrict_Mgr::instance();
    //通信管理器初始化
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();
    //控制台初始化
    Zerg_Console_Handler::instance(new Zerg_Console_Handler(ZCE_Reactor::instance()));

    //加载动态配置部分
    ret = reload_daynamic_config();

    if ( ret != SOAR_RET::SOAR_RET_SUCC )
    {
        ZLOG_INFO("[zergsvr] ReloadDynamicConfig fail,ret = %d.", ret);
        return ret;
    }

    //
    ZLOG_INFO("[zergsvr] ReloadDynamicConfig Succ.Ooooo!Some people believe that God created the world,but.");

    //-----------------------------------------------------------------------------------------------
    //初始化静态数据
    ret = TCP_Svc_Handler::init_all_static_data();

    if ( ret != SOAR_RET::SOAR_RET_SUCC )
    {
        return ret;
    }

    //
    ret = UDP_Svc_Handler::init_all_static_data();

    if ( ret != SOAR_RET::SOAR_RET_SUCC )
    {
        return ret;
    }

    //初始化主端口, 如果配置中有设置这个值才使用, 否则使用默认值, window和linux的默认值不同
    if (Zerg_Server_Config::instance()->backlog_ == 0)
    {
        ret = zerg_comm_mgr_->init_socketpeer(Zerg_Server_Config::instance()->self_svc_info_);
    }
    else
    {
        ret = zerg_comm_mgr_->init_socketpeer(Zerg_Server_Config::instance()->self_svc_info_,
            Zerg_Server_Config::instance()->backlog_);
    }

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    //初始化备份端口
    for (unsigned int i = 0; i < config_->self_cfg.slave_svr_count; ++i)
    {
        ret = zerg_comm_mgr_->init_socketpeer(Zerg_Server_Config::instance()->slave_svc_ary_[i]);

        if (ret != SOAR_RET::SOAR_RET_SUCC)
        {
            return ret;
        }
    }

    // 初始化对外提供Udp服务的端口
    uint32_t ary_size = static_cast<uint32_t>(Zerg_Server_Config::instance()->external_udp_svr_ary_.size());
    for (unsigned int i = 0; i < ary_size; i ++)
    {
        ret = zerg_comm_mgr_->init_socketpeer(Zerg_Server_Config::instance()->external_udp_svr_ary_[i]);
        if (ret != SOAR_RET::SOAR_RET_SUCC)
        {
            return ret;
        }
    }

    ary_size = static_cast<uint32_t>(Zerg_Server_Config::instance()->extern_svc_ary_.size());
    //初始化外部服务端口
    for (unsigned int i = 0; i < ary_size; ++ i)
    {
        ret = zerg_comm_mgr_->init_socketpeer(Zerg_Server_Config::instance()->extern_svc_ary_[i],
                                              zce_DEFAULT_BACKLOG,
                                              true,
                                              config_->extern_svc_cfg.extern_svc_item[i].proto_cfg_index);

        if (ret != SOAR_RET::SOAR_RET_SUCC)
        {
            return ret;
        }
    }

    ZLOG_INFO("[zergsvr] init_instance Succ.Have Fun.!!!");
    //进程监控，这个最好，或者说必须放在程序初始化的最后，这样可以保证与分配的内存的初始化基本完成了,

    return SOAR_RET::SOAR_RET_SUCC;
}

int 
Zerg_Service_App::reload()
{
    int ret = reload_daynamic_config();

    if ( ret != SOAR_RET::SOAR_RET_SUCC )
    {
        ZLOG_INFO("[zergsvr] reload config fail,ret = %d.", ret);
        return ret;
    }

    TCP_Svc_Handler::adjust_svc_handler_pool();

    return SOAR_RET::SOAR_RET_SUCC;
}

//重新加载配置文件
int Zerg_Service_App::reload_daynamic_config()
{
    int ret = 0;

    //部分配置可以动态重复加载
    //看IP限制部分部分
    ret = Zerg_IPRestrict_Mgr::instance()->get_iprestrict_conf(*config_);

    if ( SOAR_RET::SOAR_RET_SUCC != ret )
    {
        return ret;
    }

    //通信管理器读取配置文件
    ret = zerg_comm_mgr_->get_config(*config_);

    if (SOAR_RET::SOAR_RET_SUCC != ret )
    {
        return ret;
    }

    //控制台初始化
    ret = Zerg_Console_Handler::instance()->initialize(*config_);
    ZLOG_INFO("[zergsvr] Console initialize ret =%d .", ret);

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
    set_log_priority(zce_LogTrace_Basic::log_priorities((const char *)(config_->log_cfg.log_level)));

    // 配置重新加载
    ret = Zerg_Server_Config::instance()->init(config_);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("zerg reload config fail. ret=%d", ret);
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月29日
Function        : Zerg_Service_App::exit_instance
Return          : int
Parameter List  : NULL
Description     : 处理退出的清理工作
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Zerg_Service_App::exit_instance()
{
    ZLOG_INFO("[zergsvr] exit_instance Succ.Have Fun.!!!");

    //释放所有的静态资源，关闭所有的句柄
    TCP_Svc_Handler::uninit_all_staticdata();

    //清理管理器的实例
    Zerg_Comm_Manager::clean_instance();

    //清理单子
    Zerg_IPRestrict_Mgr::clean_instance();
    Zerg_Console_Handler::clean_instance();
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

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月29日
Function        : Zerg_Service_App::run_instance
Return          : int
Parameter List  : NULL
Description     : 运行函数,不到万不得已,不会退出
Calls           :
Called By       :
Other           : 最重要的函数,但是也最简单,
Modify Record   : 为了加快发送的速度，对多种请求做了不同的微调。
******************************************************************************************/
int Zerg_Service_App::run_instance()
{
    size_t num_io_event = 0, num_send_frame = 0, want_send_frame = NORMAL_MAX_ONCE_SEND_FRAME;

    ZLOG_INFO("[zergsvr] Zerg_Service_App::run_instance start.");

    //microsecond
    ZCE_Reactor *preactor = ZCE_Reactor::instance();
    ZCE_Timer_Queue *p_timer_queue = ZCE_Timer_Queue::instance();

    ZCE_Time_Value run_interval(0, IDLE_REACTOR_WAIT_USEC);

    for (size_t i = 0; app_run_; ++i)
    {
        // 检查是否需要重新加载配置
        if (app_reload_)
        {
            // 重新加载配置
            reload_config();
        }

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

    return SOAR_RET::SOAR_RET_SUCC;
}

bool 
Zerg_Service_App::if_proxy()
{
    unsigned short self_svc_type = Comm_Svrd_Config::instance()->self_svr_id_.services_type_;
    if (self_svc_type == SVC_PROXY_SERVER
        || self_svc_type == SVC_PROXY_SERVER_MOBILE)
    {
        return true;
    }

    if (Comm_Svrd_Config::instance()->if_proxy_)
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
