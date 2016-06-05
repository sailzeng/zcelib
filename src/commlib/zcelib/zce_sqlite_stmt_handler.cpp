
#include "zce_predefine.h"

//对于SQLITE的最低版本限制
#if SQLITE_VERSION_NUMBER >= 3005000

#include "zce_trace_debugging.h"
#include "zce_sqlite_db_handler.h"
#include "zce_sqlite_stmt_handler.h"



/******************************************************************************************
SQLite3_STMT_Handler
******************************************************************************************/
//构造函数,从很细小的地方就可以看出SQLITE的设计有不足，一个INDEX从1开始，1个从0
ZCE_SQLite_STMTHdl::ZCE_SQLite_STMTHdl(ZCE_SQLite_DB_Handler *sqlite3_handler):
    sqlite_handler_(sqlite3_handler),
    sqlite3_stmt_handler_(NULL),
    current_col_(0),
    current_bind_(1)
{
    assert(sqlite3_handler != NULL && sqlite3_handler->get_sqlite_handler() != NULL);
}

ZCE_SQLite_STMTHdl::~ZCE_SQLite_STMTHdl()
{
    finalize();
}


//销毁SQLITE3的STMT HANDLER，恢复初始化值等。
int ZCE_SQLite_STMTHdl::finalize()
{
    //销毁SQLITE3的STMT HANDLER
    int ret =  ::sqlite3_finalize(sqlite3_stmt_handler_);

    if ( SQLITE_OK  != ret)
    {
        return -1;
    }

    sqlite3_stmt_handler_ = NULL;
    current_bind_ = 1;
    current_col_ = 0;
    return 0;
}

//
int ZCE_SQLite_STMTHdl::reset()
{
    int ret = ::sqlite3_reset(sqlite3_stmt_handler_);

    if ( SQLITE_OK  != ret)
    {
        return -1;
    }

    current_bind_ = 1;
    current_col_ = 0;

    return 0;
}


//分析SQL语句，检查是否能够正确执行
int ZCE_SQLite_STMTHdl::prepare(const char *sql_string)
{
    if (sqlite3_stmt_handler_)
    {
        finalize();
    }

    int ret =  ::sqlite3_prepare_v2(sqlite_handler_->get_sqlite_handler(),
                                    sql_string,
                                    -1,                                      //注意这个参数，必须小于0
                                    &sqlite3_stmt_handler_,
                                    NULL);

    //如果分析结果错误，或者不是一个SQL
    if ( SQLITE_OK  != ret || sqlite3_stmt_handler_ == NULL)
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

//
int ZCE_SQLite_STMTHdl::execute_stmt_sql(bool &has_reuslt)
{
    has_reuslt = false;
    //
    int ret = ::sqlite3_step(sqlite3_stmt_handler_);

    //
    if (SQLITE_ROW == ret)
    {
        has_reuslt = true;
        current_col_ = 0;
        return 0;
    }
    //
    else if (SQLITE_DONE == ret)
    {
        has_reuslt = false;
        return 0;
    }

    //其他返回错误
    ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]", error_code(), error_message());
    return -1;
}

//Bind 函数群
template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, char val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_,
                                 bind_index,
                                 static_cast<int>( val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }
    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, short val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, static_cast<int>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, int val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, val);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, long val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, static_cast<int>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, long long val)
{
    int ret = ::sqlite3_bind_int64(sqlite3_stmt_handler_, bind_index, val);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, unsigned char val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, static_cast<int>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]",
                error_code(),
                error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, unsigned short val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, static_cast<int>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return ret;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, unsigned int val)
{
    int ret = sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, val);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

//注意long这儿bind的是32位的喔
template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, unsigned long val)
{
    int ret = ::sqlite3_bind_int(sqlite3_stmt_handler_, bind_index, static_cast<int>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, unsigned long long val)
{
    int ret = ::sqlite3_bind_int64(sqlite3_stmt_handler_, bind_index, val);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_int64 error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, float val)
{
    int ret = ::sqlite3_bind_double(sqlite3_stmt_handler_, bind_index, static_cast<double>(val));
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, double val)
{

    int ret = ::sqlite3_bind_double(sqlite3_stmt_handler_,
                                    bind_index,
                                    val);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_double error :[%d][%s]", error_code(), error_message());
        return ret;
    }
    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, const char *val)
{
    //从参数上看，SQLite的STMT不是bind变量，而是取了数据
    int ret = ::sqlite3_bind_text(sqlite3_stmt_handler_,
                                  bind_index,
                                  val,
                                  static_cast<int>(strlen(val)),
                                  SQLITE_TRANSIENT);
    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, const  std::string &val)
{
    //
    int ret = ::sqlite3_bind_text(sqlite3_stmt_handler_,
                                  bind_index,
                                  val.c_str(),
                                  static_cast<int>(val.length()),
                                  SQLITE_TRANSIENT);

    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_text error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}

