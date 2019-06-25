

#include "ogre_predefine.h"
#include "ogre_configure.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_comm_manager.h"


//======================================================================================

int TCP_PEER_CONFIG_INFO::from_str(const char *peer_info_str)
{
    const size_t PEER_INFO_STR_LEN = 512;
    if (strlen(peer_info_str) > PEER_INFO_STR_LEN - 1)
    {
        return SOAR_RET::ERROR_STRING_TO_PEERINFO_FAIL;
    }

    //去掉里面所有的空格，避免污染sscanf, 格式化字符串为"%u.%u.%u.%u%#%hu|%u|%u",
    //char pure_str[SVC_INFO_STR_LEN];
    //zce::str_replace(svc_info_str, pure_str," ","");
    char mod_file_name[MAX_PATH];
    uint32_t u[4] = { 0 };
    uint16_t port = 0;
    //测试发现其实不需要手动去掉多余空格的干扰，把特殊字符前面也增加%控制就可以了。
    int ret_num = sscanf(peer_info_str,
                         " %u.%u.%u.%u # %hu | %128s",
                         &u[0], &u[1], &u[2], &u[3],
                         &port,
                         mod_file_name);
    //返回9表示所有数据都读取了
    if (ret_num != 9 || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF)
    {
        return SOAR_RET::ERROR_STRING_TO_PEERINFO_FAIL;
    }

    uint32_t u32_addr = u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3];
    peer_socketin_.set(u32_addr, port);


    module_file_ = mod_file_name;

    return 0;

}


//======================================================================================

TCP_PEER_MODULE_INFO::TCP_PEER_MODULE_INFO()
{

}

TCP_PEER_MODULE_INFO::~TCP_PEER_MODULE_INFO()
{
}

///加载模块
int TCP_PEER_MODULE_INFO::open_module()
{
    recv_mod_handler_ = zce::dlopen(peer_info_.module_file_.c_str());

    if (ZCE_SHLIB_INVALID_HANDLE == recv_mod_handler_)
    {
        ZCE_LOG(RS_ERROR, "Open Module [%s] fail. recv_mod_handler =%u .\n",
                peer_info_.module_file_.c_str(),
                recv_mod_handler_);
        return SOAR_RET::ERROR_LOAD_DLL_OR_SO_FAIL;
    }

    fp_judge_whole_frame_ = (FP_JudgeRecv_WholeFrame)zce::dlsym(recv_mod_handler_,
                                                                STR_JUDGE_RECV_WHOLEFRAME);

    if (NULL == fp_judge_whole_frame_)
    {
        ZCE_LOG(RS_ERROR, "Open Module [%s|%s] fail. recv_mod_handler =%u .\n",
                peer_info_.module_file_.c_str(),
                STR_JUDGE_RECV_WHOLEFRAME,
                recv_mod_handler_);
        return SOAR_RET::ERROR_LOAD_DLL_OR_SO_FAIL;
    }

    return 0;
}

//关闭模块
int TCP_PEER_MODULE_INFO::close_module()
{
    if (ZCE_SHLIB_INVALID_HANDLE != recv_mod_handler_)
    {
        zce::dlclose(recv_mod_handler_);
        recv_mod_handler_ = ZCE_SHLIB_INVALID_HANDLE;
    }

    return 0;
}

//======================================================================================

//
Ogre_Server_Config::Ogre_Server_Config()
{
}


Ogre_Server_Config::~Ogre_Server_Config()
{

}


int Ogre_Server_Config::read_cfgfile()
{
    //
    int ret = 0;
    ret = Server_Config_Base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    // 未指定通讯服务器配置
    ogre_cfg_file_ = app_run_dir_ + "/cfg/ogre4asvrd.cfg";

    ZCE_Conf_PropertyTree pt_tree;
    ret = ZCE_INI_Implement::read(ogre_cfg_file_.c_str(), &pt_tree);
    ZCE_LOG(RS_INFO, "zergsvr read config file [%s] ret [%d].",
            ogre_cfg_file_.c_str(), ret);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_log_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_ogre_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    //设置reactor的句柄数量
    max_reactor_hdl_num_ = ogre_cfg_data_.max_accept_svr_ + ogre_cfg_data_.auto_connect_num_ + 64;
    max_reactor_hdl_num_ = max_reactor_hdl_num_ > 64 ? max_reactor_hdl_num_ : 1024;

    return 0;
}

