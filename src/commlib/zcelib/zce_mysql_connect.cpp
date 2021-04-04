#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_mysql_connect.h"

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

/*********************************************************************************
class ZCE_Mysql_Connect
*********************************************************************************/

ZCE_Mysql_Connect::ZCE_Mysql_Connect()
{
    //���ڶ���conect��ʱ����г�ʼ���ˡ���Ӧ��disconnect ��ʱ��close
    ::mysql_init(&mysql_handle_);
    //�ÿ�ʼ״̬
    if_connected_ = false;
}

ZCE_Mysql_Connect::~ZCE_Mysql_Connect()
{
    // disconnect if if_connected_ to ZCE_Mysql_Connect
    disconnect();
}


//���ʹ��ѡ���ļ���������
int ZCE_Mysql_Connect::connect_by_optionfile(const char *optfile, const char *group)
{
    //����Ѿ�����,�ر�ԭ��������
    if (if_connected_ == true)
    {
        disconnect();
    }

    //��ʼ��MYSQL���
    ::mysql_init(&mysql_handle_);

    if (optfile != NULL)
    {
        int opret = mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_FILE, optfile);

        //���ʹgroup==NULL,����дoptfile��[client]����,�����дgroup�µ�����
        if (group != NULL)
        {
            opret = mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_GROUP, group);
        }

        if (opret != 0)
        {
            return -1;
        }
    }

    //�������ݿ�
    MYSQL *ret = mysql_real_connect(&mysql_handle_, NULL, NULL, NULL, NULL, 0, NULL, 0);
    if (ret == NULL)
    {
        return -1;
    }

    if_connected_ = true;
    //���سɹ� 0=0
    return 0;
}


//�������ݷ�����
int ZCE_Mysql_Connect::connect_i(const char *host_name,
                                 const char *socket_file,
                                 const char *user,
                                 const char *pwd,
                                 const char *db,
                                 const unsigned int port,
                                 const unsigned int timeout,
                                 bool if_multi_sql)
{

    //����Ѿ�����,�ر�ԭ��������
    if (if_connected_ == true)
    {
        disconnect();
    }

    //��ʼ��MYSQL���
    mysql_init(&mysql_handle_);

    //�������ӵ�timeout
    if (timeout != 0)
    {
        mysql_options(&mysql_handle_, MYSQL_OPT_CONNECT_TIMEOUT, (char *)(&timeout));
    }

    //50013,�汾���ṩ�����ѡ���ԭ���İ汾�����ѡ����Ĭ�ϴ򿪵ġ�
#if MYSQL_VERSION_ID >= 50013
    mysql_options(&mysql_handle_, MYSQL_OPT_RECONNECT, "1");
#endif

    unsigned long client_flag = 0;

#if MYSQL_VERSION_ID > 40100

    if (if_multi_sql)
    {
        client_flag |= CLIENT_MULTI_STATEMENTS;
    }

#endif

    //�������ݿ�
    MYSQL *ret = NULL;

    //���ʹ����������IP��ַ��������
    if (host_name)
    {
        ret = ::mysql_real_connect(&mysql_handle_,
                                   host_name,
                                   user,
                                   pwd,
                                   db,
                                   port,
                                   NULL,
                                   client_flag);
    }
    //���ʹ��UNIXSOCKET���������ܵ����б�������
    else if (socket_file)
    {
        //����ط�����ע��һ�£�WINDOWS�£�����mysql_real_connect�������host_name����ΪNULL�����Ƚ��������ܵ����ӣ����������TCP/IP���ӱ���
        //���Ҫ����֤����ʹ�������ܵ��������host_name=".",
        ret =  ::mysql_real_connect(&mysql_handle_,
                                    NULL,
                                    user,
                                    pwd,
                                    db,
                                    port,
                                    socket_file,
                                    client_flag);
    }
    //����ʹ�ô��󣬲���host��unixsocket��ΪNULL
    else
    {
        ZCE_ASSERT(false);
    }

    //�����,
    if (ret != 0)
    {
        return -1;
    }

    if_connected_ = true;
    //���سɹ� 0=0
    return 0;
}

//�������ݷ�����,ͨ��IP��ַ����������
int ZCE_Mysql_Connect::connect_by_host(const char *host_name,
                                       const char *user,
                                       const char *pwd,
                                       const char *db,
                                       const unsigned int port,
                                       unsigned int timeout,
                                       bool if_multi_sql)
{
    return connect_i(host_name, NULL, user, pwd, db, port, timeout, if_multi_sql);
}

//�������ݿ��������ͨ��UNIXSOCKET�ļ���UNIX�£����������ܵ���WINDOWS�£�����ͨ�ţ�ֻ�����ڱ���
int ZCE_Mysql_Connect::connect_by_socketfile(const char *socket_file,
                                             const char *user,
                                             const char *pwd,
                                             const char *db,
                                             unsigned int timeout,
                                             bool if_multi_sql)
{
    return connect_i(NULL, socket_file, user, pwd, db, 0, timeout, if_multi_sql);
}

//�Ͽ����ݿ����������
void ZCE_Mysql_Connect::disconnect()
{
    //û������
    if (if_connected_ == false)
    {
        return;
    }

    ::mysql_close(&mysql_handle_);
    if_connected_ = false;
}

//ѡ��һ��Ĭ�����ݿ�,���������ݿ������
int ZCE_Mysql_Connect::select_database(const char *db)
{
    int ret = mysql_select_db(&mysql_handle_, db);

    //�����,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//������ӶϿ����������ӣ��ͳɱ��ĺ÷���,����ʲô��������
int ZCE_Mysql_Connect::ping()
{
    int ret = mysql_ping(&mysql_handle_);

    //�����,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//�õ���ǰ���ݷ�������״̬
const char *ZCE_Mysql_Connect::get_mysql_status()
{
    return mysql_stat(&mysql_handle_);
}

//�õ�ת����Escaple String ,û�и��ݵ�ǰ���ַ����Ͻ��в���,
unsigned int ZCE_Mysql_Connect::make_escape_string(char *tostr, const char *fromstr, unsigned int fromlen)
{
    return mysql_escape_string(tostr, fromstr, fromlen);
}


unsigned int ZCE_Mysql_Connect::make_real_escape_string(char *tostr,
                                                        const char *fromstr,
                                                        unsigned int fromlen)
{
    return mysql_real_escape_string(&mysql_handle_,
                                    tostr,
                                    fromstr,
                                    fromlen);
}

//��Щ��������4.1��İ汾����
#if MYSQL_VERSION_ID > 40100

//�����Ƿ��Զ��ύ
int ZCE_Mysql_Connect::set_auto_commit(bool bauto)
{
    //my_bool��ʵ��char
    my_bool mode =  (bauto == true ) ? 1 : 0;

    int ret = mysql_autocommit(&mysql_handle_, mode);

    //�����,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//�ύ����Commit Transaction
int ZCE_Mysql_Connect::trans_commit()
{

    int ret = mysql_commit(&mysql_handle_);

    //�����,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//�ع�����Rollback Transaction
int ZCE_Mysql_Connect::trans_rollback()
{
    int ret = mysql_rollback(&mysql_handle_);

    //�����,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

#endif // MYSQL_VERSION_ID > 40100

//�����Ҫ��MYSQL�Ŀ�
#endif //#if defined ZCE_USE_MYSQL

