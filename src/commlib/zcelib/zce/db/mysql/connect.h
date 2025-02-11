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
#if defined ZCE_USE_MYSQL

#include "zce/db/base.h"
#include "zce/db/mysql/result.h"

namespace zce::mysql
{
/*!
* @brief      MYSQL的连接器
*/
class connect :public zce::db::connect_base
{
public:

    //构造函数,析构函数
    connect();
    ~connect();

    connect(connect &&) noexcept;
    connect& operator=(connect&&) noexcept;

    //避免拷贝
    connect(const connect &) = delete;
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
    int connect_by_optionfile(const char* optfile, const char* group);

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
    const char* error_message();

    /*!
    * @brief      返回错误号
    * @return     unsigned int 返回的错误ID
    */
    unsigned int error_no();

    /*!
    * @brief      得到MYSQL的句柄
    * @return     MYSQL* 返回的MYSQL句柄
    */
    inline MYSQL* get_mysql_handle();

    /*!
    * @brief      执行SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      lastid      插入ID等，对于有自增字段的时，(UINT32也许，还不够用，呵呵)
    */
    int query(uint64_t& num_affect, uint64_t& last_id);

    /*!
    * @brief      执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      sqlresult   返回的结果集合
    */
    int query(uint64_t& num_affect, zce::mysql::result& sql_result);

    /*!
    * @brief      执行SQL语句,SELECT语句,USE结果集合的那种,注意其调用的是mysql_use_result,num_affect对它无效
    *             用于结果集太多的处理,一次处理会占用太多内存的的处理,不推荐使用,
    * @return     int
    * @param      sqlresult 返回的结果集合
    */
    int query(zce::mysql::result& sqlresult);

    /*!
* @brief      设置Command的zce::mysql::connect
* @return     int  0成功，-1失败
* @param      conn 链接对象，必须已经链接成功喔
*/
    int set_connect(zce::mysql::connect* conn);

    /*!
    * @brief      得到此Command的zce::mysql::Connect对象
    * @return     zce::mysql::connect*
    */
    zce::mysql::connect* get_connect();

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

    /*!
    * @brief      如果一次执行多行SQL语句，这个方法用于取回结果集合
    * @return     int       0表示成功，否则标识失败
    * @param[out] sqlresult 返回的MySQL结果集合
    * @param[out] bstore    使用mysql_store_result取回结果集合，还是mysql_use_result
    */
    int fetch_next_result(zce::mysql::result& sqlresult,
                          bool bstore = true);

    /*!
    * @brief      编码转换，得到Real Escape String ,Real表示根据
    *             当前的MYSQL Connet的字符集,得到Escape String
    *             Escape String 为将字符传中的相关字符进行转义后的语
    *             句,比如',",\等字符
    * @return     unsigned int 编码后字符串的长度
    * @param      tostr        转换得到的字符串,最好保证有fromlen *2的长度
    * @param      fromstr      进行转换的字符串
    * @param      fromlen      转换的字符串长度
    */
    unsigned int real_escape_string(char* tostr,
                                    const char* fromstr,
                                    unsigned int fromlen);

protected:

    /*!
    * @brief      连接数据库服务器，内部函数,具体的实现在此，
    * @return     int
    * @param      host_name
    * @param      socket_file  UNIX SOCKET文件名称或者命名管道名称
    * @param      user  用户,默认为mysql
    * @param      pwd  用户密码,默认为""
    * @param      db 使用的默认数据库,默认为空表示不选择
    * @param      port  端口,默认为MYSQL_PORT
    * @param      timeout  连接数据库的超时时间，默认为0,表示不设置
    * @param      bmultisql 是否使用多语句同时执行的方式,默认为false,可能在事物等处理上有些效果
    */
    int connect_i(const char* host_name,
                  const char* socket_file,
                  const char* user = "mysql",
                  const char* pwd = "",
                  const char* db = nullptr,
                  const unsigned int port = MYSQL_PORT,
                  unsigned int timeout = 0,
                  bool bmultisql = false);

    /*!
    * @brief      执行SQL语句,内部的基础函数,让大家共同调用的基础函数
    * @return     int         int  0成功，-1失败
    * @param[out] num_affect  影响的数据条数，或者返回结果的条数
    * @param[out] lastid      最后的插入ID是什么，
    * @param[out] sqlresult   SQL执行后的结果集合
    * @param[out] bstore      使用什么方式获得结果，ture是使用mysql_store_result,false是使用mysql_use_result（需要多次交互）,
    */
    int query_i(uint64_t* num_affect,
                uint64_t* last_id,
                zce::mysql::result* sqlresult,
                bool bstore);
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
    static unsigned int escape_string(char* tostr,
                                      const char* fromstr,
                                      unsigned int fromlen);

private:

    ///MYSQL的句柄
    MYSQL     mysql_handle_;

    ///是否连接MYSQL数据库
    bool      if_connected_ = false;
};

//得到MYSQL的句柄
inline MYSQL* zce::mysql::connect::get_mysql_handle()
{
    return &mysql_handle_;
}
//检查状态是否连接
inline bool zce::mysql::connect::is_connected()
{
    return if_connected_;
}

//得到错误信息
inline const char* zce::mysql::connect::error_message()
{
    return mysql_error(&mysql_handle_);
}

//得到错误的ID
inline unsigned int zce::mysql::connect::error_no()
{
    return mysql_errno(&mysql_handle_);
}
}
#endif //#if defined ZCE_USE_MYSQL