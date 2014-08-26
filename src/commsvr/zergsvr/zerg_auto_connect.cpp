/******************************************************************************************
Copyright           : 2000-2004, Tencent Technology (Shenzhen) Company Limited.
FileName            : zerg_autoconnect.cpp
Author              : Sail(ZENGXING)//Author name here
Version             :
Date Of Creation    : 2005年11月17日
Description         :

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
******************************************************************************************/
#include "zerg_predefine.h"
#include "zerg_auto_connect.h"
#include "zerg_application.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_configure.h"
//#include "zerg_external_connect_handler.h"
#include "zerg_comm_manager.h"

/****************************************************************************************************
class SERVICES_ROUTE_INFO
****************************************************************************************************/
SERVICES_ROUTE_INFO::SERVICES_ROUTE_INFO():
    back_route_valid_(false)
{
}

SERVICES_ROUTE_INFO::~SERVICES_ROUTE_INFO()
{
}

/****************************************************************************************************
class Zerg_Auto_Connector
****************************************************************************************************/
Zerg_Auto_Connector::Zerg_Auto_Connector():
    size_of_wantconnect_(0)
{
    srand((unsigned int)time(NULL));
}
//
Zerg_Auto_Connector::~Zerg_Auto_Connector()
{
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年1月12日
Function        : Zerg_Auto_Connector::get_autoconnect_cfg
Return          : int
Parameter List  :
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Zerg_Auto_Connector::get_autoconnect_cfg(const conf_zerg::ZERG_CONFIG *config)
{
    int ret = 0;
    SERVICES_ROUTE_INFO svc_route;

    int numsvr = (int)config->auto_connect_cfg.auto_connect_num;
    TESTCONFIG((numsvr >= 0 && ret == 0), "AUTOCONNECT|NUMSVRINFO key error.");

    //循环处理所有的数据
    for (int i = 0; i < numsvr; ++ i)
    {
        unsigned short svrtype = config->auto_connect_cfg.auto_connect_svrs[i].main_svr.svr_type;
        unsigned int svrid = config->auto_connect_cfg.auto_connect_svrs[i].main_svr.svr_id;

        svc_route.main_route_info_.set_serviceid(svrtype, svrid);

        //找到相关的IP配置
        ZCE_Sockaddr_In     svr_ip_addr;
        ret = Zerg_Server_Config::instance()->GetServicesIPInfo(svc_route.main_route_info_, svr_ip_addr);

        //如果查询不到
        if (ret != SOAR_RET::SOAR_RET_SUCC)
        {
            ZLOG_ERROR("[zergsvr] Count find Auto Connect Services Info SvrType=%u,SvrID=%u .Please Check Config file. ",
                       svrtype,
                       svrid);
            return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
        }

        svc_route.main_route_ip_ = svr_ip_addr;
        svc_route.back_route_valid_ = false;
        svc_route.back_route_info_.set_serviceid(0, 0);
        svc_route.back_route_ip_.set(static_cast<uint32_t>(0), 0);

        ZCE_Sockaddr_In     back_addr;

        if (config->auto_connect_cfg.auto_connect_svrs[i].backup_svr.svr_type != 0)
        {
            // 配置了备份的，则使用
            unsigned short backtype = config->auto_connect_cfg.auto_connect_svrs[i].backup_svr.svr_type;

            //必须保证备份路由的服务器类型和主路由服务器类型服务器类型一样
            if (backtype != svc_route.main_route_info_.services_type_ )
            {
                ZLOG_ERROR("[zergsvr] Backup route services type [%u] not equal to main route services id[%u].",
                           backtype,
                           svc_route.main_route_info_.services_type_);
                return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
            }

            unsigned int backid = config->auto_connect_cfg.auto_connect_svrs[i].backup_svr.svr_id;

            if (backid != 0)
            {
                svc_route.back_route_info_.set_serviceid(backtype, backid);

                ret = Zerg_Server_Config::instance()->GetServicesIPInfo(svc_route.back_route_info_, svr_ip_addr);

                //如果查询不到
                if (ret != SOAR_RET::SOAR_RET_SUCC)
                {
                    ZLOG_ERROR("[zergsvr] Count find Backup services Connect Services Info SvrType=%u,SvrID=%u .Please Check Config file. ", svrtype, svrid);
                    return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
                }

                svc_route.back_route_valid_ = true;
                svc_route.back_route_ip_ = svr_ip_addr;

            }
        }

        svc_route.external_= (config->auto_connect_cfg.auto_connect_svrs[i].external == 0) ? false : true;
        svc_route.proto_cfg_index_ = config->auto_connect_cfg.auto_connect_svrs[i].proto_cfg_index;

        ary_want_connect_.push_back(svc_route);

        //按type添加主动连接服务信息
        add_auto_connect_info(svc_route.main_route_info_, svc_route.back_route_info_);

        //由于该死的C/C++的返回静态指针的问题，这儿要输出两个地址，所以只能先打印到其他地方
        const size_t TMP_ADDR_LEN = 32;
        char mainroute_addr[TMP_ADDR_LEN + 1], backroute_addr[TMP_ADDR_LEN + 1];

        ZLOG_INFO("[zergsvr] Add one auto connect data, main route services id[%u|%u] ip[%s|%u],it have %s backup route services id[%u|%u] ip[%s|%u]",
                  svc_route.main_route_info_.services_type_,
                  svc_route.main_route_info_.services_id_,
                  svc_route.main_route_ip_.get_host_addr(mainroute_addr, TMP_ADDR_LEN),
                  svc_route.main_route_ip_.get_port_number(),
                  svc_route.back_route_valid_ ? "" : "not",
                  svc_route.back_route_info_.services_type_,
                  svc_route.back_route_info_.services_id_,
                  svc_route.back_route_ip_.get_host_addr(backroute_addr, TMP_ADDR_LEN),
                  svc_route.back_route_ip_.get_port_number()
                 );
    }

    size_of_wantconnect_ = ary_want_connect_.size();
    ZLOG_INFO("[zergsvr] Get AutoConnect Config Success.");

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年6月26日
Function        : Zerg_Auto_Connector::ConnectAllServer
Return          : void
Parameter List  :
  Param1: size_t& szsucc 成功开始连接的服务器个数
  Param2: size_t& szfail 链接失败的服务器数量，但是由于是异步链接，这个地方并不一定真是进行了链接
Description     : 链接所有的服务器,如果已经有链接，就跳过,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Zerg_Auto_Connector::reconnect_tcpserver(size_t &szvalid, size_t &szsucc, size_t &szfail)
{
    int ret = 0;
    szvalid = szsucc = szfail = 0;
    //循环将所有的SERVER链接一次,代码写的很绕口

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        if (ary_want_connect_[i].main_route_info_.services_type_ < SVC_UDP_SERVER_BEGIN)
        {
            TCP_Svc_Handler *svchandle = NULL;
            //如果已经有相应的链接了，跳过
            ret = TCP_Svc_Handler::find_connect_services_peer(ary_want_connect_[i].main_route_info_, svchandle);

            if ( SOAR_RET::SOAR_RET_SUCC != ret)
            {
                //进行连接,
                ret = connect_server_bysvcid(ary_want_connect_[i].main_route_info_,
                                             ary_want_connect_[i].main_route_ip_,
                                             i);

                if (ret == SOAR_RET::SOAR_RET_SUCC)
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
                if (svchandle->get_peer_status() ==  TCP_Svc_Handler::PEER_STATUS_ACTIVE)
                {
                    if (!ary_want_connect_[i].external_)
                    {
                        svchandle->send_zerg_heart_beat_reg();
                    }
                }
            }

            if (ary_want_connect_[i].back_route_valid_)
            {
                svchandle = NULL;
                //如果已经有相应的链接了，跳过
                ret = TCP_Svc_Handler::find_connect_services_peer(ary_want_connect_[i].back_route_info_, svchandle);

                if ( SOAR_RET::SOAR_RET_SUCC != ret)
                {
                    //进行连接,
                    ret = connect_server_bysvcid(ary_want_connect_[i].back_route_info_,
                                                 ary_want_connect_[i].back_route_ip_,
                                                 i);

                    if (ret == SOAR_RET::SOAR_RET_SUCC)
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
                    if (svchandle->get_peer_status() ==  TCP_Svc_Handler::PEER_STATUS_ACTIVE)
                    {
                        if (!ary_want_connect_[i].external_)
                        {
                            svchandle->send_zerg_heart_beat_reg();
                        }
                    }
                }
            }
        }
    }

    ZLOG_INFO("[zergsvr] Auto NONBLOCK connect server,vaild number:%d ,success Number :%d,fail number:%d .",
              szvalid,
              szsucc,
              szfail);
    return;
}

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2009年6月30日
Function        : Zerg_Auto_Connector::reconnect_server
Return          : int
Parameter List  :
  Param1: const SERVICES_ID& main_svrinfo 要进行重连的主路由信息
Description     : 根据SVRINFO,检查是否是主动连接的服务.,
Calls           :
Called By       :
Other           : 为什么不把一个TCP_Svc_Handler作为参数返回,因为在发起Connect过程中,也可能handle_close.
Modify Record   :
******************************************************************************************/
int Zerg_Auto_Connector::reconnect_server(const SERVICES_ID &reconnect_svrinfo)
{

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        if (ary_want_connect_[i].main_route_info_ == reconnect_svrinfo ||
            (ary_want_connect_[i].back_route_valid_ == true && reconnect_svrinfo == ary_want_connect_[i].back_route_info_  ) )
        {
            ZCE_Sockaddr_In     inetaddr = ary_want_connect_[i].main_route_ip_;
            return connect_server_bysvcid(reconnect_svrinfo, inetaddr, i);
        }
    }

    //这个错误无需记录,可能是这个SVCID根本不是要主动连接的服务器
    return SOAR_RET::ERR_ZERG_ISNOT_CONNECT_SERVICES;
}

