

#ifndef OGRE_COMMUNICATE_MANGER_H_
#define OGRE_COMMUNICATE_MANGER_H_

class Ogre_TCPAccept_Hdl;
class Ogre_UDPSvc_Hdl;
class Ogre4a_App_Frame;
class Ogre_Connect_Server;
class Ogre_Server_Config;

//ͨѶ���й�����
class Ogre_Comm_Manger
{


public:

    Ogre_Comm_Manger();
    ~Ogre_Comm_Manger();

    ///��ȡ�����ļ�
    int get_config(const Ogre_Server_Config *config);

    //��SEND�ܵ��ҵ����е�����ȥ����,
    int get_all_senddata_to_write(size_t &procframe);

    //��鰲ȫ�˿�
    int check_safe_port(ZCE_Sockaddr_In &inetadd);

    //��ʼ��ͨѶ������
    int init_comm_manger();
    //ע��ͨѶ������
    int uninit_comm_manger();

public:
    //�õ����ӵ�ʵ��
    static Ogre_Comm_Manger *instance();
    //�����ӵ�ʵ��
    static void clean_instance();

protected:

    //һ����෢��1024֡
    static const unsigned int MAX_ONCE_SEND_FRAME = 1024;

protected:
    //
    static Ogre_Comm_Manger *instance_;

protected:

    //����ʵ��ָ��
    const Ogre_Server_Config *ogre_config_;
};

#endif //#ifndef OGRE_COMMUNICATE_MANGER_H_

