#include "zerg/predefine.h"
#include "zerg/application.h"
#include "zerg/console_handler.h"
#include "zerg/udp_ctrl_handler.h"
#include "zerg/tcp_ctrl_handler.h"
#include "zerg/ip_restrict.h"
#include "zerg/configure.h"
#include "zerg/stat_define.h"

/****************************************************************************************************
class  Zerg_App
****************************************************************************************************/

//单子的static实例

//我又要偷偷藏着
Zerg_App::Zerg_App() :
    zerg_comm_mgr_(NULL),
    conf_timestamp_(0)
{
}

Zerg_App::~Zerg_App()
{
}

//根据启动参数启动
int Zerg_App::app_start(int argc, const char* argv[])
{
    int ret = 0;

    ret = soar::Svrd_Appliction::app_start(argc, argv);
    if (ret != 0)
    {
        return ret;
    }
    Zerg_Config* config = reinterpret_cast<Zerg_Config*>(config_base_);
    ret = zerg::IPRestrict_Mgr::instance()->get_config(config);
    if (0 != ret)
    {
        return ret;
    }

    //通信管理器读取配置文件
    ret = zerg::Comm_Manager::instance()->get_config(config);
    if (0 != ret)
    {
        return ret;
    }

    ret = TCP_Svc_Handler::get_config(config);
    if (0 != ret)
    {
        return ret;
    }

    ret = UDP_Svc_Handler::get_config(config);
    if (0 != ret)
    {
        return ret;
    }

    //给统计模块添加自己的统计信息
    soar::Stat_Monitor::instance()->add_status_item(ZERG_MONITOR_NUMBER,
                                                    ZERG_MONITOR_ITEMS);
    //如果是多线程增加这步
    //soar::Stat_Monitor::instance()->multi_thread_guard(false);

    size_t max_accept = 0, max_connect = 0, max_peer = 0;
    TCP_Svc_Handler::get_max_peer_num(max_accept, max_connect);
    max_peer = max_accept + max_connect + 16;

    //设置发送接收缓冲的数量
    zerg::Buffer_Storage::instance()->init_buflist_by_hdlnum(max_peer);

    //通信管理器初始化
    zerg_comm_mgr_ = zerg::Comm_Manager::instance();

    //
    ZCE_LOG(RS_INFO, "[zergsvr] ReloadDynamicConfig Succ.Ooooo!"
            "Some people believe that God created the world,but.");

    //-----------------------------------------------------------------------------------------------
    //初始化静态数据
    ret = TCP_Svc_Handler::init_all_static_data();

    if (ret != 0)
    {
        return ret;
    }
    ret = UDP_Svc_Handler::init_all_static_data();
    if (ret != 0)
    {
        return ret;
    }

    //
    ret = zerg_comm_mgr_->init_allpeer();
    if (ret != 0)
    {
        return ret;
    }

    ZCE_LOG(RS_INFO, "[zergsvr] init_instance Succ.Have Fun.!!!");
    //进程监控，这个最好，或者说必须放在程序初始化的最后，这样可以保证与分配的内存的初始化基本完成了,

    return 0;
}

int Zerg_App::app_exit()
{
    ZCE_LOG(RS_INFO, "[zergsvr] exit_instance Succ.Have Fun.!!!");

    //释放所有的静态资源，关闭所有的句柄
    TCP_Svc_Handler::uninit_all_staticdata();

    //清理管理器的实例
    zerg::Comm_Manager::clean_instance();

    //清理单子
    zerg::IPRestrict_Mgr::clean_instance();

    //最后调用基类的退出函数
    soar::Svrd_Appliction::app_exit();

    return 0;
}

//运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。最重要的函数,但是也最简单,
int Zerg_App::app_run()
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

    ZCE_LOG(RS_INFO, "[zergsvr] Zerg_App::run_instance start.");

    //microsecond
    zce::ZCE_Reactor* preactor = zce::ZCE_Reactor::instance();
    zce::Timer_Queue* p_timer_queue = zce::Timer_Queue::instance();

    zce::Time_Value run_interval(0, IDLE_REACTOR_WAIT_USEC);

    for (size_t i = 0; app_run_; ++i)
    {
        //如果比较忙，
        if (num_send_frame >= SEND_BUSY_JUDGE_STANDARD)
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
        else if (num_send_frame < SEND_IDLE_JUDGE_STANDARD)
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

        //每次都在这儿初始化zce::Time_Value不好,其要调整.
        zerg_comm_mgr_->popall_sendpipe_write(want_send_frame, num_send_frame);

        //如果发送队列很忙，再进行一次发送
        if ((num_send_frame > SEND_BUSY_JUDGE_STANDARD && num_io_event == 0) || num_send_frame >= NORMAL_MAX_ONCE_SEND_FRAME)
        {
            zerg_comm_mgr_->popall_sendpipe_write(want_send_frame, num_send_frame);
        }

        //偶尔处理一下定时器
        if (i % DEFAULT_IO_FIRST_RATIO == 0)
        {
            p_timer_queue->expire();
        }
    }

    ZCE_LOG(RS_INFO, "[zergsvr] Zerg_App::run_instance end.");

    return 0;
}