
#include "zerg_predefine.h"
#include "zerg_auto_connect.h"
#include "zerg_application.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_configure.h"


/****************************************************************************************************
class Zerg_Auto_Connector
****************************************************************************************************/
Zerg_Auto_Connector::Zerg_Auto_Connector() :
    size_of_wantconnect_(0),
    zerg_svr_cfg_(NULL)
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
            ZLOG_ERROR("[zergsvr] Count find Auto Connect Services Info SvrType=%u,SvrID=%u .Please Check Config file. ",
                       svc_route.svc_id_.services_type_,
                       svc_route.svc_id_.services_id_);
            return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
        }

        ary_want_connect_.push_back(svc_route);


        //由于该死的C/C++的返回静态指针的问题，这儿要输出两个地址，所以只能先打印到其他地方
        const size_t TMP_ADDR_LEN = 32;
        char mainroute_addr[TMP_ADDR_LEN + 1];

        ZLOG_INFO("[zergsvr] Add one auto connect data, main route services id[%u|%u] ip[%s|%u].",
                  svc_route.svc_id_.services_type_,
                  svc_route.svc_id_.services_id_,
                  svc_route.ip_address_.get_host_addr(mainroute_addr, TMP_ADDR_LEN),
                  svc_route.ip_address_.get_port_number()
                 );
    }

    ZLOG_INFO("[zergsvr] Get number [%lu] auto connect config success.",
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

    ZLOG_INFO("[zergsvr] Auto NONBLOCK connect server,vaild number:%d ,success Number :%d,fail number:%d .",
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
        TCP_Svc_Handler *p_handler = TCP_Svc_Handler::AllocSvcHandlerFromPool(
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
        ZLOG_ERROR("[zergsvr] My God! NonBlock Socket Connect Success , ACE is a cheat.");
    }

    return 0;
}

size_t Zerg_Auto_Connector::numsvr_connect()
{
    return size_of_wantconnect_;
}

//根据svr type获取serviceid，有多个id时随机获取一个
int Zerg_Auto_Connector::get_server(unsigned short svr_type, SERVICES_ID *svrinfo)
{
    //从map中获取相应的index
    RouteType2Index::iterator iter = want_connect_type_2_index_.find(svr_type);
    if (iter == want_connect_type_2_index_.end())
    {
        return SOAR_RET::ERR_ZERG_CHOOSE_AUTO_CONNECT_SERVICES;
    }

    unsigned int index = iter->second;
    if (index >= list_of_want_connect_main_id_.size())
    {
        //ZLOG_ERROR("[zergsvr] Not found auto connect");
        return SOAR_RET::ERR_ZERG_CHOOSE_AUTO_CONNECT_SERVICES;
    }

    //随机获取一个id, 最多重复次数为id个数
    unsigned int id_num = (unsigned int)list_of_want_connect_main_id_[index].size();
    for (unsigned int i = 0; i < id_num; i++)
    {
        unsigned int rand_num = rand() % id_num;

        //首先看主路由svr是否处于连接状态
        svrinfo->set_svcid(svr_type, list_of_want_connect_main_id_[index][rand_num]);
        if (is_connected(*svrinfo))
        {
            return 0;
        }

        //主路由不处于连接状态, 则试着选择从路由svr
        svrinfo->set_svcid(svr_type, list_of_want_connect_back_id_[index][rand_num]);
        if (is_connected(*svrinfo))
        {
            return 0;
        }
    }

    //ZLOG_ERROR("[zergsvr] Not found auto connect");

    // 未找到相应的Svr
    return SOAR_RET::ERR_ZERG_CHOOSE_AUTO_CONNECT_SERVICES;
}

bool Zerg_Auto_Connector::is_connected(const SERVICES_ID &svrinfo)
{
    TCP_Svc_Handler *svchandle = NULL;
    // 主动连接目前还不支持udp
    int ret = TCP_Svc_Handler::find_services_peer(svrinfo, svchandle);

    if (ret != 0)
    {
        // 没找到
        return false;
    }

    return true;
}



// 重新加载主动连接配置
int Zerg_Auto_Connector::reload_cfg(const Zerg_Server_Config *config)
{
    // 重新加载主动连接时，未变化的连接不能断开，只断开去除的连接，增加新加的连接
    // 先保存现有的主动连接
    RouteType2Index old_want_connect_type_2_index = want_connect_type_2_index_;
    ListOfMainRouteId old_list_of_want_connect_main_id = list_of_want_connect_main_id_;
    ListOfBackRouteId old_list_of_want_connect_back_id = list_of_want_connect_back_id_;

    // 清理现有数据
    list_of_want_connect_back_id_.clear();
    list_of_want_connect_main_id_.clear();
    want_connect_type_2_index_.clear();
    ary_want_connect_.clear();

    int ret = get_config(config);
    if (ret != 0)
    {
        // 重新加载失败，则配置回退
        ZLOG_ERROR("zergsvr reload autoconnect fail. ret=%d", ret);
        want_connect_type_2_index_ = old_want_connect_type_2_index;
        list_of_want_connect_main_id_ = old_list_of_want_connect_main_id;
        list_of_want_connect_back_id_ = old_list_of_want_connect_back_id;
        return ret;
    }

    // 关闭那些改动的连接
    RouteType2Index::iterator iter = old_want_connect_type_2_index.begin();
    for (; iter != old_want_connect_type_2_index.end(); ++iter)
    {
        std::vector<unsigned int> &old_main_id_list = old_list_of_want_connect_main_id[iter->second];
        std::vector<unsigned int> &old_back_id_list = old_list_of_want_connect_back_id[iter->second];

        for (unsigned int i = 0; i < old_main_id_list.size(); ++i)
        {
            SERVICES_ID service(iter->first, old_main_id_list[i]);
            // 判定是否在当前主动连接中
            if (!is_current_auto_connect(service, true))
            {
                // 不在当前主动连接中，则关闭当前连接
                ZLOG_INFO("zergsvr close old auto connect. svr_type=%u svr_id=%u",
                          service.services_type_, service.services_id_);
                TCP_Svc_Handler::close_services_peer(service);
            }
        }

        for (unsigned int i = 0; i < old_back_id_list.size() && old_back_id_list[i] != 0; ++i)
        {
            SERVICES_ID service(iter->first, old_back_id_list[i]);
            // 判定是否在当前主动连接中
            if (!is_current_auto_connect(service, false))
            {
                // 不在当前主动连接中，则关闭当前连接
                TCP_Svc_Handler::close_services_peer(service);
            }
        }

    }

    return 0;
}

// 判定一个svr是否在当前的主动连接中
bool Zerg_Auto_Connector::is_current_auto_connect(const SERVICES_ID &service,
                                                  bool is_main_service)
{
    RouteType2Index::iterator iter = want_connect_type_2_index_.find(service.services_type_);
    if (iter == want_connect_type_2_index_.end())
    {
        return false;
    }

    // 找到对应的id列表，
    std::vector<unsigned int> *list_id = NULL;
    if (is_main_service)
    {
        list_id = &list_of_want_connect_main_id_[iter->second];
    }
    else
    {
        list_id = &list_of_want_connect_main_id_[iter->second];
    }

    // 看列表中是否存在对应的id
    for (unsigned int i = 0; i < list_id->size(); ++i)
    {
        if (list_id->at(i) == service.services_id_)
        {
            // svr_type和id都一样，还要检查一下对应的ip和端口是否有变化
            TCP_Svc_Handler *svchandle = NULL;
            int ret = TCP_Svc_Handler::find_services_peer(service, svchandle);
            if (ret != 0)
            {
                // 这个连接不存在
                return true;
            }

            // 找到，则比较一下ip和端口
            SERVICES_INFO     svr_info;
            ret = zerg_svr_cfg_->get_svcinfo_by_svcid(service, svr_info);
            if (ret != 0)
            {
                // 如果这个service已经找不到对应的IP配置，可能已经去除了
                return false;
            }

            if (svr_info.ip_address_ != svchandle->get_peer_sockaddr())
            {
                // 地址不相等
                return false;
            }

            return true;
        }
    }

    return false;
}

