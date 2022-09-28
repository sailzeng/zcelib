#pragma once

#include "zerg/configure.h"
#include "zerg/stat_define.h"

//左小祖咒：借我那把枪吧，或者借我五毛钱
//这儿必须记录一下，为什么每个端口要配置一个SVC ID，这个问题，
//我隔三差五就会考虑一边。而且每次都会考虑到UDP穿透这个问题上,太TMD技术情节了。
//脑子太笨的结果，其实很简单
//如果一个SVCID对应多个端口，你不区分标识开，那么其他连接者知道连接的SVCID到底是那个端口吗？

class Accept_Handler;
class UDP_Svc_Handler;

#include "zerg/buf_storage.h"

namespace zerg
{
/*!
* @brief      通信管理器，
*
* note
*/
class Comm_Manager
{
    //
    typedef std::vector<Accept_Handler*> TCPACCEPT_HANDLER_LIST;
    //
    typedef std::vector<UDP_Svc_Handler*> UDPSVC_HANDLER_LIST;

protected:

    //构造函数和析构函数
    Comm_Manager();
    ~Comm_Manager();

public:

    /*!
    * @brief      初始化,从配置文件读取配置
    * @return     int
    * @param      config
    */
    int get_config(const Zerg_Config* config);

    /*!
    * @brief      初始化所有的监听，UDP端口，
    * @return     int
    */
    int init_allpeer();

    /*!
    * @brief      根据SVC INFO 初始化Socket,
    * @return     int
    * @param      init_svcid 初始化所依据的SVC INFO
    */
    int init_socketpeer(const soar::SERVICES_ID& init_svcid);

    /*!
    * @brief      检查端口是否安全,安全端口必须不使用保险(FALSE)
    * @return     int
    * @param      inetadd 检查的的地址信息
    */
    int check_safeport(const zce::skt::addr_in& inetadd);

    /*!
    * @brief      取得发送数据进行发送
    * @return     int
    * @param      want_send_frame  希望发送的数量，想了想，还是加了一个最多发送的帧的限额
    * @param      proc_frame_num   实际处理的数量
    * @note
    */
    int popall_sendpipe_write(size_t want_send_frame, size_t& proc_frame_num);

    //
    void pushback_recvpipe(soar::zerg_frame* recv_frame);

    //检查发包频率
    void check_freamcount(time_t now);

    //
    int send_single_buf(zerg::Buffer* tmpbuf);

public:

    //单子实例函数
    static Comm_Manager* instance();
    //清理单子实例
    static void clear_inst();

protected:

    ///一次最多发送2048帧
    static const unsigned int MAX_ONCE_SEND_FRAME = 4096;

    ///发包数告警值
    static const unsigned int SEND_FRAME_ALERT_VALUE = 40000;

protected:
    //单子实例
    static Comm_Manager* instance_;

protected:

    ///ACCEPET的HANDLER数组
    TCPACCEPT_HANDLER_LIST zerg_acceptor_;
    ///UPD的HANDLER数组
    UDPSVC_HANDLER_LIST zerg_updsvc_;

    ///对于错误的数据,尝试发送的次数,只是了保证一定的网络瞬断
    unsigned int error_try_num_;

    ///监控命令的数量，为了加快速度，多用变量。
    size_t monitor_size_;
    ///监控的命令
    unsigned int monitor_cmd_[ZERG_CONFIG_DATA::MAX_MONITOR_FRAME_NUMBER];

    ///内存管道类的实例对象，保留它仅仅为了加速
    soar::Svrd_BusPipe* zerg_mmap_pipe_;
    ///发送和接收缓冲的BUFF的实例对象，保留它仅仅为了加速
    zerg::Buffer_Storage* zbuffer_storage_;
    ///统计，使用单子类的指针，保留它仅仅为了加速
    soar::stat_monitor* server_status_;

    ///计数起始时间
    time_t count_start_time_;
    ///协议包发送计数器
    unsigned int send_frame_count_;

    ///配置实例指针
    const Zerg_Config* zerg_config_;
};
}
