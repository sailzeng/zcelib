#ifndef WORMHOLE_PROXY_PROCESS_H_
#define WORMHOLE_PROXY_PROCESS_H_


/*!
* @brief
*
* @note
*/
class Interface_WH_Proxy
{
public:

    /// 代理的类型
    enum PROXY_TYPE
    {
        /// 无效的TYPE
        INVALID_PROXY_TYPE           = 0,

        /// 将所有的数据数据回显
        PROXY_TYPE_ECHO              = 1,
        /// 直接进行转发处理，不对数据帧进行任何处理
        PROXY_TYPE_TRANSMIT          = 2,
        /// 将数据复制转发给所有配置的服务器
        PROXY_TYPE_BROADCAST         = 3,

        /// 按照UID取模进行Proxy转发，
        PROXY_TYPE_MODULO_UID        = 101,
        /// 按照SENDSVC_ID取模进行Proxy转发，
        PROXY_TYPE_MODULO_SENDSVCID = 102,

        /// CH,Consistent Hash,根据UID进行一致性Hash处理，
        PROXY_TYPE_CH_UID            = 201,


    };

public:

    ///构造函数
    Interface_WH_Proxy();
    virtual ~Interface_WH_Proxy();

    //初始化代理的实例
    virtual int init_proxy_instance();

    /// 通过配置文件取得代理的配置信息,这个函数只取过滤命令部分的代码,要使用指定基类访问
    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);

    ///代理的处理,返回生产的帧的个数
    virtual int process_proxy(Zerg_App_Frame *proc_frame) = 0;


protected:

    /*!
    * @brief      命令的过滤器,看不出有什么扩展必要,所以放在这儿,
    * @return     int
    * @param      cmd  要检查的CMD
    * @note
    */
    inline int filter_command(unsigned long cmd);

public:

    /*!
    * @brief      代理接口制造的工厂
    * @return     Interface_WH_Proxy*
    * @param      proxytype
    */
    static Interface_WH_Proxy *create_proxy_factory(PROXY_TYPE proxytype);



    /*!
    * @brief
    * @return     PROXY_TYPE
    * @param      str_proxy
    */
    static PROXY_TYPE str_to_proxytype(const char *str_proxy);

protected:

    // 默认初始化的长度
    static const size_t INIT_PROCESS_FRAME = 64;


    //
    Soar_MMAP_BusPipe *zerg_mmap_pipe_ = NULL;
};



//====================================================================================


/*!
* @brief      回送处理数据
*
* @note
*/
class Echo_Proxy_Process : public Interface_WH_Proxy
{

public:
    // Echo处理
    Echo_Proxy_Process();
    virtual ~Echo_Proxy_Process();

    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
    // 进行代理的处理
    virtual int process_proxy(Zerg_App_Frame *proc_frame);
};


//====================================================================================

/*!
* @brief      直接进行转发，不进行任何处理的Proxy方式
*
* @note
*/
class Transmit_Proxy : public Interface_WH_Proxy
{
public:

    Transmit_Proxy();
    virtual ~Transmit_Proxy();

    ///处理配置文件
    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
    //
    virtual int process_proxy(Zerg_App_Frame *proc_frame);
};



//====================================================================================

/*!
* @brief      将数据复制转发给所有配置的服务器
*
* @note
*/
class Broadcast_ProxyProcess : public Interface_WH_Proxy
{


public:
    Broadcast_ProxyProcess();
    virtual ~Broadcast_ProxyProcess();

    ///处理配置文件
    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
    ///
    virtual int process_proxy(Zerg_App_Frame *proc_frame);


protected:
    //
    static const size_t MAX_NUM_COPY_SVC = 512;
protected:

    // 要复制的服务器类型
    uint16_t broadcast_svctype_ = SERVICES_ID::INVALID_SERVICES_ID;
    // 要复制的数量
    size_t broadcast_svcnum_ = 0;
    // 要复制的FRAME的尺寸
    uint32_t broadcast_svcid_[MAX_NUM_COPY_SVC];
};

//====================================================================================


class Modulo_ProxyProcess : public Interface_WH_Proxy
{
public:

    enum MODULO_TYPE
    {
        //
        MODULO_UID = 1,
        //
        MODULO_SENDSVC_ID = 2,
    };

public:

    Modulo_ProxyProcess(MODULO_TYPE modulo_type);
    virtual ~Modulo_ProxyProcess();