///从配置中读取OGRE的配置
int Ogre_Server_Config::get_ogre_cfg(const ZCE_Conf_PropertyTree *conf_tree)
{

    int ret = 0;
    std::string temp_value;
    std::vector <std::string> str_ary;

    //最大Accept 数量
    ret = conf_tree->path_get_leaf("OGRE_CFG", "MAX_FRAMEDATA_LEN",
                                   ogre_cfg_data_.max_data_len_);
    if (0 != ret || ogre_cfg_data_.max_data_len_ > 8 * 1024 * 1024)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //最大Accept 数量
    ret = conf_tree->path_get_leaf("OGRE_CFG", "MAX_ACCEPT_SVR",
                                   ogre_cfg_data_.max_accept_svr_);
    if (0 != ret || ogre_cfg_data_.max_accept_svr_ < 32)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }


    //最小的端口的发送队列长度
    const uint32_t MIN_SEND_DEQUE_SIZE = 4;
    //默认Accept 端口的发送队列长度
    const uint32_t MAX_SEND_DEQUE_SIZE = 512;

    //Accept和conect端口的发送队列长度
    ret = conf_tree->path_get_leaf("OGRE_CFG", "ACPT_SEND_DEQUE_SIZE",
                                   ogre_cfg_data_.acpt_send_deque_size_);
    if (0 != ret
        || ogre_cfg_data_.acpt_send_deque_size_ < MIN_SEND_DEQUE_SIZE
        || ogre_cfg_data_.acpt_send_deque_size_ > MAX_SEND_DEQUE_SIZE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("OGRE_CFG", "CNNT_SEND_DEQUE_SIZE",
                                   ogre_cfg_data_.cnnt_send_deque_size_);
    if (0 != ret
        || ogre_cfg_data_.cnnt_send_deque_size_ < MIN_SEND_DEQUE_SIZE
        || ogre_cfg_data_.cnnt_send_deque_size_ > MAX_SEND_DEQUE_SIZE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //保险
    ret = conf_tree->path_get_leaf("OGRE_CFG", "ZERG_INSURANCE",
                                   ogre_cfg_data_.ogre_insurance_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //BACKLOG
    ret = conf_tree->path_get_leaf("OGRE_CFG", "LISTEN_BACKLOG",
                                   ogre_cfg_data_.accept_backlog_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    if (ogre_cfg_data_.accept_backlog_ == 0)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        ogre_cfg_data_.accept_backlog_ = ZCE_DEFAULT_BACKLOG;
    }

    //各种超时处理的时间
    ret = conf_tree->path_get_leaf("OGRE_CFG", "ACCEPTED_TIMEOUT",
                                   ogre_cfg_data_.accepted_timeout_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("OGRE_CFG", "RECV_TIMEOUT",
                                   ogre_cfg_data_.receive_timeout_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }


    //允许和拒绝IP
    ret = conf_tree->path_get_leaf("OGRE_CFG", "REJECT_IP",
                                   ogre_cfg_data_.reject_ip_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("OGRE_CFG", "ALLOW_IP",
                                   ogre_cfg_data_.allow_ip_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }


    //读取自动连接的配置
    ret = conf_tree->path_get_leaf("TCP_ACCEPT", "ACCEPT_PEER_NUM",
                                   ogre_cfg_data_.auto_connect_num_);
    if (0 != ret || ogre_cfg_data_.auto_connect_num_ > OGRE_CONFIG_DATA::MAX_TCPACCEPT_PEERID_NUM)
    {
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    for (size_t i = 0; i < ogre_cfg_data_.auto_connect_num_; ++i)
    {
        ret = conf_tree->pathseq_get_leaf("TCP_ACCEPT", "ACCEPT_PEER_INFO_", i + 1, temp_value);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }
        ret = ogre_cfg_data_.accept_peer_ary_[i].from_str(temp_value.c_str());
        if (0 != ret)
        {
            return ret;
        }
    }


    //读取自动连接的配置
    ret = conf_tree->path_get_leaf("AUTO_CONNECT", "AUTO_CONNECT_NUM",
                                   ogre_cfg_data_.auto_connect_num_);
    if (0 != ret || ogre_cfg_data_.auto_connect_num_ > OGRE_CONFIG_DATA::MAX_AUTO_CONNECT_PEER_NUM)
    {
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    for (size_t i = 0; i < ogre_cfg_data_.auto_connect_num_; ++i)
    {
        ret = conf_tree->pathseq_get_leaf("AUTO_CONNECT", "CNT_PEER_INFO_", i + 1, temp_value);
        if (0 != ret)
        {
            SOAR_CFG_READ_FAIL(RS_ERROR);
            return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
        }
        ret = ogre_cfg_data_.auto_cnt_peer_ary_[i].from_str(temp_value.c_str());
        if (0 != ret)
        {
            return ret;
        }
    }

    return 0;
}

