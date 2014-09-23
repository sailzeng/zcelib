
//Jovi(HuangHao),Sail(ZENGXING)

#ifndef OGRE_SERVER_AUTO_CONNECT_H_
#define OGRE_SERVER_AUTO_CONNECT_H_

class Ogre_TCP_Svc_Handler;

struct PEER_FPRECV_MODULE
{
public:
    //
    Socket_Peer_Info        peer_socket_info_;

    //
    std::string             rec_mod_file_;
    //TCP收取数据的模块HANDLER
    ZEN_SHLIB_HANDLE        recv_mod_handler_;
    //
    FPJudgeRecvWholeFrame   fp_judge_whole_frame_;
};

typedef std::vector <PEER_FPRECV_MODULE> ARRAY_OF_PEER_FPRECV_MODULE;

class Ogre_Connect_Server
{
protected:

    //
    ZEN_Socket_Connector            ogre_connector_;
    //
    ARRAY_OF_PEER_FPRECV_MODULE     ary_peer_fprecv_module_;

public:

    //构造函数
    Ogre_Connect_Server();
    ~Ogre_Connect_Server();

    //
    int get_configure(Zen_INI_PropertyTree &cfg_file);

    //链接所有的服务器
    int connect_all_server(size_t &szserver, size_t &szsucc) ;

    size_t num_svr_to_connect() const;

    //根据SVRINFO,检查是否是主动连接的服务.并进行连接
    int connect_server_by_svrinfo(const Socket_Peer_Info &svrinfo) ;

};

#endif //_OGRE_SERVER_AUTO_CONNECT_H_

