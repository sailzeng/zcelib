#ifndef ZERG_SERVER_CONFIG_H_
#define ZERG_SERVER_CONFIG_H_



//===================================================================================

//服务配置
struct SERVICES_INFO_TABLE
{

    typedef unordered_set<SERVICES_INFO, HASH_OF_SVCINFO, EQUAL_OF_SVCINFO> SET_OF_SVCINFO;

public:
    //构造函数,
    SERVICES_INFO_TABLE(size_t sz_table = INIT_NUM_OF_SVR_CONFG);
    ~SERVICES_INFO_TABLE();


    /*!
    * @brief      根据SvrInfo信息查询IP配置信息
    * @return     int
    * @param      svc_id      传入参数,服务器信息SVCINFO
    * @param      ip_address  输出参数,服务器信息SVCINFO
    * @param      idc_no      输出参数,IDC信息
    * @param      business_id 输出参数,服务器编号
    * @note
    */
    int find_svcinfo(const SERVICES_ID &svc_id,
        ZCE_Sockaddr_In &ip_address,
        unsigned int &idc_no,
        unsigned int &business_id) const;

    //根据SvrInfo信息查询IP配置信息
    int find_svcinfo(const SERVICES_ID &svc_id,
        SERVICES_INFO &svc_info) const;

    //检查是否拥有相应的Services Info
    bool hash_svcinfo(const SERVICES_ID &svc_id) const;

    //设置配置信息
    int add_svcinfo(const SERVICES_INFO &svc_info);


    //清理SVR配置信息.
    void clear();


protected:
    //配置的个数
    static const int INIT_NUM_OF_SVR_CONFG = 2048;

protected:
    //
    SET_OF_SVCINFO    services_table_;

};

//===================================================================================

/*!
* @brief     小虫的配置数据
*
* @note
*/
struct ZERG_CONFIG_DATA
{

public:

    ///最大的的绑定的SVC ID数量，也就是监听数量
    static const size_t MAX_BIND_SERVICES_ID = 4;
    ///
    static const size_t MAX_SLAVE_SERVICES_ID = MAX_BIND_SERVICES_ID -1;

    ///最大的监控的FRAME的数量,不希望太多,可能严重影响效率
    static const size_t MAX_MONITOR_FRAME_NUMBER = 16;

    ///最大的主动链接服务器数量
    static const size_t MAX_AUTO_CONNECT_SVRS = 512;

    ///最大的跟踪记录的最大服务器类型，用于自动选择路由，广播等
    static const size_t MAX_RECORD_SERVICES_TYPE = 16;

    ///默认的ACCEPT端口的backlog，LightHTTP这个值得配置是1024，如果需要猛然间登陆很多人，可以调整到更大
    static const int DEFUALT_ZERG_BACKLOG = 128;

    ///最小的端口的发送队列长度
    static const uint32_t MIN_SEND_DEQUE_SIZE = 4;
    ///默认Accept 端口的发送队列长度
    static const uint32_t MAX_SEND_DEQUE_SIZE = 512;

public:


    /// 接收管道长度, 默认50M
    uint32_t recv_pipe_len_ = 50 * 1024 * 1024;
    /// 发送管道长度, 默认50M
    uint32_t send_pipe_len_ = 50 * 1024 * 1024;


    /// #最大连接的服务器个数 ##前端128000，后端1024
    size_t max_accept_svr_ = 1024;

    ///ZERG的保险是否使用
    bool zerg_insurance_ = true;

    ///
    int accept_backlog_ = DEFUALT_ZERG_BACKLOG;



    /// 被动连接的发送BUFFER可容纳最大FRAME的个数 连接数少而流量较大的后端服务器可填的大一些, 
    uint32_t acpt_send_deque_size_ = 32;
    /// 每个connect 出去的，tcp连接的发送队列长度
    uint32_t cnnt_send_deque_size_ = 128;


    ///服务器支持的SVC ID的数量，至少>=1
    size_t bind_svcid_num_ = 0;
    ///服务器支持的SVC ID，注意，self_svc_id_永远配置在第一个
    SERVICES_ID  bind_svcid_ary_[MAX_BIND_SERVICES_ID];

    ///主动链接的服务器梳理
    size_t auto_connect_num_ = 0;
    ///主动链接的服务器数组
    SERVICES_ID  auto_connect_svrs_[MAX_AUTO_CONNECT_SVRS];





    /// #从CONNECT到收到数据,最小时长,0-50，接入层必须配置>0,建议15-60秒以内
    uint32_t accepted_timeout_ = 60;
    /// RECEIVE一个数据的超时时间,为0表示不限制,建议根据业务层的逻辑判断一下
    uint32_t receive_timeout_ = 0;

    /// 是否做为代理服务器
    bool is_proxy_ = false;

    

    /// #对一个错误数据重复尝试发送的次数,目前这个值没用用处了，
    uint32_t retry_error_ = 3;



    ///拒绝不允许链接的IP地址列表，用空格分开
    std::string reject_ip_;
    ///允许链接的IP地址列表，用空格分开
    std::string allow_ip_;

    ///监控的命令的总数
    size_t monitor_cmd_count_ = 0;
    ///监控的命令
    uint32_t monitor_cmd_list_[MAX_MONITOR_FRAME_NUMBER];
};


//===================================================================================

/*!
* @brief      小虫的配置处理类
*
* @note
*/
class Zerg_Server_Config : public Server_Config_Base
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


    ///从配置中读取ZERG的配置
    int get_zerg_cfg(const ZCE_Conf_PropertyTree *conf_tree);


    ///得到某个配置文件的配置信息,配置文件其中有[SERVICES_TABLE]字段
    int get_svcidtable_cfg(const ZCE_Conf_PropertyTree *conf_tree);


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
    SERVICES_INFO_TABLE  services_info_table_;
};

#endif //_ZERG_SERVER_CONFIG_H_

