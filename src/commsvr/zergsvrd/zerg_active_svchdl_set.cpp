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
    //ZCE_LOG(RS_INFO,"[zergsvr] Service_Info_Set::~Service_Info_Set.");
}

void Active_SvcHandle_Set::initialize(size_t sz_peer)
{
    max_peer_size_ = sz_peer;
    svr_info_set_.rehash(sz_peer);
    const size_t MAX_TYPE_KIND = 64;
    type_to_idtable_.rehash(MAX_TYPE_KIND);
}

//根据SERVICEINFO查询PEER信息
int Active_SvcHandle_Set::find_handle_by_svcid(const SERVICES_ID &svc_id,
                                               TCP_Svc_Handler *&svc_handle)
{
    MAP_OF_SVCPEERINFO::iterator iter = svr_info_set_.find(svc_id);

    if (iter == svr_info_set_.end())
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] Can't find svchanle info. Svrinfo Type|ID:[%u|%u] .",
                svc_id.services_type_,
                svc_id.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }

    svc_handle = (*(iter)).second;
    return 0;
}



//以负载均衡的方式，根据services type查询一个的SVC，按照数组顺序轮询的返回，
//这样查询保证发送的数据尽量负载均衡
int Active_SvcHandle_Set::find_lbseqhdl_by_type(uint16_t services_type,
                                                uint32_t &find_services_id,
                                                TCP_Svc_Handler *&svc_handle)
{
    //看type类型的MAP里面是否有这种类型的数据了
    MAP_OF_TYPE_TO_IDTABLE::iterator table_iter =
        type_to_idtable_.find(services_type);
    if (table_iter == type_to_idtable_.end())
    {
        ZCE_LOG(RS_ALERT, "[zergsvr][%s]Can't find typetoid table info.services type :[%hu] .",
                __ZCE_FUNC__,
                services_type);
        return SOAR_RET::ERR_ZERG_NO_FIND_SVCTYPE_RECORD;
    }
    SERVICES_ID_TABLE *id_table = &(table_iter->second);
    size_t ary_size = id_table->services_id_ary_.size();

    //orderid_use_id_是一个自增值，用于负载均衡
    for (size_t j = 0; j < ary_size; ++j)
    {
        size_t ary_id = (id_table->orderid_use_id_) % ary_size;
        id_table->orderid_use_id_ += 1;

        find_services_id = id_table->services_id_ary_[ary_id];
        SERVICES_ID lb_svcid(services_type, find_services_id);
        auto iter = svr_info_set_.find(lb_svcid);

        //到这儿就是你代码写错了，
        ZCE_ASSERT(iter != svr_info_set_.end());
        if (iter == svr_info_set_.end())
        {
            ZCE_LOG(RS_ALERT, "[zergsvr]Code error, can't find svc hanle info. SVC ID:[%u.%u] .",
                    lb_svcid.services_type_,
                    lb_svcid.services_id_);
            return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
        }

        svc_handle = (*(iter)).second;

        //会尽量选择一个激活状态的发送
        if (TCP_Svc_Handler::PEER_STATUS_ACTIVE == svc_handle->get_peer_status())
        {
            break;
        }
    }
    return 0;
}


int Active_SvcHandle_Set::find_lbfactorhdl_by_type(uint16_t services_type,
                                                   uint32_t lb_factor,
                                                   uint32_t &find_services_id,
                                                   TCP_Svc_Handler *&svc_handle)
{
    //看type类型的MAP里面是否有这种类型的数据了
    MAP_OF_TYPE_TO_IDTABLE::iterator table_iter =
        type_to_idtable_.find(services_type);
    if (table_iter == type_to_idtable_.end())
    {
        ZCE_LOG(RS_ALERT, "[zergsvr][%s]Can't find typetoid table info.services type :[%hu] .",
                __ZCE_FUNC__,
                services_type);
        return SOAR_RET::ERR_ZERG_NO_FIND_SVCTYPE_RECORD;
    }

    SERVICES_ID_TABLE *id_table = &(table_iter->second);
    size_t ary_size = id_table->services_id_ary_.size();

    for (size_t j = 0; j < ary_size; ++j)
    {
        size_t ary_id = (lb_factor) % ary_size;

        find_services_id = id_table->services_id_ary_[ary_id];
        SERVICES_ID lb_svcid(services_type, find_services_id);
        auto iter = svr_info_set_.find(lb_svcid);

        //到这儿就是你代码写错了，
        ZCE_ASSERT(iter != svr_info_set_.end());
        if (iter == svr_info_set_.end())
        {
            ZCE_LOG(RS_ALERT, "[zergsvr]Code error, can't find svc hanle info. SVC ID:[%u.%u] .",
                    lb_svcid.services_type_,
                    lb_svcid.services_id_);
            return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
        }

        svc_handle = (*(iter)).second;

        //会尽量选择一个激活状态的发送
        if (TCP_Svc_Handler::PEER_STATUS_ACTIVE == svc_handle->get_peer_status())
        {
            break;
        }
    }
    return 0;
}


