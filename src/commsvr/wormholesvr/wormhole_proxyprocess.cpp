#include "wormhole_predefine.h"
#include "wormhole_proxyprocess.h"
#include "wormhole_application.h"
#include "wormhole_stat_define.h"
#include "wormhole_flow_stat.h"

/****************************************************************************************************
class Interface_Proxy_Process
****************************************************************************************************/
InterfaceProxyProcess::InterfaceProxyProcess()
{
}
InterfaceProxyProcess::~InterfaceProxyProcess()
{
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月23日
Function        : Interface_Proxy_Process::CreatePorxyFactory
Return          : Interface_Proxy_Process*
Parameter List  :
Param1: PROXY_TYPE proxytype 代理类型
Description     :
Calls           :
Called By       :
Other           : 根据代理类型返回代理的接口，
Modify Record   :
******************************************************************************************/
InterfaceProxyProcess *InterfaceProxyProcess::CreatePorxyFactory(PROXY_TYPE proxytype)
{
    InterfaceProxyProcess *tmpintface = NULL;
    ZLOG_INFO("Interface_Proxy_Process::CreatePorxyFactory PROXY_TYPE: %d.", proxytype);

    switch (proxytype)
    {
        // 回显服务器
        case PROXY_TYPE_ECHO:
        {
            tmpintface = new Echo_Proxy_Process();
            break;
        }

        // DBPROXY的模式，采用UIN取模的方式的到服务器的ID
        case PROXY_TYPE_DB_MODAL:
        {
            tmpintface = new DBModalProxyProcess();
            break;
        }

        // 透转转发的方式
        case PROXY_TYPE_TRANSMIT:
        {
            tmpintface = new TransmitProxyProcess();
            break;
        }

        // 对数据进行拷贝分发广播
        case PROXY_TYPE_COPY_TRANS_ALL:
        {
            tmpintface = new CopyTransmitAllProxyProcess();
            break;
        }

        // DBPROXY的模式，采用APPID和UIN的方式的到服务器的ID
        case PROXY_TYPE_DB_MODAL_MG:
        {
            tmpintface = new DBModalMGProxyProcess();
            break;
        }

        // 对数据进行拷贝分发广播,手游类业务
        case PROXY_TYPE_COPY_TRANS_ALL_MG:
        {
            tmpintface = new CopyTransmitAllMGProxyProcess();
            break;
        }

        default:
            // 错误
            ZLOG_ERROR("Error Proxy Type define. Please check you code. ");
            return NULL;
    }

    return tmpintface;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月12日
Function        : Interface_Proxy_Process::InitProxyInstance
Return          : int
Parameter List  : NULL
Description     : 初始化化代理,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int InterfaceProxyProcess::InitProxyInstance()
{
    // int ret =0;
    // 初始化MMAP内存的PIPE
    zerg_mmap_pipe_ = Zerg_MMAP_BusPipe::instance();

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月12日
Function        : Interface_Proxy_Process::get_proxy_config
Return          : int
Parameter List  : NULL
Description     : 读取配置
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int InterfaceProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{
    unsigned int numcmd = 0;

    // 是否检查帧
    if_check_frame_ = cfg->if_check_frame_;
    ZLOG_DEBUG("If check frame: %d.", if_check_frame_);

    numcmd = cfg->process_cmd_info_.process_cmd_num_;
    ZLOG_DEBUG("Process cmd count: %d.", numcmd);

    char tmp[64];
    char tmpbuf[128];

    proxy_processcmd_.resize(numcmd);

    for (unsigned int i = 0; i < numcmd; ++i)
    {
        // 如果有重复的我只能默哀了
        unsigned int prccmd;
        snprintf(tmp, 64, "PROCESSCMD%i", i);
        snprintf(tmpbuf, 64, "NORMALPROCESSCMD|%s key error.", tmp);
        prccmd = cfg->process_cmd_info_.process_cmd_[i];
        TESTCONFIG(prccmd != 0, tmpbuf);

        proxy_processcmd_.insert(prccmd);
    }

    return TSS_RET::TSS_RET_SUCC;
}

/****************************************************************************************************
class  Echo_Proxy_Process 回显得的代理服务器处理 PROXY_TYPE_ECHO
****************************************************************************************************/
Echo_Proxy_Process::Echo_Proxy_Process()
{

}
Echo_Proxy_Process::~Echo_Proxy_Process()
{

}

int Echo_Proxy_Process::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{
    //
    int ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月12日
Function        : Echo_Proxy_Process::process_proxy
Return          : int
Parameter List  :
Param1: Comm_App_Frame* proc_frame 要处理的帧
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Echo_Proxy_Process::process_proxy(Comm_App_Frame *proc_frame)
{

    ZLOG_DEBUG("Receive a echo frame to process,"
               "send svr:[%u|%u], "
               "recv svr:[%u|%u], "
               "frame_uin:%u, "
               "frame_cmd:%u, "
               "frame_len:%u. ",
               proc_frame->send_service_.services_type_,
               proc_frame->send_service_.services_id_,
               proc_frame->recv_service_.services_type_,
               proc_frame->recv_service_.services_id_,
               proc_frame->frame_uin_,
               proc_frame->frame_command_,
               proc_frame->frame_length_);

    int ret = 0;
    // 内部处理的命令
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_DEBUG("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                   proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    // 返回这个帧
    proc_frame->exchange_rcvsnd_svcid();

    ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

    //
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
    }

    ZLOG_DEBUG( "Echo to [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u. ",
                proc_frame->recv_service_.services_type_,
                proc_frame->recv_service_.services_id_,
                proc_frame->frame_uin_,
                proc_frame->frame_command_,
                proc_frame->frame_length_);
    return TSS_RET::TSS_RET_SUCC;
}

/****************************************************************************************************
class DBModalProxyProcess 按照DB取模进行Proxy转发，用于DBServer和金融服务器 PROXY_TYPE_DB_MODAL
****************************************************************************************************/
DBModalProxyProcess::DBModalProxyProcess():
    InterfaceProxyProcess()
{
}
DBModalProxyProcess::~DBModalProxyProcess()
{
    std::map<unsigned short, DBModalProxyInfo*>::iterator iter = dbmodal_proxy_map_.begin();

    for (; iter != dbmodal_proxy_map_.end(); iter++)
    {
        // 释放分配的内存
        delete iter->second;
        iter->second = NULL;
    }
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月24日
Function        : DBModalProxyProcess::get_proxy_config
Return          : int
Parameter List  :
Param1: const char* cfgfile
Description     : 读取NORMAL的配置
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int DBModalProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{

    int ret = 0;

    //得到过滤得命令
    ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    for (unsigned int i = 0; i < cfg->dbmodal_info_.route_num_; i++)
    {
        DBModalProxyInfo *dbmodal_proxy_info = new DBModalProxyInfo();
        conf_proxysvr::RouteInfo *route_info = &(cfg->dbmodal_info_.route_info_[i]);

        dbmodal_proxy_info->distribute_module_ = route_info->distribute_module_;
        dbmodal_proxy_info->distribute_offset_ = route_info->distribute_offset_;
        dbmodal_proxy_info->router_svr_type_ = route_info->svr_type_;

        ZLOG_INFO("[DBModalProxy] route_svr_type:%u, distribute_offset:%u, distribute_module:%u",
            dbmodal_proxy_info->router_svr_type_,
            dbmodal_proxy_info->distribute_offset_,
            dbmodal_proxy_info->distribute_module_);

        dbmodal_proxy_info->normal_router_cfg_.resize(dbmodal_proxy_info->distribute_module_);
        dbmodal_proxy_info->clone_router_cfg_.resize(dbmodal_proxy_info->distribute_module_);

        for (unsigned int k = 0; k < dbmodal_proxy_info->distribute_module_; k++)
        {
            dbmodal_proxy_info->normal_router_cfg_[k] = route_info->svr_id_[k].nomal_service_id_;

            if (route_info->svr_id_[k].clone_service_id_)
            {
                dbmodal_proxy_info->clone_router_cfg_[k] = route_info->svr_id_[k].clone_service_id_;
            }
            else
            {
                dbmodal_proxy_info->clone_router_cfg_[k] = SERVICES_ID::INVALID_SERVICES_ID;
            }

            ZLOG_INFO("[DBModalProxy] normal service:%u|%u, clone service:%u|%u, passby service:%u|%u",
                dbmodal_proxy_info->router_svr_type_, dbmodal_proxy_info->normal_router_cfg_[k],
                dbmodal_proxy_info->router_svr_type_, dbmodal_proxy_info->clone_router_cfg_[k]);
        }

        dbmodal_proxy_map_.insert(std::make_pair<unsigned short, DBModalProxyInfo*>
            (dbmodal_proxy_info->router_svr_type_, dbmodal_proxy_info));
    }

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月12日
Function        : DBModalProxyProcess::process_proxy
Return          : int
Parameter List  :
Param1: Comm_App_Frame* proc_frame 要处理的帧
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int DBModalProxyProcess::process_proxy(Comm_App_Frame *proc_frame)
{
    ZLOG_DEBUG("Receive a dbmode frame to process,"
               "send svr:[%u|%u], "
               "recv svr:[%u|%u], "
               "frame_uin:%u, "
               "frame_cmd:%u, "
               "frame_len:%u. ",
               proc_frame->send_service_.services_type_,
               proc_frame->send_service_.services_id_,
               proc_frame->recv_service_.services_type_,
               proc_frame->recv_service_.services_id_,
               proc_frame->frame_uin_,
               proc_frame->frame_command_,
               proc_frame->frame_length_);

    int ret = 0;
    // 内部处理的命令,跳过
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_INFO("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                  proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    std::map<unsigned short, DBModalProxyInfo*>::iterator iter =
        dbmodal_proxy_map_.find(proc_frame->recv_service_.services_type_);

    if (iter != dbmodal_proxy_map_.end())
    {
        // 要转发的类型已配置, 获取对应路由信息
        DBModalProxyInfo *dbmodal_proxy_info = iter->second;

        //------------------------------------------------------------------
        unsigned int uin = proc_frame->frame_uin_;

        // 过滤掉uin为0的数据
        if (uin == 0 )
        {

            proc_frame->dumpoutput_framehead("[FROM RECV FRAME]", RS_ERROR);

            Comm_Stat_Monitor::instance()->increase_once(ARBITER_TRANS_PKG_ERROR);

            return TSS_RET::ERROR_APPFRAME_ERROR;
        }

        // 关键代码处
        unsigned int mod =
            (uin >> dbmodal_proxy_info->distribute_offset_) % dbmodal_proxy_info->distribute_module_;

        // ------------------------------------------------------------------
        proc_frame->recv_service_.services_type_ = dbmodal_proxy_info->router_svr_type_;
        proc_frame->recv_service_.services_id_ =  dbmodal_proxy_info->normal_router_cfg_[mod];

        // 日志调整为DEBUG级别的
        ZLOG_DEBUG("Send to main services [%u|%u], frame_uin:%u, "
            "frame_command:%u, frame_len:%u, trans_id[%u]. ",
                   proc_frame->recv_service_.services_type_,
                   proc_frame->recv_service_.services_id_,
                   proc_frame->frame_uin_,
                   proc_frame->frame_command_,
                   proc_frame->frame_length_,
                   proc_frame->transaction_id_);

        // 只生成了一个帧
        int ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
        }

        // 如果有备份路由,则将数据转发给一个备份的代理
        if (dbmodal_proxy_info->clone_router_cfg_[mod] != SERVICES_ID::INVALID_SERVICES_ID )
        {

            proc_frame->recv_service_.services_id_ = dbmodal_proxy_info->clone_router_cfg_[mod];
            ZLOG_INFO("Send to backup services [%u|%u], frame_uin:%u,"
                " frame_command:%u, frame_len:%u, back trans_id[%u]. ",
                      proc_frame->recv_service_.services_type_,
                      proc_frame->recv_service_.services_id_,
                      proc_frame->frame_uin_,
                      proc_frame->frame_command_,
                      proc_frame->frame_length_,
                      proc_frame->backfill_trans_id_);

            ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

            if (ret != TSS_RET::TSS_RET_SUCC)
            {
                return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
            }

            // 备份路由, 多增加一次
            ArbiterFlowStat::instance()->increase_channel_pkg_num(proc_frame->send_service_.services_type_,
                                                                  proc_frame->recv_service_.services_type_);
        }

    }
    // 另外一个路由方向的事情，直接透传
    else
    {
        //
        // 加一条日志，方便跟踪
        ZLOG_INFO("Send back [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u. ",
                  proc_frame->recv_service_.services_type_,
                  proc_frame->recv_service_.services_id_,
                  proc_frame->frame_uin_,
                  proc_frame->frame_command_,
                  proc_frame->frame_length_);

        ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
        }
    }

    // else ELSE不做处理，DB Modal PROXY只是将发向DB一侧的数据进行修改。

    ArbiterFlowStat::instance()->increase_channel_pkg_num(proc_frame->send_service_.services_type_,
                                                          proc_frame->recv_service_.services_type_);

    return TSS_RET::TSS_RET_SUCC;
}

/****************************************************************************************************
class TransmitProxyProcess 直接进行转发处理，不对数据帧进行任何处理
****************************************************************************************************/
TransmitProxyProcess::TransmitProxyProcess()
{
}

TransmitProxyProcess::~TransmitProxyProcess()
{
}

int TransmitProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg )
{
    //
    int ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    return TSS_RET::TSS_RET_SUCC;
}

int TransmitProxyProcess::process_proxy(Comm_App_Frame *proc_frame)
{
    ZLOG_DEBUG("Receive a transmit frame to process,"
               "send svr:[%u|%u], "
               "recv svr:[%u|%u], "
               "frame_uin:%u, "
               "frame_cmd:%u, "
               "frame_len:%u. ",
               proc_frame->send_service_.services_type_,
               proc_frame->send_service_.services_id_,
               proc_frame->recv_service_.services_type_,
               proc_frame->recv_service_.services_id_,
               proc_frame->frame_uin_,
               proc_frame->frame_command_,
               proc_frame->frame_length_);

    int ret = 0;

    // 内部处理的命令,跳过
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_DEBUG("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                   proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

    //
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
    }

    ArbiterFlowStat::instance()->increase_channel_pkg_num(proc_frame->send_service_.services_type_,
                                                          proc_frame->recv_service_.services_type_);

    ZLOG_DEBUG( "Transmit to [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, trans_id[%u]. ",
                proc_frame->recv_service_.services_type_,
                proc_frame->recv_service_.services_id_,
                proc_frame->frame_uin_,
                proc_frame->frame_command_,
                proc_frame->frame_length_,
                proc_frame->transaction_id_);

    return TSS_RET::TSS_RET_SUCC;
}

/****************************************************************************************************
class CopyTransmitAllProxyProcess 将数据复制转发给所有配置的服务器 PROXY_TYPE_COPY_TRANS_ALL
****************************************************************************************************/
CopyTransmitAllProxyProcess::CopyTransmitAllProxyProcess():
    InterfaceProxyProcess(),
    copytrans_svctype_(0),
    copytrans_svcnum_(0)
{
    memset(copytrans_svcid_, 0, sizeof(copytrans_svcid_));
}

CopyTransmitAllProxyProcess::~CopyTransmitAllProxyProcess()
{
}

int CopyTransmitAllProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{
    //
    int ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    unsigned int tmp_unit = 0;

    tmp_unit = cfg->copy_info_.svr_type_;
    ZLOG_DEBUG("Copy service type: %u.", tmp_unit);
    copytrans_svctype_ = static_cast<unsigned short>(tmp_unit);

    tmp_unit = cfg->copy_info_.copy_num_;
    ZLOG_DEBUG("Copy service num: %u.", tmp_unit);
    copytrans_svcnum_ = static_cast<size_t>(tmp_unit);

    const size_t SIZE_OF_BUF_LEN = 256;
    char tmp_buf[256], err_buf[256];

    // 读取所有的路由数据
    for (unsigned int i = 0; i < copytrans_svcnum_; ++i)
    {
        snprintf(tmp_buf, SIZE_OF_BUF_LEN, "COPYSVCID%u", i + 1);
        tmp_unit = cfg->copy_info_.svr_id_[i].service_id_;

        snprintf(err_buf, SIZE_OF_BUF_LEN, "COPYTRANS|%s key error.", tmp_buf);
        TESTCONFIG(tmp_unit != 0 , err_buf);
        ZLOG_DEBUG("Copytrans%d: %u|%u.", i, copytrans_svctype_, tmp_unit);
        copytrans_svcid_[i] = tmp_unit;
    }

    // 检查是否有重复的ID
    std::list <unsigned int> check_list;

    for (size_t i = 0; i < copytrans_svcnum_; ++i)
    {
        check_list.push_back(copytrans_svcid_[i]);
    }

    // 不讲求效率的地方
    check_list.sort();
    check_list.unique();

    if (check_list.size() != copytrans_svcnum_ )
    {
        ZLOG_ERROR("Cfg file have repeat COPYSVCID,Please check.");
        return TSS_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//
int CopyTransmitAllProxyProcess::process_proxy(Comm_App_Frame *proc_frame)
{
    int ret = 0;

    // 输出包头，看看
    proc_frame->dumpoutput_framehead("[FROM RECV FRAME]", RS_DEBUG);

    // 内部处理的命令,跳过
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_DEBUG("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                   proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    // 这样处理是否好，我不知道，
    if (proc_frame->recv_service_.services_type_ != copytrans_svctype_)
    {
        ZLOG_ERROR("Can't Porcess services_type_%u. ", proc_frame->recv_service_.services_type_);
        return TSS_RET::ERR_PROXY_RCVSVC_TYPE_ERROR;
    }

    // 复制生成N个帧，转发到不同的服务器
    for (size_t i = 0; i < copytrans_svcnum_; ++i)
    {
        // 修改为新的ID
        proc_frame->recv_service_.services_id_ = copytrans_svcid_[i];
        ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }

        ZLOG_DEBUG( "Copy to [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, trans_id[%u]. ",
                    proc_frame->recv_service_.services_type_,
                    proc_frame->recv_service_.services_id_,
                    proc_frame->frame_uin_,
                    proc_frame->frame_command_,
                    proc_frame->frame_length_,
                    proc_frame->transaction_id_);
    }

    return TSS_RET::TSS_RET_SUCC;
}

const DBModalMGRouteItem *DBModalMGProxyInfo::find_route(unsigned int uin)
{
    if (route_cfg_.size() == 0)
    {
        // 没有配置路由,这一定是个错误
        ZLOG_ERROR("[%s] no route configed", __ZEN_FUNCTION__);
        return NULL;
    }

    DBModalMGRouteItem tmp;
    // 取uin的低16位作为hash值,应该够随机的
    tmp.hash_ = (uin & 0xFFFF);

    std::vector<DBModalMGRouteItem>::iterator iter
        = std::upper_bound(route_cfg_.begin(), route_cfg_.end(), tmp);

    // 如果指向路由表的末尾,那么实际上应该是是路由表的第一项
    if(iter == route_cfg_.end())
    {
        iter = route_cfg_.begin();
    }

    return &(*iter);
}

/****************************************************************************************************
class DBModalMGProxyProcess 按照APPID和UIN进行Proxy转发，用于手游类业务 PROXY_TYPE_DB_MODAL_MG
****************************************************************************************************/
DBModalMGProxyProcess::DBModalMGProxyProcess():
    InterfaceProxyProcess()
{
}

DBModalMGProxyProcess::~DBModalMGProxyProcess()
{
    clear_all_entrys();
}

int DBModalMGProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{
    int ret = TSS_RET::TSS_RET_SUCC;

    // 得到过滤得命令
    ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    clear_all_entrys();

    for(size_t i = 0; i < cfg->dbmodal_info_mg_.entry_cnt_; i++)
    {
        for(size_t j = 0; j < cfg->dbmodal_info_mg_.entrys_[i].route_cnt_; j++)
        {
            DBModalMGProxyInfo *dbmodal_mg_proxy_info = add_proxy(&cfg->dbmodal_info_mg_.entrys_[i].route_info_[j]);

            if(cfg->dbmodal_info_mg_.entrys_[i].app_id_cnt_ > 0)
            {
                for(size_t k = 0; k < cfg->dbmodal_info_mg_.entrys_[i].app_id_cnt_; k++)
                {
                    ret = add_entry(cfg->dbmodal_info_mg_.entrys_[i].app_id_[k],
                        dbmodal_mg_proxy_info->router_svr_type_, dbmodal_mg_proxy_info);
                    if(ret != TSS_RET::TSS_RET_SUCC)
                    {
                        return ret;
                    }
                }
            }
            else
            {
                // 没有指定appid的是默认路由
                ret = add_entry(0, dbmodal_mg_proxy_info->router_svr_type_, dbmodal_mg_proxy_info);
                if(ret != TSS_RET::TSS_RET_SUCC)
                {
                    return ret;
                }
            }
        }
    }

    return ret;
}

int DBModalMGProxyProcess::process_proxy(Comm_App_Frame *proc_frame)
{
    ZLOG_DEBUG("Receive a dbmodemg frame to process,"
               "send svr:[%u|%u], "
               "recv svr:[%u|%u], "
               "frame_uin:%u, "
               "frame_cmd:%u, "
               "frame_len:%u. ",
               proc_frame->send_service_.services_type_,
               proc_frame->send_service_.services_id_,
               proc_frame->recv_service_.services_type_,
               proc_frame->recv_service_.services_id_,
               proc_frame->frame_uin_,
               proc_frame->frame_command_,
               proc_frame->frame_length_);

    int ret = 0;
    // 内部处理的命令,跳过
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_INFO("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                  proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    uint32_t recv_service;
    const DBModalMGRouteItem *route_item =
        find_proxy(proc_frame->app_id_, proc_frame->recv_service_.services_type_, proc_frame->frame_uin_, recv_service);

    if(route_item != NULL)
    {
        proc_frame->recv_service_.services_type_ = recv_service;
        proc_frame->recv_service_.services_id_ =  route_item->normal_router_;

        // 日志调整为DEBUG级别的
        ZLOG_DEBUG("Send to main services [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, trans_id[%u]. ",
                   proc_frame->recv_service_.services_type_,
                   proc_frame->recv_service_.services_id_,
                   proc_frame->frame_uin_,
                   proc_frame->frame_command_,
                   proc_frame->frame_length_,
                   proc_frame->transaction_id_);

        // 只生成了一个帧
        int ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
        }

        // 如果有备份路由,则将数据转发给一个备份的代理
        if (route_item->clone_router_ != SERVICES_ID::INVALID_SERVICES_ID )
        {
            proc_frame->recv_service_.services_id_ =  route_item->clone_router_;
            ZLOG_INFO("Send to backup services [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, back trans_id[%u]. ",
                      proc_frame->recv_service_.services_type_,
                      proc_frame->recv_service_.services_id_,
                      proc_frame->frame_uin_,
                      proc_frame->frame_command_,
                      proc_frame->frame_length_,
                      proc_frame->backfill_trans_id_);

            ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

            if (ret != TSS_RET::TSS_RET_SUCC)
            {
                return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
            }

            // 备份路由, 多增加一次
            ArbiterFlowStat::instance()->increase_channel_pkg_num(proc_frame->send_service_.services_type_,
                                                                  proc_frame->recv_service_.services_type_);
        }

        // 如果有旁路路由,则将数据转发给一个旁路的代理
        if (route_item->passby_router_ != SERVICES_ID::INVALID_SERVICES_ID )
        {
            proc_frame->recv_service_.services_id_ =  route_item->passby_router_;
            ZLOG_INFO("Send to passby services [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, back trans_id[%u]. ",
                proc_frame->recv_service_.services_type_,
                proc_frame->recv_service_.services_id_,
                proc_frame->frame_uin_,
                proc_frame->frame_command_,
                proc_frame->frame_length_,
                proc_frame->backfill_trans_id_);

            ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

            if (ret != TSS_RET::TSS_RET_SUCC)
            {
                return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
            }

            //备份路由, 多增加一次
            ArbiterFlowStat::instance()->increase_channel_pkg_num(
                proc_frame->send_service_.services_type_,
                proc_frame->recv_service_.services_type_);
        }

    }
    //另外一个路由方向的事情，直接透传
    else
    {
        //
        //加一条日志，方便跟踪
        ZLOG_INFO("Send back [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u. ",
                  proc_frame->recv_service_.services_type_,
                  proc_frame->recv_service_.services_id_,
                  proc_frame->frame_uin_,
                  proc_frame->frame_command_,
                  proc_frame->frame_length_);

        ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return TSS_RET::ERR_PROXY_SEND_PIPE_IS_FULL;
        }
    }

    //else ELSE不做处理，DB Modal PROXY只是将发向DB一侧的数据进行修改。

    ArbiterFlowStat::instance()->increase_channel_pkg_num(proc_frame->send_service_.services_type_,
                                                          proc_frame->recv_service_.services_type_);

    // 出包量统计
    ArbiterSendStat::instance()->send_flow_increase_once(
        proc_frame->recv_service_.services_type_,
        proc_frame->recv_service_.services_id_);


    return TSS_RET::TSS_RET_SUCC;
}

DBModalMGProxyInfo *DBModalMGProxyProcess::add_proxy(conf_proxysvr::RouteInfo *route_info)
{
    DBModalMGProxyInfo *dbmodal_mg_proxy_info = new DBModalMGProxyInfo();

    dbmodal_mg_proxy_info->router_svr_type_ = route_info->svr_type_;

    ZLOG_INFO("[DBModalMGProxy] route_svr_type:%u", dbmodal_mg_proxy_info->router_svr_type_);

    for(size_t i = 0; i < route_info->distribute_module_; i++)
    {
        DBModalMGRouteItem item;

        item.normal_router_ = route_info->svr_id_[i].nomal_service_id_;

        if(route_info->svr_id_[i].clone_service_id_)
        {
            item.clone_router_ = route_info->svr_id_[i].clone_service_id_;
        }
        else
        {
            item.clone_router_ = SERVICES_ID::INVALID_SERVICES_ID;
        }

        if(route_info->svr_id_[i].passby_service_id_)
        {
            item.passby_router_ = route_info->svr_id_[i].passby_service_id_;
        }
        else
        {
            item.passby_router_ = SERVICES_ID::INVALID_SERVICES_ID;
        }

        //对主路由id进行hash,很关键的一步
        unsigned int svc_id = item.normal_router_;
        for(size_t i = 0; i < 100; i++, svc_id += 1000)
        {
            item.hash_ = ZEN_LIB::crc16((const unsigned char *)&svc_id, sizeof(svc_id));

            dbmodal_mg_proxy_info->route_cfg_.push_back(item);
        }
    }

    std::sort(dbmodal_mg_proxy_info->route_cfg_.begin(), dbmodal_mg_proxy_info->route_cfg_.end());

    for(size_t i = 0; i < dbmodal_mg_proxy_info->route_cfg_.size(); i++)
    {
        ZLOG_INFO("[DBModalMGProxy] service:%u|%u|%u|%u",
            dbmodal_mg_proxy_info->route_cfg_[i].hash_,
            dbmodal_mg_proxy_info->route_cfg_[i].normal_router_,
            dbmodal_mg_proxy_info->route_cfg_[i].clone_router_,
            dbmodal_mg_proxy_info->route_cfg_[i].passby_router_);
    }

    dbmodal_mg_proxys_.push_back(dbmodal_mg_proxy_info);

    return dbmodal_mg_proxy_info;
}

int DBModalMGProxyProcess::add_entry(uint32_t app_id, uint32_t service_type, DBModalMGProxyInfo *proxy_info)
{
    DBModalMGKey key;
    std::map<DBModalMGKey, DBModalMGProxyInfo*>::iterator itor;

    key.app_id_ = app_id;
    key.service_type_ = service_type;
    itor = dbmodal_mg_proxy_map_.find(key);
    if(itor != dbmodal_mg_proxy_map_.end())
    {
        ZLOG_ERROR("[%s] proxy entry is conflict, app id: %u, service_type: %u",
            __ZEN_FUNCTION__, app_id, service_type);
        return -1;
    }

    dbmodal_mg_proxy_map_.insert(std::map<DBModalMGKey, DBModalMGProxyInfo*>::value_type(key, proxy_info));

    ZLOG_INFO("[%s] add proxy entry succ, app id: %u, service_type: %u",
        __ZEN_FUNCTION__, app_id, service_type);

    return TSS_RET::TSS_RET_SUCC;
}

const DBModalMGRouteItem *DBModalMGProxyProcess::find_proxy(uint32_t app_id, uint32_t service_type, uint32_t uin, uint32_t &recv_service)
{
    DBModalMGKey key;
    std::map<DBModalMGKey, DBModalMGProxyInfo*>::iterator itor;

    //首先尝试查找直接配置的路由表
    key.app_id_ = app_id;
    key.service_type_ = service_type;
    itor = dbmodal_mg_proxy_map_.find(key);
    if(itor != dbmodal_mg_proxy_map_.end())
    {
        recv_service = itor->second->router_svr_type_;
        return itor->second->find_route(uin);
    }

    //没有找到直接配置的路由表,尝试使用默认路由表
    key.app_id_ = 0;
    itor = dbmodal_mg_proxy_map_.find(key);
    if(itor != dbmodal_mg_proxy_map_.end())
    {
        recv_service = itor->second->router_svr_type_;
        return itor->second->find_route(uin);
    }

    //默认路由表也不存在,真的是出问题了
    ZLOG_ERROR("[%s] proxy entry is not exist, app id: %u, service_type: %u",
        __ZEN_FUNCTION__, app_id, service_type);

    return NULL;
}

void DBModalMGProxyProcess::clear_all_entrys()
{
    for(size_t i = 0; i < dbmodal_mg_proxys_.size(); i++)
    {
        delete dbmodal_mg_proxys_[i];
    }
    dbmodal_mg_proxys_.clear();
    dbmodal_mg_proxy_map_.clear();
}

/****************************************************************************************************
class CopyTransmitAllMGProxyProcess 将数据复制转发给所有配置的服务器,手游类业务 PROXY_TYPE_COPY_TRANS_ALL_MG
****************************************************************************************************/
CopyTransmitAllMGProxyProcess::CopyTransmitAllMGProxyProcess()
{
}

CopyTransmitAllMGProxyProcess::~CopyTransmitAllMGProxyProcess()
{
    clear_all_entrys();
}

int CopyTransmitAllMGProxyProcess::get_proxy_config(conf_proxysvr::LPCONFIG cfg)
{
    //
    int ret = InterfaceProxyProcess::get_proxy_config(cfg);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    clear_all_entrys();

    for(size_t i = 0; i < cfg->copy_info_mg_.entry_cnt_; i++)
    {
        CopyTransmitAllMGProxyInfo *proxy_info = add_proxy(&cfg->copy_info_mg_.entrys_[i]);
        if(cfg->copy_info_mg_.entrys_[i].app_id_cnt_ > 0)
        {
            for(size_t j = 0; j < cfg->copy_info_mg_.entrys_[i].app_id_cnt_; j++)
            {
                ret = add_entry(cfg->copy_info_mg_.entrys_[i].app_id_[j], proxy_info);
                if(ret != TSS_RET::TSS_RET_SUCC)
                {
                    return ret;
                }
            }
        }
        else
        {
            //没有appid的是默认路由
            ret = add_entry(0, proxy_info);
            if(ret != TSS_RET::TSS_RET_SUCC)
            {
                return ret;
            }
        }
    }

    return TSS_RET::TSS_RET_SUCC;
}

int CopyTransmitAllMGProxyProcess::process_proxy(Comm_App_Frame *proc_frame)
{
    int ret = 0;

    //输出包头，看看
    proc_frame->dumpoutput_framehead("[FROM RECV FRAME]", RS_DEBUG);

    //内部处理的命令,跳过
    bool bsnderr;

    if (proc_frame->is_internal_process(bsnderr) == true)
    {
        ZLOG_DEBUG("Receive a internal command, frame_uin:%u, frame_command:%u. ",
                   proc_frame->frame_uin_, proc_frame->frame_command_);
        return TSS_RET::TSS_RET_SUCC;
    }

    CopyTransmitAllMGProxyInfo *proxy_info = find_proxy(proc_frame->app_id_);
    if(proxy_info == NULL)
    {
        ZLOG_ERROR("[%s] proxy is not exist,app id: %u",
            __ZEN_FUNCTION__, proc_frame->app_id_);
        return -1;
    }

    //这样处理是否好，我不知道，
    if (proc_frame->recv_service_.services_type_ != proxy_info->service_type_)
    {
        ZLOG_ERROR("Can't Porcess services_type_%u. ", proc_frame->recv_service_.services_type_);
        return TSS_RET::ERR_PROXY_RCVSVC_TYPE_ERROR;
    }

    //复制生成N个帧，转发到不同的服务器
    for(size_t i = 0; i < proxy_info->svcid_.size(); i++)
    {
        //修改为新的ID
        proc_frame->recv_service_.services_id_ = proxy_info->svcid_[i];
        ret = zerg_mmap_pipe_->push_back_sendpipe(proc_frame);

        //
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }

        ZLOG_DEBUG( "Copy to [%u|%u], frame_uin:%u, frame_command:%u, frame_len:%u, trans_id[%u]. ",
                    proc_frame->recv_service_.services_type_,
                    proc_frame->recv_service_.services_id_,
                    proc_frame->frame_uin_,
                    proc_frame->frame_command_,
                    proc_frame->frame_length_,
                    proc_frame->transaction_id_);
    }

    return TSS_RET::TSS_RET_SUCC;
}

CopyTransmitAllMGProxyInfo *CopyTransmitAllMGProxyProcess::add_proxy(conf_proxysvr::CopySvrIdMG *copy_svr_id)
{
    CopyTransmitAllMGProxyInfo *proxy_info = new CopyTransmitAllMGProxyInfo;

    proxy_info->service_type_ = copy_svr_id->svr_type_;
    for(size_t i = 0; i < copy_svr_id->copy_cnt_; i++)
    {
        proxy_info->svcid_.push_back(copy_svr_id->svr_id_[i].service_id_);
    }

    return proxy_info;
}

int CopyTransmitAllMGProxyProcess::add_entry(uint32_t app_id, CopyTransmitAllMGProxyInfo *proxy_info)
{
    std::map<uint32_t, CopyTransmitAllMGProxyInfo*>::iterator itor = proxy_map_.find(app_id);

    if(itor != proxy_map_.end())
    {
        ZLOG_ERROR("[%s] proxy is conflict, app id: %u",
            __ZEN_FUNCTION__, app_id);
        return -1;
    }

    proxy_map_.insert(std::map<uint32_t, CopyTransmitAllMGProxyInfo*>::value_type(app_id, proxy_info));

    return TSS_RET::TSS_RET_SUCC;
}

CopyTransmitAllMGProxyInfo *CopyTransmitAllMGProxyProcess::find_proxy(uint32_t app_id)
{
    std::map<uint32_t, CopyTransmitAllMGProxyInfo*>::iterator itor = proxy_map_.find(app_id);

    if(itor != proxy_map_.end())
    {
        return itor->second;
    }

    //尝试使用默认配置

    itor = proxy_map_.find(0);
    if(itor != proxy_map_.end())
    {
        return itor->second;
    }

    ZLOG_ERROR("[%s] proxy entry is not exist, app id: %u",
        __ZEN_FUNCTION__, app_id);

    return NULL;
}

void CopyTransmitAllMGProxyProcess::clear_all_entrys()
{
    for(size_t i = 0; i < proxys_.size(); i++)
    {
        delete proxys_[i];
    }
    proxys_.clear();
    proxy_map_.clear();
}
