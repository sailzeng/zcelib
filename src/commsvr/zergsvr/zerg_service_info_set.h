
#ifndef ZERG_TCP_SERVICES_INFO_SET_H_
#define ZERG_TCP_SERVICES_INFO_SET_H_

class TCP_Svc_Handler;
/****************************************************************************************************
class  Service_Info_Set
****************************************************************************************************/
class Service_Info_Set
{
protected:
    typedef unordered_map<SERVICES_ID, TCP_Svc_Handler *, HashofSvrInfo> MapOfSvrPeerInfo;
    
    // accept的连接
    MapOfSvrPeerInfo  accept_svr_info_set_;

    // connect的连接
    MapOfSvrPeerInfo  connect_svr_info_set_;

public:
    //构造函数,
    Service_Info_Set();
    ~Service_Info_Set();

    //
    void init_services_peerinfo(size_t max_accept, size_t max_connect);

    //查询配置信息, 先查找主动连接, 再查找被动连接
    int find_services_peerinfo(const SERVICES_ID &svrinfo,
                               TCP_Svc_Handler*&);

    //查询主动连接
    int find_connect_services_peerinfo(const SERVICES_ID &svrinfo,
        TCP_Svc_Handler*&);

    //增加设置配置信息
    int add_services_peerinfo(const SERVICES_ID &svrinfo,
                              TCP_Svc_Handler *new_svchdl);

    //更新设置配置信息
    int replace_services_peerInfo(const SERVICES_ID &svrinfo,
                                  TCP_Svc_Handler *new_svchdl,
                                  TCP_Svc_Handler *&old_svchdl);

    //删除配置信息
    size_t del_services_peerInfo(const SERVICES_ID &svrinfo,
                                 TCP_Svc_Handler *svchdl);

    //当前的数量
    size_t get_services_peersize();

    //
    void dump_svr_peerinfo(std::ostringstream &ostr_stream, size_t startno, size_t numquery);

    //关闭所有的PEER
    void clear_and_closeall();

private:
    MapOfSvrPeerInfo* get_svr_peer_set(TCP_Svc_Handler *svchdl);

    void dump_peerinfo_set(const MapOfSvrPeerInfo &peer_set, std::ostringstream &ostr_stream, size_t startno, size_t numquery);
};

#endif //_ZERG_TCP_SERVICES_INFO_SET_H_

