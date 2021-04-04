/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_connect.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004��7��24��
* @brief      MYSQL�������������ڷ�װMYSQL��MYSQL���
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_MYSQL_DB_CONNECT_H_
#define ZCE_LIB_MYSQL_DB_CONNECT_H_

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

#include "zce_boost_non_copyable.h"

/*!
* @brief      MYSQL��������
*/
class ZCE_Mysql_Connect : public ZCE_NON_Copyable
{

public:
    /*!
    * @brief      ���캯��
    */
    ZCE_Mysql_Connect();

    /*!
    * @brief      ��������
    */
    ~ZCE_Mysql_Connect();

    /*!
    * @brief      �������ݷ�����,ͨ��IP��ַ����������
    * @return     int ����0��ʶ�ɹ�
    * @param[in]  host_name    IP��ַ��������������
    * @param[in]  user         �û�����
    * @param[in]  pwd          ����
    * @param[in]  db           ���Ӻ�Ĭ��ʹ�õ�DB�����ƣ�������
    * @param[in]  port         �˿ں�
    * @param[in]  timeout      ���ӵĳ�ʱʱ�䣬s
    * @param[in]  if_multi_sql �Ƿ�ʹ��MULTI SQL���
    */
    int connect_by_host(const char *host_name,
                        const char *user = "mysql",
                        const char *pwd = "",
                        const char *db = NULL,
                        const unsigned int port = MYSQL_PORT,
                        unsigned int timeout = 0,
                        bool if_multi_sql = false);

    /*!
    * @brief      �������ݿ��������ͨ��UNIXSOCKET�ļ���UNIX�£����������ܵ���WINDOWS�£�����ͨ�ţ�ֻ�����ڱ���
    * @return     int           ����0��ʶ�ɹ�
    * @param      socket_file   UNIXSOCKET ���ƣ�UNIX�£������������ܵ����ƣ�WINDOWS�£�
    * @param      user          �û�����
    * @param      pwd           ����
    * @param      db            ���Ӻ�Ĭ��ʹ�õ�DB�����ƣ�������
    * @param      timeout       ���ӵĳ�ʱʱ�䣬s
    * @param      if_multi_sql  �Ƿ�ʹ��MULTI SQL���
    */
    int connect_by_socketfile(const char *socket_file,
                              const char *user = "mysql",
                              const char *pwd = "",
                              const char *db = NULL,
                              unsigned int timeout = 0,
                              bool if_multi_sql = false);

    /*!
    * @brief      ʹ�������ļ��������ݿ������
    * @return     int
    * @param      optfile
    * @param      group
    */
    int connect_by_optionfile(const char *optfile, const char *group);

    /*!
    * @brief      �Ͽ����ݷ�����
    */
    void disconnect();

    /*!
    * @brief      �Ƿ�����
    * @return     bool  �Ƿ�����
    */
    inline bool is_connected();

    /*!
    * @brief      ѡ��һ��Ĭ�����ݿ�
    * @return     int   ����0��ʶ�ɹ�
    * @param      char* ���ݿ������
    */
    int select_database(const char *);

    /*!
    * @brief      ������ӶϿ����������ӣ��ͳɱ��ĺ÷���,����ʲô��������
    *             ������connect���ӳɹ���ʹ��,�����ݿ����ӳ���û�к����ӿ����Զ��Ͽ���PING������������,
    *             ���ִ���2013 ,Error =Lost connection to MySQL server during query �����ֱ��ʹ��
    * @return     int 0 �ɹ�
    */
    int ping();

    /*!
    * @brief      �õ����ݷ�����״̬
    * @return     const char* ���ص�״̬����
    */
    const char *get_mysql_status();

    /*!
    * @brief      ���ش�����Ϣ
    * @return     const char* ���ش���������Ϣ
    */
    inline const char *get_error_message();

    /*!
    * @brief      ���ش����
    * @return     unsigned int ���صĴ���ID
    */
    inline unsigned int get_error_no();

