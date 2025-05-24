#include "zce/predefine.h"

//对于SQLITE的最低版本限制
#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

#include "zce/logger/logging.h"
#include "zce/db/sqlite/connect.h"
#include "zce/db/sqlite/command.h"
#include "zce/db/sqlite/result.h"

/*
* command
*/

namespace zce::sqlite
{
///****************************************************************************************
/// class bind_data
///****************************************************************************************
//构造函数,从很细小的地方就可以看出SQLITE的设计有不足，一个INDEX从1开始，1个从0
command::command(connect* sqlite3_handler) :
    sqlite3_(sqlite3_handler->get_handler()),
    statement_(nullptr),
    current_bind_(1),
    current_col_(0)
{
    //检查连接是否正常
    ZCE_ASSERT(sqlite3_handler != nullptr);
    assert(sqlite3_handler != nullptr &&
           sqlite3_handler->get_handler() != nullptr);
}

command::~command()
{
    terminate();
}

//销毁SQLITE3的STMT HANDLER，恢复初始化值等。
int command::terminate()
{
    //销毁SQLITE3的STMT HANDLER
    int ret = ::sqlite3_finalize(statement_);
    if (SQLITE_OK != ret)
    {
        return -1;
    }

    statement_ = nullptr;
    current_bind_ = 1;
    current_col_ = 0;
    return 0;
}

int command::bind(size_t id, const bool& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 val ? 1 : 0);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const char& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const short& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const int& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const long& val)
{
    int ret = ::sqlite3_bind_int64(statement_, (int)(id + 1), static_cast<int64_t>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const long long& val)
{
    int ret = ::sqlite3_bind_int64(statement_,
                                   (int)(id + 1),
                                   val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const unsigned char& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const unsigned short& val)
{
    int ret = ::sqlite3_bind_int(statement_,
                                 (int)(id + 1),
                                 static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return ret;
}

int command::bind(size_t id, const unsigned int& val)
{
    int ret = sqlite3_bind_int(statement_,
                               (int)(id + 1),
                               val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return 0;
}

int command::bind(size_t id, const unsigned long& val)
{
    int ret = ::sqlite3_bind_int64(statement_,
                                   (int)(id + 1),
                                   static_cast<uint64_t>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return 0;
}

int command::bind(size_t id, const unsigned long long& val)
{
    int ret = ::sqlite3_bind_int64(statement_,
                                   (int)(id + 1),
                                   val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return 0;
}

int command::bind(size_t id, const float& val)
{
    int ret = ::sqlite3_bind_double(statement_,
                                    (int)(id + 1),
                                    static_cast<double>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return 0;
}

int command::bind(size_t id, const double& val)
{
    int ret = ::sqlite3_bind_double(statement_,
                                    (int)(id + 1),
                                    val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const char* val)
{
    //从参数上看，SQLite的STMT不是bind变量，而是取了数据
    int ret = ::sqlite3_bind_text(statement_,
                                  (int)(id + 1),
                                  val,
                                  static_cast<int>(strlen(val)),
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }

    return 0;
}

int command::bind(size_t id, const  std::string& val)
{
    //
    int ret = ::sqlite3_bind_text(statement_,
                                  (int)(id + 1),
                                  val.c_str(),
                                  static_cast<int>(val.length()),
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const  std::string_view& val)
{
    int ret = ::sqlite3_bind_text(statement_,
                                  (int)(id + 1),
                                  val.data(),
                                  static_cast<int>(val.length()),
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

int command::bind(size_t id, const zce::string_buf& val)
{
    //SQLITE_TRANSIENT 是要求SQLite对数据进行复制处理
    //SQLITE_STATIC 是告诉SQLite，数据我管理
    int ret = ::sqlite3_bind_blob(statement_,
                                  (int)(id + 1),
                                  val.data(),
                                  (int)val.size(),
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_blob error :[%d][%s]",
                error_no(),
                error_message());
        return ret;
    }
    return 0;
}

//
int command::reset_stmt()
{
    int ret = ::sqlite3_reset(statement_);
    if (SQLITE_OK != ret)
    {
        return -1;
    }

    current_bind_ = 1;
    current_col_ = 0;
    return 0;
}

//! 执行SQL语句，什么都不管的那种，DDL
int command::execute(std::string_view sqlcmd)
{
    int ret = 0;
    char* err_msg = nullptr;
    ret = ::sqlite3_exec(sqlite3_,
                         sqlcmd.data(),
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
                sqlcmd.data(),
                ret,
                err_msg);
        ::sqlite3_free(err_msg);
        return -1;
    }
}

//! 执行DML SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
int command::execute(std::string_view sqlcmd,
                     size_t& num_affect,
                     uint64_t* last_id)
{
    int ret = execute(sqlcmd);
    if (ret != SQLITE_OK)
    {
        return ret;
    }
    num_affect = sqlite3_changes(sqlite3_);
    if (last_id)
    {
        *last_id = ::sqlite3_last_insert_rowid(sqlite3_);
    }
    return 0;
}

//! 执行SQL语句,SELECT语句,转储结果集合的那种,
int command::execute(std::string_view sqlcmd,
                     size_t& num_affect,
                     zce::sqlite::result& sqlite_res)
{
    int ret = execute(sqlcmd);
    if (ret != SQLITE_OK)
    {
        return ret;
    }
    zce::sqlite::result result;
    char* err_msg = nullptr;
    ret = ::sqlite3_get_table(sqlite3_,
                              sqlcmd.data(),
                              &(result.sq_result_),
                              &(result.num_result_row_),
                              &(result.num_result_column_),
                              &(err_msg));
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_get_table exe fail:[%d][%s]",
                ret,
                err_msg);
        ::sqlite3_free(err_msg);
        return -1;
    }
    sqlite_res = std::move(result);
    num_affect = result.num_result_row_;
    return 0;
}

//开始一个事务
int command::trans_begin()
{
    return execute("BEGIN;");
}

//提交一个事务
int command::trans_commit()
{
    return execute("COMMIT;");
}

//回滚一个事务
int command::trans_rollback()
{
    return execute("ROLLBACK;");
}

//将同步选项关闭，可以适当的提高insert的速度，但是为了安全起见，建议不要使用
int command::turn_off_synch()
{
    return execute("PRAGMA synchronous=OFF;");
}

//分析SQL语句，检查是否能够正确执行
int command::stmt_prepare(std::string_view sqlcmd)
{
    if (statement_)
    {
        terminate();
    }

    int ret = ::sqlite3_prepare_v2(sqlite3_,
                                   sqlcmd.data(),
                                   -1,                                      //注意这个参数，必须小于0
                                   &statement_,
                                   nullptr);
    //如果分析结果错误，或者不是一个SQL
    if (SQLITE_OK != ret || statement_ == nullptr)
    {
        //其他返回错误
        ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]",
                error_no(),
                error_message());
        return -1;
    }
    num_bind_ = ::sqlite3_bind_parameter_count(statement_);
    //
    current_bind_ = 1;
    return 0;
}

int command::stmt_execute(size_t& num_affect,
                          uint64_t* last_id)
{
    //执行SQL
    bool has_result = false;
    int ret = stmt_execute(has_result);
    if (ret != 0)
    {
        return ret;
    }
    num_affect = sqlite3_changes(sqlite3_);
    if (last_id)
    {
        *last_id = ::sqlite3_last_insert_rowid(sqlite3_);
    }
    return 0;
}

int command::stmt_execute(size_t& num_affect,
                          zce::sqlite::stmt_result& sq_stmt_res)
{   //执行SQL
    bool has_result = false;
    int ret = stmt_execute(has_result);
    if (ret != 0)
    {
        return ret;
    }
    num_affect = sqlite3_changes(sqlite3_);
    if (has_result)
    {
        sq_stmt_res = statement_;
    }
    else
    {
    }
    return 0;
}

//执行一次stmt SQL，如果执行成功，返回0，如果SQL有结果返回，has_result置为true
int command::stmt_execute(bool& has_result)
{
    has_result = false;
    //
    int ret = ::sqlite3_step(statement_);

    //执行成功，而且有结果返回
    if (SQLITE_ROW == ret)
    {
        has_result = true;
        current_col_ = 0;
        return 0;
    }
    //执行成功，但没有结果
    else if (SQLITE_DONE == ret)
    {
        has_result = false;
        return 0;
    }
    else
    {
        //其他返回错误
        ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]",
                error_no(),
                error_message());
        return -1;
    }
}

//command& command::operator << (const command::BLOB_bind& val)
//{
//    bind<const command::BLOB_bind&>(current_bind_, val);
//    ++current_bind_;
//    return *this;
//}
//command& command::operator << (const std::string& val)
//{
//    bind<const std::string&>(current_bind_, val);
//    ++current_bind_;
//    return *this;
//}
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
