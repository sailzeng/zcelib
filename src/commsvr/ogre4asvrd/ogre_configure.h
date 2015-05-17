

#ifndef OGRE_SERVER_CONFIG_H_
#define OGRE_SERVER_CONFIG_H_

//======================================================================================



/*!
* @brief
*
* @note
*/
struct TCP_PEER_CONFIG_INFO
{
public:
    ///从字符串中转换得到
    int from_str(const char *peer_info_str);

public:

    ///PEER 的地址
    ZCE_Sockaddr_In       peer_socketin_;
    ///
    std::string          module_file_;
};


/*!
* @brief
*
* @note
*/
struct TCP_PEER_MODULE_INFO
{
public:

    TCP_PEER_MODULE_INFO();
    ~TCP_PEER_MODULE_INFO();

public:

    ///加载模块
    int open_module();

    ///关闭模块
    int close_module();

public:

    ///
    OGRE_PEER_ID  peer_id_;

    ///
    TCP_PEER_CONFIG_INFO peer_info_;

    ///TCP收取数据的模块HANDLER
    ZCE_SHLIB_HANDLE     recv_mod_handler_ = ZCE_SHLIB_INVALID_HANDLE;
    ///
    FP_JudgeRecv_WholeFrame   fp_judge_whole_frame_ = NULL;

};


//得到KEY的HASH函数
struct HASH_OF_PEER_MODULE
{
public:
    size_t operator()(const TCP_PEER_MODULE_INFO &peer_module) const
    {
        return (size_t(peer_module.peer_id_.peer_port_) << 16) + peer_module.peer_id_.peer_ip_address_;
    }
};

struct EQUAL_OF_PEER_MODULE
{
public:
    //注意判断条件不是所有的变量
    bool operator()(const TCP_PEER_MODULE_INFO &left, const TCP_PEER_MODULE_INFO &right) const
    {
        //检查SVC INFO的相等,就认为相等
        if (right.peer_id_ == left.peer_id_)
        {
            return true;
        }

        return false;
    }
};

//======================================================================================


/*!
* @brief
*
* @note
*/
struct OGRE_CONFIG_DATA
{

public:

    ///最大的的绑定的SVC ID数量，也就是监听数量
    static const size_t MAX_TCPACCEPT_PEERID_NUM = 4;

    static const size_t MAX_UDP_PEERID_NUM = 4;

    ///最大的主动链接服务器数量
    static const size_t MAX_AUTO_CONNECT_PEER_NUM = 512;

    ///最大的跟踪记录的最大服务器类型，用于自动选择路由，广播等
    static const size_t MAX_RECORD_SERVICES_TYPE = 16;

    ///默认的ACCEPT端口的backlog，LightHTTP这个值得配置是1024，如果需要猛然间登陆很多人，可以调整到更大
    static const int DEFUALT_ZERG_BACKLOG = 128;

public:

    ///最大的帧的长度
    unsigned int max_data_len_ = 32 * 1024;

    /// #最大连接的服务器个数 ##前端128000，后端1024
    size_t max_accept_svr_ = 1024;

    ///保险是否使用
    bool ogre_insurance_ = true;

    ///
    int accept_backlog_ = DEFUALT_ZERG_BACKLOG;


    /// 被动连接的发送BUFFER可容纳最大FRAME的个数 连接数少而流量较大的后端服务器可填的大一些,
    uint32_t acpt_send_deque_size_ = 32;
    /// 每个connect 出去的，tcp连接的发送队列长度
    uint32_t cnnt_send_deque_size_ = 128;

    /// #从CONNECT到收到数据,最小时长,0-50，接入层必须配置>0,建议15-60秒以内
    uint32_t accepted_timeout_ = 60;
    /// RECEIVE一个数据的超时时间,为0表示不限制,建议根据业务层的逻辑判断一下
    uint32_t receive_timeout_ = 0;


    /// #对一个错误数据重复尝试发送的次数,目前这个值没用用处了，
    uint32_t retry_error_ = 3;

    ///拒绝不允许链接的IP地址列表，用空格分开
    std::string reject_ip_;
    ///允许链接的IP地址列表，用空格分开
    std::string allow_ip_;



    ///服务器支持的Accept TCP PEER的数量，
    size_t accept_peer_num_ = 0;
    ///服务器支持Accept TCP PEER
    TCP_PEER_CONFIG_INFO  accept_peer_ary_[MAX_TCPACCEPT_PEERID_NUM];

    ///服务器支持的Connect TCP PEER的数量，主动连接出去的链接数量
    size_t auto_connect_num_ = 0;
    ///主动链接的服务器数组
    TCP_PEER_CONFIG_INFO  auto_cnt_peer_ary_[MAX_AUTO_CONNECT_PEER_NUM];


    ///服务器支持的UDP PEER的数量，
    size_t udp_peer_num_ = 0;
    ///服务器支持UDP PEER
    ZCE_Sockaddr_In udp_peer_ary_[MAX_UDP_PEERID_NUM];

};


//===================================================================================


class Ogre_Server_Config : public Server_Config_Base
{

public:

    Ogre_Server_Config();
    virtual ~Ogre_Server_Config();

public:

    ///读取配置文件，得到文件配置参数
    virtual int read_cfgfile();


    ///从配置中读取OGRE的配置
    int get_ogre_cfg(const ZCE_Conf_PropertyTree *conf_tree);

public:


    //OGRE配置文件的路径
    std::string             ogre_cfg_file_;

    //OGRE配置模块的路径
    std::string             ogre_mod_path_;

    ///ZERG的配置数据
    OGRE_CONFIG_DATA        ogre_cfg_data_;


};

#endif //OGRE_SERVER_CONFIG_H_

