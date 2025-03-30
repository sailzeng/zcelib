/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/connect.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MYSQL的连接器，用于封装MYSQL的MYSQL句柄
*
* @details
*
* @note
*
*/

#pragma once

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

#include "zce/db/base.h"
#include "zce/db/mysql/result.h"

namespace zce::mysql
{
class stmt_bind;
class result;

/*!
* @brief      MYSQL的Handle,负责连接，命令执行等
*/
class connect
{
public:

    //构造函数,析构函数
    connect();
    ~connect() noexcept;

    //避免拷贝
    connect(connect&&) noexcept = delete;
    connect& operator=(connect&&) noexcept = delete;
    connect(const connect&) = delete;
    connect& operator=(const connect&) = delete;

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
    int connect_by_host(const char* host_name,
                        const char* user = "mysql",
                        const char* pwd = "",
                        const char* db = nullptr,
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
    int connect_by_socketfile(const char* socket_file,
                              const char* user = "mysql",
                              const char* pwd = "",
                              const char* db = nullptr,
                              unsigned int timeout = 0,
                              bool if_multi_sql = false);

    /*!
    * @brief      使用配置文件连接数据库服务器
    * @return     int
    * @param      optfile
    * @param      group
    */
    int connect_by_optionfile(const char* optfile,
                              const char* group);

    /*!
    * @brief      断开数据服务器
    */
    void disconnect();

    /*!
    * @brief      选择一个默认数据库
    * @return     int   返回0标识成功
    * @param      char* 数据库的名称
    */
    int select_database(const char*);

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
    const char* get_mysql_status();

    /*!
    * @brief      返回错误消息
    * @return     const char* 返回错误描述消息
    */
    const char* error_message()
    {
        return mysql_error(&mysql_handle_);
    }

    /*!
    * @brief      返回错误号
    * @return     unsigned int 返回的错误ID
    */
    unsigned int error_no()
    {
        return mysql_errno(&mysql_handle_);
    }

    //!得到MYSQL的句柄
    inline MYSQL* get_handle()
    {
        return &mysql_handle_;
    }

    /*!
    * @brief      设置是否自动提交
    * @return     int
    * @param      bauto
    */
    int set_auto_commit(bool bauto);

protected:

    enum class CONNECT_BY
    {
        HOST,
        SOCKET_FILE,
        OPTION_FILE
    };
    /*!
    * @brief      连接数据库服务器，内部函数,具体的实现在此，
    * @return     int
    * @param      by 连接方式
    */
    int connect_i(CONNECT_BY by,
                  const char* host_name,
                  const char* socket_file,
                  const char* user = "mysql",
                  const char* pwd = "",
                  const char* db = nullptr,
                  const unsigned int port = MYSQL_PORT,
                  unsigned int timeout = 0,
                  bool bmultisql = false,
                  const char* optfile = nullptr,
                  const char* group = nullptr);

private:

    ///MYSQL的句柄
    MYSQL mysql_handle_;

    ///是否连接MYSQL数据库
    bool if_connected_ = false;

    ///
    bool is_bind_result_ = false;
};
}
#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
