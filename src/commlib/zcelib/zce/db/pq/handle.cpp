#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/handle.h"
#include "zce/db/pq/result.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
//构造函数,析构函数
int handle::connect_by_info(const char* conninfo)
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

int handle::connect_by_host(const char* pghost,
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

void handle::disconnect()
{
    if(conn_)
    {
        ::PQfinish(conn_);
        conn_ = nullptr;
    }
}

int handle::execute(size_t& num_affect, uint64_t* last_id)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd_.c_str());
    ::ExecStatusType status = ::PQresultStatus(res);
    if(status != ::PGRES_TUPLES_OK && status != ::PGRES_COMMAND_OK)
    {
        ::PQclear(res);
        return -1;
    }
    if(status == ::PGRES_TUPLES_OK)
    {
        auto s_num = ::PQcmdTuples(res);
        num_affect = std::stoull(s_num);
    }
    if(last_id)
    {
        if(PQresultStatus(res) == PGRES_TUPLES_OK)
        {
            char* id_str = PQgetvalue(res, 0, 0);
            *last_id = atoll(id_str);
        }
        else
        {
            ZCE_LOG(RS_ALERT,
                    "[pq] execute sql don't have last id [%s].",
                    ::PQresultErrorMessage(res));
            return -1;
        }
    }
    PQclear(res);
    return 0;
}

int handle::execute(size_t& num_affect, zce::pq::result* /*pq_res*/)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd_.c_str());
    ::ExecStatusType status = ::PQresultStatus(res);
    if(status != ::PGRES_TUPLES_OK && status != ::PGRES_COMMAND_OK)
    {
        ::PQclear(res);
        return -1;
    }
    if(status == ::PGRES_TUPLES_OK)
    {
        auto s_num = ::PQcmdTuples(res);
        num_affect = std::stoull(s_num);
    }
    //pq_res->set_pg_result(res);
    return 0;
}
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
