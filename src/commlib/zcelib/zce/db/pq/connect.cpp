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
    if(conninfo == nullptr)
    {
        return -1;
    }
    //连接数据库
    conn_ = ::PQconnectdb(conninfo);
    if(conn_ == nullptr)
    {
        return -1;
    }
    //检查连接状态
    if(::PQstatus(conn_) != ::CONNECTION_OK)
    {
        ::PQfinish(conn_);
        conn_ = nullptr;
        return -1;
    }
    return 0;
}

int connect::connect_by_host(const char* pghost,
                            const char* pgport,
                            const char* pgoptions,
                            const char* dbname,
                            const char* user,
                            const char* pwd)
{
    if(pghost == nullptr)
    {
        return -1;
    }
    //连接数据库
    conn_ = ::PQsetdbLogin(pghost,
                           pgport,
                           pgoptions,
                           nullptr,
                           dbname,
                           user,
                           pwd);
    if(conn_ == nullptr)
    {
        return -1;
    }
    //检查连接状态
    if(::PQstatus(conn_) != ::CONNECTION_OK)
    {
        ::PQfinish(conn_);
        conn_ = nullptr;
        return -1;
    }
    return 0;
}

void connect::disconnect()
{
    if(conn_)
    {
        ::PQfinish(conn_);
        conn_ = nullptr;
    }
}
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
