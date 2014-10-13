
//Jovi(HuangHao),Sail(ZENGXING)

#ifndef OGRE_SERVER_AUTO_CONNECT_H_
#define OGRE_SERVER_AUTO_CONNECT_H_


#include "ogre_configure.h"

class Ogre_TCP_Svc_Handler;



typedef std::vector <TCP_PEER_MODULE_INFO> ARRAY_OF_PEER_FPRECV_MODULE;




class Ogre_Connect_Server
{
public:

    //构造函数
    Ogre_Connect_Server();
    ~Ogre_Connect_Server();

    ///读取配置
    int get_config(const Ogre_Server_Config *config);

    //链接所有的服务器
    int connect_all_server(size_t &szserver, size_t &szsucc) ;


    //根据SVRINFO,检查是否是主动连接的服务.并进行连接
    int connect_server_by_peerid(const SOCKET_PERR_ID &socket_peer) ;


protected:

    ///
    ZCE_Socket_Connector ogre_connector_;

    ///
    size_t auto_connect_num_ = 0;
    ///
    TCP_PEER_MODULE_INFO     autocnt_module_ary_[OGRE_CONFIG_DATA::MAX_AUTO_CONNECT_PEER_NUM];

};

#endif //OGRE_SERVER_AUTO_CONNECT_H_

