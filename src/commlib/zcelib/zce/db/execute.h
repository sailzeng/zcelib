#pragma once

namespace zce::db
{
template <typename DBHANDLE, typename RESULT, typename STMT>
class execute
{
public:
    //!构造函数，不用处理什么，相关的成员变量的析构都进行了处理
    execute() = delete;
    ~execute() = delete;
    execute(const execute &) = delete;
    execute& operator=(const execute&) = delete;

    //!链接MYSQL数据库
    static int connect(DBHANDLE *db_connect,
                       const char* host_name,
                       const char* user,
                       const char* pwd,
                       unsigned int port)
    {
        int ret = 0;

        //连接数据库
        if (db_connect->is_connected() == false)
        {
            //如果设置过HOST，用HOST NAME进行连接

            ret = db_connect->connect_by_host(host_name,
                                              user,
                                              pwd,
                                              nullptr,
                                              port);

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
    void disconnect(CONNECT *db_connect)
    {
        if (db_connect->is_connected() == true)
        {
            db_connect->disconnect();
        }
    }

    //!查询，非SELECT语句
    static int query(CONNECT *db_connect,
                     std::string_view sql,
                     uint64_t* num_affect,
                     uint64_t* insert_id)
    {
        //连接数据库
        if (db_connect->is_connected() == false)
        {
            return -1;
        }
        //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
        else
        {
            db_connect->ping();
        }

        ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sql.size(), sql.data());

        int ret = db_connect->query(*num_affect, *insert_id);
        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql);
            return -1;
        }

        //成功
        return 0;
    }

    //!查询，SELECT语句
    static int query(CONNECT* db_connect,
                     std::string_view sql,
                     uint64_t* num_affect,
                     RESULT* db_result)
    {
        int ret = 0;
        //连接数据库
        if (db_connect->is_connected() == false)
        {
            return -1;
        }
        //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
        else
        {
            db_connect->ping();
        }

        ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql.size(), sql.data());
        ret = db_connect->query(*num_affect, *db_result);
        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql);
            return -1;
        }

        //成功
        return 0;
    }

    //!查询,SELECT语句，用USE result的方式进行查询
    static int query(CONNECT* db_connect,
                     std::string_view sql,
                     RESULT* db_result)
    {
        int ret = 0;
        //连接数据库
        if (db_connect->is_connected() == false)
        {
            return -1;
        }
        //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
        else
        {
            db_connect->ping();
        }

        ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql.size(), sql.data());
        ret = db_connect->query(*db_result);

        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                    db_connect->error_no(),
                    db_connect->error_message(),
                    sql);
            return -1;
        }
        //成功
        return 0;
    }
};
}

#if defined ZCE_USE_MYSQL

#include "zce/db/mysql/connect.h"

namespace zce::mysql
{
//typedef zce::db::execute<zce::mysql::connect,
//> mysql_exe;
}
#endif