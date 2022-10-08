#pragma once

#include "zerg/ip_restrict.h"
#include "zerg/configure.h"
#include "zerg/comm_manager.h"

//forward declaration
class soar::zerg_frame;
class zerg_config;

namespace zerg
{
class svc_udp : public zce::event_handler
{
protected:

    //避免在堆中间分配,所以析构函数不要
public:
    //
    svc_udp(const soar::SERVICES_ID& my_svcinfo,
            const zce::skt::addr_in& addr,
            bool sessionkey_verify = true);
protected:
    ~svc_udp();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int read_event();
    //
    virtual int close_event();

public:

    //初始化UPD端口
    int init_udp_services();

protected:

    //从PEER读取数据
    int read_data_from_udp(size_t& szrevc);

    //发送UDP的数据
    int write_data_to_udp(soar::zerg_frame* send_frame);

public:
    //初始化静态参数
    static int init_all_static_data();

    //
    static int send_all_to_udp(soar::zerg_frame* send_frame);

    ///读取配置
    static int get_config(const zerg_config* config);

protected:

    //一次从UDP的端口读取的数据数量，
    static const size_t ONCE_MAX_READ_UDP_NUMBER = 256;

protected:

    ///
    typedef std::vector< svc_udp*>  ARY_OF_UDPSVC_HANDLER;

    ///UPD的数组，可以有多个UDP
    static ARY_OF_UDPSVC_HANDLER    ary_udpsvc_handler_;

    ///统计，使用单子类的指针
    static soar::stat_monitor* server_status_;

    ///通讯管理器,保存是为了加快速度
    static zerg::comm_manager* zerg_comm_mgr_;

    ///是否是代理服务器
    static bool                     if_proxy_;

protected:

    ///数据包UDP发送的Socket
    zce::skt::datagram        dgram_peer_;

    ///邦定的地址
    zce::skt::addr_in         udp_bind_addr_;

    ///
    soar::SERVICES_ID         my_svc_info_;
    ///是否进行SESSION校验
    bool                      sessionkey_verify_;
    ///数据缓冲区，UDP只有一个
    zce::queue_buffer* dgram_databuf_;
    ///IP限制管理器
    zerg::IPRestrict_Mgr* ip_restrict_;
};
}