

#ifndef OGRE_COMMUNICATE_MANGER_H_
#define OGRE_COMMUNICATE_MANGER_H_

class Ogre_TCPAccept_Hdl;
class Ogre_UDPSvc_Hdl;
class Ogre4a_App_Frame;
class Ogre_Connect_Server;
class Ogre_Server_Config;

//通讯集中管理器
class Ogre_Comm_Manger
{


public:

    Ogre_Comm_Manger();
    ~Ogre_Comm_Manger();

    ///读取配置文件
    int get_config(const Ogre_Server_Config *config);

    //从SEND管道找到所有的数据去发送,
    int get_all_senddata_to_write(size_t &procframe);

    //检查安全端口
    int check_safe_port(ZCE_Sockaddr_In &inetadd);

    //初始化通讯管理器
    int init_comm_manger();
    //注销通讯管理器
    int uninit_comm_manger();

public:
    //得到单子的实例
    static Ogre_Comm_Manger *instance();
    //清理单子的实例
    static void clean_instance();

protected:

    //一次最多发送1024帧
    static const unsigned int MAX_ONCE_SEND_FRAME = 1024;

protected:
    //
    static Ogre_Comm_Manger *instance_;

protected:

    //配置实例指针
    const Ogre_Server_Config *ogre_config_;
};

#endif //#ifndef OGRE_COMMUNICATE_MANGER_H_

