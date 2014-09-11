#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_active_svchdl_set.h"


/****************************************************************************************************
class  Service_Info_Set ,
，


****************************************************************************************************/
Active_SvcHandle_Set::Active_SvcHandle_Set()
{
}

Active_SvcHandle_Set::~Active_SvcHandle_Set()
{
    //ZLOG_INFO("[zergsvr] Service_Info_Set::~Service_Info_Set.");
}

void Active_SvcHandle_Set::init_services_peerinfo(size_t szpeer)
{
    svr_info_set_.rehash(szpeer);
}

//根据SERVICEINFO查询PEER信息
int Active_SvcHandle_Set::find_services_peerinfo(const SERVICES_ID &svc_id, TCP_Svc_Handler *&svrhandle)
{
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svc_id);

    if (iter == svr_info_set_.end())
    {
        ZLOG_ERROR("[zergsvr] Can't find svchanle info. Svrinfo Type|ID:[%u|%u] .",
                   svc_id.services_type_,
                   svc_id.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }

    svrhandle = (*(iter)).second;
    return 0;
}



//更新设置配置信息
int Active_SvcHandle_Set::replace_services_peerInfo(const SERVICES_ID &svc_id,
                                                    TCP_Svc_Handler *new_svchdl,
                                                    TCP_Svc_Handler *&old_svchdl)
{
    old_svchdl = NULL;
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svc_id);

    //已经有相关的记录了
    if (iter != svr_info_set_.end())
    {
        old_svchdl = (*iter).second;

        const size_t TMP_ADDR_LEN = 32;
        char new_addr[TMP_ADDR_LEN], old_addr[TMP_ADDR_LEN];
        strncpy(new_addr, new_svchdl->get_peer_address(), TMP_ADDR_LEN);
        strncpy(old_addr, old_svchdl->get_peer_address(), TMP_ADDR_LEN);

        ZLOG_INFO("[zergsvr] replace_services_peerInfo:%u|%u ,Find Old IP|Port:[%s|%u],New IP Port[%s|%u],Replace old.",
                  svc_id.services_type_,
                  svc_id.services_id_,
                  old_addr,
                  old_svchdl->get_peer_port(),
                  new_addr,
                  new_svchdl->get_peer_port()
                 );

    }

    svr_info_set_[svc_id] = new_svchdl;

    return 0;
}

//增加设置配置信息
int Active_SvcHandle_Set::add_services_peerinfo(const SERVICES_ID &svc_id,
                                                TCP_Svc_Handler *new_svchdl)
{
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svc_id);

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
                   svc_id.services_type_,
                   svc_id.services_id_,
                   old_addr,
                   old_svchdl->get_peer_port(),
                   new_addr,
                   new_svchdl->get_peer_port()
                  );
        return SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN;
    }

    svr_info_set_[svc_id] = new_svchdl;

    return 0;
}



//根据SERVICES_ID,删除PEER信息,
size_t Active_SvcHandle_Set::del_services_peerInfo(const SERVICES_ID &svc_id)
{
    MapOfSvrPeerInfo::iterator iter = svr_info_set_.find(svc_id);

    size_t szdel = svr_info_set_.erase(svc_id);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了handle_close.
    if (szdel <= 0)
    {
        ZLOG_INFO("[zergsvr] Can't Service_Info_Set::del_services_peerInfo Size svr_info_set_ %u: szdel:%u svc_id:%u|%u .",
                  svr_info_set_.size(),
                  szdel,
                  svc_id.services_type_,
                  svc_id.services_id_);
    }

    //ZCE_ASSERT(szdel >0 );
    return szdel;
}

//
size_t Active_SvcHandle_Set::get_services_peersize()
{
    return svr_info_set_.size();
}


//
void Active_SvcHandle_Set::dump_svr_peerinfo(ZCE_LOG_PRIORITY out_lvl)
{
    //

    MapOfSvrPeerInfo::iterator iter_tmp = svr_info_set_.begin();
    MapOfSvrPeerInfo::iterator iter_end = svr_info_set_.end();

    //这是一个非常非常耗时的操作.
    for (size_t i = 0; iter_tmp != iter_end; ++iter_tmp, ++i)
    {
        SERVICES_ID svr_info = (*(iter_tmp)).first;
        TCP_Svc_Handler *svrhandle = (*(iter_tmp)).second;
        svrhandle->dump_status_info(out_lvl);
    }

    return;
}

//关闭所有的PEER
void Active_SvcHandle_Set::clear_and_closeall()
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


