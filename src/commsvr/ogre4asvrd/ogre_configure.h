

#ifndef OGRE_SERVER_CONFIG_H_
#define OGRE_SERVER_CONFIG_H_

//用XML作为配置文件还是INI作为配置文件,经过投票调查,大部分人喜欢INI,
//看来喜欢简单的土人居多,
//
class Ogre_Svr_Config
{

protected:

    Ogre_Svr_Config();
    ~Ogre_Svr_Config();

public:
    //得到启动参数
    int get_startup_param(int argc, char *argv[]);

    //得到文件配置参数
    int get_file_configure();

public:
    //打印启动参数说明
    static int print_startup_paraminfo(const char *programname);

    //单子实例获得函数
    static Ogre_Svr_Config *instance();
    //单子实例清理函数
    static void clean_instance();

public:

    //自己的服务器编号
    SERVICES_ID             self_svr_info_;

    //是否恢复共享内存的管道
    bool                    if_restore_pipe_;
    //是否高危端口保险
    bool                    zerg_insurance_;

    //OGRE配置文件的路径
    std::string             ogre_cfg_path_;

    //OGRE配置模块的路径
    std::string             ogre_mod_path_;

    //TCP通信服务器的配置
    Zen_INI_PropertyTree    cfg_ogre4a_;

    //日志文件前缀
    std::string             log_file_prefix_;
    //ACE_TRACE的日志输出级别,大于这个级别的都输出
    ZCE_LOG_PRIORITY        log_priority_;

    //自己帮定的TCP IP地址
    ZCE_Sockaddr_In         self_tcp_ip_addr_;

    //自己帮定的UDP IP地址
    ZCE_Sockaddr_In         self_udp_ip_addr_;

    //最大的帧的长度
    unsigned int            max_data_len_ ;

protected:
    //单子实例指针
    static Ogre_Svr_Config *instance_;

};

#endif //_OGRE_SERVER_CONFIG_H_

