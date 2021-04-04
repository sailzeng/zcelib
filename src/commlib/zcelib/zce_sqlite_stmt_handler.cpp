
#include "zce_predefine.h"

//����SQLITE����Ͱ汾����
#if SQLITE_VERSION_NUMBER >= 3005000

#include "zce_log_logging.h"
#include "zce_sqlite_db_handler.h"
#include "zce_sqlite_stmt_handler.h"



/******************************************************************************************
SQLite3_STMT_Handler
******************************************************************************************/
//���캯��,�Ӻ�ϸС�ĵط��Ϳ��Կ���SQLITE������в��㣬һ��INDEX��1��ʼ��1����0
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


//����SQLITE3��STMT HANDLER���ָ���ʼ��ֵ�ȡ�
int ZCE_SQLite_STMTHdl::finalize()
{
    //����SQLITE3��STMT HANDLER
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


//����SQL��䣬����Ƿ��ܹ���ȷִ��
int ZCE_SQLite_STMTHdl::prepare(const char *sql_string)
{
    if (sqlite3_stmt_handler_)
    {
        finalize();
    }

    int ret =  ::sqlite3_prepare_v2(sqlite_handler_->get_sqlite_handler(),
                                    sql_string,
                                    -1,                                      //ע���������������С��0
                                    &sqlite3_stmt_handler_,
                                    NULL);

    //�������������󣬻��߲���һ��SQL
    if ( SQLITE_OK  != ret || sqlite3_stmt_handler_ == NULL)
    {
        //�������ش���
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

    //�������ش���
    ZCE_LOG(RS_ERROR, "[zcelib] Error:[%d][%s]", error_code(), error_message());
    return -1;
}

//Bind ����Ⱥ
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

//ע��long���bind����32λ���
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
    //�Ӳ����Ͽ���SQLite��STMT����bind����������ȡ������
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
    //Fisk�����̬���Ҹ��˵ط���Ϊ�˰�ȫ��顣
    strncpy(val,
            reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt_handler_,
                                                               result_col)),
            static_cast<size_t>(sqlite3_column_bytes(sqlite3_stmt_handler_, result_col)) + 1);

    return;
}

//�����Ƶ�����Ҫ�ر���һ��,�ַ������ر�+1��,�����������ݲ�Ҫ��������
template<>
void ZCE_SQLite_STMTHdl::column(int result_col, ZCE_SQLite_STMTHdl::BIN_Result &val)
{
    *val.binary_len_ = ::sqlite3_column_bytes(sqlite3_stmt_handler_, result_col);
    //Ϊ�˻�ȡ���������ݣ���ZCE_Mysql_Result���Ӧ,���Ȳ�+1
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

//��ʼһ������
int ZCE_SQLite_STMTHdl::begin_transaction()
{
    return sqlite_handler_->begin_transaction();
}

//�ύһ������
int ZCE_SQLite_STMTHdl::commit_transction()
{
    return sqlite_handler_->commit_transction();
}

int ZCE_SQLite_STMTHdl::turn_off_synch()
{
    return sqlite_handler_->turn_off_synch();
}

#endif //#if SQLITE_VERSION_NUMBER >= 3005000