//以主备的方式，根据services type尽量查询得到一个的SVC ID以及对应的Handle，
//主备的顺序按照Auto 那儿的配置顺序来处理。可以不是2个
int Active_SvcHandle_Set::find_mshdl_by_type(uint16_t services_type,
                                             uint32_t &find_services_id,
                                             TCP_Svc_Handler *&svc_handle)
{
    int ret = 0;
    std::vector<uint32_t> *ms_svcid_ary = NULL;
    ret = TCP_Svc_Handler::find_conf_ms_svcid_ary(services_type,
                                                  ms_svcid_ary);
    if (ret != 0)
    {
        return ret;
    }

    size_t ary_size = ms_svcid_ary->size();
    for (size_t j = 0; j < ary_size; ++j)
    {
        find_services_id = (*ms_svcid_ary)[j];
        SERVICES_ID ms_svcid(services_type, find_services_id);

        auto iter = svr_info_set_.find(ms_svcid);
        //如果没有找到
        if (iter == svr_info_set_.end())
        {
            continue;
        }

        svc_handle = (*(iter)).second;

        //会尽量选择一个激活状态的发送
        if (TCP_Svc_Handler::PEER_STATUS_ACTIVE == svc_handle->get_peer_status())
        {
            break;
        }
    }
    return 0;
}


//查询类型对应的所有active的SVC ID数组，用于广播等
int Active_SvcHandle_Set::find_hdlary_by_type(uint16_t services_type, std::vector<uint32_t> *&id_ary)
{
    MAP_OF_TYPE_TO_IDTABLE::iterator table_iter =
        type_to_idtable_.find(services_type);
    if (table_iter == type_to_idtable_.end())
    {
        ZCE_LOG(RS_ALERT, "[zergsvr][%s]Can't find typetoid table info.services type :[%hu] .",
                __ZCE_FUNC__,
                services_type);
        return SOAR_RET::ERR_ZERG_NO_FIND_SVCTYPE_RECORD;
    }
    SERVICES_ID_TABLE *id_table = &(table_iter->second);
    id_ary = &(id_table->services_id_ary_);
    return 0;
}

//增加设置配置信息
int Active_SvcHandle_Set::add_services_peerinfo(const SERVICES_ID &svc_id,
                                                TCP_Svc_Handler *new_svchdl)
{
    MAP_OF_SVCPEERINFO::iterator iter = svr_info_set_.find(svc_id);

    //已经有相关的记录了
    if (iter != svr_info_set_.end())
    {
        TCP_Svc_Handler *old_svchdl = (*iter).second;

        //一个很有意思的问题导致了代码必须这样写。如果你能直接知道为什么，可以直接找Scottxu要求请客
        const size_t TMP_ADDR_LEN = 31;
        char new_addr[TMP_ADDR_LEN+1], old_addr[TMP_ADDR_LEN + 1];
        size_t use_len = 0;

        ZCE_LOG(RS_ERROR, "[zergsvr] add_services_peerinfo:%u|%u fail.Find old IP:[%s],new IP[%s],no replace old.",
                svc_id.services_type_,
                svc_id.services_id_,
                old_svchdl->get_peer().to_string(old_addr, TMP_ADDR_LEN,use_len),
                new_svchdl->get_peer().to_string(new_addr, TMP_ADDR_LEN,use_len)
               );
        return SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN;
    }

    svr_info_set_[svc_id] = new_svchdl;

    //在Type to id 的table里面增加这个ID，
    MAP_OF_TYPE_TO_IDTABLE::iterator table_iter =
        type_to_idtable_.find(svc_id.services_type_);
    if (table_iter == type_to_idtable_.end())
    {
        auto iter_insert =
            type_to_idtable_.insert(std::make_pair(svc_id.services_type_, SERVICES_ID_TABLE()));
        if (iter_insert.second  == false)
        {
            return SOAR_RET::ERR_ZERG_INSERT_TYPETOIDTABLE_FAIL;
        }
        table_iter = iter_insert.first;
    }

    SERVICES_ID_TABLE *id_table = &(table_iter->second);
    id_table->services_id_ary_.reserve(max_peer_size_ / 2);
    id_table->services_id_ary_.push_back(svc_id.services_id_);
    size_t ary_id = id_table->services_id_ary_.size();
    new_svchdl->set_tptoid_table_id(ary_id);
    return 0;
}