// 连接udp服务器，其实是为了创建socket和handler，然后注册到reactor而已
void Zerg_Auto_Connector::reconnect_udpserver(size_t& szvalid, size_t& szsucc, size_t& szfail)
{
    int ret = 0;
    szvalid = szsucc = szfail = 0;

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        // 处理udp的主动连接
        if (ary_want_connect_[i].main_route_info_.services_type_ >= SVC_UDP_SERVER_BEGIN)
        {
            UDP_Svc_Handler *svchandle = NULL;
            //如果已经有相应的链接了，跳过
            ret = UDP_Svc_Handler::find_services_peer(ary_want_connect_[i].main_route_info_, svchandle);

            if (ret != SOAR_RET::SOAR_RET_SUCC)
            {
                // 没有找到，创建
                ret = connect_udpserver_bysvcid(ary_want_connect_[i].main_route_info_,
                                                ary_want_connect_[i].main_route_ip_,
                                                (int)i);

                if (ret == SOAR_RET::SOAR_RET_SUCC)
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
                // 找到了
                ++szvalid;
            }

            if (ary_want_connect_[i].back_route_valid_)
            {
                // 如果有备机
                svchandle = NULL;
                //如果已经有相应的链接了，跳过
                ret = UDP_Svc_Handler::find_services_peer(ary_want_connect_[i].back_route_info_, svchandle);

                if ( SOAR_RET::SOAR_RET_SUCC != ret)
                {
                    //进行连接,
                    ret = connect_udpserver_bysvcid(ary_want_connect_[i].back_route_info_,
                                                    ary_want_connect_[i].back_route_ip_,
                                                    (int)i);

                    if (ret == SOAR_RET::SOAR_RET_SUCC)
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
                }
            }
        }
    }

    ZLOG_INFO("[zergsvr] Auto connect udp server, vaild number:%d, success Number:%d, fail number:%d.",
              szvalid,
              szsucc,
              szfail);
    return;
}

