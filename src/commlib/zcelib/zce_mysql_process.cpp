
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_mysql_process.h"

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

//-----------------------------------------------------------------
//ʵ��,Ϊ��SingleTon��׼��
ZCE_Mysql_Process *ZCE_Mysql_Process::instance_ = NULL;

//���캯��������������
ZCE_Mysql_Process::ZCE_Mysql_Process():
    db_port_(MYSQL_PORT)
{

}
ZCE_Mysql_Process::~ZCE_Mysql_Process()
{
    //���ô���ʲô����صĳ�Ա�����������������˴���
}

//��ʼ��������,ʹ��hostname��������,���Բ��������Ӻ��������ӣ����Լ����ơ�
int ZCE_Mysql_Process::init_mysql_server(const char *host_name,
                                         const char *user,
                                         const char *pwd,
                                         unsigned int port,
                                         bool connect_atonce)
{

    db_hostname_ = host_name;
    db_user_name_ = user;
    db_password_ = pwd;
    db_port_ = port;

    if (connect_atonce)
    {
        return connect_mysql_server();
    }

    return 0;
}

//��ʼ��MYSQL��ʹ��UNIX socket file����(UNIX��)�����������ܵ�(Windows��),ֻ��ʼ��Ҳ������,ֻ�����ڱ���
int ZCE_Mysql_Process::init_mysql_socketfile(const char *socket_file,
                                             const char *user,
                                             const char *pwd,
                                             bool connect_atonce)
{
    db_socket_file_ = socket_file;
    db_user_name_ = user;
    db_password_ = pwd;

    if (connect_atonce)
    {
        return connect_mysql_server();
    }

    return 0;
}

//
int ZCE_Mysql_Process::connect_mysql_server( )
{
    int ret = 0;

    //�������ݿ�
    if (db_connect_.is_connected() == false)
    {

        //������ù�HOST����HOST NAME��������
        if (db_hostname_.length() > 0)
        {
            ret = db_connect_.connect_by_host(db_hostname_.c_str(),
                                              db_user_name_.c_str(),
                                              db_password_.c_str(),
                                              NULL,
                                              db_port_);
        }
        else if (db_socket_file_.length() > 0)
        {
            ret = db_connect_.connect_by_socketfile(db_socket_file_.c_str(),
                                                    db_user_name_.c_str(),
                                                    db_password_.c_str());
        }
        else
        {
            ZCE_ASSERT(false);
        }

        //�������
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error : [%u]:%s.",
                    db_connect_.get_error_no(),
                    db_connect_.get_error_message()
                   );
            return -1;
        }

        //
        db_command_.set_connection(&db_connect_);
    }

    return  0;
}

//�Ͽ�����
void ZCE_Mysql_Process::disconnect_mysql_server()
{

    if (db_connect_.is_connected() == true)
    {
        db_connect_.disconnect();
    }

}

//���ڷ�SELECT���(INSERT,UPDATE)��
int ZCE_Mysql_Process::db_process_query(const char *sql,
                                        size_t sqllen,
                                        uint64_t &numaffect,
                                        uint64_t &insertid)
{
    int ret = 0;

    //�������ݿ�
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //����Ѿ����ӹ����ݿ�,�����ٴ�����,pingһ�ξ�OK��,�ɱ���
    else
    {
        db_connect_.ping();
    }

    //
    ZCE_LOGMSG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sqllen, sql);
    db_command_.set_sql_command(sql, sqllen);


    ret = db_command_.execute(numaffect, insertid);

    //�������
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //�ɹ�
    return  0;
}


//ִ�м����SQL���,����SELECT���,ֱ��ת��������ϵķ���
int ZCE_Mysql_Process::db_process_query(const char *sql, size_t sqllen,
                                        uint64_t &numaffect,
                                        ZCE_Mysql_Result &dbresult)
{
    int ret = 0;

    //�������ݿ�
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //����Ѿ����ӹ����ݿ�,�����ٴ�����,pingһ�ξ�OK��,�ɱ���
    else
    {
        db_connect_.ping();
    }


    ZCE_LOGMSG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sqllen, sql);
    db_command_.set_sql_command(sql, sqllen);


    ret = db_command_.execute(numaffect, dbresult);

    //�������
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //�ɹ�
    return  0;
}


//
int ZCE_Mysql_Process::db_process_query(const char *sql, size_t sqllen, ZCE_Mysql_Result &dbresult)
{
    int ret = 0;

    //�������ݿ�
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //����Ѿ����ӹ����ݿ�,�����ٴ�����,pingһ�ξ�OK��,�ɱ���
    else
    {
        db_connect_.ping();
    }

    ZCE_LOGMSG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sqllen, sql);
    db_command_.set_sql_command(sql, sqllen);


    ret = db_command_.execute(dbresult);

    //�������
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //�ɹ�
    return  0;
}

//���صĵ�DB���ʵĴ�����Ϣ
unsigned int ZCE_Mysql_Process::get_return_error(char *szerr, size_t buflen)
{
    snprintf(szerr, buflen, "[%d]:%s ", db_connect_.get_error_no(), db_connect_.get_error_message());
    return db_connect_.get_error_no();
}


//�õ�DB���ʵ����
const char *ZCE_Mysql_Process::get_query_sql(void)
{
    return db_command_.get_sql_command();
}

//�õ�������Ϣ���
const char *ZCE_Mysql_Process::get_return_error_str()
{
    return db_connect_.get_error_message();
}

//�õ�������ϢID
unsigned int ZCE_Mysql_Process::get_return_error_id()
{
    return db_connect_.get_error_no();
}

//�õ�Real Escape String ,Real��ʾ���ݵ�ǰ��MYSQL Connet���ַ���,�õ�Escape String
//Escape String Ϊ���ַ����е�����ַ�����ת�������,����',",\���ַ�
unsigned int ZCE_Mysql_Process::make_real_escape_string(char *tostr,
                                                        const char *fromstr,
                                                        unsigned int fromlen)
{
    return mysql_real_escape_string(db_connect_.get_mysql_handle(), tostr, fromstr, fromlen);
}

//-----------------------------------------------------------------
//ʵ��,Ϊ��SingleTon��׼��

//ʵ����ֵ
void ZCE_Mysql_Process::instance(ZCE_Mysql_Process *instance)
{
    clean_instance();
    instance_ = instance;
}
//���ʵ��
ZCE_Mysql_Process *ZCE_Mysql_Process::instance()
{
    if (instance_)
    {
        delete instance_ ;
        instance_ = NULL;
    }

    instance_ = new ZCE_Mysql_Process();
    return  instance_;

}

//���ʵ��
void ZCE_Mysql_Process::clean_instance()
{
    if (instance_)
    {
        delete instance_ ;
        instance_ = NULL;
    }
}

//�����Ҫ��MYSQL�Ŀ�
#endif //#if defined ZCE_USE_MYSQL

