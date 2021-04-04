
#include "zce_predefine.h"
#include "zce_mysql_command.h"

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL


ZCE_Mysql_Command::ZCE_Mysql_Command():
    mysql_connect_(NULL)
{
    //����INITBUFSIZE�Ŀռ�
    mysql_command_.reserve(INITBUFSIZE);
    sql_buffer_ = new char[INITBUFSIZE];
}

ZCE_Mysql_Command::ZCE_Mysql_Command(ZCE_Mysql_Connect *conn):
    mysql_connect_(NULL)
{
    //assert(conn != NULL);
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
    }

    //����INITBUFSIZE�Ŀռ�
    mysql_command_.reserve(INITBUFSIZE);
    sql_buffer_ = new char[INITBUFSIZE];
}

ZCE_Mysql_Command::~ZCE_Mysql_Command()
{
    if (sql_buffer_)
    {
        delete sql_buffer_;
        sql_buffer_ = NULL;
    }
}

//ΪZCE_Mysql_Command������ص����Ӷ��󣬶����Ǳ����Ѿ��ɹ����������ݵ�
int ZCE_Mysql_Command::set_connection(ZCE_Mysql_Connect *conn)
{
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
        return 0;
    }

    return -1;
}

///����SQL Command���,��̬�����汾
int ZCE_Mysql_Command::set_sql_command(const char *sql_format, ...)
{
    va_list args;
    va_start(args, sql_format);

    //_vsnprintf����ANSI C��׼����,���Ǵ󲿷ֺ�����Ӧ��ʵ������,�Ͼ�vsprintfȱ�������İ�ȫ��
    int ret = vsnprintf(sql_buffer_, INITBUFSIZE, sql_format, args);

    va_end(args);

    //������ؽ������,_vsnprintf =-1��ʾBuf���Ȳ���
    if (ret < 0)
    {
        return -1;
    }

    //����
    mysql_command_.assign(sql_buffer_);

    //�ɹ�
    return 0;
}

//�õ�SQL Command. ���ض�����ʽ,�����ı�����
const char *ZCE_Mysql_Command::get_sql_command() const
{
    return mysql_command_.c_str();
}

// �õ�SQL ���. ��������,�����char buf�����Ƿ��㹻�Լ���֤
int ZCE_Mysql_Command::get_sql_command( char *cmdbuf, size_t &szbuf) const
{
    if (cmdbuf == NULL )
    {
        ZCE_ASSERT(false);
        return -1;
    }

    size_t size_sql = mysql_command_.length();

    if (size_sql + 1 > szbuf)
    {
        return -1;
    }

    szbuf = size_sql;
    memcpy(cmdbuf, mysql_command_.c_str(), szbuf);
    return 0;
}

//
void ZCE_Mysql_Command::get_sql_command(std::string &strcmd) const
{
    //Ԥ�ȷ����ڴ�,��֤Ч��
    strcmd.reserve(mysql_command_.length());
    strcmd = mysql_command_;
}

