
#include "zce_predefine.h"
#include "zce_sqlite3_process.h"

//对于SQLITE的最低版本限制
#if SQLITE_VERSION_NUMBER >= 3003000

/******************************************************************************************
SQLite3_DB_Handler SQLite3DB Handler 连接处理一个SQLite3数据库的Handler
******************************************************************************************/
SQLite3_DB_Handler::SQLite3_DB_Handler():
    sqlite3_handler_(NULL)
{
}

SQLite3_DB_Handler::~SQLite3_DB_Handler()
{
    close_database();
}

//const char* db_file ,数据库名称文件路径,接口要求UTF8编码，
//int == 0表示成功，否则失败
int SQLite3_DB_Handler::open_database(const char *db_file, bool string_ansi = true)
{
    //这段代码是客户端兄弟们加的。
#ifdef ZCE_OS_WINDOWS
    //////////////////////////////////////////////////////////////////////////
    // Begin(把一个ascii码字符转换成UTF-8)

    DWORD utf16_buffer_len = MultiByteToWideChar( CP_ACP, 0, db_file, -1, NULL, 0 );
    wchar_t *utf16_buffer = new wchar_t[utf16_buffer_len];

    // 第一次先把ascii码转换成UTF-16
    MultiByteToWideChar( CP_ACP, 0, db_file, -1, utf16_buffer, utf16_buffer_len );

    DWORD utf8_buffer_len = WideCharToMultiByte( CP_UTF8, NULL, utf16_buffer, -1, NULL, 0, NULL, FALSE );
    char *utf8_buffer = new char[utf8_buffer_len];

    // 第二次再把UTF-16编码转换为UTF-8编码
    WideCharToMultiByte( CP_UTF8, NULL, utf16_buffer, -1, utf8_buffer, utf8_buffer_len, NULL, 0 );

    // End(把一个ascii码字符转换成UTF-8)
    //////////////////////////////////////////////////////////////////////////

    int ret = sqlite3_open(utf8_buffer, &sqlite3_handler_);

    // 需要释放申请的内存
    delete[] utf16_buffer;
    delete[] utf8_buffer;

    if (ret != SQLITE_OK )
    {
        return -1;
    }

    return 0;

#else

    int ret = sqlite3_open(db_file, &sqlite3_handler_);

    //
    if (ret != SQLITE_OK )
    {
        return -1;
    }

    return 0;
#endif //
}

//这个特性要3.5以后的版本才可以用
#if SQLITE_VERSION_NUMBER >= 3005000

//以只读的方式打开一个数据库
//这个特性要3.5以后的版本才可以用。
//数据库文件路径,在win32平台对传入参数进行UTF-8转码，
int SQLite3_DB_Handler::open_readonly_db(const char *db_file)
{
    //这段代码是客户端兄弟们加的。
#ifdef WIN32
    //////////////////////////////////////////////////////////////////////////
    // Begin(把一个ascii码字符转换成UTF-8)

    DWORD utf16_buffer_len = MultiByteToWideChar( CP_ACP, 0, db_file, -1, NULL, 0 );
    wchar_t *utf16_buffer = new wchar_t[utf16_buffer_len];

    // 第一次先把ascii码转换成UTF-16
    MultiByteToWideChar( CP_ACP, 0, db_file, -1, utf16_buffer, utf16_buffer_len );

    DWORD utf8_buffer_len = WideCharToMultiByte( CP_UTF8, NULL, utf16_buffer, -1, NULL, 0, NULL, FALSE );
    char *utf8_buffer = new char[utf8_buffer_len];

    // 第二次再把UTF-16编码转换为UTF-8编码
    WideCharToMultiByte( CP_UTF8, NULL, utf16_buffer, -1, utf8_buffer, utf8_buffer_len, NULL, 0 );

    // End(把一个ascii码字符转换成UTF-8)
    //////////////////////////////////////////////////////////////////////////

    int ret = sqlite3_open_v2(utf8_buffer, &sqlite3_handler_, SQLITE_OPEN_READONLY, NULL);

    // 需要释放申请的内存
    delete[] utf16_buffer;
    delete[] utf8_buffer;

    if (ret != SQLITE_OK )
    {
        return -1;
    }

    return 0;
#else
    int ret = sqlite3_open_v2(db_file, &sqlite3_handler_, SQLITE_OPEN_READONLY, NULL);

    //
    if (ret != SQLITE_OK )
    {
        return -1;
    }

    return 0;
#endif
}

#endif //#if SQLITE_VERSION_NUMBER >= 3005000

//关闭数据库。
void SQLite3_DB_Handler::close_database()
{
    if (sqlite3_handler_)
    {
        sqlite3_close(sqlite3_handler_);
        sqlite3_handler_ = NULL;
    }
}

//错误语句Str
const char *SQLite3_DB_Handler::get_dbret_errstr()
{
    return sqlite3_errmsg(sqlite3_handler_);
}

//DB返回的错误ID
unsigned int SQLite3_DB_Handler::get_dbret_errid()
{
    return sqlite3_errcode(sqlite3_handler_);
}

//开始一个事务
int SQLite3_DB_Handler::begin_transaction()
{
    int ret = 0;
    char *err_msg = NULL;
    ret = sqlite3_exec(sqlite3_handler_,
                       "BEGIN TRANSACTION;",
                       NULL,
                       NULL,
                       &err_msg);

    if (ret == SQLITE_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//提交一个事务
int SQLite3_DB_Handler::commit_transction()
{
    int ret = 0;
    char *err_msg = NULL;
    ret = sqlite3_exec(sqlite3_handler_,
                       "COMMIT TRANSACTION;",
                       NULL,
                       NULL,
                       &err_msg);

    if (ret == SQLITE_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//将同步选项关闭，可以适当的提高insert的速度，但是为了安全起见，建议不要使用
int SQLite3_DB_Handler::turn_off_synch()
{
    int ret = 0;
    char *err_msg = NULL;
    ret = sqlite3_exec(sqlite3_handler_,
                       "PRAGMA synchronous=OFF;",
                       NULL,
                       NULL,
                       &err_msg);

    if (ret == SQLITE_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

#endif //#if SQLITE_VERSION_NUMBER >= 3003000

