

#ifndef OGRE_COMMUNICATE_MANGER_H_
#define OGRE_COMMUNICATE_MANGER_H_

class OgreTCPAcceptHandler;
class OgreUDPSvcHandler;
class Ogre4a_AppFrame;
class Ogre_Connect_Server;

//通讯集中管理器
class Ogre_Comm_Manger
{
protected:
    //一次最多发送1024帧
    static const unsigned int MAX_ONCE_SEND_FRAME = 1024;

protected:

    //侦听端口的Handler句柄队列
    std::vector<OgreTCPAcceptHandler *> accept_handler_ary_;
    //UDP端口的Handler句柄队列
    std::vector<OgreUDPSvcHandler *>    udp_handler_ary_;

protected:
    //
    static Ogre_Comm_Manger              *instance_;

public:

    Ogre_Comm_Manger();
    ~Ogre_Comm_Manger();

    //读取配置文件
    int get_configure(Zen_INI_PropertyTree &cfg_file);
    //
    int init_accept_by_conf(Zen_INI_PropertyTree &cfg_file);
    //
    int init_udp_by_conf(Zen_INI_PropertyTree &cfg_file);

    //从SEND管道找到所有的数据去发送,
    int get_all_senddata_to_write(size_t &procframe);

    //检查安全端口
    int check_safe_port(ZCE_Sockaddr_In &inetadd);

    //初始化通讯管理器
    int init_comm_manger(Zen_INI_PropertyTree &cfg_file);
    //注销通讯管理器
    int uninit_comm_manger();

public:
    //得到单子的实例
    static Ogre_Comm_Manger *instance();
    //清理单子的实例
    static void clean_instance();
};

#endif //#ifndef _OGRE_COMMUNICATE_MANGER_H_