    ///处理配置文件
    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
    ///
    virtual int process_proxy(Zerg_App_Frame *proc_frame);


protected:
    //
    static const size_t MAX_NUM_MODULO_SVC = 1024;

protected:
    //
    MODULO_TYPE modulo_type_ = MODULO_UID;

    // 要复制的服务器类型
    uint16_t modulo_svctype_ = SERVICES_ID::INVALID_SERVICES_ID;
    // 要复制的数量
    size_t modulo_svcnum_ = 0;
    // 要复制的FRAME的尺寸
    uint32_t modulo_svcid_[MAX_NUM_MODULO_SVC];
};


////====================================================================================
////取模进行数据转发的处理方式
//
//
//class DBModalProxyInfo
//{
//public:
//    // 分布的位移
//    unsigned int distribute_offset_;
//
//    // 分布的取模
//    unsigned int distribute_module_;
//
//    // 路由的服务器类型
//    unsigned short router_svr_type_;
//
//    // 主路由配置，
//    std::vector<unsigned int> normal_router_cfg_;
//
//    // 克隆路由的配置，
//    std::vector<unsigned int> clone_router_cfg_;
//
//};
//
//class DBModalProxyProcess : public Interface_WH_Proxy
//{
//
//protected:
//    // 从测试上来看偏移取8和16差不多,肯定可以保证数据量最大偏差在10%以内,所以你尽管放心
//    // 我测试的最差的取模的方式是模10类似的方式.
//
//    // 这个分布的方法来自于QQGame,我认为OFFSET取8应该有更好的表现,但是,但是
//    // 取16可以保证TC的Leader们都在机器1上,安全第一,安全第一,
//    // 为什么已经固定了256个模数,因为不会超过,日后也容易改,所以我放弃配置
//
//    // 路由表, key为service_type, value为对应路由信息
//    std::map<unsigned short, DBModalProxyInfo*> dbmodal_proxy_map_;
//
//public:
//    DBModalProxyProcess();
//    virtual ~DBModalProxyProcess();
//    //
//    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
//    //
//    virtual int process_proxy(Zerg_App_Frame *proc_frame);
//};
//
//
//
//
//
///****************************************************************************
//struct DBModalMGKey DBModalMGProxyProcess使用的索引表的key
//**************************************************************************/
//struct DBModalMGKey
//{
//public:
//    uint32_t app_id_;
//    uint32_t service_type_;
//    bool operator<(const DBModalMGKey r)const
//    {
//        return ((this->app_id_!=r.app_id_)?this->app_id_<r.app_id_:this->service_type_<r.service_type_);
//    }
//};
//
//struct DBModalMGRouteItem
//{
//public:
//    // 路由入口的hash值
//    uint16_t hash_;
//    // 主路由id
//    unsigned int normal_router_;
//    // 克隆路由id
//    unsigned int clone_router_;
//    // 旁路路由id
//    unsigned int passby_router_;
//
//    bool operator<(DBModalMGRouteItem r)const
//    {
//        return this->hash_ < r.hash_;
//    }
//};
//
//class DBModalMGProxyInfo
//{
//public:
//    // 路由的服务器类型
//    unsigned short router_svr_type_;
//    // 路由表
//    std::vector<DBModalMGRouteItem> route_cfg_;
//
//    const DBModalMGRouteItem *find_route(unsigned int uid);
//};
//
//
///****************************************************************************************************
//class  DBModalProxyMGProcess 手游类按照APPID和uid进行数据转发的处理方式
//****************************************************************************************************/
//class DBModalMGProxyProcess : public Interface_WH_Proxy
//{
//protected:
//    //
//    std::vector<DBModalMGProxyInfo*> dbmodal_mg_proxys_;
//    //
//    std::map<DBModalMGKey, DBModalMGProxyInfo*> dbmodal_mg_proxy_map_;
//
//public:
//    DBModalMGProxyProcess();
//    virtual ~DBModalMGProxyProcess();
//    //
//    virtual int get_proxy_config(const ZCE_Conf_PropertyTree *conf_tree);
//    //
//    virtual int process_proxy(Zerg_App_Frame *proc_frame);
//
//private:
//    //
//    DBModalMGProxyInfo *add_proxy();
//    //
//    int add_entry(uint32_t app_id, uint32_t service_type, DBModalMGProxyInfo *proxy_info);
//    //
//    const DBModalMGRouteItem *find_proxy(uint32_t app_id, uint32_t service_type, uint32_t uid, uint32_t &recv_service);
//    //
//    void clear_all_entrys();
//};




#endif  //WORMHOLE_PROXY_PROCESS_H_

