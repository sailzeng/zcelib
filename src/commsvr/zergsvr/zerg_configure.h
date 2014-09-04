#ifndef ZERG_SERVER_CONFIG_H_
#define ZERG_SERVER_CONFIG_H_


/*!
* @brief      
*             
* @note       
*/
struct ZERG_SERVICES_INFO
{
public:
    ZERG_SERVICES_INFO();
    ~ZERG_SERVICES_INFO();

    //
    SERVICES_ID      zerg_svc_info_;

    //
    ZCE_Sockaddr_In  zerg_ip_addr_;

};






struct ZERG_CONFIG
{
public:

    ///最大的
    static const size_t MAX_SELF_SERVICES_ID = 4;

    ///最大的监控的FRAME的数量,不希望太多,可能严重影响效率
    static const size_t MAX_MONITOR_FRAME_NUMBER = 16;

    ///最大的主动链接服务器数量
    static const size_t MAX_AUTO_CONNECT_SVRS = 512;
    
    ///最大的跟踪记录的最大服务器类型，用于自动选择路由，广播等
    static const size_t MAX_RECORD_SERVICES_TYPE = 16;


public:
    //
    ZERG_SERVICES_INFO  svc_info_ary_[MAX_SELF_SERVICES_ID];


    //服务的配置集合
    Services_Table_Config  services_info_cfg_;



    //ZERG的保险是否使用
    bool zerg_insurance_;


    // 被动连接的发送BUFFER可容纳最大FRAME的个数 默认32，连接数少而流量较大的后端服务器可填的大一些, 目前DB填256，其它全部是32
    uint32_t accept_send_buf_size_; 
    // 每个tcp连接的发送队列长度
    uint32_t connect_send_deque_size_; 

    // #从CONNECT到收到数据,最小时长,0-50，接入层必须配置>0,建议15-60秒以内
    uint32_t connect_timeout_; 
    // RECEIVE一个数据的超时时间,为0表示不限制,建议根据业务层的逻辑判断一下
    uint32_t recv_timeout_; 

    // 是否做为代理服务器
    bool is_proxy_; 

    // #最大连接的服务器个数 ##前端128000，后端1024
    uint32_t max_accept_svr_;
    // #对一个错误数据重复尝试发送的次数
    uint32_t retry_error_;

    // 接收管道长度, 默认50M
    uint32_t recv_pipe_len_;
    // 发送管道长度, 默认50M
    uint32_t send_pipe_len_;

    ///拒绝不允许链接的IP地址
    std::string reject_ip_;
    ///允许链接的IP地址列表
    std::string allow_ip_;

    ///
    size_t monitor_cmd_count_;
    ///
    uint32_t monitor_cmd_list_[MAX_MONITOR_FRAME_NUMBER];

    //
    size_t auto_connect_num_;
    //
    SERVICES_ID  auto_connect_svrs_[MAX_AUTO_CONNECT_SVRS];

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

    /*!
    * @brief      取配置信息,取得配置信息后, 需要将各启动参数设置OK
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int initialize(int argc, const char *argv[]);


    //得到文件配置参数
    //这个接口有点怪，为了把一些配置信息打出来，必须在之前就把日志策略初始化好
    //virtual int get_file_configure();

    //Dump配置的数据信息
    //void dump_status_info(std::ostringstream &ostr_stream);

    //根据SVCINFO得到IP地址信息
    int GetServicesIPInfo(const SERVICES_ID &svc_info, ZCE_Sockaddr_In     &ipaddr);

    

public:

    //单子函数
    static Zerg_Server_Config *instance();
    //清理单子指针函数
    static void clean_instance();



public:

    //
    ZERG_CONFIG  zerg_config_;


    //单子实例
    static Zerg_Server_Config  *instance_;



};

#endif //_ZERG_SERVER_CONFIG_H_