//int �����Ƿ�ɹ�����ʧ�� MYSQL_RETURN_FAIL��ʾʧ��
//ִ��SQL��䣬����ȫ����������ʹ��
int ZCE_Mysql_Command::execute(uint64_t *num_affect,
                               uint64_t *last_id,
                               ZCE_Mysql_Result *sql_result,
                               bool bstore)
{
    //���û���������ӻ���û����������
    if (mysql_connect_ == NULL || mysql_command_.empty())
    {
        return -1;
    }

    //ִ��SQL����
    int tmpret = ::mysql_real_query(mysql_connect_->get_mysql_handle(),
                                    mysql_command_.c_str(),
                                    (unsigned long)mysql_command_.length());
    if (tmpret != 0)
    {
        return tmpret;
    }

    //����û�Ҫ��ת�������
    if (sql_result)
    {
        MYSQL_RES *tmp_res = NULL;

        if (bstore)
        {
            //ת�����
            tmp_res = ::mysql_store_result(mysql_connect_->get_mysql_handle());
        }
        else
        {
            //ת�����
            tmp_res = ::mysql_use_result(mysql_connect_->get_mysql_handle());
        }

        //��������INSERT��䵫��,��Ҫȡ�ؽ����,����ʱ��Ϊ���ǶԵ�,ֻ�Ƿ��صĽ����Ϊ�ջ����㲻��ע��
        //���ת��ʧ��,Ϊʲô������,��MySQL�ĵ�"Ϊʲô��mysql_query()���سɹ���mysql_store_result()��ʱ����NULL? "
        //�����INSERT��䣬��ômysql_store_result���Ƿ���NULL��mysql_field_countҲӦ�õ���0��
        //���MYSQL�ڲ�����ĳ��������ômysql_store_result ����NULL����mysql_field_count �����0����ʱ�Ǹ�����
        if ( tmp_res == NULL && mysql_field_count(mysql_connect_->get_mysql_handle()) > 0)
        {
            return -1;
        }

        //�õ������,��ѯ�������Ϣ
        sql_result->set_mysql_result(tmp_res);
    }

    //ִ��SQL����Ӱ���˶�����,mysql_affected_rows ������ת���������,������Ҫע������Ĳ���
    if (num_affect)
    {
        *num_affect = (uint64_t) ::mysql_affected_rows(mysql_connect_->get_mysql_handle());
    }

    if (last_id)
    {
        *last_id = (uint64_t) ::mysql_insert_id(mysql_connect_->get_mysql_handle());
    }

    //�ɹ�
    return 0;
}

//ִ��SQL���,�������������ϵ�����,��SELECT���
//num_affect Ϊ���ز���,�������޸��˼���
int ZCE_Mysql_Command::execute(uint64_t &num_affect, uint64_t &last_id)
{
    return execute(&num_affect, &last_id, NULL, false);
}

//ִ��SQL���,SELECT���,ת��������ϵ�����,ע������������õ���mysql_store_result.
//num_affect Ϊ���ز���,�������޸��˼���,SELECT�˼���
int ZCE_Mysql_Command::execute(uint64_t &num_affect, ZCE_Mysql_Result &sql_result)
{
    return execute(&num_affect, NULL, &sql_result, true);
}

//ִ��SQL���,SELECT���,USE������ϵ�����,ע������õ���mysql_use_result,num_affect������Ч
//���ڽ����̫��Ĵ���,���һ��ת���������ռ��̫���ڴ�Ĵ���,���Կ�������,
//�����Ƽ�ʹ��,һ��ȡһ��,����̫��
int ZCE_Mysql_Command::execute(ZCE_Mysql_Result &sql_result)
{
    return execute(NULL, NULL, &sql_result, false);
}


#if MYSQL_VERSION_ID > 40100

//���� multiple-statement executions �еõ����
//���
int ZCE_Mysql_Command::fetch_next_multi_result(ZCE_Mysql_Result &sqlresult, bool bstore)
{

    int tmpret = ::mysql_next_result(mysql_connect_->get_mysql_handle());

    //tmpret == -1��ʾû�н����,����<0��ֵ��ʾ����
    if (tmpret < 0 )
    {
        return -1;
    }

    MYSQL_RES *tmp_res = NULL;

    if (bstore)
    {
        //ת�����
        tmp_res = ::mysql_store_result(mysql_connect_->get_mysql_handle());
    }
    else
    {
        //ת�����
        tmp_res = ::mysql_use_result(mysql_connect_->get_mysql_handle());
    }

    //��������INSERT��䵫��,��Ҫȡ�ؽ����,����ʱ��Ϊ���ǶԵ�,ֻ�Ƿ��صĽ����Ϊ�ջ����㲻��ע��
    //���ת��ʧ��,Ϊʲô������,��MySQL�ĵ�"Ϊʲô��mysql_query()���سɹ���mysql_store_result()��ʱ����NULL? "
    if ( tmp_res == NULL && ::mysql_field_count(mysql_connect_->get_mysql_handle()) > 0)
    {
        return -1;
    }

    //�õ������,��ѯ�������Ϣ
    sqlresult.set_mysql_result(tmp_res);

    //�ɹ�
    return 0;
}

#endif //MYSQL_VERSION_ID > 40100

//�����Ҫ��MYSQL�Ŀ�
#endif //#if defined ZCE_USE_MYSQL