//根据主服务器，得到备份服务器的信息
int Zerg_Auto_Connector::get_backupsvcinfo(const SERVICES_ID &main_svrinfo,
                                           bool &backroute_valid,
                                           SERVICES_ID &backroute_svrinfo)
{
    backroute_valid = false;

    for (size_t i = 0; i < size_of_wantconnect_; ++i)
    {
        if (ary_want_connect_[i].main_route_info_ == main_svrinfo)
        {
            ZCE_Sockaddr_In     inetaddr = ary_want_connect_[i].main_route_ip_;
            backroute_valid = ary_want_connect_[i].back_route_valid_;
            backroute_svrinfo = ary_want_connect_[i].back_route_info_;
            return SOAR_RET::SOAR_RET_SUCC;
        }
    }

    //这个错误无需记录,可能是这个SVCID根本不是要主动连接的服务器
    return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
}

//根据SVRINFO+IP,检查是否是主动连接的服务.并进行连接
int Zerg_Auto_Connector::connect_server_bysvcid(const SERVICES_ID &svrinfo,
                                                const ZCE_Sockaddr_In &inetaddr,
                                                size_t index)
{
    ZLOG_DEBUG("[zergsvr] Try NONBLOCK connect services[%u|%u] IP|Port :[%s|%u] .",
               svrinfo.services_type_,
               svrinfo.services_id_,
               inetaddr.get_host_addr(),
               inetaddr.get_port_number()
              );

    ZCE_Socket_Stream tcpscoket;
    tcpscoket.open();

    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    tcpscoket.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    tcpscoket.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);

    //tcpscoket.sock_enable (O_NONBLOCK);

    //记住,是这个时间标志使SOCKET异步连接,
    int ret = zerg_connector_.connect(tcpscoket, &inetaddr, true);

    //必然失败!?
    if (ret < 0)
    {
        //按照UNIX网络编程 V1的说法是 EINPROGRESS,但ACE的介绍说是 EWOULDBLOCK,
        if (ZCE_OS::last_error() != EWOULDBLOCK && ZCE_OS::last_error() != EINPROGRESS )
        {
            tcpscoket.close();
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }

        //HANDLER_MODE_CONNECT模式理论不会失败
        TCP_Svc_Handler *p_handler = TCP_Svc_Handler::AllocSvcHandlerFromPool(TCP_Svc_Handler::HANDLER_MODE_CONNECT);
        zce_ASSERT(p_handler);
        //以self_svc_info出去链接其他服务器.
        p_handler->init_tcpsvr_handler(Zerg_Server_Config::instance()->self_svc_info_.zerg_svc_info_,
                                       svrinfo,
                                       tcpscoket,
                                       inetaddr);

        if (ary_want_connect_[index].external_)
        {
            TcpHandlerImpl *tcp_handler_impl =
                Zerg_Comm_Manager::instance()->get_extern_proto_handler((unsigned int)ary_want_connect_[index].proto_cfg_index_);
            if (tcp_handler_impl == NULL)
            {
                ZLOG_ERROR("[zergsvr] External proto handler NULL, auto_index %u, proto_index %u, services[%u|%u] IP|Port :[%s|%u].",
                    index, ary_want_connect_[index].proto_cfg_index_,
                    svrinfo.services_type_,
                    svrinfo.services_id_,
                    inetaddr.get_host_addr(),
                    inetaddr.get_port_number());

                p_handler->handle_close();

                return SOAR_RET::ERR_ZERG_GET_EXTERNAL_PROTO_HANDLER;
            }

            p_handler->change_impl(tcp_handler_impl);
        }
    }
    //tmpret == 0 那就是让我去跳楼,但按照 UNIX网络编程 说应该是有本地连接时可能的.(我的测试还是返回错误)
    //而ACE的说明是立即返回错误,我暂时不处理这种情况,实在不行又只有根据类型写晦涩的朦胧诗了
    else
    {
        ZLOG_ERROR("[zergsvr] My God! NonBlock Socket Connect Success , ACE is a cheat.");
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

int Zerg_Auto_Connector::connect_udpserver_bysvcid(const SERVICES_ID& svrinfo,
                                                   const ZCE_Sockaddr_In& inetaddr,
                                                   int index)
{
    ZLOG_DEBUG("[zergsvr] Try connect udp services[%u|%u] IP|Port[%s|%u].",
               svrinfo.services_type_,
               svrinfo.services_id_,
               inetaddr.get_host_addr(),
               inetaddr.get_port_number()
              );

    if (index < 0 || index >= (int)ary_want_connect_.size())
    {
        ZLOG_ERROR("[zergsvr] connect index is invalid. index=%d", index);
        return SOAR_RET::ERR_ZERG_CONNECT_NO_FIND_SVCINFO;
    }


    size_t proto_cfg_index = ary_want_connect_[index].proto_cfg_index_;

    UDP_Svc_Handler *p_handler = new UDP_Svc_Handler(Zerg_Server_Config::instance()->self_svc_info_.zerg_svc_info_,
                                                     svrinfo,
                                                     inetaddr,
                                                     ary_want_connect_[index].external_,
                                                     Zerg_Server_Config::instance()->config_->external_proto_cfg.item[proto_cfg_index].pkg_len_offset,
                                                     Zerg_Server_Config::instance()->config_->external_proto_cfg.item[proto_cfg_index].pkg_len_bytes);
    zce_ASSERT(p_handler);
    //链接其他服务器.
    return p_handler->init_udpsvr_handler();
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
    unsigned int id_num = (unsigned int) list_of_want_connect_main_id_[index].size();

    for (unsigned int i = 0; i < id_num; i++)
    {
        unsigned int rand_num = rand() % id_num;

        //首先看主路由svr是否处于连接状态
        svrinfo->set_serviceid(svr_type, list_of_want_connect_main_id_[index][rand_num]);

        if (is_connected(*svrinfo))
        {
            //只有一个连接时, 不判定是否堵塞
            //最后一次随机选择时, 也不判定是否堵塞
            if (id_num > 1 && i != id_num - 1)
            {
                //查看连接是否有阻塞
                if (!is_socket_block(*svrinfo))
                {
                    return SOAR_RET::SOAR_RET_SUCC;
                }
                else
                {
                    ZLOG_COUNT_ERROR("MainServer [%u|%u] block and choose others",
                        svrinfo->services_type_, svrinfo->services_id_);
                    continue;
                }
            }
            else
            {
                return SOAR_RET::SOAR_RET_SUCC;
            }

        }

        //主路由不处于连接状态, 则试着选择从路由svr
        if (list_of_want_connect_back_id_[index][rand_num])
        {
            svrinfo->set_serviceid(svr_type, list_of_want_connect_back_id_[index][rand_num]);

            if (is_connected(*svrinfo))
            {
                if (id_num > 1)
                {
                    //查看连接是否有阻塞
                    if (!is_socket_block(*svrinfo))
                    {
                        return SOAR_RET::SOAR_RET_SUCC;
                    }
                    else
                    {
                        ZLOG_COUNT_ERROR("BackServer [%u|%u] block and choose others",
                            svrinfo->services_type_, svrinfo->services_id_);
                        continue;
                    }
                }
                else
                {
                    //只有一个连接，堵塞也要选
                    return SOAR_RET::SOAR_RET_SUCC;
                }
            }
        }
    }

    //ZLOG_ERROR("[zergsvr] Not found auto connect");

    // 未找到相应的Svr
    return SOAR_RET::ERR_ZERG_CHOOSE_AUTO_CONNECT_SERVICES;
}

//根据svr type按uin hash方式获取serviceid
int Zerg_Auto_Connector::get_server_byuinhash(unsigned short svr_type, SERVICES_ID *svrinfo, unsigned int uin)
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
        ZLOG_ERROR("[zergsvr] Not found auto connect in get_server_byuinhash");
        return SOAR_RET::ERR_ZERG_CHOOSE_AUTO_CONNECT_SERVICES;
    }

    //这里实现取模hash
    unsigned int id_num = (unsigned int) list_of_want_connect_main_id_[index].size();
    unsigned int rand = uin % id_num;

    //直接取主路由svr。不考虑是否连接是否阻塞，不考虑备份路由
    svrinfo->set_serviceid(svr_type, list_of_want_connect_main_id_[index][rand]);

    return SOAR_RET::SOAR_RET_SUCC;
}

