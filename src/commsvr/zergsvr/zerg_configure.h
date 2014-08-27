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
    ZCE_Sockaddr_In    zerg_ip_addr_;

};


struct ZERG_CONFIG
{
    //最大的
    static const size_t MAX_SELF_SERVICES_ID = 4;

    //
    ZERG_SERVICES_INFO  svc_info_ary_[MAX_SELF_SERVICES_ID];


    //服务的配置集合
    Services_Table_Config  services_info_cfg_;


    //统计文件
    std::string            zerg_stat_file_;

    //ZERG的保险是否使用
    bool                   zerg_insurance_;


    // 被动连接的发送BUFFER可容纳最大FRAME的个数 默认32，连接数少而流量较大的后端服务器可填的大一些, 目前DB填256，其它全部是32
    uint32_t accept_send_buf_size; 
    // 每个tcp连接的发送队列长度
    uint32_t connect_send_deque_size; 
    // #从CONNECT到收到数据,最小时长,0-50，接入层必须配置>0,建议15-60秒以内
    uint32_t connect_timeout; 

    // RECEIVE一个数据的超时时间,为0表示不限制,建议根据业务层的逻辑判断一下
    uint32_t recv_timeout; 

    // 是否做为代理服务器
    uint8_t is_proxy; 
    // #最大连接的服务器个数 ##前端128000，后端1024
    uint32_t max_accept_svr; 
    // #对一个错误数据重复尝试发送的次数
    uint8_t retry_error; 
    // 接收管道长度, 默认50M
    uint32_t recv_pipe_len;
    // 发送管道长度, 默认50M
    uint32_t send_pipe_len;
    
};


/****************************************************************************************************
class  Zerg_Server_Config
****************************************************************************************************/
class Zerg_Server_Config :public Server_Config_Base
{

    Zerg_Server_Config();
    ~Zerg_Server_Config();

    //
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



public:

    ZERG_CONFIG   zerg_config_;


    //单子实例
    static Zerg_Server_Config  *instance_;



};

#endif //_ZERG_SERVER_CONFIG_H_

