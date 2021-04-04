

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
        //���ڵ�������Ĭ��const �ģ�ֻ������д���������Υ���Ҷ�MAPʹ��SETʵ�ֲ�����֪��
        TCP_PEER_MODULE_INFO peer_module =  *iter_tmp;
        peer_module.close_module();
    }
}

//ȡ������
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
            size_t use_len = 0;
            ZCE_LOG(RS_ERROR, "Insert fail,may be have repeat peer id [%u|%u] ip[%s],please check your config .",
                    peer_module.peer_id_.peer_ip_address_,
                    peer_module.peer_id_.peer_port_,
                    peer_module.peer_info_.peer_socketin_.to_string(out_buf, 32,use_len)
                   );
            return SOAR_RET::ERR_OGRE_CFG_REPEAT_PEERID;
        }
    }

    return 0;
}


//�������еķ�����
//Ҫ���ӵķ���������,�ɹ���ʼ���ӵķ���������,
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
    //���ؿ�ʼ���ӵķ���������
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
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //����Ѿ��������ˣ�����������
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

    ZCE_LOG(RS_INFO, "Try NONBLOCK connect server IP|Port :[%s] .\n",
            inetaddr.to_string(ip_addr_str,IP_ADDR_LEN,use_len));

    //��ס,�����ʱ���־ʹSOCKET�첽����,
    ret = ogre_connector_.connect(tcpscoket, &inetaddr, true);

    //��Ȼʧ��!?
    if (ret < 0)
    {
        //����UNIX������ V1��˵���� EINPROGRESS,��ACE�Ľ���˵�� EWOULDBLOCK,
        if (zce::last_error() != EWOULDBLOCK && zce::last_error() != EINPROGRESS)
        {
            tcpscoket.close();
            return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }

        //�ӳ�����ȡ��HDL����ʼ��֮��CONNECThdl������������С��0
        Ogre_TCP_Svc_Handler *connect_hdl = Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool(
                                                Ogre_TCP_Svc_Handler::HANDLER_MODE_CONNECT);
        ZCE_ASSERT(connect_hdl);
        connect_hdl->init_tcp_svc_handler(tcpscoket, inetaddr, peer_module.fp_judge_whole_frame_);

    }
    //tmpret == 0 �Ǿ�������ȥ��¥,������ UNIX������ ˵Ӧ�����б�������ʱ���ܵ�.(�ҵĲ��Ի��Ƿ��ش���)
    //��ACE��˵�����������ش���,����ʱ�������������,ʵ�ڲ�����ֻ�и�������д��ɬ������ʫ��
    else
    {
        ZCE_LOG(RS_ERROR, "My God! NonBlock Socket Connect Success , ACE is a cheat.\n");
        ZCE_LOG(RS_ERROR, "My God! NonBlock Socket Connect Success , ACE is a cheat....\n");
    }

    return 0;
}