/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_connect.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MYSQL的连接器，用于封装MYSQL的MYSQL句柄
* 
* @details
* 
* @note
* 
*/

#ifndef ZCE_LIB_MYSQL_DB_CONNECT_H_
#define ZCE_LIB_MYSQL_DB_CONNECT_H_

//如果你要用MYSQL的库
#if defined MYSQL_VERSION_ID

#include "zce_mysql_predefine.h"
#include "zce_boost_non_copyable.h"

/*!
* @brief      MYSQL的连接器
*/
class ZCELIB_EXPORT ZCE_Mysql_Connect : public ZCE_NON_Copyable
{

public:
    /*!
    * @brief      构造函数
    */
    ZCE_Mysql_Connect();

    /*!
    * @brief      析构函数
    */
    ~ZCE_Mysql_Connect();

    /*!
    * @brief      连接数据服务器,通过IP地址，主机名称
    * @return     int 返回0标识成功
    * @param[in]  host_name    IP地址，或者主机名称
    * @param[in]  user         用户名称
    * @param[in]  pwd          密码
    * @param[in]  db           链接后，默认使用的DB库名称，不建议
    * @param[in]  port         端口号
    * @param[in]  timeout      链接的超时时间，s
    * @param[in]  if_multi_sql 是否使用MULTI SQL语句
    */
    int connect_by_host(const char *host_name,
                        const char *user = "mysql",
                        const char *pwd = "",
                        const char *db = NULL,
                        const unsigned int port = MYSQL_PORT,
                        unsigned int timeout = 0,
                        bool if_multi_sql = false);

    /*!
    * @brief      连接数据库服务器，通过UNIXSOCKET文件（UNIX下）或者命名管道（WINDOWS下）进行通信，只能用于本机
    * @return     int           返回0标识成功
    * @param      socket_file   UNIXSOCKET 名称（UNIX下），或者命名管道名称（WINDOWS下）
    * @param      user          用户名称
    * @param      pwd           密码
    * @param      db            链接后，默认使用的DB库名称，不建议
    * @param      timeout       链接的超时时间，s
    * @param      if_multi_sql  是否使用MULTI SQL语句
    */
    int connect_by_socketfile(const char *socket_file,
                              const char *user = "mysql",
                              const char *pwd = "",
                              const char *db = NULL,
                              unsigned int timeout = 0,
                              bool if_multi_sql = false);

    /*!
    * @brief      使用配置文件连接数据库服务器
    * @return     int
    * @param      optfile
    * @param      group
    */
    int connect_by_optionfile(const char *optfile, const char *group);

    /*!
    * @brief      断开数据服务器
    */
    void disconnect();

    /*!
    * @brief      是否连接
    * @return     bool  是否连接
    */
    inline bool is_connected();

    /*!
    * @brief      选择一个默认数据库
    * @return     int   返回0标识成功
    * @param      char* 数据库的名称
    */
    int select_database(const char *);

    /*!
    * @brief      如果连接断开，重新连接，低成本的好方法,否则什么都不做，
    *             必须在connect连接成功后使用,当数据库连接长期没有后，连接可能自动断开，PING函数用于重联,
    *             发现错误2013 ,Error =Lost connection to MySQL server during query 后可以直接使用
    * @return     int 0 成功
    */
    int ping();

    /*!
    * @brief      得到数据服务器状态
    * @return     const char* 返回的状态描述
    */
    const char *get_mysql_status();

    /*!
    * @brief      返回错误消息
    * @return     const char* 返回错误描述消息
    */
    inline const char *get_error_message();

    /*!
    * @brief      返回错误号
    * @return     unsigned int 返回的错误ID
    */
    inline unsigned int get_error_no();

    /*!
    * @brief      得到MYSQL的句柄
    * @return     MYSQL* 返回的MYSQL句柄
    */
    inline MYSQL *get_mysql_handle();

    //这些函数都是4.1后的版本功能
#if MYSQL_VERSION_ID > 40100

    /*!
    * @brief      设置是否自动提交
    * @return     int
    * @param      bauto
    */
    int set_auto_commit(bool bauto);

    /*!
    * @brief      提交事务Commit Transaction
    * @return     int 返回0标识成功
    */
    int trans_commit();

    /*!
    * @brief      回滚事务Rollback Transaction
    * @return     int 返回0标识成功
    */
    int trans_rollback();

#endif

    /*!
    * @brief      得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
    *             Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    * @return     unsigned int 编码后字符串的长度
    * @param      tostr        转换得到的字符串,最好保证有fromlen *2的长度
    * @param      fromstr      进行转换的字符串
    * @param      fromlen      转换的字符串长度
    * @note
    */
    unsigned int make_real_escape_string(char *tostr,
                                         const char *fromstr,
                                         unsigned int fromlen);

protected:

    ///连接数据库服务器，内部函数,具体的实现在此，
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
    * @brief      得到转意后的Escaple String ,没有根据当前的字符集合进行操作,
    *             Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    *             为什么采用这样的奇怪参数顺序,因为mysql_escape_string
    * @return     unsigned int 编码后字符串的长度
    * @param      tostr        转换得到的字符串,最好保证有fromlen *2的长度
    * @param      fromstr      进行转换的字符串
    * @param      fromlen      转换的字符串长度
    */
    static unsigned int make_escape_string(char *tostr,
                                           const char *fromstr,
                                           unsigned int fromlen);

private:
    //MYSQL的句柄
    MYSQL     mysql_handle_;

    //是否连接MYSQL数据库
    bool      if_connected_;
};

//得到MYSQL的句柄
inline MYSQL *ZCE_Mysql_Connect::get_mysql_handle()
{
    return &mysql_handle_;
}
//检查状态是否连接
inline bool ZCE_Mysql_Connect::is_connected()
{
    return if_connected_;
}

//得到错误信息
inline const char *ZCE_Mysql_Connect::get_error_message()
{
    return mysql_error(&mysql_handle_);
}

//得到错误的ID
inline unsigned int ZCE_Mysql_Connect::get_error_no()
{
    return mysql_errno(&mysql_handle_);
}

#endif //#if defined MYSQL_VERSION_ID

#endif //ZCE_LIB_MYSQL_DB_CONNECT_H_

