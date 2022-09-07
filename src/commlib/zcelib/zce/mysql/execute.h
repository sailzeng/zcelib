/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/process.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年05月18日
* @brief      读取MYSQL数据库的类,用于读取DB的配置信息
*
* @details    内部有保证自动重连的机制，你不用操心这个问题
*
* @note
*
*/
#pragma once

#include "zce/util/non_copyable.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

#include "zce/mysql/connect.h"
#include "zce/mysql/command.h"

namespace zce::mysql
{
/*!
@brief      读取MYSQL数据库的类,用于操作MySQL DB的访问
            这个类里面包装了connect,command,你可以通过zce::mysql::Result获得结果
*/
class Execute : zce::NON_Copyable
{
public:

    ///构造函数，
    Execute();
    ///析构函数
    ~Execute();

    /*!
    * @brief      初始化服务器,使用hostname进行连接,可以不立即连接和立即连接，你自己控制。
    * @return     int             0成功，-1失败
    * @param      host_name       DB HOST名称,IP地址
    * @param      user            DB USER名称
    * @param      pwd             DB PWD密码
    * @param      port            端口
    * @param      connect_atonce  是否立即连接服务器
    */
    int init_connect(const char* host_name,
                     const char* user,
                     const char* pwd,
                     unsigned int port = MYSQL_PORT,
                     bool connect_atonce = false);

    //!连接Query 服务器,如果希望初始化后进行连接,使用这个函数
    int connect();
    //!断开连接
    void disconnect();

    /*!
    * @brief      用于非SELECT语句(INSERT,UPDATE)，
    * @return     int
    * @param      sql
    * @param      sql_len SQL语句长度
    * @param      num_affect 返回的收到影响的记录条数
    * @param      insert_id  返回的插入的LAST_INSERT_ID
    */
    int query(const char* sql,
              size_t sql_len,
              uint64_t& num_affect,
              uint64_t& insert_id);

    /*!
    * @brief      执行家族的SQL语句,用于SELECT语句,直接转储结果集合的方法
    * @return     int
    * @param      sql  SQL语句
    * @param      sql_len
    * @param      num_affect  返回参数,返回的查询的记录个数
    * @param      db_result  返回参数,查询的结果集合
    * @note       几个query函数连接周期不会关闭链接,zce::mysql::Connect对象再析构时断链接
    */
    int query(const char* sql,
              size_t sql_len,
              uint64_t& num_affect,
              zce::mysql::Result& db_result);

    /*!
    * @brief      用于SELECT语句,用于use_result得到结果集合的方法
    * @return     int
    * @param      sql SQL语句
    * @param      sql_len SQL语句长度
    * @param      db_result 返回的结果结合
    * @note       用于结果集太多,会占用太多内存的的处理,需要一个个取结果,不推荐使用,
    */
    int query(const char* sql,
              size_t sql_len,
              zce::mysql::Result& db_result);

    ///得到MYSQL定义的错误返回
    unsigned int error_message(char* szerr,
                               size_t buflen);

    ///错误语句Str
    const char* error_message();
    ///DB返回的错误ID
    unsigned int error_id();

    ///得到DB访问的语句
    const char* get_query_sql(void);

    ///得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
    ///Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    unsigned int make_real_escape_string(char* tostr,
                                         const char* fromstr,
                                         unsigned int fromlen);

protected:

    ///数据库IP地址
    std::string       db_hostname_;
    ///DB数据库的端口号码
    unsigned int      db_port_;

    ///UNIX soket file名称，或者命名管道名称
    std::string       db_socket_file_;

    ///数据库用户名称
    std::string       db_user_name_;
    ///DB数据库密码ID
    std::string       db_password_;

    ///MYSQL数据库连接对象
    zce::mysql::Connect db_connect_;

    ///MYSQL命令执行对象
    zce::mysql::Command db_command_;
};

namespace execute
{
//!链接MYSQL数据库
int connect(zce::mysql::Connect* db_connect,
            const char* host_name,
            const char* user,
            const char* pwd,
            unsigned int port = MYSQL_PORT);

//!断开链接
void disconnect(zce::mysql::Connect* db_connect);

//!查询，非SELECT语句
int query(zce::mysql::Connect* db_connect,
          const char* sql,
          size_t sql_len,
          uint64_t* num_affect,
          uint64_t* insert_id);

//!查询，SELECT语句
int query(zce::mysql::Connect* db_connect,
          const char* sql,
          size_t sql_len,
          uint64_t* num_affect,
          zce::mysql::Result* db_result);

//!
int query(zce::mysql::Connect* db_connect,
          const char* sql,
          size_t sql_len,
          zce::mysql::Result* db_result);
} //namespace execute
} //namespace zce::mysql

#endif //#if defined ZCE_USE_MYSQL
