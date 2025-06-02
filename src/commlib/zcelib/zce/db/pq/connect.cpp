#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/connect.h"
#include "zce/db/pq/result.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
//构造函数,析构函数
int connect::connect_by_info(const char* conninfo)
{
    if (conninfo == nullptr)
    {
        return -1;
    }
    //连接数据库
    conn_ = ::PQconnectdb(conninfo);
    if (conn_ == nullptr)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] PQconnectdb fail return nullptr.conninfo:[%s]",
                conninfo);
        return -1;
    }
    //检查连接状态
    if (::PQstatus(conn_) != ::CONNECTION_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ::PQstatus(conn_) != ::CONNECTION_OK:[%s]",
                error_message());
        ::PQfinish(conn_);
        conn_ = nullptr;
        return -1;
    }
    return 0;
}

int connect::connect_host(const char* host_name,
                          const unsigned int port,
                          const char* user,
                          const char* pwd,
                          const char* db)
{
    if (host_name == nullptr)
    {
        return -1;
    }
    char pq_port[16] = { 0 };
    snprintf(pq_port, 16, "%u", port);
    //连接数据库
    conn_ = ::PQsetdbLogin(host_name,
                           pq_port,
                           nullptr,
                           nullptr,
                           db,
                           user,
                           pwd);
    if (conn_ == nullptr)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] PQconnectdb fail return nullptr."
                "host:[%s][%u][%s][%s]",
                host_name,
                port,
                user,
                db);
        return -1;
    }
    //检查连接状态
    if (::PQstatus(conn_) != ::CONNECTION_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ::PQstatus(conn_) != ::CONNECTION_OK:[%s]",
                error_message());
        ::PQfinish(conn_);
        conn_ = nullptr;
        return -1;
    }
    if_connected_ = true;
    return 0;
}

int connect::connect_url(const char* url)
{
    if (url == nullptr)
    {
        return -1;
    }
    conn_ = ::PQconnectdb(url);
    if (conn_ == nullptr)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] PQconnectdb fail return nullptr.url:[%s]",
                url);
        return -1;
    }
    if (::PQstatus(conn_) != ::CONNECTION_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ::PQstatus(conn_) != ::CONNECTION_OK:[%s]",
                error_message());
        ::PQfinish(conn_);
        conn_ = nullptr;
        return -1;
    }
    if_connected_ = true;
    return 0;
}

void connect::disconnect()
{
    if (conn_)
    {
        ::PQfinish(conn_);
        conn_ = nullptr;
    }
}
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
