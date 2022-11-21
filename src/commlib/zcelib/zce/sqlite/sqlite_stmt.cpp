#include "zce/predefine.h"

//对于SQLITE的最低版本限制
#if SQLITE_VERSION_NUMBER >= 3005000

#include "zce/logger/logging.h"
#include "zce/sqlite/sqlite_hdl.h"
#include "zce/sqlite/sqlite_stmt.h"

/*
* sqlite_stmt
*/

namespace zce
{
//构造函数,从很细小的地方就可以看出SQLITE的设计有不足，一个INDEX从1开始，1个从0
sqlite_stmt::sqlite_stmt(sqlite_handle* sqlite3_handler) :
    sqlite_hdl_(sqlite3_handler)
{
    assert(sqlite3_handler != nullptr &&
           sqlite3_handler->get_sqlite_handler() != nullptr);
}

sqlite_stmt::~sqlite_stmt()
{
    terminate();
}

//销毁SQLITE3的STMT HANDLER，恢复初始化值等。
int sqlite_stmt::terminate()
{
    //销毁SQLITE3的STMT HANDLER
    int ret = ::sqlite3_finalize(prepared_statement_);
    if (SQLITE_OK != ret)
    {
        return -1;
    }

    prepared_statement_ = nullptr;
    current_bind_ = 1;
    current_col_ = 0;
    return 0;
}

//
int sqlite_stmt::reset()
{
    int ret = ::sqlite3_reset(prepared_statement_);
    if (SQLITE_OK != ret)
    {
        return -1;
    }

    current_bind_ = 1;
    current_col_ = 0;
    return 0;
}

//分析SQL语句，检查是否能够正确执行
int sqlite_stmt::prepare(const char* sql_string)
{
    if (prepared_statement_)
    {
        terminate();
    }

    int ret = ::sqlite3_prepare_v2(sqlite_hdl_->get_sqlite_handler(),
                                   sql_string,
                                   -1,                                      //注意这个参数，必须小于0
                                   &prepared_statement_,
                                   nullptr);
    //如果分析结果错误，或者不是一个SQL
    if (SQLITE_OK != ret || prepared_statement_ == nullptr)
    {
        //其他返回错误
        ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]",
                error_code(),
                error_message());
        return -1;
    }

    //
    current_bind_ = 1;
    return 0;
}

//执行一次stmt SQL，如果执行成功，返回0，如果SQL有结果返回，has_result置为true
int sqlite_stmt::step(bool& has_result)
{
    has_result = false;
    //
    int ret = ::sqlite3_step(prepared_statement_);

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

    //其他返回错误
    ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]", error_code(), error_message());
    return -1;
}

