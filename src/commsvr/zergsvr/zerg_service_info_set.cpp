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

void Service_Info_Set::init_services_peerinfo(size_t szpeer)
{
    svr_info_set_.rehash(szpeer);
}

//根据SERVICEINFO查询PEER信息
int Service_Info_Set::find_services_peerinfo(const SERVICES_ID &svrinfo, TCP_Svc_Handler *&svrhandle)
{
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svrinfo);

    if (iter == svr_info_set_.end())
    {
        ZLOG_ERROR("[zergsvr] Can't find svchanle info. Svrinfo Type|ID:[%u|%u] .",
            svrinfo.services_type_,
            svrinfo.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }

    svrhandle = (*(iter)).second;
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
    old_svchdl = NULL;
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svrinfo);

    //已经有相关的记录了
    if (iter != svr_info_set_.end())
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

    svr_info_set_[svrinfo] = new_svchdl;

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
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svrinfo);

    //已经有相关的记录了
    if (iter != svr_info_set_.end())
    {
        TCP_Svc_Handler *old_svchdl = (*iter).second;

        //一个很有意思的问题导致了代码必须这样写。如果你能直接知道为什么，可以直接找Scottxu要求请客
        const size_t TMP_ADDR_LEN = 32;
        char new_addr[TMP_ADDR_LEN], old_addr[TMP_ADDR_LEN];
        strncpy(new_addr, new_svchdl->get_peer_address(), TMP_ADDR_LEN);
        strncpy(old_addr, old_svchdl->get_peer_address(), TMP_ADDR_LEN);

        ZLOG_ERROR("[zergsvr] add_services_peerinfo:%u|%u Fail,Find Old IP|Port:[%s|%u],New IP Port[%s|%u],Replace old.",
            svrinfo.services_type_,
            svrinfo.services_id_,
            old_addr,
            old_svchdl->get_peer_port(),
            new_addr,
            new_svchdl->get_peer_port()
            );
        return SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN;
    }

    svr_info_set_[svrinfo] = new_svchdl;

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
size_t Service_Info_Set::del_services_peerInfo(const SERVICES_ID &svrinfo)
{
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svrinfo);

    size_t szdel = svr_info_set_.erase(svrinfo);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了handle_close.
    if (szdel <= 0)
    {
        ZLOG_INFO("[zergsvr] Can't Service_Info_Set::del_services_peerInfo Size svr_info_set_ %u: szdel:%u svrinfo:%u|%u .",
            svr_info_set_.size(),
            szdel,
            svrinfo.services_type_,
            svrinfo.services_id_);
    }

    //ZCE_ASSERT(szdel >0 );
    return szdel;
}

//
size_t Service_Info_Set::get_services_peersize()
{
    return svr_info_set_.size();
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
    //

    MapOfSvrPeerInfo::iterator iter_tmp = svr_info_set_.begin();
    MapOfSvrPeerInfo::iterator iter_end = svr_info_set_.end();

    //这是一个非常非常耗时的操作.
    for (size_t i = 0; iter_tmp != iter_end; ++iter_tmp, ++i)
    {
        //查询起始
        if (i < startno)
        {
            continue;
        }

        //对于查询的个数计数
        if (numquery > 0 && i > numquery + startno)
        {
            break;
        }

        SERVICES_ID svr_info = (*(iter_tmp)).first;
        TCP_Svc_Handler *svrhandle = (*(iter_tmp)).second;
        svrhandle->dump_status_info(ostr_stream);
    }

    return;
}

//关闭所有的PEER
void Service_Info_Set::clear_and_closeall()
{
    const size_t SHOWINFO_NUMBER = 500;

    ZLOG_INFO("[zergsvr] Has %u peer want to close. Please wait. ACE that is accursed.", svr_info_set_.size());

    //这个函数可能是绝对的慢
    while (svr_info_set_.size() > 0)
    {
        if (svr_info_set_.size() % SHOWINFO_NUMBER == 0)
        {
            ZLOG_INFO("[zergsvr] Now remain %u peer want to close. Please wait. ACE that is accursed.", svr_info_set_.size());
        }

        MapOfSvrPeerInfo::iterator iter = svr_info_set_.begin();
        TCP_Svc_Handler *svrhandle = (*(iter)).second;

        //TCP_Svc_Handler::handle_close调用了del_services_peerInfo
        svrhandle->handle_close();
    }
}


