
#include "zerg_predefine.h"
#include "zerg_auto_connect.h"
#include "zerg_application.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_configure.h"


/****************************************************************************************************
class Zerg_Auto_Connector
****************************************************************************************************/
Zerg_Auto_Connector::Zerg_Auto_Connector()
{
}
//
Zerg_Auto_Connector::~Zerg_Auto_Connector()
{
}

//取得配置信息
int Zerg_Auto_Connector::get_config(const Zerg_Server_Config *config)
{
    int ret = 0;

    zerg_svr_cfg_ = config;

    size_of_wantconnect_ = config->zerg_cfg_data_.auto_connect_num_;

    //循环处理所有的数据
    SERVICES_INFO svc_route;
    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {

        svc_route.svc_id_ = config->zerg_cfg_data_.auto_connect_svrs_[i];

        //找到相关的IP配置
        ret = config->get_svcinfo_by_svcid(svc_route.svc_id_,
                                           svc_route);

        //如果查询不到
        if (ret != 0)
        {
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] Count find Auto Connect Services Info SvrType=%u,SvrID=%u .Please Check Config file. ",
                       svc_route.svc_id_.services_type_,
                       svc_route.svc_id_.services_id_);
            return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
        }

        ary_want_connect_.push_back(svc_route);


        //由于该死的C/C++的返回静态指针的问题，这儿要输出两个地址，所以只能先打印到其他地方
        const size_t TMP_ADDR_LEN = 32;
        char mainroute_addr[TMP_ADDR_LEN + 1];

        ZCE_LOGMSG(RS_INFO, "[zergsvr] Add one auto connect data, main route services id[%u|%u] ip[%s|%u].",
                   svc_route.svc_id_.services_type_,
                   svc_route.svc_id_.services_id_,
                   svc_route.ip_address_.get_host_addr(mainroute_addr, TMP_ADDR_LEN),
                   svc_route.ip_address_.get_port_number()
                  );

        auto map_iter = type_to_idary_map_.find(svc_route.svc_id_.services_type_);
        if (type_to_idary_map_.end() == map_iter)
        {
            std::vector<uint32_t> id_ary;
            auto insert_iter = type_to_idary_map_.insert(
                                   std::make_pair(svc_route.svc_id_.services_type_, id_ary));
            //理论上除非空间不足，不可能失败
            ZCE_ASSERT(false == insert_iter.second);

            map_iter = insert_iter.first;
            map_iter->second.reserve(size_of_wantconnect_ + 16);
        }
        std::vector<uint32_t> *ptr_ary = &(map_iter->second);
        ptr_ary->push_back(svc_route.svc_id_.services_id_);
    }

    ZCE_LOGMSG(RS_INFO, "[zergsvr] Get number [%lu] auto connect config success.",
               size_of_wantconnect_);

    return 0;
}


//链接所有的服务器,如果已经有链接，就跳过,
void Zerg_Auto_Connector::reconnect_allserver(size_t &szvalid, size_t &szsucc, size_t &szfail)
{
    int ret = 0;
    szvalid = szsucc = szfail = 0;
    //循环将所有的SERVER链接一次,代码写的很绕口

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        TCP_Svc_Handler *svchandle = NULL;
        //如果已经有相应的链接了，跳过
        ret = TCP_Svc_Handler::find_services_peer(ary_want_connect_[i].svc_id_, svchandle);

        if (0 != ret)
        {
            //进行连接,
            ret = connect_server_bysvcid(ary_want_connect_[i].svc_id_, ary_want_connect_[i].ip_address_);

            if (ret == 0)
            {
                ++szsucc;
            }
            else
            {
                ++szfail;
            }
        }
        else
        {
            ++szvalid;

            //如果是激活状态才进行心跳包发送
            if (svchandle->get_peer_status() == TCP_Svc_Handler::PEER_STATUS_ACTIVE)
            {
                svchandle->send_zergheatbeat_reg();
            }
        }
    }

    ZCE_LOGMSG(RS_INFO, "[zergsvr] Auto NONBLOCK connect server,vaild number:%d ,success Number :%d,fail number:%d .",
               szvalid,
               szsucc,
               szfail);
    return;
}


//根据SVC ID,检查是否是主动连接的服务.,
int Zerg_Auto_Connector::reconnect_server(const SERVICES_ID &reconnect_svcid)
{

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        if (ary_want_connect_[i].svc_id_ == reconnect_svcid)
        {
            ZCE_Sockaddr_In     inetaddr = ary_want_connect_[i].ip_address_;
            return connect_server_bysvcid(reconnect_svcid, inetaddr);
        }
    }

    //这个错误无需记录,可能是这个SVCID根本不是要主动连接的服务器
    return SOAR_RET::ERR_ZERG_ISNOT_CONNECT_SERVICES;
}


//根据SVRINFO+IP,检查是否是主动连接的服务.并进行连接
int Zerg_Auto_Connector::connect_server_bysvcid(const SERVICES_ID &svrinfo, const ZCE_Sockaddr_In     &inetaddr)
{
    ZLOG_DEBUG("[zergsvr] Try NONBLOCK connect services[%u|%u] IP|Port :[%s|%u] .",
               svrinfo.services_type_,
               svrinfo.services_id_,
               inetaddr.get_host_addr(),
               inetaddr.get_port_number()
              );

    ZCE_Socket_Stream sockstream;
    sockstream.open();

    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    sockstream.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    sockstream.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);

    //tcpscoket.sock_enable (O_NONBLOCK);

    //记住,是这个时间标志使SOCKET异步连接,第3个参数true表示是非阻塞
    int ret = zerg_connector_.connect(sockstream, &inetaddr, true);

    //必然失败!?
    if (ret < 0)
    {
        //按照UNIX网络编程 V1的说法是 EINPROGRESS,但ACE的介绍说是 EWOULDBLOCK,
        if (ZCE_LIB::last_error() != EWOULDBLOCK && ZCE_LIB::last_error() != EINPROGRESS)
        {
            sockstream.close();
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }


        //HANDLER_MODE_CONNECT模式理论不会失败
        TCP_Svc_Handler *p_handler = TCP_Svc_Handler::alloce_hdl_from_pool(
                                         TCP_Svc_Handler::HANDLER_MODE_CONNECT);
        ZCE_ASSERT(p_handler);
        //以self_svc_info出去链接其他服务器.
        p_handler->init_tcpsvr_handler(zerg_svr_cfg_->self_svc_id_,
                                       svrinfo,
                                       sockstream,
                                       inetaddr);

        //避免析构的时候close句柄
        sockstream.release_noclose();
    }
    //tmpret == 0 那就是让我去跳楼,但按照 UNIX网络编程 说应该是有本地连接时可能的.(我的测试还是返回错误)
    //而ACE的说明是立即返回错误,我暂时不处理这种情况,实在不行又只有根据类型写晦涩的朦胧诗了
    else
    {
        ZCE_LOGMSG(RS_ERROR, "[zergsvr] My God! NonBlock Socket Connect Success , ACE is a cheat.");
    }

    return 0;
}

size_t Zerg_Auto_Connector::numsvr_connect()
{
    return size_of_wantconnect_;
}

//根据services_type查询对应的配置主备服务器列表数组 MS（主备）
int Zerg_Auto_Connector::find_confms_svcid_ary(uint16_t services_type,
                                               std::vector<uint32_t> *& ms_svcid_ary)
{
    auto map_iter = type_to_idary_map_.find(services_type);
    if (type_to_idary_map_.end() == map_iter)
    {
        return -1;
    }
    ms_svcid_ary = &(map_iter->second);
    return 0;
}