
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_sqlite_db_handler.h"

//����SQLITE����Ͱ汾����
#if SQLITE_VERSION_NUMBER >= 3005000

//=========================================================================================

/******************************************************************************************
SQLite3_DB_Handler SQLite3DB Handler ���Ӵ���һ��SQLite3���ݿ��Handler
******************************************************************************************/
ZCE_SQLite_DB_Handler::ZCE_SQLite_DB_Handler():
    sqlite3_handler_(NULL)
{
}

ZCE_SQLite_DB_Handler::~ZCE_SQLite_DB_Handler()
{
    close_database();
}

//const char* db_file ,���ݿ������ļ�·��,�ӿ�Ҫ��UTF8���룬
//int == 0��ʾ�ɹ�������ʧ��
int ZCE_SQLite_DB_Handler::open_database(const char *db_file,
                                         bool read_only,
                                         bool create_db)
{
    int flags = SQLITE_OPEN_READWRITE;
    if (create_db)
    {
        flags |= SQLITE_OPEN_CREATE;
        //����ͬʱ����
        ZCE_ASSERT(read_only == false);
    }

    if (read_only)
    {
        flags = SQLITE_OPEN_READONLY;
    }

    int ret = ::sqlite3_open_v2(db_file,
                                &sqlite3_handler_,
                                flags,
                                NULL);
    if (ret != SQLITE_OK )
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_open_v2 open db [%s] fail:[%d][%s]",
                db_file,
                error_code(),
                error_message());
        return -1;
    }

    return 0;

}

#if defined ZCE_OS_WINDOWS

//��MBCS·���Ĵ�һ��DB�ļ�
int ZCE_SQLite_DB_Handler::open_mbcs_path_db(const char *db_file,
                                             bool read_only,
                                             bool create_db)
{

    // Begin(��һ��ascii���ַ�ת����UTF-8)

    DWORD utf16_buffer_len = ::MultiByteToWideChar(CP_ACP, 0, db_file, -1, NULL, 0);
    if (utf16_buffer_len > MAX_PATH)
    {
        return -1;
    }
    wchar_t utf16_buffer[MAX_PATH + 1];

    // ��һ���Ȱ�ascii��ת����UTF-16
    ::MultiByteToWideChar(CP_ACP, 0, db_file, -1, utf16_buffer, utf16_buffer_len);

    DWORD utf8_buffer_len = WideCharToMultiByte(CP_UTF8, NULL, utf16_buffer, -1, NULL, 0, NULL, FALSE);
    if (utf8_buffer_len > MAX_PATH)
    {
        return -1;
    }

    char utf8_buffer [MAX_PATH + 1];

    // �ڶ����ٰ�UTF-16����ת��ΪUTF-8����
    ::WideCharToMultiByte(CP_UTF8, NULL, utf16_buffer, -1, utf8_buffer, utf8_buffer_len, NULL, 0);

    // End(��һ��ascii���ַ�ת����UTF-8)


    int ret = open_database(utf8_buffer,
                            read_only,
                            create_db);
    //
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}


#endif


//�ر����ݿ⡣
void ZCE_SQLite_DB_Handler::close_database()
{
    if (sqlite3_handler_)
    {
        ::sqlite3_close_v2(sqlite3_handler_);
        sqlite3_handler_ = NULL;
    }
}

//�������Str
const char *ZCE_SQLite_DB_Handler::error_message()
{
    return ::sqlite3_errmsg(sqlite3_handler_);
}

//DB���صĴ���ID
int ZCE_SQLite_DB_Handler::error_code()
{
    return ::sqlite3_errcode(sqlite3_handler_);
}

//��ʼһ������
int ZCE_SQLite_DB_Handler::begin_transaction()
{
    return execute("BEGIN TRANSACTION;");
}

//�ύһ������
int ZCE_SQLite_DB_Handler::commit_transction()
{
    return execute("COMMIT TRANSACTION;");
}

//��ͬ��ѡ��رգ������ʵ������insert���ٶȣ�����Ϊ�˰�ȫ��������鲻Ҫʹ��
int ZCE_SQLite_DB_Handler::turn_off_synch()
{
    return execute("PRAGMA synchronous=OFF;");
}

//!ִ��DDL�Ȳ���Ҫ�����SQL
int ZCE_SQLite_DB_Handler::execute(const char *sql_string)
{
    int ret = 0;
    char *err_msg = NULL;
    ret = ::sqlite3_exec(sqlite3_handler_,
                         sql_string,
                         NULL,
                         NULL,
                         &err_msg);
    if (ret == SQLITE_OK)
    {
        return 0;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_exec execute sql [%s] fail.:[%d][%s].",
                sql_string,
                ret,
                err_msg);
        ::sqlite3_free(err_msg);
        return -1;
    }
}


//ִ��SQL ��ѯ��ȡ�ý��
int ZCE_SQLite_DB_Handler::get_table(const char *sql_string,
                                     ZCE_SQLite_Result *result)
{
    int ret = SQLITE_OK;
    ret = ::sqlite3_get_table(sqlite3_handler_, sql_string,
                              &(result->result_),
                              &(result->row_),
                              &(result->column_),
                              &(result->err_msg_));
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_get_table execute fail:[%d][%s]",
                ret,
                result->err_msg_);
    }
    return 0;
}

//=========================================================================================

ZCE_SQLite_Result::ZCE_SQLite_Result()
{

}


ZCE_SQLite_Result::~ZCE_SQLite_Result()
{

}

//�ͷŽ������
void ZCE_SQLite_Result::free_result()
{
    if (err_msg_)
    {
        ::sqlite3_free(err_msg_);
    }
    if (result_)
    {
        ::sqlite3_free_table(result_);
    }

    column_ = 0;
    row_ = 0;
}



#endif //#if SQLITE_VERSION_NUMBER >= 3005000

