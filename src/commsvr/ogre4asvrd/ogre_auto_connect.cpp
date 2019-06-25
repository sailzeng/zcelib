

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
    SET_OF_TCP_PEER_MODULE::iterator iter_tmp = autocnt_module_set_.begin();
    SET_OF_TCP_PEER_MODULE::iterator iter_end = autocnt_module_set_.end();
    for (; iter_tmp != iter_end; ++iter_tmp)
    {
        //由于迭代器是默认const 的，只能这样写，这个问题违背我对MAP使用SET实现部分认知，
        TCP_PEER_MODULE_INFO peer_module =  *iter_tmp;
        peer_module.close_module();
    }
}

//取得配置
int Ogre_Connect_Server::get_config(const Ogre_Server_Config *config)
{
    int ret = 0;
    auto_connect_num_ = config->ogre_cfg_data_.auto_connect_num_;

    for (size_t i = 0; i < auto_connect_num_; ++i)
    {
        TCP_PEER_MODULE_INFO peer_module;

        peer_module.peer_info_ = config->ogre_cfg_data_.auto_cnt_peer_ary_[i];

        peer_module.peer_id_.set(peer_module.peer_info_.peer_socketin_);
        ret = peer_module.open_module();
        if (ret != 0)
        {
            return ret;
        }

        auto insert_ret = autocnt_module_set_.insert(peer_module);
        if (insert_ret.second == false)
        {
            char out_buf[32 + 1];
            out_buf[32] = '\0';
            ZCE_LOG(RS_ERROR, "Insert fail,may be have repeat peer id [%u|%u] ip[%s],please check your config .",
                    peer_module.peer_id_.peer_ip_address_,
                    peer_module.peer_id_.peer_port_,
                    peer_module.peer_info_.peer_socketin_.to_string(out_buf, 32)
                   );
            return SOAR_RET::ERR_OGRE_CFG_REPEAT_PEERID;
        }
    }

    return 0;
}


//链接所有的服务器
//要链接的服务器总数,成功开始连接的服务器个数,
int Ogre_Connect_Server::connect_all_server(size_t &num_vaild, size_t &num_succ, size_t &num_fail)
{
    int ret = 0;

    num_vaild = 0;
    num_succ = 0;
    num_fail = 0;

    SET_OF_TCP_PEER_MODULE::iterator iter_tmp = autocnt_module_set_.begin();
    SET_OF_TCP_PEER_MODULE::iterator iter_end = autocnt_module_set_.end();
    for (; iter_tmp != iter_end; ++iter_tmp)
    {
        ret = connect_one_server(*iter_tmp);
        if (0 == ret)
        {
            ++num_succ;
        }
        else
        {
            if (SOAR_RET::ERR_OGRE_ALREADY_CONNECTED == ret)
            {
                ++num_vaild;
            }
            else
            {
                ++num_fail;
            }
        }
    }

    ZCE_LOG(RS_INFO, "Auto NONBLOCK Connect Server,vaild number :%u,success :%u fail %u.\n",
            num_vaild,
            num_succ,
            num_fail);
    //返回开始连接的服务器数量
    return 0;
}


int Ogre_Connect_Server::connect_server_by_peerid(const OGRE_PEER_ID &socket_peer)
{
    TCP_PEER_MODULE_INFO peer_module;
    peer_module.peer_id_ = socket_peer;

    auto find_ret = autocnt_module_set_.find(peer_module);

    if (autocnt_module_set_.end() == find_ret)
    {
        ZCE_LOG(RS_ERROR, "Can't find connect peer id [%u|%u],please check your code .",
                socket_peer.peer_ip_address_,
                socket_peer.peer_port_
               );
        return SOAR_RET::ERR_OGRE_CANNOT_FIND_PEERID;
    }
    return connect_one_server(*find_ret);
}


int Ogre_Connect_Server::connect_one_server(const TCP_PEER_MODULE_INFO &peer_module)
{
    int ret = 0;

    //如果已经连接上了，不进行连接
    Ogre_TCP_Svc_Handler *tcp_hdl = NULL;
    ret = Ogre_TCP_Svc_Handler::find_services_peer(peer_module.peer_id_, tcp_hdl);
    if (ret == 0)
    {
        return SOAR_RET::ERR_OGRE_ALREADY_CONNECTED;
    }

    ZCE_Sockaddr_In inetaddr(peer_module.peer_id_.peer_ip_address_,
                             peer_module.peer_id_.peer_port_);

    ZCE_Socket_Stream tcpscoket;
    tcpscoket.sock_enable(O_NONBLOCK);

    ZCE_LOG(RS_INFO, "Try NONBLOCK connect server IP|Port :[%s|%u] .\n",
            inetaddr.get_host_addr(),
            inetaddr.get_port_number());

    //记住,是这个时间标志使SOCKET异步连接,
    ret = ogre_connector_.connect(tcpscoket, &inetaddr, true);

    //必然失败!?
    if (ret < 0)
    {
        //按照UNIX网络编程 V1的说法是 EINPROGRESS,但ACE的介绍说是 EWOULDBLOCK,
        if (zce::last_error() != EWOULDBLOCK && zce::last_error() != EINPROGRESS)
        {
            tcpscoket.close();
            return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }

        //从池子中取得HDL，初始化之，CONNECThdl的数量不可能小于0
        Ogre_TCP_Svc_Handler *connect_hdl = Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool(
                                                Ogre_TCP_Svc_Handler::HANDLER_MODE_CONNECT);
        ZCE_ASSERT(connect_hdl);
        connect_hdl->init_tcp_svc_handler(tcpscoket, inetaddr, peer_module.fp_judge_whole_frame_);

    }
    //tmpret == 0 那就是让我去跳楼,但按照 UNIX网络编程 说应该是有本地连接时可能的.(我的测试还是返回错误)
    //而ACE的说明是立即返回错误,我暂时不处理这种情况,实在不行又只有根据类型写晦涩的朦胧诗了
    else
    {
        ZCE_LOG(RS_ERROR, "My God! NonBlock Socket Connect Success , ACE is a cheat.\n");
        ZCE_LOG(RS_ERROR, "My God! NonBlock Socket Connect Success , ACE is a cheat....\n");
    }

    return 0;
}