bool Zerg_Auto_Connector::is_connected(const SERVICES_ID &svrinfo)
{
    TCP_Svc_Handler *svchandle = NULL;
    // 主动连接目前还不支持udp
    int ret = TCP_Svc_Handler::find_connect_services_peer(svrinfo, svchandle);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        // 没找到
        return false;
    }

    return true;
}

/******************************************************************************************
Author          : stefzhou  Date Of Creation: 2012年6月23日
Function        : Zerg_Auto_Connector::add_auto_connect_info
Return          : void
Parameter List  :
  Param1: const SERVICES_ID &main_svrinfo 主动连接的主路由信息
  Param1: const SERVICES_ID &back_svrinfo 主动连接的备份路由信息
Description     : 按type保存主动连接的服务信息，用于随机获取一个主动连接服务
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Zerg_Auto_Connector::add_auto_connect_info(const SERVICES_ID &main_svrinfo,
                                                const SERVICES_ID &back_svrinfo)
{
    RouteType2Index::iterator iter = want_connect_type_2_index_.find(main_svrinfo.services_type_);

    // 服务type是否已经添加到map
    if (iter == want_connect_type_2_index_.end())
    {
        //type没有添加到map，说明为第一个主动连接服务, 需要插入一个新的数组
        std::vector<unsigned int> main_id_list;
        std::vector<unsigned int> back_id_list;
        main_id_list.push_back(main_svrinfo.services_id_);
        back_id_list.push_back(back_svrinfo.services_id_);

        list_of_want_connect_main_id_.push_back(main_id_list);
        list_of_want_connect_back_id_.push_back(back_id_list);

        want_connect_type_2_index_.insert(std::make_pair<unsigned short, unsigned int>( \
                                                (unsigned short)main_svrinfo.services_type_, \
                                                (unsigned int)list_of_want_connect_main_id_.size() - 1));
    }
    else
    {
        //type已经添加到map，说明不是第一个主动连接服务, 只需要插入服务id到id列表
        unsigned int index = iter->second;
        // 这里为何不使用引用？
        std::vector<unsigned int> main_id_list = list_of_want_connect_main_id_[index];
        std::vector<unsigned int> back_id_list = list_of_want_connect_back_id_[index];
        main_id_list.push_back(main_svrinfo.services_id_);
        back_id_list.push_back(back_svrinfo.services_id_);

        list_of_want_connect_main_id_[index] = main_id_list;
        list_of_want_connect_back_id_[index] = back_id_list;
    }
}

// 重新加载主动连接配置
int
Zerg_Auto_Connector::reload_cfg(const conf_zerg::ZERG_CONFIG *config)
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

    int ret = get_autoconnect_cfg(config);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
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

    for (; iter != old_want_connect_type_2_index.end(); ++ iter)
    {
        std::vector<unsigned int> &old_main_id_list = old_list_of_want_connect_main_id[iter->second];
        std::vector<unsigned int> &old_back_id_list = old_list_of_want_connect_back_id[iter->second];

        for (unsigned int i = 0; i < old_main_id_list.size(); ++ i)
        {
            SERVICES_ID service(iter->first, old_main_id_list[i]);

            // 判定是否在当前主动连接中
            if (!is_current_auto_connect(service, true))
            {
                // 不在当前主动连接中，则关闭当前连接
                ZLOG_INFO("zergsvr close old auto connect. svr_type=%u svr_id=%u",
                          service.services_type_, service.services_id_);
                TCP_Svc_Handler::close_connect_services_peer(service);
            }
        }

        for (unsigned int i = 0; i < old_back_id_list.size() && old_back_id_list[i] != 0; ++ i)
        {
            SERVICES_ID service(iter->first, old_back_id_list[i]);

            // 判定是否在当前主动连接中
            if (!is_current_auto_connect(service, false))
            {
                // 不在当前主动连接中，则关闭当前连接
                TCP_Svc_Handler::close_connect_services_peer(service);
            }
        }

    }

    return SOAR_RET::SOAR_RET_SUCC;
}

// 判定一个svr是否在当前的主动连接中
bool
Zerg_Auto_Connector::is_current_auto_connect(const SERVICES_ID &service, bool is_main_service)
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
        list_id = &list_of_want_connect_back_id_[iter->second];
    }

    // 看列表中是否存在对应的id
    for (unsigned int i = 0; i < list_id->size(); ++ i)
    {
        if (list_id->at(i) == service.services_id_)
        {
            // svr_type和id都一样，还要检查一下对应的ip和端口是否有变化
            TCP_Svc_Handler *svchandle = NULL;
            int ret = TCP_Svc_Handler::find_connect_services_peer(service, svchandle);

            if (ret != SOAR_RET::SOAR_RET_SUCC)
            {
                // 这个连接不存在
                return true;
            }

            // 找到，则比较一下ip和端口
            ZCE_Sockaddr_In     svr_ip_addr;
            ret = Zerg_Server_Config::instance()->GetServicesIPInfo(service, svr_ip_addr);

            if (ret != SOAR_RET::SOAR_RET_SUCC)
            {
                // 如果这个service已经找不到对应的IP配置，可能已经去除了
                return false;
            }

            if (svr_ip_addr != svchandle->get_peer_sockaddr())
            {
                // 地址不相等
                return false;
            }

            return true;
        }
    }

    return false;
}

bool
Zerg_Auto_Connector::is_socket_block(const SERVICES_ID &svrinfo)
{
    TCP_Svc_Handler *svchandle = NULL;
    int ret = TCP_Svc_Handler::find_connect_services_peer(svrinfo, svchandle);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        // 没找到
        return true;
    }

    return svchandle->if_socket_block();
}

int Zerg_Auto_Connector::get_all_conn_server( unsigned short svr_type,
                                              std::vector<SERVICES_ID> &svrinfo_vec )
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
    unsigned int id_num = (unsigned int) list_of_want_connect_main_id_[index].size();

    for (unsigned int i = 0; i < id_num; i++)
    {
        SERVICES_ID svrinfo;

        //首先看主路由svr是否处于连接状态
        svrinfo.set_serviceid(svr_type, list_of_want_connect_main_id_[index][i]);

        if (is_connected(svrinfo))
        {
            svrinfo_vec.push_back(svrinfo);
            continue;
        }

        //主路由不处于连接状态, 则试着选择从路由svr
        if (list_of_want_connect_back_id_[index][i])
        {
            svrinfo.set_serviceid(svr_type, list_of_want_connect_back_id_[index][i]);

            if (is_connected(svrinfo))
            {
                svrinfo_vec.push_back(svrinfo);
                continue;
            }
        }

        ZLOG_ERROR("[%s] fail get should connect svr, svrinfo:[%u|%u]",
            __zce_FUNCTION__,
            svrinfo.services_type_,
            svrinfo.services_id_);
    }

    return SOAR_RET::SOAR_RET_SUCC;
}



