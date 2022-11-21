#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/sqlite/sqlite_hdl.h"
#include "zce/sqlite/sqlite_result.h"

//对于SQLITE的最低版本限制
#if SQLITE_VERSION_NUMBER >= 3005000

namespace zce
{
/******************************************************************************************
SQLite3_DB_Handler SQLite3DB Handler 连接处理一个SQLite3数据库的Handler
******************************************************************************************/
sqlite_handle::sqlite_handle() :
    sqlite3_handler_(nullptr)
{
}

sqlite_handle::~sqlite_handle()
{
    close_db();
}

//const char* db_file ,数据库名称文件路径,接口要求UTF8编码，
//int == 0表示成功，否则失败
int sqlite_handle::open_db(const char* db_file,
                           bool read_only,
                           bool create_db)
{
    int flags = SQLITE_OPEN_READWRITE;
    if (create_db)
    {
        flags |= SQLITE_OPEN_CREATE;
        //不能同时存在
        ZCE_ASSERT(read_only == false);
    }

    if (read_only)
    {
        flags = SQLITE_OPEN_READONLY;
    }

    int ret = ::sqlite3_open_v2(db_file,
                                &sqlite3_handler_,
                                flags,
                                nullptr);
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_open_v2 open db [%s] fail:[%d][%s]",
                db_file,
                error_code(),
                error_message());
        return -1;
    }

    return 0;
}

//关闭数据库。
void sqlite_handle::close_db()
{
    if (sqlite3_handler_)
    {
        ::sqlite3_close_v2(sqlite3_handler_);
        sqlite3_handler_ = nullptr;
    }
}

//错误语句Str
const char* sqlite_handle::error_message()
{
    return ::sqlite3_errmsg(sqlite3_handler_);
}

//DB返回的错误ID
int sqlite_handle::error_code()
{
    return ::sqlite3_errcode(sqlite3_handler_);
}

//开始一个事务
int sqlite_handle::begin_transaction()
{
    return exe("BEGIN TRANSACTION;");
}

//提交一个事务
int sqlite_handle::commit_transction()
{
    return exe("COMMIT TRANSACTION;");
}

//将同步选项关闭，可以适当的提高insert的速度，但是为了安全起见，建议不要使用
int sqlite_handle::turn_off_synch()
{
    return exe("PRAGMA synchronous=OFF;");
}

//!执行DDL等不需要结果的SQL
int sqlite_handle::exe(const char* sql_string)
{
    int ret = 0;
    char* err_msg = nullptr;
    ret = ::sqlite3_exec(sqlite3_handler_,
                         sql_string,
                         nullptr,
                         nullptr,
                         &err_msg);
    if (ret == SQLITE_OK)
    {
        return 0;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_exec exe sql [%s] fail.:[%d][%s].",
                sql_string,
                ret,
                err_msg);
        ::sqlite3_free(err_msg);
        return -1;
    }
}

//执行SQL 查询，取得结果
int sqlite_handle::get_table(const char* sql_string,
                             zce::sqlite_result* result)
{
    int ret = SQLITE_OK;
    char* err_msg = nullptr;
    ret = ::sqlite3_get_table(sqlite3_handler_, sql_string,
                              &(result->result_),
                              &(result->row_),
                              &(result->column_),
                              &(err_msg));
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_get_table exe fail:[%d][%s]",
                ret,
                err_msg);
        ::sqlite3_free(err_msg);
        return -1;
    }
    return 0;
}
}

#endif //#if SQLITE_VERSION_NUMBER >= 3005000