//Bind 函数群
template<>
int sqlite_stmt::bind(int bind_index, char val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_,
                                 bind_index,
                                 static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }
    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, short val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, int val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, long val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, long long val)
{
    int ret = ::sqlite3_bind_int64(prepared_statement_, bind_index, val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, unsigned char val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, unsigned short val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return ret;
}

template<>
int sqlite_stmt::bind(int bind_index, unsigned int val)
{
    int ret = sqlite3_bind_int(prepared_statement_, bind_index, val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

//注意long这儿bind的是32位的喔
template<>
int sqlite_stmt::bind(int bind_index, unsigned long val)
{
    int ret = ::sqlite3_bind_int(prepared_statement_, bind_index, static_cast<int>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, unsigned long long val)
{
    int ret = ::sqlite3_bind_int64(prepared_statement_, bind_index, val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, float val)
{
    int ret = ::sqlite3_bind_double(prepared_statement_, bind_index, static_cast<double>(val));
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, double val)
{
    int ret = ::sqlite3_bind_double(prepared_statement_,
                                    bind_index,
                                    val);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]", error_code(), error_message());
        return ret;
    }
    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, const char* val)
{
    //从参数上看，SQLite的STMT不是bind变量，而是取了数据
    int ret = ::sqlite3_bind_text(prepared_statement_,
                                  bind_index,
                                  val,
                                  static_cast<int>(strlen(val)),
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, const  std::string& val)
{
    //
    int ret = ::sqlite3_bind_text(prepared_statement_,
                                  bind_index,
                                  val.c_str(),
                                  static_cast<int>(val.length()),
                                  SQLITE_TRANSIENT);

    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int sqlite_stmt::bind(int bind_index, const sqlite_stmt::BLOB_bind& val)
{
    //SQLITE_TRANSIENT 是要求SQLite对数据进行复制处理
    //SQLITE_STATIC 是告诉SQLite，数据我管理
    int ret = ::sqlite3_bind_blob(prepared_statement_,
                                  bind_index,
                                  val.bind_data_,
                                  val.bind_size_,
                                  SQLITE_TRANSIENT);
    if (SQLITE_OK != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_blob error :[%d][%s]", error_code(), error_message());
        return ret;
    }
    return 0;
}

sqlite_stmt& sqlite_stmt::operator << (const sqlite_stmt::BLOB_bind& val)
{
    bind<const sqlite_stmt::BLOB_bind&>(current_bind_, val);
    ++current_bind_;
    return *this;
}
sqlite_stmt& sqlite_stmt::operator << (const std::string& val)
{
    bind<const std::string&>(current_bind_, val);
    ++current_bind_;
    return *this;
}

template<>
void sqlite_stmt::column(int result_col, char& val)
{
    val = static_cast<char>(::sqlite3_column_int(prepared_statement_, result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, short& val)
{
    val = static_cast<short>(::sqlite3_column_int(prepared_statement_,
                             result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, int& val)
{
    val = ::sqlite3_column_int(prepared_statement_,
                               result_col);
    return;
}

template<>
void sqlite_stmt::column(int result_col, long& val)
{
    val = ::sqlite3_column_int(prepared_statement_,
                               result_col);
    return;
}

template<>
void sqlite_stmt::column(int result_col, long long& val)
{
    val = ::sqlite3_column_int64(prepared_statement_,
                                 result_col);
    return;
}

template<>
void sqlite_stmt::column(int result_col, unsigned char& val)
{
    val = static_cast<unsigned char>(::sqlite3_column_int(prepared_statement_,
                                     result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, unsigned short& val)
{
    val = static_cast<unsigned short>(::sqlite3_column_int(prepared_statement_,
                                      result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, unsigned int& val)
{
    val = static_cast<unsigned int>(sqlite3_column_int(prepared_statement_,
                                    result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, unsigned long& val)
{
    val = static_cast<unsigned long>(sqlite3_column_int(prepared_statement_,
                                     result_col));

    return;
}

template<>
void sqlite_stmt::column(int result_col, unsigned long long& val)
{
    val = static_cast<unsigned long long> (sqlite3_column_int64(prepared_statement_,
                                           result_col));
    return;
}

template<>
void sqlite_stmt::column(int result_col, float& val)
{
    val = static_cast<float> (sqlite3_column_double(prepared_statement_,
                              result_col));

    return;
}

template<>
void sqlite_stmt::column(int result_col, double& val)
{
    val = sqlite3_column_double(prepared_statement_,
                                result_col);
    return;
}

template<>
void sqlite_stmt::column(int result_col, char* val)
{
    //Fisk这个变态让我改了地方，为了安全检查。
    strncpy(val,
            reinterpret_cast<const char*>(sqlite3_column_text(prepared_statement_,
            result_col)),
            static_cast<size_t>(sqlite3_column_bytes(prepared_statement_, result_col)));
    return;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
template<>
void sqlite_stmt::column(int result_col, sqlite_stmt::BLOB_column& val)
{
    *val.binary_len_ = ::sqlite3_column_bytes(prepared_statement_, result_col);
    //为了获取二进制数据，与zce::mysql::Result相对应,长度不+1
    memcpy(val.binary_data_, ::sqlite3_column_blob(prepared_statement_, result_col),
           *val.binary_len_);
    return;
}

template<>
void sqlite_stmt::column(int result_col, std::string& val)
{
    val.assign(reinterpret_cast<const char*>(sqlite3_column_text(prepared_statement_,
               result_col)),
               ::sqlite3_column_bytes(prepared_statement_, result_col));
    return;
}
}

#endif //#if SQLITE_VERSION_NUMBER >= 3005000