template<>
int ZCE_SQLite_STMTHdl::bind(int bind_index, const ZCE_SQLite_STMTHdl::BIN_Param &val)
{
    int ret = ::sqlite3_bind_blob(sqlite3_stmt_handler_,
                                  bind_index,
                                  val.binary_data_,
                                  val.binary_len_,
                                  SQLITE_TRANSIENT);

    if ( SQLITE_OK  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_bind_blob error :[%d][%s]", error_code(), error_message());
        return ret;
    }

    return 0;
}



ZCE_SQLite_STMTHdl &ZCE_SQLite_STMTHdl::operator << (const ZCE_SQLite_STMTHdl::BIN_Param &val)
{
    bind<const ZCE_SQLite_STMTHdl::BIN_Param &>(current_bind_, val);
    ++current_bind_;
    return *this;
}
ZCE_SQLite_STMTHdl &ZCE_SQLite_STMTHdl::operator << (const std::string &val)
{
    bind<const std::string &>(current_bind_, val);
    ++current_bind_;
    return *this;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, char &val)
{
    val = static_cast<char>(::sqlite3_column_int(sqlite3_stmt_handler_, result_col));
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, short &val)
{
    val = static_cast<short>(::sqlite3_column_int(sqlite3_stmt_handler_,
                                                  result_col));

    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, int &val)
{
    val = ::sqlite3_column_int(sqlite3_stmt_handler_,
                               result_col);
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, long &val)
{
    val = ::sqlite3_column_int(sqlite3_stmt_handler_,
                               result_col);
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, long long &val)
{
    val = ::sqlite3_column_int64(sqlite3_stmt_handler_,
                                 result_col);
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, unsigned char &val)
{
    val = static_cast<unsigned char>( ::sqlite3_column_int(sqlite3_stmt_handler_,
                                                           result_col));

    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, unsigned short &val)
{
    val = static_cast<unsigned short>(::sqlite3_column_int(sqlite3_stmt_handler_,
                                                           result_col));
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, unsigned int &val)
{
    val = static_cast<unsigned int>(  sqlite3_column_int(sqlite3_stmt_handler_,
                                                         result_col));
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, unsigned long &val)
{
    val  = static_cast<unsigned long>(  sqlite3_column_int(sqlite3_stmt_handler_,
                                                           result_col));


    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, unsigned long long &val)
{
    val = static_cast<unsigned long long > (sqlite3_column_int64(sqlite3_stmt_handler_,
                                                                 result_col));


    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, float &val)
{
    val = static_cast<float > (sqlite3_column_double(sqlite3_stmt_handler_,
                                                     result_col));


    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, double &val)
{
    val = sqlite3_column_double(sqlite3_stmt_handler_,
                                result_col);


    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, char *val)
{
    //Fisk这个变态让我改了地方，为了安全检查。
    strncpy(val,
            reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt_handler_,
                                                               result_col)),
            sqlite3_column_bytes(sqlite3_stmt_handler_, result_col) + 1);

    return;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
template<>
void ZCE_SQLite_STMTHdl::column(int result_col, ZCE_SQLite_STMTHdl::BIN_Result &val)
{
    *val.binary_len_ = ::sqlite3_column_bytes(sqlite3_stmt_handler_, result_col);
    //为了获取二进制数据，与ZCE_Mysql_Result相对应,长度不+1
    memcpy(val.binary_data_, ::sqlite3_column_blob(sqlite3_stmt_handler_, result_col),
           *val.binary_len_);
    return;
}

template<>
void ZCE_SQLite_STMTHdl::column(int result_col, std::string &val)
{
    val = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt_handler_,
                                                             result_col));
    return;
}

//开始一个事务
int ZCE_SQLite_STMTHdl::begin_transaction()
{
    return sqlite_handler_->begin_transaction();
}

//提交一个事务
int ZCE_SQLite_STMTHdl::commit_transction()
{
    return sqlite_handler_->commit_transction();
}

int ZCE_SQLite_STMTHdl::turn_off_synch()
{
    return sqlite_handler_->turn_off_synch();
}

#endif //#if SQLITE_VERSION_NUMBER >= 3005000

