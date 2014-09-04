#include "zerg_predefine.h"
#include "zerg_configure.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_tcp_ctrl_handler.h"

//自己的服务器的标示ID,


/****************************************************************************************************
class  ZERG_SERVICES_INFO
****************************************************************************************************/
ZERG_SERVICES_INFO::ZERG_SERVICES_INFO():
    zerg_svc_info_(0, 0),
    zerg_ip_addr_("1.1.1.1", 6868)
{
}

ZERG_SERVICES_INFO::~ZERG_SERVICES_INFO()
{
}

//单子示例static初始化
Zerg_Server_Config *Zerg_Server_Config::instance_ = NULL;
/****************************************************************************************************
class  Zerg_Server_Config
****************************************************************************************************/
Zerg_Server_Config::Zerg_Server_Config()
{
}

Zerg_Server_Config::~Zerg_Server_Config()
{
}

//根据SVCINFO得到IP地址信息
int Zerg_Server_Config::GetServicesIPInfo(const SERVICES_ID &svc_info,
                                          ZCE_Sockaddr_In     &ipaddr)
{
    int ret = 0;

    //打印一下
    ZLOG_INFO("[zergsvr] Svc_info [%u|%u] ipaddress info [%s|%u].",
        svc_info.services_type_,
        svc_info.services_id_,
        ipaddr.get_host_addr(),
        ipaddr.get_port_number());

    return ret;
}

//Dump配置的数据信息
//void Zerg_Server_Config::dump_status_info(std::ostringstream &ostr_stream)
//{
//    ostr_stream << "Dump Zerg_Server_Config StatusInfo:" << std::endl;
//
//    ostr_stream << std::setw(24) << "SELF_SVR_INFO :" << self_svc_info_.zerg_svc_info_.services_type_ << "|" << self_svc_info_.zerg_svc_info_.services_id_;
//    ostr_stream << " SEESIONKEY:" << self_svc_info_.zerg_sessionkey_;
//    ostr_stream << " IP:" << self_svc_info_.zerg_ip_addr_.get_host_addr() << "|" << self_svc_info_.zerg_ip_addr_.get_port_number() << std::endl;
//
//    for (unsigned int i = 0; i < slave_svc_ary_.size(); ++i )
//    {
//        ostr_stream << "SLAVE_SVR_INFO :" << slave_svc_ary_[i].zerg_svc_info_.services_type_ << "|" << slave_svc_ary_[i].zerg_svc_info_.services_id_;
//        ostr_stream << " SEESIONKEY:" << slave_svc_ary_[i].zerg_sessionkey_;
//        ostr_stream << " IP:" << slave_svc_ary_[i].zerg_ip_addr_.get_host_addr() << "|" << slave_svc_ary_[i].zerg_ip_addr_.get_port_number() << std::endl;
//    }
//
//    ostr_stream << std::setw(24) << "RESTORE_PIPE:" << Server_Config_FSM::instance()->if_restore_pipe_ << std::endl;
//    ostr_stream << std::setw(24) << "ZERG_CONFG_PATH:" << Server_Config_FSM::instance()->zerg_cfg_file_ << std::endl;
//    ostr_stream << std::setw(24) << "ZERG_STAT_PATH:" << zerg_stat_file_ << std::endl;
//    ostr_stream << std::setw(24) << "LOG_FILE_PREFIX :" << Server_Config_FSM::instance()->log_file_prefix_ << std::endl;
//    //ostr_stream<<std::setw(24)<<"LOG_PRIORITY:"<<Comm_Svrd_Config::instance()->log_priority_<<std::endl;
//}



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


//得到唯一的单子实例
Zerg_Server_Config *Zerg_Server_Config::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Zerg_Server_Config();
    }

    return instance_;
}

//清除单子实例
void Zerg_Server_Config::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}