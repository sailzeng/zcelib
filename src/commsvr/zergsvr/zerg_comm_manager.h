
#ifndef ZERG_COMMUNICATION_MANAGER_H_
#define ZERG_COMMUNICATION_MANAGER_H_

#include "zerg_configure.h"
#include "zerg_stat_define.h"

//左小祖咒：借我那把枪吧，或者借我五毛钱
//这儿必须记录一下，为什么每个端口要配置一个SVC ID，这个问题，
//我隔三差五就会考虑一边。而且每次都会考虑到UDP穿透这个问题上,太TMD技术情节了。
//脑子太笨的结果，其实很简单
//如果一个SVCID对应多个端口，你不区分标识开，那么其他连接者知道连接的SVCID到底是那个端口吗？

class TCP_Accept_Handler;
class UDP_Svc_Handler;
class Zerg_MMAP_BusPipe;
class ZBuffer_Storage;
class TcpHandlerImpl;
class ExternalConnectHandler;

/****************************************************************************************************
class  Zerg_Comm_Manager
****************************************************************************************************/
class Zerg_Comm_Manager
{

    //
    typedef std::vector<TCP_Accept_Handler *> TCPACCEPT_HANDLER_LIST;
    //
    typedef std::vector<UDP_Svc_Handler *> UDPSVC_HANDLER_LIST;

protected:

    //构造函数和析构函数
    Zerg_Comm_Manager();
    ~Zerg_Comm_Manager();

public:

    //初始化,从配置文件读取配置
    int get_config(const conf_zerg::ZERG_CONFIG &cfg_file);

    //初始化Socket, backlog默认值和open中使用的默认值保持一致
    int init_socketpeer(ZERG_SERVICES_INFO &init_svcid,
        unsigned int backlog = ZCE_DEFAULT_BACKLOG,
        bool is_extern_svc = false,
        unsigned int proto_index = 0);

    //检查端口是否安全,安全端口必须不使用保险(FALSE)
    int check_safeport(ZCE_Sockaddr_In     &inetadd);

    //取得发送数据进行发送
    int popall_sendpipe_write(size_t want_, size_t &proc_frame_num);

    //
    inline void pushback_recvpipe(Zerg_App_Frame *recv_frame);

    //检查发包频率
    void check_freamcount(unsigned int now);

    TcpHandlerImpl* get_extern_proto_handler(unsigned int index);

protected:
    int init_extern_proto_handler();

protected:

    //一次最多发送2048帧
    static const unsigned int MAX_ONCE_SEND_FRAME  = 4096;

    //最大的监控的FRAME的数量,不希望太多,可能严重影响效率
    static const size_t       MAX_NUMBER_OF_MONITOR_FRAME  = 16;

protected:

    //ACCEPET的HANDLER数组
    TCPACCEPT_HANDLER_LIST    zerg_acceptor_;
    //UPD的HANDLER数组
    UDPSVC_HANDLER_LIST       zerg_updsvc_;



    //对于错误的数据,尝试发送的次数,只是了保证一定的网络瞬断
    unsigned int              error_try_num_;

    //监控命令的数量，为了加快速度，多用变量。
    size_t                    monitor_size_;
    //监控的命令
    unsigned int              monitor_cmd_[MAX_NUMBER_OF_MONITOR_FRAME];

    //内存管道类的实例对象，保留它仅仅为了加速
    Zerg_MMAP_BusPipe        *zerg_mmap_pipe_;
    //发送和接收缓冲的BUFF的实例对象，保留它仅仅为了加速
    ZBuffer_Storage          *zbuffer_storage_;
    //统计，使用单子类的指针，保留它仅仅为了加速
    Comm_Stat_Monitor          *server_status_;

protected:
    //发包数告警值
    static const unsigned int SEND_FRAME_ALERT_VALUE = 40000;
    //计数起始时间
    unsigned int              count_start_time_;
    //协议包发送计数器
    unsigned int              send_frame_count_;

    // 框架配置对象
    Comm_Svrd_Config          *comm_config_;

protected:
    //单子实例
    static Zerg_Comm_Manager  *instance_;

public:

    //单子实例函数
    static Zerg_Comm_Manager *instance();
    //清理单子实例
    static void clean_instance();

    // 统计心跳包收发间隔
    void stat_heart_beat_gap(const Zerg_App_Frame * proc_frame);
    // 统计收发分布
    void stat_heart_beat_distribute(const Zerg_App_Frame * proc_frame, unsigned int use_time,
        ZERG_MONITOR_FEATURE_ID feature_id);

    // 收到心跳包请求，打上接收时间戳
    void proc_zerg_heart_beat(Zerg_App_Frame * recv_frame);

    int send_single_buf( ZByteBuffer * tmpbuf );

};

//希望加快所以用了inline，不过看函数长度，优化的可能性小。
inline void Zerg_Comm_Manager::pushback_recvpipe(Zerg_App_Frame *recv_frame)
{
    // 如果是通信服务器的命令,不进行任何处理
    if (true == recv_frame->is_zerg_processcmd())
    {
        // 心跳回包则对心跳包收发间隔进行监控
        if (true == recv_frame->is_zerg_heart_beat_rsp())
        {
            stat_heart_beat_gap(recv_frame);
            return;
        }
        else if (true == recv_frame->is_zerg_heart_beat_req())
        {
            // 心跳请求包，打上接收时间戳继续丢给APP去处理
            proc_zerg_heart_beat(recv_frame);
        }
        else
        {
            return;
        }
    }


    //为了提高效率，先检查标志位，
    if (recv_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        Zerg_App_Frame::dumpoutput_framehead(recv_frame, "[TRACK MONITOR][RECV]opt", RS_INFO);
    }
    else
    {
        //这段代码在发送接收时都要检查，否则都有可能漏掉
        //如果是要跟踪的命令
        for (size_t i = 0; i < monitor_size_; ++i)
        {
            if (monitor_cmd_[i] == recv_frame->frame_command_)
            {
                recv_frame->frame_option_ |= Zerg_App_Frame::DESC_MONITOR_TRACK;
                Zerg_App_Frame::dumpoutput_framehead(recv_frame, "[TRACK MONITOR][RECV]cmd", RS_INFO);
            }
        }
    }

    int ret = zerg_mmap_pipe_->push_back_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID,
        reinterpret_cast< const ZCE_LIB::dequechunk_node *>(recv_frame));

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        server_status_->increase_once(ZERG_RECV_PIPE_FULL_COUNTER,
            recv_frame->app_id_);
    }
    else
    {
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER,
            recv_frame->app_id_);
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER_BY_CMD,
            recv_frame->app_id_,
            recv_frame->frame_command_);
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER_BY_SVR_TYPE,
            recv_frame->app_id_,
            recv_frame->send_service_.services_type_);
    }

}

#endif //_ZERG_COMMUNICATION_MANAGER_H_

