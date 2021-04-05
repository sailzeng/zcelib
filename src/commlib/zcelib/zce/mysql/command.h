/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_command.h
* @author     Sailzeng <sailerzeng@gmail.com>
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

#ifndef ZCE_LIB_MYSQL_DB_COMMAND_H_
#define ZCE_LIB_MYSQL_DB_COMMAND_H_

//如果你要用MYSQL的库，会包含MYSQL的头文件
#if defined ZCE_USE_MYSQL

#include "zce/util/non_copyable.h"
#include "zce/mysql/connect.h"
#include "zce/mysql/result.h"

/*!
* @brief      MYSQL的命令对象，用于处理SQL语句的执行，获得结果集
*
*/
class ZCE_Mysql_Command: public ZCE_NON_Copyable
{

public:
    ///命令对象的构造函数
    ZCE_Mysql_Command();
    ///命令对象的构造函数，指定一个connect
    ZCE_Mysql_Command(ZCE_Mysql_Connect *);
    ///命令对象的析构函数
    ~ZCE_Mysql_Command();

    /*!
    * @brief      设置Command的ZCE_Mysql_Connect
    * @return     int  0成功，-1失败
    * @param      conn 链接对象，必须已经链接成功喔
    */
    int set_connection(ZCE_Mysql_Connect *conn);

    /*!
    * @brief      得到此Command的ZCE_Mysql_Connect对象
    * @return     ZCE_Mysql_Connect*
    */
    inline ZCE_Mysql_Connect *get_connection();

    /*!
    * @brief      设置SQL Command语句,为BIN型的SQL语句准备
    * @param      sqlcmd SQL语句
    * @param      szsql  SQL语句长度
    */
    inline int set_sql_command(const char *sqlcmd, size_t szsql);

    /*!
    * @brief      设置SQL Command语句,TXT,BIN语句都可以
    * @param      sqlcmd SQL语句
    */
    inline int set_sql_command(const std::string &sqlcmd);

    /*!
    * @brief      设置SQL Command语句,动态参数版本
    * @return     int  0成功，-1失败
    * @param      sql_format    格式化的SQL语句，
    * @param      ...           动态参数
    */
    int set_sql_command( const char *sql_format, ...);

    /*!
    * @brief      得到SQL Command语句,TXT型
    * @return     const char*
    */
    const char *get_sql_command() const;

    /*!
    * @brief      得到SQL Command语句,为BIN型语句
    * @return     int  0成功，-1失败
    * @param[out]    cmdbuf   返回字符串
    * @param[in,out] szbuf 字符串buffer的长度
    * @note
    */
    int get_sql_command(char *cmdbuf, size_t &szbuf) const;

    /*!
    * @brief      得到SQL Command语句
    * @param      std::string& 返回的SQL语句
    */
    void get_sql_command(std::string &) const;

    /*!
    * @brief      执行SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      lastid      插入ID等，对于有自增字段的时，(UINT32也许，还不够用，呵呵)
    */
    int execute(uint64_t &num_affect, uint64_t &last_id);

    /*!
    * @brief      执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      sqlresult   返回的结果集合
    */
    int execute(uint64_t &num_affect, ZCE_Mysql_Result &sql_result);

    /*!
    * @brief      执行SQL语句,SELECT语句,USE结果集合的那种,注意其调用的是mysql_use_result,num_affect对它无效
    *             用于结果集太多的处理,一次处理会占用太多内存的的处理,不推荐使用,
    * @return     int
    * @param      sqlresult 返回的结果集合
    */
    int execute(ZCE_Mysql_Result &sqlresult);

#if MYSQL_VERSION_ID > 40100

    /*!
    * @brief      如果一次执行多行SQL语句，这个方法用于取回结果集合
    * @return     int       0表示成功，否则标识失败
    * @param[out] sqlresult 返回的MySQL结果集合
    * @param[out] bstore    使用mysql_store_result取回结果集合，还是mysql_use_result
    */
    int fetch_next_multi_result(ZCE_Mysql_Result &sqlresult, bool bstore = true);

#endif //MYSQL_VERSION_ID > 40100

    ///C++ 的一些习惯用法，为了执着的C++爱好使用者编写,在写这段代码的时候，好像对C++的流颇有兴趣……
    /// =操作符,
    inline ZCE_Mysql_Command &operator =(const char *sqlcmd);
    inline ZCE_Mysql_Command &operator =(const std::string &sqlcmd);

    ///+=操作符号,用于向SQL Command 后部添加STR
    inline ZCE_Mysql_Command &operator +=(const char *sqlcmd);
    inline ZCE_Mysql_Command &operator +=(const std::string &sqlcmd);

    /*!
    * @brief      返回错误消息
    * @return     const char*
    */
    inline const char *get_error_message();

    /*!
    * @brief      返回错误ID
    * @return     unsigned int
    */
    inline unsigned int get_error_no();

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
    int execute(uint64_t *num_affect,
                uint64_t *last_id,
                ZCE_Mysql_Result *sqlresult,
                bool bstore);

protected:
    //命令缓冲buf的大小
    static const size_t INITBUFSIZE = 64 * 1024;

protected:

    ///联接
    ZCE_Mysql_Connect  *mysql_connect_ = NULL;
    ///SQL
    std::string mysql_command_;

    //
    char *sql_buffer_ = NULL;

};

//这些函数都是4.1后的版本功能
#if MYSQL_VERSION_ID > 40100

//得到错误信息
inline int ZCE_Mysql_Command::set_auto_commit(bool bauto)
{
    return mysql_connect_->set_auto_commit(bauto);
}

//得到错误信息
inline int ZCE_Mysql_Command::trans_commit()
{
    return mysql_connect_->trans_commit();
}

//得到错误信息
inline int ZCE_Mysql_Command::trans_rollback()
{
    return mysql_connect_->trans_rollback();
}

#endif //MYSQL_VERSION_ID > 40100

//得到connect 的句柄
inline ZCE_Mysql_Connect *ZCE_Mysql_Command::get_connection()
{
    return mysql_connect_;
}

//得到错误信息
inline const char *ZCE_Mysql_Command::get_error_message()
{
    return mysql_connect_->get_error_message();
}

//得到错误的ID
inline unsigned int ZCE_Mysql_Command::get_error_no()
{
    return mysql_connect_->get_error_no();
}

//SQL预计的赋值，
inline int ZCE_Mysql_Command::set_sql_command(const char *sqlcmd, size_t szsql)
{
    //如果错误,返回
    if (sqlcmd == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    //
    mysql_command_.assign(sqlcmd, szsql);
    return 0;
}

//为TXT,BIN二进制的SQL命令提供的赋值方式 ,
inline int ZCE_Mysql_Command::set_sql_command(const std::string &sqlcmd)
{
    mysql_command_ = sqlcmd;
    return 0;
}

//
inline ZCE_Mysql_Command &ZCE_Mysql_Command::operator =(const char *sqlcmd)
{
    set_sql_command(sqlcmd);
    return *this;
}
//
inline ZCE_Mysql_Command &ZCE_Mysql_Command::operator =(const std::string &sqlcmd)
{
    set_sql_command(sqlcmd);
    return *this;
}

//
inline ZCE_Mysql_Command &ZCE_Mysql_Command::operator +=(const char *sqlcmd)
{
    mysql_command_.append(sqlcmd);
    return *this;
}

inline ZCE_Mysql_Command &ZCE_Mysql_Command::operator +=(const std::string &sqlcmd)
{
    mysql_command_.append(sqlcmd);
    return *this;
}

#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_DB_COMMAND_H_

