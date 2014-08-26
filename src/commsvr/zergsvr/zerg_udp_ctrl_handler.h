
#ifndef ZERG_UDP_CONTROL_SERVICE_H_
#define ZERG_UDP_CONTROL_SERVICE_H_

//forward declaration
class Zerg_App_Frame;
class Zerg_Buffer;
class UDPSessionKeyMgr;
class Zerg_IPRestrict_Mgr;
class Zerg_Comm_Manager;

class UDP_Svc_Handler: public ZCE_Event_Handler
{
protected:

    //避免在堆中间分配,所以析构函数不要
public:
    // 监听handler的初始化
    UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                    const ZCE_Sockaddr_In &addr,
                    bool sessionkey_verify = true,
                    bool is_external_pkg = false,
                    uint32_t cmd_offset = 0,
                    uint32_t cmd_len = 0);

    // 主动发送handler的初始化
    UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                    const SERVICES_ID &peer_svcinfo,
                    const ZCE_Sockaddr_In &peer_addr,
                    bool is_external_pkg = false,
                    uint32_t cmd_offset = 0,
                    uint32_t cmd_len = 0);
protected:
    ~UDP_Svc_Handler();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input();
    //
    virtual int handle_close();

    //主动发送数据的handler初始化
    int init_udpsvr_handler();

public:

    //初始化UPD端口
    int init_udp_services();

protected:

    //从PEER读取数据
    int read_data_from_udp(size_t &szrevc);

    //发送UDP的数据
    int write_data_to_udp(Zerg_App_Frame *send_frame);

    // 获取外部协议包的cmd
    int get_external_pkg_cmd(uint32_t &cmd, const uint8_t *buff, const ssize_t buf_len);

public:
    //初始化静态参数
    static int init_all_static_data();

    //
    static int send_all_to_udp(Zerg_App_Frame *send_frame);

    //
    static int get_udpctrl_conf(const conf_zerg::ZERG_CONFIG *config);

    //根据有的SVR INFO，查询相应的handle
    static int find_services_peer(const SERVICES_ID &svr_info, UDP_Svc_Handler *&svchanle);

protected:

    //一次从UDP的端口读取的数据数量，
    static const size_t ONCE_MAX_READ_UDP_NUMBER = 256;

protected:

    //数据包
    ZCE_Socket_DataGram      dgram_peer_;

    // 自己的监听地址
    ZCE_Sockaddr_In          self_udp_addr_;

    // 对方的地址
    ZCE_Sockaddr_In          peer_udp_addr_;

    // 自己的type和id
    SERVICES_ID              self_svc_info_;

    // 对方的type和id
    SERVICES_ID              peer_svc_info_;

    //是否进行SESSION校验
    bool                     sessionkey_verify_;

    // 是否是接收框架包
    bool                    is_external_pkg_;
    // 如果是接收非框架包，需要知道cmd偏移
    uint32_t                cmd_offset_;
    uint32_t                cmd_len_;

    //数据缓冲区，UDP只有一个
    Zerg_Buffer             *dgram_databuf_;
    //IP限制管理器
    Zerg_IPRestrict_Mgr      *ip_restrict_;

    // 本handler的类型，是“监听handler”还是“主动发送handler”
    bool is_initiative_handler_;

protected:
    //
    typedef std::vector< UDP_Svc_Handler *>  ARY_OF_UDPSVC_HANDLER;

    //UPD的数组，可以有多个UDP
    static ARY_OF_UDPSVC_HANDLER    ary_udpsvc_handler_;

    //UDP的SESSION管理器
    static UDPSessionKeyMgr        *udp_session_mgr_;

    //统计，使用单子类的指针
    static Comm_Stat_Monitor       *server_status_;

    //通讯管理器,保存是为了加快速度
    static Zerg_Comm_Manager       *zerg_comm_mgr_;

    //是否是代理服务器
    static bool                     if_proxy_;

    // game_id
    static unsigned int             game_id_;

};

#endif //#ifndef _ZERG_UDP_CONTROL_SERVICE_H_

