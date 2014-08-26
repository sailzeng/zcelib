#ifndef ZERG_SERVER_CONFIG_H_
#define ZERG_SERVER_CONFIG_H_

/****************************************************************************************************
class  ZERG_SERVICES_INFO
****************************************************************************************************/
struct ZERG_SERVICES_INFO
{
public:
    //
    SERVICES_ID        zerg_svc_info_;
    //
    bool               zerg_sessionkey_;
    //
    ZCE_Sockaddr_In    zerg_ip_addr_;

    // 是否接收外部包
    bool is_recv_external_pkg_;
    // cmd offset
    uint32_t cmd_offset_;
    // cmd len
    uint32_t cmd_len_;

    ZERG_SERVICES_INFO();
    ~ZERG_SERVICES_INFO();
};

//
typedef std::vector<ZERG_SERVICES_INFO>  SELF_SERVICESINFO_LIST;

//用XML作为配置文件还是INI作为配置文件,经过投票调查,大部分人喜欢INI,
//看来喜欢简单的土人居多,
/****************************************************************************************************
class  Zerg_Server_Config
****************************************************************************************************/
class Zerg_Server_Config
{

    Zerg_Server_Config();
    ~Zerg_Server_Config();

    //

    //最大的
    static const size_t MAX_SELF_SERVICES_ID = 4;
public:
    //早上6:00更新Operation key
    static const time_t UPDATE_OPERATION_KEY_TIME = 3600 * 6;

public:

    //统计文件
    std::string            zerg_stat_file_;

    //命令统计文件
    std::string            cmd_stat_file_;

    //ZERG的保险是否使用
    bool                   zerg_insurance_;

    //
    bool                   zerg_need_opkey_;
    //是否有UDP SESSION
    bool                   zerg_udp_session_;

    //服务的配置集合
    ServicesConfig         services_info_cfg_;

    //监听队列长度
    unsigned int backlog_;

    //下面这几个为了加快使用速度，使用了STATIC,在美学和速度面前选择速度
public:

    //自己的服务器的标示ID,
    static ZERG_SERVICES_INFO         self_svc_info_;

    //SLAVE的服务器的标示ID,用于启动第2个端口，或者N个端口
    static SELF_SERVICESINFO_LIST     slave_svc_ary_;

    // 接收外部Udp端口信息
    static SELF_SERVICESINFO_LIST     external_udp_svr_ary_;

    //
    static SELF_SERVICESINFO_LIST     extern_svc_ary_;

    //单子实例
    static Zerg_Server_Config        *instance_;

public:

    //得到文件配置参数
    //这个接口有点怪，为了把一些配置信息打出来，必须在之前就把日志策略初始化好
    //virtual int get_file_configure();

    //Dump配置的数据信息
    void dump_status_info(std::ostringstream &ostr_stream);

    //根据SVCINFO得到IP地址信息
    int GetServicesIPInfo(const SERVICES_ID &svc_info, ZCE_Sockaddr_In     &ipaddr);

    int init(const conf_zerg::ZERG_CONFIG *config);

    //初始化外部服务
    int init_extern_svc();

public:

    //单子函数
    static Zerg_Server_Config *instance();
    //清理单子指针函数
    static void clean_instance();

    const conf_zerg::ZERG_CONFIG *config_;
};

#endif //_ZERG_SERVER_CONFIG_H_

