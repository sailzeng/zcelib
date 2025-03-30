#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/command.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
int command::execute(std::string_view sql_cmd,
                     size_t& num_affect,
                     uint64_t* last_id)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
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

int command::execute(std::string_view sql_cmd,
                     size_t& num_affect,
                     zce::pq::result* /*pq_res*/)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
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

#endif
