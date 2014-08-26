#ifndef ZERG_SERVER_AUTO_CONNECT_H_
#define ZERG_SERVER_AUTO_CONNECT_H_

//class TcpHandlerImpl;

/****************************************************************************************************
strcut SERVICES_ROUTE_INFO
****************************************************************************************************/
struct SERVICES_ROUTE_INFO
{
public:

    SERVICES_ROUTE_INFO();
    ~SERVICES_ROUTE_INFO();

public:
    //主路由SVCID
    SERVICES_ID                 main_route_info_;
    //主路由IP地址信息
    ZCE_Sockaddr_In             main_route_ip_;

    //备份路由是否有效
    bool                        back_route_valid_;
    //备份路由信息
    SERVICES_ID                 back_route_info_;
    //备份路由IP地址信息
    ZCE_Sockaddr_In             back_route_ip_;

    // 是否使用非标准协议
    bool  external_;

    // 如果是外部协议，协议解析相关配置项索引
    size_t proto_cfg_index_;
};

//
typedef std::vector< SERVICES_ROUTE_INFO > ArrayOfSvcInfoIPAddr;

/****************************************************************************************************
按services type保存服务器主动连接信息

用两个二维数组保存服务器主动连接的services id，一个数组对应主路由id列表，一个数组对应备份路由id列表
数组的第一级数组成员对应services type(对应关系保存用map保存)，第二级数组成员为services id
****************************************************************************************************/
//保存services type到数组索引对应关系的map
typedef std::map<unsigned short, unsigned int>  RouteType2Index;

//主路由id列表
typedef std::vector<std::vector<unsigned int> > ListOfMainRouteId;

//备份路由id列表
typedef std::vector<std::vector<unsigned int> > ListOfBackRouteId;

/****************************************************************************************************
class Zerg_Auto_Connector
****************************************************************************************************/
class Zerg_Auto_Connector
{

protected:

    //
    ArrayOfSvcInfoIPAddr      ary_want_connect_;
    //
    size_t                    size_of_wantconnect_;

    //连接器
    ZCE_Socket_Connector      zerg_connector_;

private:
    //主路由id列表
    ListOfMainRouteId           list_of_want_connect_main_id_;

    //备份路由id列表
    ListOfMainRouteId           list_of_want_connect_back_id_;

    //主动连接type到index的map
    RouteType2Index             want_connect_type_2_index_;
//    std::vector<TcpHandlerImpl*> external_proto_handler_;

public:

    //构造函数
    Zerg_Auto_Connector();
    ~Zerg_Auto_Connector();

    //
    int get_autoconnect_cfg(const conf_zerg::ZERG_CONFIG *config);

    // 重新加载主动连接配置
    int reload_cfg(const conf_zerg::ZERG_CONFIG *config);

    //链接所有的tcp服务器
    void reconnect_tcpserver(size_t &szvalid, size_t &sz_succ, size_t &szfail);

    //连接所有的udp服务器
    void reconnect_udpserver(size_t &szvalid, size_t &sz_succ, size_t &szfail);

    //链接的数量
    size_t numsvr_connect();

    //根据主服务器，得到备份服务器的信息
    int get_backupsvcinfo(const SERVICES_ID &main_svrinfo,
                          bool &backroute_valid,
                          SERVICES_ID &backroute_svrinfo);

    //根据SVRINFO,检查是否是主动连接的服务.,
    int reconnect_server(const SERVICES_ID &reconnect_svrinfo);

    //根据svr type获取serviceid，有多个id时随机获取一个
    int get_server(unsigned short svr_type,
                   SERVICES_ID *svrinfo);

    //根据svr type按uin hash方式获取serviceid
    int get_server_byuinhash(unsigned short svr_type,
                   SERVICES_ID *svrinfo,
                   unsigned int uin);

    int get_all_conn_server(unsigned short svr_type,
        std::vector<SERVICES_ID> &vec);
protected:

    //根据SVRINFO+IP,检查是否是主动连接的服务.并进行连接
    int connect_server_bysvcid(const SERVICES_ID &svrinfo, const ZCE_Sockaddr_In &inet_addr, size_t index);

    //主动连接udp服务器
    int connect_udpserver_bysvcid(const SERVICES_ID &svrinfo, const ZCE_Sockaddr_In &inet_addr, int index);

    //根据svrinfo 检查是否已经连接
    bool is_connected(const SERVICES_ID &svrinfo);

    //根据svrinfo 检查连接是否有阻塞
    bool is_socket_block(const SERVICES_ID &svrinfo);

private:
    //添加主动连接服务信息
    void add_auto_connect_info(const SERVICES_ID &main_svrinfo, const SERVICES_ID &back_svrinfo);

    // 判定是否当前主动连接
    bool is_current_auto_connect(const SERVICES_ID &service, bool is_main_service);

};

#endif //_ZERG_SERVER_AUTO_CONNECT_H_

