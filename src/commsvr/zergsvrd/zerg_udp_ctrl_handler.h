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

    //�����ڶ��м����,��������������Ҫ
public:
    //
    UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                    const ZCE_Sockaddr_In &addr,
                    bool sessionkey_verify = true);
protected:
    ~UDP_Svc_Handler();


public:
    //ȡ�þ��
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input();
    //
    virtual int handle_close();

public:

    //��ʼ��UPD�˿�
    int init_udp_services();

protected:

    //��PEER��ȡ����
    int read_data_from_udp(size_t &szrevc);

    //����UDP������
    int write_data_to_udp(Zerg_App_Frame *send_frame);

public:
    //��ʼ����̬����
    static int init_all_static_data();

    //
    static int send_all_to_udp(Zerg_App_Frame *send_frame);

    ///��ȡ����
    static int get_config(const Zerg_Server_Config *config);

protected:

    //һ�δ�UDP�Ķ˿ڶ�ȡ������������
    static const size_t ONCE_MAX_READ_UDP_NUMBER = 256;

protected:

    ///
    typedef std::vector< UDP_Svc_Handler *>  ARY_OF_UDPSVC_HANDLER;

    ///UPD�����飬�����ж��UDP
    static ARY_OF_UDPSVC_HANDLER    ary_udpsvc_handler_;

    ///ͳ�ƣ�ʹ�õ������ָ��
    static Soar_Stat_Monitor       *server_status_;

    ///ͨѶ������,������Ϊ�˼ӿ��ٶ�
    static Zerg_Comm_Manager       *zerg_comm_mgr_;

    ///�Ƿ��Ǵ��������
    static bool                     if_proxy_;


protected:

    ///���ݰ�UDP���͵�Socket
    ZCE_Socket_DataGram      dgram_peer_;

    ///��ĵ�ַ
    ZCE_Sockaddr_In          udp_bind_addr_;

    ///
    SERVICES_ID              my_svc_info_;
    ///�Ƿ����SESSIONУ��
    bool                     sessionkey_verify_;
    ///���ݻ�������UDPֻ��һ��
    Zerg_Buffer             *dgram_databuf_;
    ///IP���ƹ�����
    Zerg_IPRestrict_Mgr      *ip_restrict_;



};



#endif //#ifndef _ZERG_UDP_CONTROL_SERVICE_H_

