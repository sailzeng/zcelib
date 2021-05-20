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

/*!
@brief      读取MYSQL数据库的类,用于操作MySQL DB的访问
            这个类里面包装了connect,command,你可以通过ZCE_Mysql_Result获得结果
*/
class ZCE_Mysql_Process : zce::NON_Copyable
{
protected:
    //实例子
    static ZCE_Mysql_Process* instance_;

public:

    ///构造函数，
    ZCE_Mysql_Process();
    ///析构函数
    ~ZCE_Mysql_Process();

    /*!
    * @brief      初始化服务器,使用hostname进行连接,可以不立即连接和立即连接，你自己控制。
    * @return     int             0成功，-1失败
    * @param      host_name       DB HOST名称,IP地址
    * @param      user            DB USER名称
    * @param      pwd             DB PWD密码
    * @param      port            端口
    * @param      connect_atonce  是否立即连接服务器
    */
    int init_mysql_server(const char* host_name,
                          const char* user,
                          const char* pwd,
                          unsigned int port = MYSQL_PORT,
                          bool connect_atonce = false);

    /*!
    * @brief      初始化MYSQL，使用UNIX socket file连接(UNIX下)，或者命名管道(Windows下),
    *             只初始化也可以了,只能用于本地
    * @return     int              0成功，-1失败
    * @param      unix_socket_file SOCKET FILE名称或者命名管道名称
    * @param      user             DB USER名称
    * @param      pwd              DB PWD密码
    * @param      connect_atonce   是否立即连接服务器
    */
    int init_mysql_socketfile(const char* unix_socket_file,
                              const char* user,
                              const char* pwd,
                              bool connect_atonce = false);

    ///连接Query 服务器,如果希望初始化后进行连接,使用这个函数
    int connect_mysql_server();
    ///断开连接
    void disconnect_mysql_server();

    /*!
    * @brief      用于非SELECT语句(INSERT,UPDATE)，
    * @return     int
    * @param      sql
    * @param      sqllen SQL语句长度
    * @param      numaffect 返回的收到影响的记录条数
    * @param      insertid  返回的插入的LAST_INSERT_ID
    */
    int db_process_query(const char* sql,
                         size_t sqllen,
                         uint64_t& numaffect,
                         uint64_t& insertid);

    /*!
    * @brief      执行家族的SQL语句,用于SELECT语句,直接转储结果集合的方法
    * @return     int
    * @param      sql  SQL语句
    * @param      sqllen
    * @param      numaffect  返回参数,返回的查询的记录个数
    * @param      dbresult  返回参数,查询的结果集合
    * @note       几个db_process_query函数连接周期不会关闭链接,ZCE_Mysql_Connect对象再析构时断链接
    */
    int db_process_query(const char* sql,
                         size_t sqllen,
                         uint64_t& numaffect,
                         ZCE_Mysql_Result& dbresult);

    /*!
    * @brief      用于SELECT语句,用于use_result得到结果集合的方法
    * @return     int
    * @param      sql SQL语句
    * @param      sqllen SQL语句长度
    * @param      dbresult 返回的结果结合
    * @note       用于结果集太多,会占用太多内存的的处理,需要一个个取结果,不推荐使用,
    */
    int db_process_query(const char* sql,
                         size_t sqllen,
                         ZCE_Mysql_Result& dbresult);

    ///得到MYSQL定义的错误返回
    unsigned int get_return_error(char* szerr, size_t buflen);

    ///错误语句Str
    const char* get_return_error_str();
    ///DB返回的错误ID
    unsigned int get_return_error_id();

    ///得到DB访问的语句
    const char* get_query_sql(void);

    ///得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
    ///Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    unsigned int make_real_escape_string(char* tostr,
                                         const char* fromstr,
                                         unsigned int fromlen);

public:
    ///为了SingleTon类准备
    ///实例赋值
    static void instance(ZCE_Mysql_Process*);
    ///获得实例
    static ZCE_Mysql_Process* instance();
    ///清除实例
    static void clean_instance();

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
    ZCE_Mysql_Connect db_connect_;

    ///MYSQL命令执行对象
    ZCE_Mysql_Command db_command_;
};

#endif //#if defined ZCE_USE_MYSQL
