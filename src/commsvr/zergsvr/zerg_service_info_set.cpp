#include "zerg_predefine.h"
#include "zerg_service_info_set.h"
#include "zerg_tcp_ctrl_handler.h"

/****************************************************************************************************
class  Service_Info_Set ,
反思一下，
是否应该做这个封装，我也很矛盾，如果在TCP_Svc_Handler直接使用STL的封装数据，
那么我们的封装后再聚和的方式不是那么的好。在TCP_Svc_Handler还要再次封装。。。。
****************************************************************************************************/
Service_Info_Set::Service_Info_Set()
{
}

Service_Info_Set::~Service_Info_Set()
{
    //ZLOG_INFO("[zergsvr] Service_Info_Set::~Service_Info_Set.");
}

void Service_Info_Set::init_services_peerinfo(size_t max_accept, size_t max_connect)
{
    accept_svr_info_set_.resize(max_accept);
    connect_svr_info_set_.resize(max_connect);
}

//根据SERVICEINFO查询PEER信息
int Service_Info_Set::find_services_peerinfo(const SERVICES_ID &svrinfo, TCP_Svc_Handler *&svrhandle)
{
    //先查找主动连接再查找被动连接
    MapOfSvrPeerInfo::iterator iter;
    if(((iter = connect_svr_info_set_.find(svrinfo)) != connect_svr_info_set_.end())
      ||((iter = accept_svr_info_set_.find(svrinfo)) != accept_svr_info_set_.end())) 
    {
        svrhandle = (*(iter)).second;
    }
    else
    {
        ZLOG_ERROR("[zergsvr] Can't find svchanle info. Svrinfo Type|ID:[%u|%u] .",
            svrinfo.services_type_,
            svrinfo.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

int 
Service_Info_Set::find_connect_services_peerinfo(const SERVICES_ID &svrinfo, TCP_Svc_Handler*&svrhandle)
{
    //先查找主动连接再查找被动连接
    MapOfSvrPeerInfo::iterator iter;
    if(((iter = connect_svr_info_set_.find(svrinfo)) != connect_svr_info_set_.end())) 
    {
        svrhandle = (*(iter)).second;
    }
    else
    {
        ZLOG_ERROR("[zergsvr] Can't find auto connect svchanle info. Svrinfo Type|ID:[%u|%u] .",
            svrinfo.services_type_,
            svrinfo.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年5月29日
Function        : Service_Info_Set::replace_services_peerInfo
Return          : int
Parameter List  :
  Param1: const SERVICES_ID& svrinfo 原油的SVC INFO服务器
  Param2: TCP_Svc_Handler* new_svchdl  新增加的服务器
  Param3: TCP_Svc_Handler*& old_svchdl 如果原来有一个svrinfo对应的Hdler,返回通知你,
Description     : 增加PEER信息的
Calls           :
Called By       :
Other           : 返回的old_svchdl,可以用于清理
Modify Record   :
******************************************************************************************/
int Service_Info_Set::replace_services_peerInfo(const SERVICES_ID &svrinfo,
                                                TCP_Svc_Handler *new_svchdl,
                                                TCP_Svc_Handler *&old_svchdl)
{
    MapOfSvrPeerInfo *svr_info_set = get_svr_peer_set(new_svchdl);

    old_svchdl = NULL;
    MapOfSvrPeerInfo::iterator iter = svr_info_set->find(svrinfo);

    //已经有相关的记录了
    if (iter != svr_info_set->end() )
    {
        old_svchdl = (*iter).second;

        const size_t TMP_ADDR_LEN = 32;
        char new_addr[TMP_ADDR_LEN], old_addr[TMP_ADDR_LEN];
        strncpy(new_addr, new_svchdl->get_peer_address(), TMP_ADDR_LEN);
        strncpy(old_addr, old_svchdl->get_peer_address(), TMP_ADDR_LEN);

        ZLOG_INFO("[zergsvr] replace_services_peerInfo:%u|%u ,Find Old IP|Port:[%s|%u],New IP Port[%s|%u],Replace old.",
                  svrinfo.services_type_,
                  svrinfo.services_id_,
                  old_addr,
                  old_svchdl->get_peer_port(),
                  new_addr,
                  new_svchdl->get_peer_port()
                 );
    }
    else
    {
        ZLOG_INFO("[zergsvr] add services peerInfo:%u|%u Succ,IP|Port:[%s|%u],Handler_mode:%u",
            svrinfo.services_type_,
            svrinfo.services_id_,
            new_svchdl->get_peer_address(),
            new_svchdl->get_peer_port(),
            new_svchdl->get_handle_mode());
    }

    (*svr_info_set)[svrinfo] = new_svchdl;

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年5月29日
Function        : Service_Info_Set::add_services_peerinfo
Return          : int
Parameter List  :
Param1: const SERVICES_ID& svrinfo SVC INFO服务器
Param2: TCP_Svc_Handler* new_svchdl  新增加的服务器
Description     : 增加PEER信息的
Calls           :
Called By       :
Other           : 返回的old_svchdl,可以用于清理
Modify Record   :
******************************************************************************************/
int Service_Info_Set::add_services_peerinfo(const SERVICES_ID &svrinfo,
                                            TCP_Svc_Handler *new_svchdl)
{
    MapOfSvrPeerInfo *svr_info_set = get_svr_peer_set(new_svchdl);

    MapOfSvrPeerInfo::iterator iter = svr_info_set->find(svrinfo);

    //已经有相关的记录了
    if (iter != svr_info_set->end() )
    {
        TCP_Svc_Handler *old_svchdl = (*iter).second;

        //一个很有意思的问题导致了代码必须这样写。如果你能直接知道为什么，可以直接找Scottxu要求请客
        const size_t TMP_ADDR_LEN = 32;
        char new_addr[TMP_ADDR_LEN], old_addr[TMP_ADDR_LEN];
        strncpy(new_addr, new_svchdl->get_peer_address(), TMP_ADDR_LEN);
        strncpy(old_addr, old_svchdl->get_peer_address(), TMP_ADDR_LEN);

        ZLOG_ERROR("[zergsvr] add_services_peerinfo:%u|%u Fail,Find Old IP|Port:[%s|%u],New IP Port[%s|%u]",
                   svrinfo.services_type_,
                   svrinfo.services_id_,
                   old_addr,
                   old_svchdl->get_peer_port(),
                   new_addr,
                   new_svchdl->get_peer_port()
                  );
        return SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN;
    }

    ZLOG_INFO("[zergsvr] add services peerInfo:%u|%u Succ,IP|Port:[%s|%u],Handler_mode:%u",
        svrinfo.services_type_,
        svrinfo.services_id_,
        new_svchdl->get_peer_address(),
        new_svchdl->get_peer_port(),
        new_svchdl->get_handle_mode());

    (*svr_info_set)[svrinfo] = new_svchdl;

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Service_Info_Set::del_services_peerInfo
Return          : size_t
Parameter List  :
Param1: const SERVICES_ID& svrinfo
Description     : 根据SERVICES_ID,删除PEER信息,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
size_t Service_Info_Set::del_services_peerInfo(const SERVICES_ID &svrinfo,
                                               TCP_Svc_Handler *svchdl)
{
    MapOfSvrPeerInfo *svr_info_set = get_svr_peer_set(svchdl);

    MapOfSvrPeerInfo::iterator iter = svr_info_set->find(svrinfo);

    size_t szdel = svr_info_set->erase(svrinfo);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了handle_close.
    if (szdel <= 0)
    {
        ZLOG_ERROR("[zergsvr] del_services_peerInfo %u|%u Fail,svr_info_set size:%u szdel:%d.",
                  svrinfo.services_type_,
                  svrinfo.services_id_,
                  svr_info_set->size(),
                  szdel
                  );
    }
    else
    {
        ZLOG_INFO("[zergsvr] del_services_peerInfo %u|%u Succ,IP|Port:[%s|%u],Handler_mode:%u.",
            svrinfo.services_type_,
            svrinfo.services_id_,
            svchdl->get_peer_address(),
            svchdl->get_peer_port(),
            svchdl->get_handle_mode());
    }

    //zce_ASSERT(szdel >0 );
    return szdel;
}

//
size_t Service_Info_Set::get_services_peersize()
{
    return connect_svr_info_set_.size() + accept_svr_info_set_.size();
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年10月17日
Function        : Service_Info_Set::dump_svr_peerinfo
Return          : void
Parameter List  :
  Param1: char* ret_string
  Param2: size_t& str_len
  Param3: size_t startno        查询
  Param4: size_t numquery       查询
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Service_Info_Set::dump_svr_peerinfo(std::ostringstream &ostr_stream, size_t startno, size_t numquery)
{
    //按照先主动连接后被连接的顺序扫描打印
    size_t connect_size = connect_svr_info_set_.size();

    if (startno + numquery <= connect_size)
    {
        //打印连接全部位于主动连接池
        dump_peerinfo_set(connect_svr_info_set_, ostr_stream, startno, numquery);
    }
    else if (startno >= connect_size)
    {
        //打印连接全部位于被动连接池
        dump_peerinfo_set(accept_svr_info_set_, ostr_stream, startno - connect_size, numquery);
    }
    else
    {
        //打印的连接一部分位于主动连接池, 一部分位于被动连接池
        //先打印主动连接池
        dump_peerinfo_set(connect_svr_info_set_, ostr_stream, startno, connect_size - startno);

        //再打印被动连接池
        dump_peerinfo_set(accept_svr_info_set_, ostr_stream, 0, numquery - connect_size + startno);
    }

    return;
}

//关闭所有的PEER
void Service_Info_Set::clear_and_closeall()
{
    const size_t SHOWINFO_NUMBER = 500;

    ZLOG_INFO("[zergsvr] Has %u peer want to close. Please wait. ACE that is accursed.", connect_svr_info_set_.size());

    //这个函数可能是绝对的慢
    while (connect_svr_info_set_.size() > 0)
    {
        if (connect_svr_info_set_.size() % SHOWINFO_NUMBER == 0 )
        {
            ZLOG_INFO("[zergsvr] Now remain %u peer want to close. Please wait. ACE that is accursed.", connect_svr_info_set_.size());
        }

        MapOfSvrPeerInfo::iterator iter = connect_svr_info_set_.begin();
        TCP_Svc_Handler *svrhandle = (*(iter)).second;

        //TCP_Svc_Handler::handle_close调用了del_services_peerInfo
        svrhandle->handle_close();
    }

    //这个函数可能是绝对的慢
    while (accept_svr_info_set_.size() > 0)
    {
        if (accept_svr_info_set_.size() % SHOWINFO_NUMBER == 0 )
        {
            ZLOG_INFO("[zergsvr] Now remain %u peer want to close. Please wait. ACE that is accursed.", accept_svr_info_set_.size());
        }

        MapOfSvrPeerInfo::iterator iter = accept_svr_info_set_.begin();
        TCP_Svc_Handler *svrhandle = (*(iter)).second;

        //TCP_Svc_Handler::handle_close调用了del_services_peerInfo
        svrhandle->handle_close();
    }
}

Service_Info_Set::MapOfSvrPeerInfo* 
Service_Info_Set::get_svr_peer_set(TCP_Svc_Handler *svchdl)
{
    if (svchdl->get_handle_mode() == TCP_Svc_Handler::HANDLER_MODE_CONNECT 
        || svchdl->get_handle_mode() == TCP_Svc_Handler::HANDLER_MODE_CONNECT_EXTERNAL)
    {
        return &connect_svr_info_set_;
    }
    else
    {
        return &accept_svr_info_set_;
    }
}

void 
Service_Info_Set::dump_peerinfo_set(const MapOfSvrPeerInfo &peer_set, 
                                    std::ostringstream &ostr_stream, 
                                    size_t startno, 
                                    size_t numquery)
{
    MapOfSvrPeerInfo::const_iterator iter_tmp = peer_set.begin();
    MapOfSvrPeerInfo::const_iterator iter_end = peer_set.end();

    //这是一个非常非常耗时的操作.
    for (size_t i = 0; iter_tmp != iter_end ; ++iter_tmp, ++i)
    {
        //查询起始
        if (i < startno)
        {
            continue;
        }

        //对于查询的个数计数
        if (numquery > 0 && i > numquery + startno )
        {
            return;
        }

        SERVICES_ID svr_info = (*(iter_tmp)).first;
        TCP_Svc_Handler *svrhandle = (*(iter_tmp)).second;
        svrhandle->dump_status_info(ostr_stream);
    }
}
