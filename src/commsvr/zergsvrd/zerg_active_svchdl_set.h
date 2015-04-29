
#ifndef ZERG_TCP_SERVICES_INFO_SET_H_
#define ZERG_TCP_SERVICES_INFO_SET_H_



class TCP_Svc_Handler;




/*!
* @brief      SVC ID SET
*
* @note       反思一下,是否应该做这个封装，我也很矛盾，如果在TCP_Svc_Handler直接使用STL的封装数据，
*             那么我们的封装后再聚和的方式不是那么的好。在TCP_Svc_Handler还要再次封装。。。。
*/
class Active_SvcHandle_Set
{


public:
    ///构造函数,
    Active_SvcHandle_Set();
    ///析构函数
    ~Active_SvcHandle_Set();

    //初始化
    void initialize(size_t sz_peer);


    /*!
    * @brief      根据SVCID，在Active的Handle里面查询是否尊重
    * @return     int ==0 表示查询成功，
    * @param[in]  svrinfo    查询的SERVICES_ID,
    * @param[out] svc_handle 返回的对应的handle
    */
    int find_handle_by_svcid(const SERVICES_ID &svc_id,
                             TCP_Svc_Handler *& svc_handle);

    /*!
    * @brief      以负载均衡的方式，根据services type查询一个的SVC，按照序列数组顺序轮询的返回，
    *             lbseq(Load Balance sequence)
    * @return     int  ==0表示查询成功
    * @param[in]  services_type 服务器类型
    * @param[out] services_id   查询到的SVC ID
    * @param      svc_handle    返回对应的Handle
    * @note       这样查询保证发送的数据尽量负载均衡，
    */
    int find_lbseqhdl_by_type(uint16_t services_type,
                              uint32_t &services_id,
                              TCP_Svc_Handler*& svc_handle);


    /*!
    * @brief
    * @return     int
    * @param      services_type
    * @param      lb_factor
    * @param      services_id
    * @param      svc_handle
    */
    int find_lbfactorhdl_by_type(uint16_t services_type,
                                 uint32_t lb_factor,
                                 uint32_t &services_id,
                                 TCP_Svc_Handler*& svc_handle);


    /*!
    * @brief      以主备的方式，根据services type尽量查询得到一个的SVC ID以及对应的Handle，
    *             只能用于在AUTO CONNECT配置的链接出去的服务器，主备顺序根据AUTO CONNECT配置
    *             顺序决定，配置在前面的服务器优先考虑 MS(Main Standby)
    * @return     int ==0表示查询成功
    * @param[in]  services_type 服务器类型
    * @param[out] find_services_id  查询到的SVC ID
    * @param[out] svc_handle    返回对应的Handle
    * @note       优先主，主有问题，选择备份。这样可以保证发送的数据都（尽量）给一个服务器
    *             注意这样模式，配置的主备服务器不要数量过多(<=4)，
    */
    int find_mshdl_by_type(uint16_t services_type,
                           uint32_t &find_services_id,
                           TCP_Svc_Handler*& svc_handle);


    /*!
    * @brief      查询类型对应的所有active的SVC ID数组，用于广播等
    * @return     int       ==0表示查询成功
    * @param      services_type  查询的服务器类型
    * @param      id_ary    这个类型对应的所有active的SVC ID的services_id数值
    * @note       可以对某个类型进行广播
    */
    int find_hdlary_by_type(uint16_t services_type, std::vector<uint32_t> *& id_ary);

    /*!
    * @brief      增加设置配置信息
    * @return     int
    * @param      svc_id      新增加的服务器的SVC ID
    * @param      new_svchdl  新增加的服务器的句柄
    */
    int add_services_peerinfo(const SERVICES_ID &svc_id,
                              TCP_Svc_Handler *new_svchdl);


    /*!
    * @brief
    * @return     int        更新设置配置信息
    * @param      svc_id     新增加的服务器的SVC ID
    * @param      new_svchdl 新增加的服务器句柄
    * @param      old_svchdl 如果原来有一个svrinfo对应的Hdler,返回通知你,
    * @note       返回的old_svchdl,可以用于清理
    */
    int replace_services_peerInfo(const SERVICES_ID &svc_id,
                                  TCP_Svc_Handler *new_svchdl,
                                  TCP_Svc_Handler *&old_svchdl);


    /*!
    * @brief      根据SERVICES_ID,删除PEER信息,
    * @return     int    ==0表示删除成功
    * @param      svc_id   要删除服务器的SVC ID
    * @note
    */
    int del_services_peerInfo(const SERVICES_ID &svc_id);

    ///当前的数量
    size_t get_services_peersize();

    ///
    void dump_svr_peerinfo(ZCE_LOG_PRIORITY out_lvl);

    ///关闭所有的PEER
    void clear_and_closeall();


protected:



    struct SERVICES_ID_TABLE
    {
        ///
        size_t orderid_use_id_ = 0;
        ///
        std::vector<uint32_t>  services_id_ary_;
    };

    ///
    typedef std::unordered_map<SERVICES_ID, TCP_Svc_Handler *, HASH_OF_SVCID> MAP_OF_SVCPEERINFO;

    ///用于根据TYPE选择一个任意服务器，或者根据TYPE广播给所有这个类型的服务器
    typedef std::unordered_map<uint16_t, SERVICES_ID_TABLE > MAP_OF_TYPE_TO_IDTABLE;


    ///
    size_t max_peer_size_ = 0;

    ///
    MAP_OF_SVCPEERINFO  svr_info_set_;
    ///
    MAP_OF_TYPE_TO_IDTABLE type_to_idtable_;
};



#endif //_ZERG_TCP_SERVICES_INFO_SET_H_


