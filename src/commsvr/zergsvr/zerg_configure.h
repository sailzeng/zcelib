#ifndef ZERG_SERVER_CONFIG_H_
#define ZERG_SERVER_CONFIG_H_


/*!
* @brief     小虫的配置数据 
*             
* @note       
*/
struct ZERG_CONFIG_DATA
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

    ///默认的ACCEPT端口的backlog，LightHTTP这个值得配置是1024，如果需要猛然间登陆很多人，可以调整到更大
    static const int DEFUALT_ZERG_BACKLOG = 512;

    ///
    static const uint32_t DEFUALT_CONNECT_SEND_DEQUE_SIZE = 128;
    ///
    static const uint32_t DEFUALT_ACCEPT_SEND_DEQUE_SIZE = 32;

public:
    
    //服务器支持的SVC ID的数量，至少>=1
    size_t valid_svc_num_=0;
    //服务器支持的SVC ID，注意，self_svc_id_永远配置在第一个
    SERVICES_ID  bind_svcid_ary_[MAX_SELF_SERVICES_ID];

    //主动链接的服务器梳理
    size_t auto_connect_num_ = 0;
    //主动链接的服务器数组
    SERVICES_ID  auto_connect_svrs_[MAX_AUTO_CONNECT_SVRS];

    //ZERG的保险是否使用
    bool zerg_insurance_ = true;

    int accept_backlog_ = DEFUALT_ZERG_BACKLOG;

    // 被动连接的发送BUFFER可容纳最大FRAME的个数 连接数少而流量较大的后端服务器可填的大一些, 目前DB填256，其它全部是32
    uint32_t acpt_send_deque_size_ = DEFUALT_ACCEPT_SEND_DEQUE_SIZE;
    // 每个connect 出去的，tcp连接的发送队列长度
    uint32_t cnnt_send_deque_size_ = DEFUALT_CONNECT_SEND_DEQUE_SIZE;

    // #从CONNECT到收到数据,最小时长,0-50，接入层必须配置>0,建议15-60秒以内
    uint32_t connect_timeout_ = 60; 
    // RECEIVE一个数据的超时时间,为0表示不限制,建议根据业务层的逻辑判断一下
    uint32_t recv_timeout_ = 0; 

    // 是否做为代理服务器
    bool is_proxy_ = false; 

    // #最大连接的服务器个数 ##前端128000，后端1024
    uint32_t max_accept_svr_ = 1024;

    // #对一个错误数据重复尝试发送的次数,目前这个值没用用处了，
    uint32_t retry_error_ = 3;

    // 接收管道长度, 默认50M
    uint32_t recv_pipe_len_ = 50 * 1024 * 1024;
    // 发送管道长度, 默认50M
    uint32_t send_pipe_len_ = 50 * 1024 * 1024;

    ///拒绝不允许链接的IP地址列表，用空格分开
    std::string reject_ip_;
    ///允许链接的IP地址列表，用空格分开
    std::string allow_ip_;

    ///监控的命令的总数
    size_t monitor_cmd_count_ = 0;
    ///监控的命令
    uint32_t monitor_cmd_list_[MAX_MONITOR_FRAME_NUMBER];
};




/*!
* @brief      小虫的配置处理类
*             
* @note       
*/
class Zerg_Server_Config :public Server_Config_Base
{
public:
    //
    Zerg_Server_Config();
    virtual ~Zerg_Server_Config();

    //
public:

    /*!
    * @brief      取配置信息,取得配置信息后, 需要将各启动参数设置OK
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int initialize(int argc, const char *argv[]);


    ///读取配置文件，得到文件配置参数
    virtual int load_cfgfile();


    /*!
    * @brief      读取，在日志里面输出一些配置信息，以便跟踪回溯
    * @param      out_lvl 输出级别，
    */
    virtual void dump_cfg_info(ZCE_LOG_PRIORITY out_lvl);


    //根据SVCID得到SVC INFO地址信息
    int get_svcinfo_by_svcid(const SERVICES_ID &svc_id, SERVICES_INFO  &svc_info) const;
    

public:

    ///ZERG的配置数据
    ZERG_CONFIG_DATA  zerg_cfg_data_;

    ///服务的配置集合标
    SERVICES_INFO_TABLE  services_info_cfg_;
};

#endif //_ZERG_SERVER_CONFIG_H_

