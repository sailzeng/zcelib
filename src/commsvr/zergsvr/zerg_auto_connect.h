
#ifndef ZERG_SERVER_AUTO_CONNECT_H_
#define ZERG_SERVER_AUTO_CONNECT_H_



class Zerg_Server_Config;



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
    //
    typedef std::vector<SERVICES_INFO > ARRAY_OF_ZERG_SVCINFO;

public:

    //构造函数
    Zerg_Auto_Connector();
    ~Zerg_Auto_Connector();

    //
    int get_config(const Zerg_Server_Config *config);

    // 重新加载主动连接配置
    int reload_cfg(const Zerg_Server_Config *config);


    /*!
    * @brief      链接所有的服务器,如果已经有链接，就跳过,
    * @return     void
    * @param      szvalid  仍然有效的的链接
    * @param      sz_succ  成功开始连接的服务器个数
    * @param      szfail   链接失败的服务器数量，但是由于是异步链接，这个地方并不一定真是进行了链接
    * @note
    */
    void reconnect_allserver(size_t &szvalid, size_t &sz_succ, size_t &szfail);


    //链接的数量
    size_t numsvr_connect();


    /*!
    * @brief      根据SVC ID,检查是否是主动连接的服务.,
    * @return     int
    * @param      reconnect_svcid 要进行重连的主路由信息
    * @note       为什么不把一个TCP_Svc_Handler作为参数返回,因为在发起Connect过程中,也可能handle_close.
    */
    int reconnect_server(const SERVICES_ID &reconnect_svcid);

    //根据svr type获取serviceid，有多个id时随机获取一个
    int get_server(unsigned short svr_type, SERVICES_ID *svrinfo);
protected:

    //根据SVRINFO+IP,检查是否是主动连接的服务.并进行连接
    int connect_server_bysvcid(const SERVICES_ID &svrinfo, const ZCE_Sockaddr_In &inet_addr);

    //根据svrinfo 检查是否已经连接
    bool is_connected(const SERVICES_ID &svrinfo);

private:
    //添加主动连接服务信息
    void add_auto_connect_info(const SERVICES_ID &main_svrinfo, const SERVICES_ID &back_svrinfo);

    // 判定是否当前主动连接
    bool is_current_auto_connect(const SERVICES_ID &service, bool is_main_service);

protected:

    //
    ARRAY_OF_ZERG_SVCINFO    ary_want_connect_;
    //
    size_t                   size_of_wantconnect_;

    //连接器
    ZCE_Socket_Connector     zerg_connector_;

private:
    //主路由id列表
    ListOfMainRouteId        list_of_want_connect_main_id_;

    //备份路由id列表
    ListOfMainRouteId        list_of_want_connect_back_id_;

    //主动连接type到index的map
    RouteType2Index          want_connect_type_2_index_;

    //配置实例指针
    const Zerg_Server_Config *zerg_svr_cfg_;

};


#endif //_ZERG_SERVER_AUTO_CONNECT_H_


