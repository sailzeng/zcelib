#pragma once

#include "zce/db/result_set.h"

//
namespace zce::db
{
template <typename CONNECT, typename COMMAND, typename RESULT, typename STMT_RES = RESULT>
class exec
{
public:
    using cnt = typename CONNECT;
    using cmd = typename COMMAND;
    using res = typename RESULT;
    using stmt_res = typename STMT_RES;

    template <typename... Types>
    using res_set = typename zce::db::result_set<RESULT, Types...>;

public:
    //!构造函数，不用处理什么，相关的成员变量的析构都进行了处理
    exec() = delete;
    ~exec() = delete;
    exec(const exec&) = delete;
    exec& operator=(const exec&) = delete;

    //!链接MYSQL数据库
    static int connect(CONNECT* db_connect,
                       const char* host_name,
                       unsigned int port,
                       const char* user,
                       const char* pwd)
    {
        int ret = 0;

        //连接数据库
        if (db_connect->is_connected() == false)
        {
            //如果设置过HOST，用HOST NAME进行连接
            ret = db_connect->connect_host(host_name,
                                           port,
                                           user,
                                           pwd,
                                           nullptr);

            //如果错误
            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "[zcelib] DB Error : [%u]:%s.",
                        db_connect->error_no(),
                        db_connect->error_message());
                return -1;
            }
        }
        return 0;
    }

    //!断开链接
    static void disconnect(CONNECT* db_connect)
    {
        if (db_connect->is_connected() == true)
        {
            db_connect->disconnect();
        }
    }

    static int execute(CONNECT* db_connect,
                       std::string_view sql)
    {
        COMMAND cmd(*db_connect);
        int ret = cmd.execute(sql);
        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql.data());
            return -1;
        }

        //成功
        return 0;
    }

    //!查询，非SELECT语句
    static int execute(CONNECT* db_connect,
                       std::string_view sql,
                       size_t& num_affect,
                       uint64_t* insert_id)
    {
        //连接数据库
        if (db_connect->is_connected() == false)
        {
            return -1;
        }

        ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sql.size(), sql.data());
        COMMAND cmd(*db_connect);
        int ret = cmd.execute(sql, num_affect, insert_id);
        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql.data());
            return -1;
        }

        //成功
        return 0;
    }

    //!查询，SELECT语句
    static int execute(CONNECT* db_connect,
                       std::string_view sql,
                       size_t& num_affect,
                       RESULT& db_result)
    {
        int ret = 0;
        //连接数据库
        if (db_connect->is_connected() == false)
        {
            return -1;
        }

        ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql.size(), sql.data());

        COMMAND cmd(*db_connect);
        ret = cmd.execute(sql, num_affect, db_result);
        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql.data());
            return -1;
        }

        //成功
        return 0;
    }
};
}
#if defined ZCE_USE_MYSQL

#include "zce/db/mysql/connect.h"
#include "zce/db/mysql/command.h"
#include "zce/db/mysql/result.h"

namespace zce::db
{
using my_exec = exec<zce::mysql::connect,
    zce::mysql::command,
    zce::mysql::result>;
}
#endif

#if defined ZCE_USE_PQ

#include "zce/db/pq/connect.h"
#include "zce/db/pq/command.h"
#include "zce/db/pq/result.h"

namespace zce::db
{
using pq_exec = exec<zce::pq::connect,
    zce::pq::command,
    zce::pq::result>;
}

#endif

#if defined ZCE_USE_SQLITE

#include "zce/db/sqlite/connect.h"
#include "zce/db/sqlite/command.h"
#include "zce/db/sqlite/result.h"

namespace zce::db
{
using sqlite_exec = exec<zce::sqlite::connect,
    zce::sqlite::command,
    zce::sqlite::result,
    zce::sqlite::stmt_result>;
}

#endif
