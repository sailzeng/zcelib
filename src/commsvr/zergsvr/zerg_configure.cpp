#include "zerg_predefine.h"
#include "zerg_configure.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_tcp_ctrl_handler.h"



//===================================================================================================
//Zerg_Server_Config
//===================================================================================================
Zerg_Server_Config::Zerg_Server_Config()
{
}

Zerg_Server_Config::~Zerg_Server_Config()
{
}

//根据SVCINFO得到IP地址信息
int Zerg_Server_Config::get_svcinfo_by_svcid(const SERVICES_ID &svc_id,
                                             SERVICES_INFO &svc_info) const
{
    int ret = 0;
    ret = services_info_cfg_.find_svcinfo(svc_id,svc_info);
    if (0 != ret)
    {
        return ret;
    }
    

    //打印一下
    ZLOG_INFO("[zergsvr] Svc id [%u|%u] ipaddress info [%s|%u].",
        svc_id.services_type_,
        svc_id.services_id_,
        svc_info.ip_address_.get_host_addr(),
        svc_info.ip_address_.get_port_number());

    return ret;
}


int Zerg_Server_Config::load_cfgfile()
{
    //
    int ret = 0;
    ret = Server_Config_Base::load_cfgfile();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//Dump配置的数据信息
void Zerg_Server_Config::dump_cfg_info(ZCE_LOG_PRIORITY out_lvl)
{
    Server_Config_Base::dump_cfg_info(out_lvl);
    //ostr_stream << "Dump Zerg_Server_Config StatusInfo:" << std::endl;


    //ostr_stream << std::setw(24) << "SELF_SVR_INFO :" << self_svc_info_.zerg_svc_info_.services_type_ << "|" << self_svc_info_.zerg_svc_info_.services_id_;
    //ostr_stream << " SEESIONKEY:" << self_svc_info_.zerg_sessionkey_;
    //ostr_stream << " IP:" << self_svc_info_.zerg_ip_addr_.get_host_addr() << "|" << self_svc_info_.zerg_ip_addr_.get_port_number() << std::endl;

    //for (unsigned int i = 0; i < slave_svc_ary_.size(); ++i )
    //{
    //    ostr_stream << "SLAVE_SVR_INFO :" << slave_svc_ary_[i].zerg_svc_info_.services_type_ << "|" << slave_svc_ary_[i].zerg_svc_info_.services_id_;
    //    ostr_stream << " SEESIONKEY:" << slave_svc_ary_[i].zerg_sessionkey_;
    //    ostr_stream << " IP:" << slave_svc_ary_[i].zerg_ip_addr_.get_host_addr() << "|" << slave_svc_ary_[i].zerg_ip_addr_.get_port_number() << std::endl;
    //}

    //ostr_stream << std::setw(24) << "RESTORE_PIPE:" << Server_Config_FSM::instance()->if_restore_pipe_ << std::endl;
    //ostr_stream << std::setw(24) << "ZERG_CONFG_PATH:" << Server_Config_FSM::instance()->zerg_cfg_file_ << std::endl;
    //ostr_stream << std::setw(24) << "ZERG_STAT_PATH:" << zerg_stat_file_ << std::endl;
    //ostr_stream << std::setw(24) << "LOG_FILE_PREFIX :" << Server_Config_FSM::instance()->log_file_prefix_ << std::endl;
    ////ostr_stream<<std::setw(24)<<"LOG_PRIORITY:"<<Comm_Svrd_Config::instance()->log_priority_<<std::endl;
}



//
int Zerg_Server_Config::initialize(int argc, const char *argv[])
{
    //
    int ret = 0;

    ret = Server_Config_Base::initialize(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    ZCE_LOGMSG(RS_INFO, "[zergsvr] Get File Configure Success.");
    return SOAR_RET::SOAR_RET_SUCC;
}


