
#ifndef ZERG_TCP_SERVICES_INFO_SET_H_
#define ZERG_TCP_SERVICES_INFO_SET_H_



class TCP_Svc_Handler;


/*!
* @brief      
*             
* @note       
*/
class Service_Info_Set
{
protected:

    //
    typedef unordered_map<SERVICES_ID, TCP_Svc_Handler *, HashofSvrInfo> MapOfSvrPeerInfo;
    //
    MapOfSvrPeerInfo  svr_info_set_;

public:
    //构造函数,
    Service_Info_Set();
    ~Service_Info_Set();

    //
    void init_services_peerinfo(size_t szpeer);

    //查询配置信息
    int find_services_peerinfo(const SERVICES_ID &svrinfo,
        TCP_Svc_Handler * &);

    //增加设置配置信息
    int add_services_peerinfo(const SERVICES_ID &svrinfo,
        TCP_Svc_Handler *new_svchdl);

    //更新设置配置信息
    int replace_services_peerInfo(const SERVICES_ID &svrinfo,
        TCP_Svc_Handler *new_svchdl,
        TCP_Svc_Handler *&old_svchdl);

    //删除配置信息
    size_t del_services_peerInfo(const SERVICES_ID &svrinfo);

    //当前的数量
    size_t get_services_peersize();

    //
    void dump_svr_peerinfo(std::ostringstream &ostr_stream, size_t startno, size_t numquery);

    //关闭所有的PEER
    void clear_and_closeall();

};



#endif //_ZERG_TCP_SERVICES_INFO_SET_H_


