#include "zerg/predefine.h"
#include "zerg/configure.h"
#include "zerg/ip_restrict.h"
#include "zerg/comm_manager.h"
#include "zerg/udp_ctrl_handler.h"
#include "zerg/tcp_ctrl_handler.h"

//===================================================================================================

SERVICES_INFO_TABLE::SERVICES_INFO_TABLE(size_t szconf)
{
    services_table_.rehash(szconf);
}

SERVICES_INFO_TABLE::~SERVICES_INFO_TABLE()
{
}

int SERVICES_INFO_TABLE::find_svcinfo(const soar::SERVICES_ID& svc_id,
                                      zce::skt::addr_in& ip_address,
                                      unsigned int& idc_no,
                                      unsigned int& business_id) const
{
    soar::SERVICES_INFO svc_ip_info;
    svc_ip_info.svc_id_ = svc_id;
    SET_OF_SVCINFO::const_iterator iter = services_table_.find(svc_ip_info);

    if (iter == services_table_.end())
    {
        ZCE_LOG(RS_ERROR, "[soarlib]Can't find svc id [%u|%u] info.",
                svc_id.services_type_,
                svc_id.services_id_);
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }

    ip_address = iter->ip_address_;
    idc_no = iter->idc_no_;
    business_id = iter->business_id_;

    return 0;
}

//根据SvrInfo信息查询IP配置信息
int SERVICES_INFO_TABLE::find_svcinfo(const soar::SERVICES_ID& svc_id,
                                      soar::SERVICES_INFO& svc_info) const
{
    svc_info.svc_id_ = svc_id;
    SET_OF_SVCINFO::const_iterator iter = services_table_.find(svc_info);

    if (iter == services_table_.end())
    {
        ZCE_LOG(RS_ERROR, "[soarlib]Can't find svc id [%u|%u] info.",
                svc_id.services_type_,
                svc_id.services_id_);
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }
    svc_info = *iter;

    return 0;
}

//检查是否拥有相应的Services Info
bool SERVICES_INFO_TABLE::hash_svcinfo(const soar::SERVICES_ID& svrinfo) const
{
    soar::SERVICES_INFO svc_ip_info;
    svc_ip_info.svc_id_ = svrinfo;
    SET_OF_SVCINFO::const_iterator iter = services_table_.find(svc_ip_info);

    if (iter == services_table_.end())
    {
        return false;
    }

    return true;
}

//设置配置信息
int SERVICES_INFO_TABLE::add_svcinfo(const soar::SERVICES_INFO& svc_info)
{
    std::pair<SET_OF_SVCINFO::iterator, bool> insert_result = services_table_.insert(svc_info);

    if (insert_result.second == false)
    {
        ZCE_LOG(RS_ERROR, "add svcinfo fail. may be have repeat svc info[%hu.%u].",
                svc_info.svc_id_.services_type_, svc_info.svc_id_.services_id_);
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }

    return 0;
}

void SERVICES_INFO_TABLE::clear()
{
    services_table_.clear();
}

//===================================================================================================
//Zerg_Config
//===================================================================================================
Zerg_Config::Zerg_Config()
{
}

Zerg_Config::~Zerg_Config()
{
}

//根据SVCINFO得到IP地址信息
int Zerg_Config::get_svcinfo_by_svcid(const soar::SERVICES_ID& svc_id,
                                      soar::SERVICES_INFO& svc_info) const
{
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    int ret = 0;
    ret = services_info_table_.find_svcinfo(svc_id, svc_info);
    if (0 != ret)
    {
        return ret;
    }

    //打印一下
    ZCE_LOG(RS_INFO, "[zergsvr] Svc id [%u|%u] ipaddress info [%s].",
            svc_id.services_type_,
            svc_id.services_id_,
            svc_info.ip_address_.to_string(ip_addr_str, IP_ADDR_LEN, use_len));

    return ret;
}

