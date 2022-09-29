/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/command.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MySQL的SQL处理对象，Command对象
*             最开始，我希望同时保有断言，返回值，和异常3种模式，
*             结果多年以来，一直还是使用返回值的方法，异常与我无爱。
*
*
* @details    这居然是刚来腾讯时写的代码，好像那时候大家推荐用的数据库还是MYSQL 3.23，
*             时间过的飞快，你的执行效率远远赶不上时间的流逝的速度。
*
*/

#pragma once

//如果你要用MYSQL的库，会包含MYSQL的头文件
#if defined ZCE_USE_MYSQL

#include "zce/mysql/connect.h"
#include "zce/mysql/result.h"

namespace zce::mysql
{
class connect;
class result;
/*!
* @brief      MYSQL的命令对象，用于处理SQL语句的执行，获得结果集
*
*/
class command
{
public:
    ///命令对象的构造函数
    command();
    ///命令对象的构造函数，指定一个connect
    command(zce::mysql::connect*);
    ///命令对象的析构函数
    ~command();

    command(const command &) = delete;
    command& operator=(const command&) = delete;

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
    * @brief      设置SQL Command语句,为BIN型的SQL语句准备
    * @param      sqlcmd SQL语句
    * @param      szsql  SQL语句长度
    */
    int set_sql_command(const char* sqlcmd, size_t szsql);

    /*!
    * @brief      设置SQL Command语句,TXT,BIN语句都可以
    * @param      sqlcmd SQL语句
    */
    int set_sql_command(const std::string& sqlcmd);

    /*!
    * @brief      设置SQL Command语句,动态参数版本
    * @return     int  0成功，-1失败
    * @param      sql_format    格式化的SQL语句，
    * @param      ...           动态参数
    */
    int set_sql_command(const char* sql_format, ...);

    /*!
    * @brief      得到SQL Command语句,TXT型
    * @return     const char*
    */
    const char* get_sql_command() const;

    /*!
    * @brief      得到SQL Command语句,为BIN型语句
    * @return     int  0成功，-1失败
    * @param[out]    cmdbuf   返回字符串
    * @param[in,out] szbuf 字符串buffer的长度
    * @note
    */
    int get_sql_command(char* cmdbuf, size_t& szbuf) const;

    /*!
    * @brief      得到SQL Command语句
    * @param      std::string& 返回的SQL语句
    */
    void get_sql_command(std::string&) const;

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

#if MYSQL_VERSION_ID > 40100

    /*!
    * @brief      如果一次执行多行SQL语句，这个方法用于取回结果集合
    * @return     int       0表示成功，否则标识失败
    * @param[out] sqlresult 返回的MySQL结果集合
    * @param[out] bstore    使用mysql_store_result取回结果集合，还是mysql_use_result
    */
    int fetch_next_multi_result(zce::mysql::result& sqlresult,
                                bool bstore = true);

#endif //MYSQL_VERSION_ID > 40100

    ///C++ 的一些习惯用法，为了执着的C++爱好使用者编写,在写这段代码的时候，好像对C++的流颇有兴趣……
    /// =操作符,
    inline command& operator =(const char* sqlcmd);
    inline command& operator =(const std::string& sqlcmd);

    ///+=操作符号,用于向SQL command 后部添加STR
    inline command& operator +=(const char* sqlcmd);
    inline command& operator +=(const std::string& sqlcmd);

    /*!
    * @brief      返回错误消息
    * @return     const char*
    */
    inline const char* error_message();

    /*!
    * @brief      返回错误ID
    * @return     unsigned int
    */
    inline unsigned int error_no();

    //这些函数都是4.1后的版本功能
#if MYSQL_VERSION_ID > 40100

    /*!
    * @brief      设置是否自动提交
    * @return     int
    * @param      bauto
    * @note
    */
    int set_auto_commit(bool bauto);

    /*!
    * @brief      提交事务Commit Transaction
    * @return     int
    * @note
    */
    int trans_commit();

    /*!
    * @brief      回滚事务Rollback Transaction
    * @return     int
    */
    int trans_rollback();

#endif //MYSQL_VERSION_ID > 40100

protected:

    /*!
    * @brief      执行SQL语句,内部的基础函数,让大家共同调用的基础函数
    * @return     int         int  0成功，-1失败
    * @param[out] num_affect  影响的数据条数，或者返回结果的条数
    * @param[out] lastid      最后的插入ID是什么，
    * @param[out] sqlresult   SQL执行后的结果集合
    * @param[out] bstore      使用什么方式获得结果，ture是使用mysql_store_result,false是使用mysql_use_result（需要多次交互）,
    */
    int query(uint64_t* num_affect,
              uint64_t* last_id,
              zce::mysql::result* sqlresult,
              bool bstore);

protected:
    //命令缓冲buf的大小
    static const size_t INITBUFSIZE = 64 * 1024;

protected:

    ///联接
    zce::mysql::connect* mysql_connect_ = NULL;
    ///SQL
    std::string mysql_command_;

    //
    char* sql_buffer_ = NULL;
};
}

#endif //#if defined ZCE_USE_MYSQL
