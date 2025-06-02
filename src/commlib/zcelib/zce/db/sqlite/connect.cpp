#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/string/url.h"
#include "zce/db/sqlite/connect.h"

//对于SQLITE的最低版本限制
#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

namespace zce::sqlite
{
/******************************************************************************************
connect SQLite3DB Handler 连接处理一个SQLite3数据库的Handler
******************************************************************************************/
connect::connect() :
    sqlite3_(nullptr)
{
}

connect::~connect()
{
    disconnect();
}

//const char* db_file ,数据库名称文件路径,接口要求UTF8编码，
//int == 0表示成功，否则失败
int connect::connect_db(const char* db_file,
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
                                &sqlite3_,
                                flags,
                                nullptr);
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_open_v2 open db [%s] fail:[%d][%s]",
                db_file,
                error_no(),
                error_message());
        return -1;
    }

    return 0;
}

int connect::connect_url(const char* db_url)
{
    zce::url url_obj;
    int ret = url_obj.regex_urlstr(db_url,true);
    if (ret != 0)
    {
        return ret;
    }
    if (url_obj.scheme() != "sqlite")
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3 connect_url db url [%s] scheme is not sqlite",
                db_url);
        return -1;
	}
    bool read_only = url_obj.query_params().get_value<bool>("READONLY");
    bool create_db = url_obj.query_params().get_value<bool>("CREATEDB");
    std::string db_file = "./" + url_obj.path();
    return connect_db(db_file.c_str(), read_only, create_db);
}

//关闭数据库。
void connect::disconnect()
{
    if (sqlite3_)
    {
        ::sqlite3_close_v2(sqlite3_);
        sqlite3_ = nullptr;
    }
}

//错误语句Str
const char* connect::error_message()
{
    return ::sqlite3_errmsg(sqlite3_);
}

//DB返回的错误ID
int connect::error_no()
{
    return ::sqlite3_errcode(sqlite3_);
}
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