int Zerg_Config::read_cfgfile()
{
    //
    int ret = 0;
    ret = Server_Config_Base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    // 未指定通讯服务器配置
    zerg_cfg_file_ = app_run_dir_ + "/cfg/zergsvrd.cfg";

    zce::propertytree pt_tree;
    ret = zce::cfg::read_ini(zerg_cfg_file_.c_str(), &pt_tree);
    ZCE_LOG(RS_INFO, "zergsvr read config file [%s] ret [%d].",
            zerg_cfg_file_.c_str(), ret);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_log_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_zerg_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    pt_tree.clear();
    ret = zce::cfg::read_ini(svc_table_file_.c_str(), &pt_tree);
    ZCE_LOG(RS_INFO, "zergsvr read svcid table file [%s] ret [%d].",
            svc_table_file_.c_str(),
            ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = get_svcidtable_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//从配置中读取ZERG的配置
int Zerg_Config::get_zerg_cfg(const zce::propertytree* conf_tree)
{
    int ret = 0;
    std::string temp_value;
    std::vector <std::string> str_ary;

    const size_t MIN_SEND_DEQUE_SIZE = 4;
    const size_t MAX_SEND_DEQUE_SIZE = 512;

    //最大Accept 数量
    ret = conf_tree->path_get_leaf("ZERG_CFG", "MAX_ACCEPT_SVR",
                                   zerg_cfg_data_.max_accept_svr_);
    if (0 != ret || zerg_cfg_data_.max_accept_svr_ < 32)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //Accept和conect端口的发送队列长度
    ret = conf_tree->path_get_leaf("ZERG_CFG", "ACPT_SEND_DEQUE_SIZE",
                                   zerg_cfg_data_.acpt_send_deque_size_);
    if (0 != ret
        || zerg_cfg_data_.acpt_send_deque_size_ < MIN_SEND_DEQUE_SIZE
        || zerg_cfg_data_.acpt_send_deque_size_ > MAX_SEND_DEQUE_SIZE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("ZERG_CFG", "CNNT_SEND_DEQUE_SIZE",
                                   zerg_cfg_data_.cnnt_send_deque_size_);
    if (0 != ret
        || zerg_cfg_data_.cnnt_send_deque_size_ < MIN_SEND_DEQUE_SIZE
        || zerg_cfg_data_.cnnt_send_deque_size_ > MAX_SEND_DEQUE_SIZE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //保险
    ret = conf_tree->path_get_leaf("ZERG_CFG", "ZERG_INSURANCE",
                                   zerg_cfg_data_.zerg_insurance_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //BACKLOG
    ret = conf_tree->path_get_leaf("ZERG_CFG", "LISTEN_BACKLOG",
                                   zerg_cfg_data_.accept_backlog_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    if (zerg_cfg_data_.accept_backlog_ == 0)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        zerg_cfg_data_.accept_backlog_ = ZCE_DEFAULT_BACKLOG;
    }

    //各种超时处理的时间
    ret = conf_tree->path_get_leaf("ZERG_CFG", "ACCEPTED_TIMEOUT",
                                   zerg_cfg_data_.accepted_timeout_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("ZERG_CFG", "RECV_TIMEOUT",
                                   zerg_cfg_data_.receive_timeout_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //允许和拒绝IP
    ret = conf_tree->path_get_leaf("ZERG_CFG", "REJECT_IP",
                                   zerg_cfg_data_.reject_ip_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("ZERG_CFG", "ALLOW_IP",
                                   zerg_cfg_data_.allow_ip_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //监控命令
    ret = conf_tree->path_get_leaf("ZERG_CFG", "MONITOR_CMD_LIST",
                                   temp_value);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    str_ary.clear();
    zce::str_split(temp_value.c_str(), "|", str_ary);
    if (str_ary.size() > ZERG_CONFIG_DATA::MAX_MONITOR_FRAME_NUMBER)
    {
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    zerg_cfg_data_.monitor_cmd_count_ = str_ary.size();
    for (size_t i = 0; i < str_ary.size(); ++i)
    {
        zerg_cfg_data_.monitor_cmd_list_[i] = static_cast<uint32_t>(std::stoul(str_ary[i]));
    }

    //错误重试次数
    ret = conf_tree->path_get_leaf("ZERG_CFG", "RETRY_ERROR",
                                   zerg_cfg_data_.retry_error_);
    if (0 != ret || zerg_cfg_data_.retry_error_ > 5)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //BIND SVCID 读取配置
    ret = conf_tree->path_get_leaf("SLAVE_SVCID", "SLAVE_SVC_NUM",
                                   zerg_cfg_data_.bind_svcid_num_);
    if (0 != ret || zerg_cfg_data_.bind_svcid_num_ > ZERG_CONFIG_DATA::MAX_SLAVE_SERVICES_ID)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //第0个位置是给self_svc_id_ 主ID的
    zerg_cfg_data_.bind_svcid_ary_[0] = self_svc_info_.svc_id_;
    zerg_cfg_data_.bind_svcid_num_ += 1;

    //注意是从1开始,注意bind_svcid_num_上面已经+1了，
    for (size_t i = 1; i < zerg_cfg_data_.bind_svcid_num_; ++i)
    {
        ret = conf_tree->pathseq_get_leaf("SLAVE_SVCID", "SLAVE_SERVICES_ID_", i, temp_value);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }
        ret = zerg_cfg_data_.bind_svcid_ary_[i].from_str(temp_value.c_str(), true);
        if (0 != ret)
        {
            return ret;
        }
    }

    //读取自动连接的配置
    ret = conf_tree->path_get_leaf("AUTO_CONNECT", "AUTO_CONNECT_NUM",
                                   zerg_cfg_data_.auto_connect_num_);
    if (0 != ret || zerg_cfg_data_.auto_connect_num_ > ZERG_CONFIG_DATA::MAX_AUTO_CONNECT_SVRS)
    {
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    for (size_t i = 0; i < zerg_cfg_data_.auto_connect_num_; ++i)
    {
        ret = conf_tree->pathseq_get_leaf("AUTO_CONNECT", "CNT_SERVICES_ID_", i + 1, temp_value);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }
        ret = zerg_cfg_data_.auto_connect_svrs_[i].from_str(temp_value.c_str(), true);
        if (0 != ret)
        {
            return ret;
        }
    }

    //设置reactor的句柄数量
    max_reactor_hdl_num_ = zerg_cfg_data_.max_accept_svr_ + zerg_cfg_data_.auto_connect_num_ + 64;
    max_reactor_hdl_num_ = max_reactor_hdl_num_ > 64 ? max_reactor_hdl_num_ : 1024;

    return 0;
}

//得到某个配置文件的配置信息,配置文件其中有[SERVICES_TABLE]字段
int Zerg_Config::get_svcidtable_cfg(const zce::propertytree* conf_tree)
{
    int ret = 0;
    std::string temp_value;

    size_t svc_table_num;
    ret = conf_tree->path_get_leaf("SERVICES_TABLE", "SERVICES_NUM",
                                   svc_table_num);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    soar::SERVICES_INFO svc_info;
    for (size_t i = 0; i < svc_table_num; ++i)
    {
        ret = conf_tree->pathseq_get_leaf("SERVICES_TABLE", "SERVICES_INFO_", i + 1, temp_value);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }

        ret = svc_info.from_str(temp_value.c_str(), true);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return ret;
        }
        ret = services_info_table_.add_svcinfo(svc_info);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }
    }

    return 0;
}

//Dump配置的数据信息
void Zerg_Config::dump_cfg_info(zce::LOG_PRIORITY out_lvl)
{
    Server_Config_Base::dump_cfg_info(out_lvl);

    ZCE_LOG(out_lvl, "Application zerg config file :%s", zerg_cfg_file_.c_str());

    ZCE_LOG(out_lvl, "[ZERG]Bind svcid number :%u", zerg_cfg_data_.bind_svcid_num_);
    for (size_t i = 0; i < zerg_cfg_data_.bind_svcid_num_; ++i)
    {
        ZCE_LOG(out_lvl, "[ZERG]Bind svcid %u :%u.%u",
                i,
                zerg_cfg_data_.bind_svcid_ary_[i].services_type_,
                zerg_cfg_data_.bind_svcid_ary_[i].services_id_
        );
    }
}