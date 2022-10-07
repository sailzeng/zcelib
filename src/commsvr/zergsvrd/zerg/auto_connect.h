#pragma once

namespace zerg
{
class zerg_config;
class svc_tcp;

class auto_connector
{
public:

    //构造函数
    auto_connector();
    ~auto_connector();

    ///读取配置
    int get_config(const zerg::zerg_config* config);

    // 重新加载主动连接配置
    int reload_cfg(const zerg::zerg_config* config);

    /*!
    * @brief      链接所有的服务器,如果已经有链接，就跳过,
    * @return     void
    * @param      szvalid  仍然有效的的链接
    * @param      sz_succ  成功开始连接的服务器个数
    * @param      szfail   链接失败的服务器数量，但是由于是异步链接，这个地方并不一定真是进行了链接
    * @note
    */
    void reconnect_allserver(size_t& szvalid, size_t& sz_succ, size_t& szfail);

    /*!
    * @brief      根据SVC ID,检查是否是主动连接的服务.,
    * @return     int
    * @param      reconnect_svcid 要进行重连的主路由信息
    * @note       为什么不把一个TCP_Svc_Handler作为参数返回,因为在发起Connect过程中,也可能close_event.
    */
    int connect_server_bysvcid(const soar::SERVICES_ID& reconnect_svcid);

    /*!
    * @brief      根据services_type查询对应的配置主备服务器列表数组 MS（主备）
    * @return     int == 0 表示成功
    * @param[in]  services_type
    * @param[out] ms_svcid_ary   配置的主备服务器列表数组
    */
    int find_conf_ms_svcid_ary(uint16_t services_type,
                               std::vector<uint32_t>*& ms_svcid_ary);

    /*!
    * @brief      检查这个SVC ID是否是主动链接的服务器
    * @return     bool
    * @param      svc_id SVC ID
    */
    bool is_auto_connect_svcid(const soar::SERVICES_ID& svc_id);

protected:

    //
    /*!
    * @brief      根据SVC ID+IP,检查是否是主动连接的服务.并进行连接
    * @return     int
    * @param      svc_id       要进行连接的SVC ID
    * @param      inet_addr    地址
    * @param      svc_handle , 如果已经有相应的连接，在这个地方返回对用的Handle
    */
    int connect_one_server(const soar::SERVICES_ID& svc_id,
                           const zce::skt::addr_in& inet_addr,
                           svc_tcp*& svc_handle);

protected:

    //
    typedef std::unordered_set<soar::SERVICES_INFO,
        soar::HASH_OF_SVCINFO,
        soar::EQUAL_OF_SVCINFO> SET_OF_SVC_INFO;

    ///类型对应的SERVICES ID 数组的MAP的类型,
    typedef std::unordered_map<uint16_t, std::vector<uint32_t> > MAP_OF_TYPE_TO_IDARY;

protected:

    //连接器
    zce::skt::connector zerg_connector_;

    //配置实例指针
    const zerg_config* zerg_svr_cfg_ = NULL;

    ///主动链接的
    size_t size_of_autoconnect_ = 0;

    ///SVC ID 对应ary_auto_connect_的数组下标，用于使用SVCID的查询，监控查询性能
    SET_OF_SVC_INFO autocnt_svcinfo_set_;

    //类型对应的SERVICES ID 数组 的MAP，数组里面的id的顺序其实是主备顺序
    MAP_OF_TYPE_TO_IDARY type_to_idary_map_;
};
}
