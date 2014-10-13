

#include "ogre_predefine.h"
#include "ogre_auto_connect.h"
#include "ogre_application.h"
#include "ogre_tcp_ctrl_handler.h"

//
Ogre_Connect_Server::Ogre_Connect_Server()
{

}
//
Ogre_Connect_Server::~Ogre_Connect_Server()
{
    for (size_t i = 0; i < auto_connect_num_; ++i)
    {
        autocnt_module_ary_[i].close_module();
    }
}

//取得配置
int Ogre_Connect_Server::get_config(const Ogre_Server_Config *config)
{
    int ret = 0;
    auto_connect_num_ = config->ogre_cfg_data_.auto_connect_num_;

    for (size_t i = 0; i < auto_connect_num_; ++i)
    {
        autocnt_module_ary_[i].peer_info_ = config->ogre_cfg_data_.auto_cnt_peer_ary_[i];

        autocnt_module_ary_[i].peer_id_.set(autocnt_module_ary_[i].peer_info_.peer_socketin_);
        ret = autocnt_module_ary_[i].open_module();
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}


//链接所有的服务器
//要链接的服务器总数,成功开始连接的服务器个数,
int Ogre_Connect_Server::connect_all_server(size_t &szserver, size_t &szsucc)
{
    int ret = 0;

    szsucc = 0;
    szserver = 0;
    //检查所有的服务器的模块十分正确
    for (size_t i = 0; i < auto_connect_num_; ++i)
    {
        ret = connect_server_by_peerid(autocnt_module_ary_[i].peer_id_);
        ++szserver;
        if (ret == 0)
        {
            ++szsucc;
        }
    }
    
    ZLOG_INFO( "Auto NONBLOCK Connect Server,Success Number :%d,Counter:%d .\n", szsucc, szserver);
    //返回开始连接的服务器数量
    return 0;
}


int Ogre_Connect_Server::connect_server_by_peerid(const SOCKET_PERR_ID &socket_peer)
{

    size_t i = 0;
    for (; i < auto_connect_num_; ++i)
    {
        if (autocnt_module_ary_[i].peer_id_ == socket_peer)
        {
            break;
        }
    }

    //如果没有找到
    if (auto_connect_num_ == i)
    {
        return SOAR_RET::ERR_OGRE_NO_FIND_SERVICES_INFO;
    }

    ZCE_Sockaddr_In inetaddr(socket_peer.peer_ip_address_, socket_peer.peer_port_);

    ZCE_Socket_Stream tcpscoket;
    tcpscoket.sock_enable(O_NONBLOCK);

    ZLOG_INFO( "Try NONBLOCK connect server IP|Port :[%s|%u] .\n", 
        inetaddr.get_host_addr(), 
        inetaddr.get_port_number());

    //记住,是这个时间标志使SOCKET异步连接,
    int ret = ogre_connector_.connect(tcpscoket, &inetaddr, true);

    //必然失败!?
    if (ret < 0)
    {
        //按照UNIX网络编程 V1的说法是 EINPROGRESS,但ACE的介绍说是 EWOULDBLOCK,
        if (ZCE_LIB::last_error() != EWOULDBLOCK && ZCE_LIB::last_error() != EINPROGRESS )
        {
            tcpscoket.close();
            return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }

        //从池子中取得HDL，初始化之，CONNECThdl的数量不可能小于0
        Ogre_TCP_Svc_Handler *connect_hdl = Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool(Ogre_TCP_Svc_Handler::HANDLER_MODE_CONNECT);
        ZCE_ASSERT(connect_hdl);
        connect_hdl->init_tcp_svc_handler(tcpscoket, inetaddr, autocnt_module_ary_[i].fp_judge_whole_frame_);

    }
    //tmpret == 0 那就是让我去跳楼,但按照 UNIX网络编程 说应该是有本地连接时可能的.(我的测试还是返回错误)
    //而ACE的说明是立即返回错误,我暂时不处理这种情况,实在不行又只有根据类型写晦涩的朦胧诗了
    else
    {
        ZLOG_ERROR( "My God! NonBlock Socket Connect Success , ACE is a cheat.\n");
        ZLOG_ERROR( "My God! NonBlock Socket Connect Success , ACE is a cheat....\n");
    }

    return 0;
}


