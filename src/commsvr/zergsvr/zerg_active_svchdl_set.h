
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
protected:

    //
    typedef unordered_map<SERVICES_ID, TCP_Svc_Handler *, HASH_OF_SVCID> MapOfSvrPeerInfo;
    //
    MapOfSvrPeerInfo  svr_info_set_;

public:
    //构造函数,
    Active_SvcHandle_Set();
    ~Active_SvcHandle_Set();

    //
    void init_services_peerinfo(size_t szpeer);

    //查询配置信息
    int find_services_peerinfo(const SERVICES_ID &svrinfo,
                               TCP_Svc_Handler * &);


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
    * @return     size_t   删除数量
    * @param      svc_id   要删除服务器的SVC ID
    * @note
    */
    size_t del_services_peerInfo(const SERVICES_ID &svc_id);

    ///当前的数量
    size_t get_services_peersize();

    //
    void dump_svr_peerinfo(ZCE_LOG_PRIORITY out_lvl);

    //关闭所有的PEER
    void clear_and_closeall();

};



#endif //_ZERG_TCP_SERVICES_INFO_SET_H_


