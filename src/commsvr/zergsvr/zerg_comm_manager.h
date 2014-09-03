
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
class Zerg_Server_Config;



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
    int get_config(const Zerg_Server_Config config);

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

    int send_single_buf( Zerg_Buffer * tmpbuf );


protected:

    //一次最多发送2048帧
    static const unsigned int MAX_ONCE_SEND_FRAME = 4096;

    //最大的监控的FRAME的数量,不希望太多,可能严重影响效率
    static const size_t       MAX_NUMBER_OF_MONITOR_FRAME = 16;


protected:
    //单子实例
    static Zerg_Comm_Manager  *instance_;


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


    //发包数告警值
    static const unsigned int SEND_FRAME_ALERT_VALUE = 40000;
    //计数起始时间
    unsigned int              count_start_time_;
    //协议包发送计数器
    unsigned int              send_frame_count_;

    // 框架配置对象
    Server_Config_FSM          *comm_config_;

};



#endif //_ZERG_COMMUNICATION_MANAGER_H_

