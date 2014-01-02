#include "soar_predefine.h"
#include "soar_frame_zerg.h"
#include "soar_svrd_app_non_ctrl.h"
#include "soar_zerg_mmappipe.h"
#include "soar_svrd_config.h"
#include "soar_error_code.h"

Comm_SvrdApp_NonCtrl::Comm_SvrdApp_NonCtrl():
    nonctrl_recv_buffer_(NULL)
{
    nonctrl_recv_buffer_ = new (Zerg_App_Frame::MAX_LEN_OF_APPFRAME) Zerg_App_Frame();
}

Comm_SvrdApp_NonCtrl::~Comm_SvrdApp_NonCtrl()
{
    //释放资源
    if (nonctrl_recv_buffer_)
    {
        delete nonctrl_recv_buffer_;
        nonctrl_recv_buffer_ = NULL;
    }
}

//运行函数
int Comm_SvrdApp_NonCtrl::run_instance()
{
    ZLOG_INFO("[framework] app %s class [%s] run_instance start.",
              get_app_basename(),
              typeid(*this).name());
    //
    size_t size_io_event = 0 , size_timer_expire  = 0;

    size_t prc_frame = 0, idle = 0, proc_data_num = 0;

    ZCE_Time_Value select_interval(0, 0);

    ZCE_Timer_Queue *time_queue = ZCE_Timer_Queue::instance();
    ZCE_Reactor *reactor = ZCE_Reactor::instance();

    for (; app_run_;)
    {
        // 检查是否需要重新加载配置
        if (app_reload_)
        {
            // 重新加载配置
            reload_config();
        }

        //处理收到的命令
        popfront_recvpipe(prc_frame);

        size_timer_expire = time_queue->expire();

        // 处理其他方式通信数据
        proc_data_num = 0;
        proc(proc_data_num);

        //如果没有处理任何帧
        // 处理管道的包少于要求处理的最大数则说明管道已经空了
        if (prc_frame < MAX_ONCE_PROCESS_FRAME && size_timer_expire <= 0 && proc_data_num <= 0)
        {
            ++idle;
        }
        else
        {
            idle = 0;
        }

        // 如果空循环太多则会白白耗电不低碳
        // 理论上这里睡眠最大时间是100毫秒
        // 因此只要内存管道大小>=100毫秒最大系统处理包数*每个包字节数
        // 则再睡醒以后能够正常处理数据而不至于管道满而丢数据
        // 假设系统所有资源够用，那么网卡速率则决定了最大处理能力
        // 网卡按1Gbit计算，则管道临界大小为1Gbit-per(S)/8/10≈12MByte，再加上管道自身的内存结构占用
        // 考虑为16MByte也即足以，所以只要管道大小超过16MByte应该就顶的住。
        // 所以这里就不用空跑那么多次了。
        if (idle < 1)
        {
            continue;
        }
        //如果空闲很多,休息一下,如果你比较空闲，在这儿SELECT相当于Sleep，
        else if (idle >= HEAVY_IDLE_SLEEP_INTERVAL)
        {
            select_interval.usec(HEAVY_IDLE_INTERVAL_MICROSECOND);
        }
        //else 相当于 else if (idle >= LIGHT_IDLE_SELECT_INTERVAL)
        else
        {
            select_interval.usec(LIGHT_IDLE_INTERVAL_MICROSECOND);
        }

        //
        reactor->handle_events(&select_interval, &size_io_event);
    }

    ZLOG_INFO("[framework] app %s class [%s] run_instance end.",
              get_app_basename(),
              typeid(*this).name());
    return SOAR_RET::SOAR_RET_SUCC;
}

//从管道中收取一组数据进行处理
int Comm_SvrdApp_NonCtrl::popfront_recvpipe(size_t &proc_frame)
{
    int ret = 0;

    //一次处理大量的数据
    for (proc_frame = 0;
         zerg_mmap_pipe_->is_empty_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID) == false
         && proc_frame < MAX_ONCE_PROCESS_FRAME ;
         ++proc_frame)
    {
        //
        ret = zerg_mmap_pipe_->pop_front_recvpipe(nonctrl_recv_buffer_);

        if (ret !=  SOAR_RET::SOAR_RET_SUCC)
        {
            return SOAR_RET::SOAR_RET_SUCC;
        }

        DEBUGDUMP_FRAME_HEAD(nonctrl_recv_buffer_, "FROM RECV PIPE FRAME", RS_DEBUG);

        //处理一个收到的数据
        ret = process_recv_appframe(nonctrl_recv_buffer_);

        //
        if (ret !=  SOAR_RET::SOAR_RET_SUCC)
        {
            continue;
        }
    }

    //
    return SOAR_RET::SOAR_RET_SUCC;
}
