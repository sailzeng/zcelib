#ifndef ZERG_UDP_CONTROL_SERVICE_H_
#define ZERG_UDP_CONTROL_SERVICE_H_





//forward declaration
class Zerg_App_Frame;
class Zerg_Buffer;
class Zerg_IPRestrict_Mgr;
class Zerg_Comm_Manager;
class Zerg_Server_Config;

class UDP_Svc_Handler : public ZCE_Event_Handler
{
protected:

    //避免在堆中间分配,所以析构函数不要
public:
    //
    UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                    const ZCE_Sockaddr_In &addr,
                    bool sessionkey_verify = true);
protected:
    ~UDP_Svc_Handler();


public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input();
    //
    virtual int handle_close();

public:

    //初始化UPD端口
    int init_udp_services();

protected:

    //从PEER读取数据
    int read_data_from_udp(size_t &szrevc);

    //发送UDP的数据
    int write_data_to_udp(Zerg_App_Frame *send_frame);

public:
    //初始化静态参数
    static int init_all_static_data();

    //
    static int send_all_to_udp(Zerg_App_Frame *send_frame);

    //
    static int get_config(const Zerg_Server_Config *config);

protected:

    //一次从UDP的端口读取的数据数量，
    static const size_t ONCE_MAX_READ_UDP_NUMBER = 256;

protected:

    //数据包
    ZCE_Socket_DataGram      dgram_peer_;

    //邦定的地址
    ZCE_Sockaddr_In          udp_bind_addr_;

    //
    SERVICES_ID              my_svc_info_;
    //是否进行SESSION校验
    bool                     sessionkey_verify_;
    //数据缓冲区，UDP只有一个
    Zerg_Buffer             *dgram_databuf_;
    //IP限制管理器
    Zerg_IPRestrict_Mgr      *ip_restrict_;

protected:
    //
    typedef std::vector< UDP_Svc_Handler *>  ARY_OF_UDPSVC_HANDLER;

    //UPD的数组，可以有多个UDP
    static ARY_OF_UDPSVC_HANDLER    ary_udpsvc_handler_;

    //统计，使用单子类的指针
    static Comm_Stat_Monitor       *server_status_;

    //通讯管理器,保存是为了加快速度
    static Zerg_Comm_Manager       *zerg_comm_mgr_;

    //是否是代理服务器
    static bool                     if_proxy_;


};



#endif //#ifndef _ZERG_UDP_CONTROL_SERVICE_H_