    /*!
    * @brief      �õ�MYSQL�ľ��
    * @return     MYSQL* ���ص�MYSQL���
    */
    inline MYSQL *get_mysql_handle();

    //��Щ��������4.1��İ汾����
#if MYSQL_VERSION_ID > 40100

    /*!
    * @brief      �����Ƿ��Զ��ύ
    * @return     int
    * @param      bauto
    */
    int set_auto_commit(bool bauto);

    /*!
    * @brief      �ύ����Commit Transaction
    * @return     int ����0��ʶ�ɹ�
    */
    int trans_commit();

    /*!
    * @brief      �ع�����Rollback Transaction
    * @return     int ����0��ʶ�ɹ�
    */
    int trans_rollback();

#endif

    /*!
    * @brief      ����ת�����õ�Real Escape String ,Real��ʾ����
    *             ��ǰ��MYSQL Connet���ַ���,�õ�Escape String
    *             Escape String Ϊ���ַ����е�����ַ�����ת������
    *             ��,����',",\���ַ�
    * @return     unsigned int ������ַ����ĳ���
    * @param      tostr        ת���õ����ַ���,��ñ�֤��fromlen *2�ĳ���
    * @param      fromstr      ����ת�����ַ���
    * @param      fromlen      ת�����ַ�������
    */
    unsigned int make_real_escape_string(char *tostr,
                                         const char *fromstr,
                                         unsigned int fromlen);

protected:

    /*!
    * @brief      �������ݿ���������ڲ�����,�����ʵ���ڴˣ�
    * @return     int
    * @param      host_name
    * @param      socket_file  UNIX SOCKET�ļ����ƻ��������ܵ�����
    * @param      user  �û�,Ĭ��Ϊmysql
    * @param      pwd  �û�����,Ĭ��Ϊ""
    * @param      db ʹ�õ�Ĭ�����ݿ�,Ĭ��Ϊ�ձ�ʾ��ѡ��
    * @param      port  �˿�,Ĭ��ΪMYSQL_PORT
    * @param      timeout  �������ݿ�ĳ�ʱʱ�䣬Ĭ��Ϊ0,��ʾ������
    * @param      bmultisql �Ƿ�ʹ�ö����ͬʱִ�еķ�ʽ,Ĭ��Ϊfalse,����������ȴ�������ЩЧ��
    */
    int connect_i(const char *host_name,
                  const char *socket_file,
                  const char *user = "mysql",
                  const char *pwd = "",
                  const char *db = NULL,
                  const unsigned int port = MYSQL_PORT,
                  unsigned int timeout = 0,
                  bool bmultisql = false);

public:

    /*!
    * @brief      �õ�ת����Escaple String ,û�и��ݵ�ǰ���ַ����Ͻ��в���,
    *             Escape String Ϊ���ַ����е�����ַ�����ת�������,����',",\���ַ�
    *             Ϊʲô������������ֲ���˳��,��Ϊmysql_escape_string
    * @return     unsigned int ������ַ����ĳ���
    * @param      tostr        ת���õ����ַ���,��ñ�֤��fromlen *2�ĳ���
    * @param      fromstr      ����ת�����ַ���
    * @param      fromlen      ת�����ַ�������
    */
    static unsigned int make_escape_string(char *tostr,
                                           const char *fromstr,
                                           unsigned int fromlen);

private:
    //MYSQL�ľ��
    MYSQL     mysql_handle_;

    //�Ƿ�����MYSQL���ݿ�
    bool      if_connected_;
};

//�õ�MYSQL�ľ��
inline MYSQL *ZCE_Mysql_Connect::get_mysql_handle()
{
    return &mysql_handle_;
}
//���״̬�Ƿ�����
inline bool ZCE_Mysql_Connect::is_connected()
{
    return if_connected_;
}

//�õ�������Ϣ
inline const char *ZCE_Mysql_Connect::get_error_message()
{
    return mysql_error(&mysql_handle_);
}

//�õ������ID
inline unsigned int ZCE_Mysql_Connect::get_error_no()
{
    return mysql_errno(&mysql_handle_);
}

#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_DB_CONNECT_H_

