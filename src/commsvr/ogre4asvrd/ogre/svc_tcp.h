#pragma once

#include "ogre/tcppeer_set.h"
#include "ogre/auto_connect.h"

class mmap_dequechunk;

namespace soar
{
class svrd_buspipe;
class ogre4a_frame;
}
namespace ogre
{
class svc_tcp : public  zce::event_handler,
    public zce::timer_handler
{
public:

    //服务的模式
    enum OGRE_HANDLER_MODE
    {
        //主动连接的Handler
        HANDLER_MODE_CONNECT,
        //被动接受某个端口的Handler
        HANDLER_MODE_ACCEPTED,
    };

protected:

    //
    enum PEER_STATUS
    {
        //PEER 没有连接上
        PEER_STATUS_NOACTIVE,
        //PEER 刚刚连接上,但是没有发送或者受到任何数据
        PEER_STATUS_JUST_CONNECT,
        //PEER 已经处于激活状态,
        PEER_STATUS_ACTIVE,
    };

public:

    //构造函数
    explicit svc_tcp(svc_tcp::OGRE_HANDLER_MODE hdl_mode);
    //为了让你无法在堆以外使用svc_tcp
protected:
    virtual ~svc_tcp();

public:

    /*!
    * @brief      初始化对象
    * @return     void
    * @param      sockstream
    * @param      fp_judge_whole
    */
    void init_tcp_svc_handler(zce::skt::stream&& sockstream,
                              FP_JudgeRecv_WholeFrame fp_judge_whole);

    /*!
    * @brief
    * @param      sockstream
    * @param      socketaddr
    * @param      fp_judge_whole
    */
    void init_tcp_svc_handler(zce::skt::stream&& sockstream,
                              const zce::skt::addr_in& socketaddr,
                              FP_JudgeRecv_WholeFrame fp_judge_whole);

    //event的一组要求自己继承的函数.
    ZCE_HANDLE get_handle(void) const override;
    //
    void read_event() override;
    //
    void write_event() override;
    //
    void connect_event(bool success) override;
    //
    int timer_timeout(const zce::time_value& time,
                      int timer_id) override;
    //
    void close_handle();

    //得到Handle对应PEER的IP地址
    const zce::skt::addr_in& get_peer();

protected:

    //从PEER读取数据
    int read_data_from_peer(size_t& szrevc);
    //将数据写入PEER
    int write_data_to_peer(size_t& szsend, bool& bfull);
    //将所有可以发送的数据都发送出去
    int write_all_data_to_peer();

    //预处理,检查数据,接收的REGISTER数据,根据第一个报决定对应关系
    int  preprocess_recv_frame();

    //处理发送的REGISTER数据,连接后发送第一个数据
    int  process_connect_register();

    //处理发送错误
    int process_senderror(soar::ogre4a_frame* tmpbuf);

    //得到一个PEER的状态
    PEER_STATUS  get_peer_status();

    //将数据帧放入管道,参数为要放入的数据长度
    int push_frame_to_recvpipe(unsigned int sz_data);

    //将一个发送的帧放入等待发送队列
    int put_frame_to_sendlist(soar::ogre4a_frame* ogre_frame);

    //合并发送的帧数据
    void unite_frame_sendlist();

public:

    ///读取配置文件
    static int get_config(const configure* config);

    ///初始化静态参数
    static int init_all_static_data();

    ///注销静态参数
    static int unInit_all_static_data();

    //从池子分配一个Handler
    static svc_tcp* alloc_svchandler_from_pool(OGRE_HANDLER_MODE handler_mode);

    //取得配置的最大PEER数量
    static void get_maxpeer_num(size_t& maxaccept, size_t& maxconnect);

    //将数据从端口发送数据
    static int process_send_data(soar::ogre4a_frame* tmpbuf);

    /*!
    * @brief
    * @return     int == 0表示找到，
    * @param      peer_id  查询的PEER ID
    * @param      svchanle 返回查询到的句柄
    */
    static int find_services_peer(const soar::OGRE_PEER_ID& peer_id,
                                  svc_tcp*& svchanle);

    ///对没有链接的的服务器进行重连
    static int connect_all_server();

protected:

    ///
    typedef zce::lord_rings<svc_tcp*> POOL_OF_TCP_HANDLER;

    ///一个未能连接上的Connect 最大可以等待发送的FRAME数量
    static const size_t   MAX_LEN_OF_SEND_LIST = 8;

    ///ACCEPT PEER最大可以等待发送的FRAME数量
    static const size_t   MAX_OF_ACCEPT_PEER_SEND_DEQUE = 64;
    ///CONNECT PEER最大可以等待发送的FRAME数量,
    static const size_t   MAX_OF_CONNECT_PEER_SEND_DEQUE = 256;

protected:

    ///最大能够Accept的PEER数量,
    static size_t                  max_accept_svr_;
    ///最大能够Connect的PEER数量
    static size_t                  max_connect_svr_;

    ///Connect后等待动作的时长,Connect超时
    static unsigned int           accept_timeout_;

    ///等待接受一个完整数据的超时时间,为0表示不限制
    static unsigned int           receive_timeout_;

    ///对于错误的数据,尝试发送的次数,只是了保证一定的网络瞬断
    static unsigned int           error_try_num_;

    ///SVRINFO对应的PEER的HASHMAP
    static tcppeer_set            svr_peer_hdl_set_;

    ///要自动链接的服务器的管理类
    static auto_connect           zerg_auto_connect_;

    ///已经Accept的PEER数量
    static size_t                 num_accept_peer_;
    ///已经Connect的PEER数量
    static size_t                 num_connect_peer_;

    ///ACCEPT SVC handler的池子
    static POOL_OF_TCP_HANDLER    pool_of_acpthdl_;

    ///CONNECT svc handler的池子
    static POOL_OF_TCP_HANDLER    pool_of_cnthdl_;

protected:

    ///服务模式
    OGRE_HANDLER_MODE             handler_mode_;

    ///接收数据的缓冲
    soar::ogre4a_frame* rcv_buffer_;

    ///发送的数据可能要排队
    zce::lord_rings<soar::ogre4a_frame*>  \
        snd_buffer_deque_;

    ///这个PEER接受数据
    size_t                        recieve_bytes_;

    ///这个PEER发送数据
    size_t                        send_bytes_;

    ///ACE Socket Stream,
    zce::skt::stream             socket_peer_;

    ///PEER连接的远端IP地址信息,有他不过是为了加快速度.
    zce::skt::addr_in               remote_address_;

    ///PEER连接的本地IP地址信息,有他不过是为了加快速度.
    zce::skt::addr_in               local_address_;

    //是否处于活动状态
    PEER_STATUS                   peer_status_;

    ///连接后无反应超时的TimeID,
    int                           timeout_time_id_;

    ///一个时间间隔内接受数据的次数
    unsigned int                  receive_times_;

    ///是否被强制要求在发送完成后关闭端口
    bool                          if_force_close_;

    ///判断接收到的数据是否是完整的处理函数指针
    FP_JudgeRecv_WholeFrame       fp_judge_whole_frame_;
};
}