//更新设置配置信息
int Active_SvcHandle_Set::replace_services_peerInfo(const SERVICES_ID &svc_id,
                                                    TCP_Svc_Handler *new_svchdl,
                                                    TCP_Svc_Handler *&old_svchdl)
{
    old_svchdl = NULL;
    MAP_OF_SVCPEERINFO::iterator iter = svr_info_set_.find(svc_id);

    //已经有相关的记录了
    if (iter != svr_info_set_.end())
    {
        old_svchdl = (*iter).second;

        const size_t TMP_ADDR_LEN = 31;
        char new_addr[TMP_ADDR_LEN+1], old_addr[TMP_ADDR_LEN + 1];
        size_t use_len = 0;
        ZCE_LOG(RS_INFO, "[zergsvr] replace_services_peerInfo:%u|%u ,Find old IP[%s],new IP [%s],replace old.",
                svc_id.services_type_,
                svc_id.services_id_,
                old_svchdl->get_peer().to_string(old_addr, TMP_ADDR_LEN ,use_len),
                new_svchdl->get_peer().to_string(new_addr, TMP_ADDR_LEN ,use_len)
               );
    }

    svr_info_set_[svc_id] = new_svchdl;

    //
    size_t ary_id = old_svchdl->get_tptoid_table_id();
    new_svchdl->set_tptoid_table_id(ary_id);

    return 0;
}


//根据SERVICES_ID,删除PEER信息,
int Active_SvcHandle_Set::del_services_peerInfo(const SERVICES_ID &svc_id)
{
    MAP_OF_SVCPEERINFO::iterator iter = svr_info_set_.find(svc_id);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了handle_close.
    if (iter == svr_info_set_.end())
    {
        ZCE_LOG(RS_INFO, "[zergsvr][%s] Can't  svr_info_set_ size:%u: svc_id:%u.%u .",
                __ZCE_FUNC__,
                svr_info_set_.size(),
                svc_id.services_type_,
                svc_id.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }
    svr_info_set_.erase(iter);

    TCP_Svc_Handler *svrhandle = (*(iter)).second;

    MAP_OF_TYPE_TO_IDTABLE::iterator table_iter =
        type_to_idtable_.find(svc_id.services_type_);
    ZCE_ASSERT(table_iter != type_to_idtable_.end());
    if (table_iter == type_to_idtable_.end())
    {
        ZCE_LOG(RS_ALERT, "[zergsvr][%s]Code error, can't find typetoid table info. Svrinfo :[%hu.%u] .",
                __ZCE_FUNC__,
                svc_id.services_type_,
                svc_id.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_SVCTYPE_RECORD;
    }

    //移动最后一个位置的数据到删除的位置，删除最后一个
    size_t del_id = svrhandle->get_tptoid_table_id();
    SERVICES_ID_TABLE *id_table = &(table_iter->second);
    size_t ary_size = id_table->services_id_ary_.size();
    id_table->services_id_ary_[del_id] = id_table->services_id_ary_[ary_size - 1];
    id_table->services_id_ary_.pop_back();

    //存在数组里面原来就只有一个数据的可能，
    if (id_table->services_id_ary_.size() == 0)
    {
        return 0;
    }

    //找到移动位置ID对应的Handle，调整其保存的ID
    SERVICES_ID move_svc_id(svc_id.services_type_,
                            id_table->services_id_ary_[del_id]);
    iter = svr_info_set_.find(move_svc_id);
    if (iter == svr_info_set_.end())
    {
        ZCE_LOG(RS_ALERT, "[zergsvr]Code error, can't find svchanle info. Svrinfo Type|ID:[%u|%u] .",
                move_svc_id.services_type_,
                move_svc_id.services_id_);
        return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
    }
    TCP_Svc_Handler *move_hdl = (*(iter)).second;
    move_hdl->set_tptoid_table_id(del_id);

    return 0;
}

//
size_t Active_SvcHandle_Set::get_services_peersize()
{
    return svr_info_set_.size();
}


//
void Active_SvcHandle_Set::dump_svr_peerinfo(zce::LOG_PRIORITY out_lvl)
{
    //
    MAP_OF_SVCPEERINFO::iterator iter_tmp = svr_info_set_.begin();
    MAP_OF_SVCPEERINFO::iterator iter_end = svr_info_set_.end();

    //如果连接上来的服务器多，这是一个非常非常耗时的操作.
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

    ZCE_LOG(RS_INFO, "[zergsvr] Has %u peer want to close. Please wait. ACE that is accursed.",
            svr_info_set_.size());

    //这个函数可能是绝对的慢
    while (svr_info_set_.size() > 0)
    {
        if (svr_info_set_.size() % SHOWINFO_NUMBER == 0)
        {
            ZCE_LOG(RS_INFO, "[zergsvr] Now remain %u peer want to close. Please wait. ACE that is accursed.",
                    svr_info_set_.size());
        }

        MAP_OF_SVCPEERINFO::iterator iter = svr_info_set_.begin();
        TCP_Svc_Handler *svrhandle = (*(iter)).second;

        //TCP_Svc_Handler::handle_close调用了del_services_peerInfo
        svrhandle->handle_close();
    }
}


