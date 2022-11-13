#pragma once

#include "ogre/configure.h"

namespace ogre
{
class svc_tcp;

class auto_connect
{
public:

    //构造函数
    auto_connect();
    ~auto_connect();

    ///读取配置
    int get_config(const configure* config);

    /*!
    * @brief      对所有的服务器进行重新链接
    * @return     int == 0表示成功
    * @param      num_vaild   仍然有效，无需连接的服务器梳理
    * @param      num_succ    成功进行链接的服务器数量，
    * @param      num_fail    失败的数量
    */
    int connect_all_server(size_t& num_vaild, size_t& num_succ, size_t& num_fail);

    /*!
    * @brief      根据SVRINFO,检查是否是主动连接的服务.并进行连接
    * @return     int  == 0表示成功
    * @param      peer_id 要链接服务器的PEER ID
    */
    int connect_server_by_peerid(const soar::OGRE_PEER_ID& peer_id);

protected:
    /*!
    * @brief      重链某个服务器
    * @return     int == 0表示成功
    * @param      peer_module 服务器的配置信息
    */
    int connect_one_server(const TCP_PEER_MODULE_INFO& peer_module);

protected:

    typedef std::unordered_set < TCP_PEER_MODULE_INFO,
        HASH_OF_PEER_MODULE,
        EQUAL_OF_PEER_MODULE > SET_OF_TCP_PEER_MODULE;

    ///连接器
    zce::skt::connector ogre_connector_;

    ///
    size_t auto_connect_num_ = 0;
    ///要主动链接的PEER的SET，用SET是查询方便
    SET_OF_TCP_PEER_MODULE     autocnt_module_set_;
};
}// namespace ogre
