#include "soar/predefine.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/svrd/app_plain.h"
#include "soar/svrd/svrd_buspipe.h"
#include "soar/enum/error_code.h"

namespace soar
{
svrdapp_plain::svrdapp_plain() :
    app_buspipe(),
    nonctrl_recv_buffer_(NULL)
{
    nonctrl_recv_buffer_ = soar::zerg_frame::new_frame(
        soar::zerg_frame::MAX_LEN_OF_FRAME);
}

svrdapp_plain::~svrdapp_plain()
{
    //释放资源
    if (nonctrl_recv_buffer_)
    {
        soar::zerg_frame::delete_frame(nonctrl_recv_buffer_);
        nonctrl_recv_buffer_ = NULL;
    }
}

//运行函数
int svrdapp_plain::app_run()
{
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance start.",
            get_app_basename(),
            typeid(*this).name());

    //空闲N次后,调整SELECT的等待时间间隔
    const unsigned int LIGHT_IDLE_SELECT_INTERVAL = 128;
    //空闲N次后,SLEEP的时间间隔
    const unsigned int HEAVY_IDLE_SLEEP_INTERVAL = 10240;

    //microsecond
    // 64位tlinux下idle的时间如果太短会导致cpu过高
    const int LIGHT_IDLE_INTERVAL_MICROSECOND = 10000;
    const int HEAVY_IDLE_INTERVAL_MICROSECOND = 100000;

    /// 一次最大处理的FRAME个数
    static const size_t MAX_ONCE_PROCESS_FRAME = 2048;

    //
    size_t size_io_event = 0, size_timer_expire = 0;

    size_t prc_frame = 0, idle = 0, proc_data_num = 0;

    zce::time_value select_interval(0, 0);

    zce::timer_queue* time_queue = zce::timer_queue::instance();
    zce::ZCE_Reactor* reactor = zce::ZCE_Reactor::instance();

    for (; app_run_;)
    {
        //处理收到的命令
        popfront_recvpipe(MAX_ONCE_PROCESS_FRAME, prc_frame);

        size_timer_expire = time_queue->expire();

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
        if (idle < LIGHT_IDLE_SELECT_INTERVAL)
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

    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance end.",
            get_app_basename(),
            typeid(*this).name());
    return 0;
}

//从管道中收取一组数据进行处理
int svrdapp_plain::popfront_recvpipe(size_t max_prc, size_t& proc_frame)
{
    int ret = 0;

    //一次处理大量的数据
    for (proc_frame = 0;
         zerg_mmap_pipe_->is_empty_recvbus() == false
         && proc_frame < max_prc;
         ++proc_frame)
    {
        //
        ret = zerg_mmap_pipe_->pop_front_recvbus(nonctrl_recv_buffer_);
        if (ret != 0)
        {
            return 0;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "FROM RECV PIPE FRAME", nonctrl_recv_buffer_);

        //处理一个收到的数据
        ret = process_recv_frame(nonctrl_recv_buffer_);

        //
        if (ret != 0)
        {
            continue;
        }
    }
    //
    return 0;
}
}