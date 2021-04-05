/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/stmtcmd.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月17日
* @brief
*
* @details
*
* @note       设计就是无数的选择,我选择我喜欢的和感觉最好的。
*
*/



#ifndef ZCE_LIB_MYSQL_STMT_COMMAND_H_
#define ZCE_LIB_MYSQL_STMT_COMMAND_H_

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

#include "zce/mysql/connect.h"

//STMT函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

/*********************************************************************************
class ZCE_Mysql_STMT_Command
*********************************************************************************/

class ZCE_Mysql_Connect;
class ZCE_Mysql_STMT_Bind;
class ZCE_Mysql_Result;

class ZCE_Mysql_STMT_Command
{



public:
    //
    ZCE_Mysql_STMT_Command();
    //指定一个connect
    ZCE_Mysql_STMT_Command(ZCE_Mysql_Connect *);
    //
    ~ZCE_Mysql_STMT_Command();


    /*!
    * @brief      设置Command的ZCE_Mysql_Connect
    * @return     int
    * @param      ZCE_Mysql_Connect* 设置的链接
    */
    int set_connection(ZCE_Mysql_Connect *);

    //得到此Command的ZCE_Mysql_Connect对象
    inline ZCE_Mysql_Connect *get_connection();

    inline MYSQL_STMT *get_stmt_handle();


    /*!
    * @brief      设置SQL Command语句,为BIN型的SQL语句准备,同时绑定参数,结果
    * @return     int
    * @param      sqlcmd
    * @param      bindparam
    * @param      bindresult
    * @note
    */
    int set_stmt_command(const std::string &sqlcmd,
                         ZCE_Mysql_STMT_Bind *bindparam,
                         ZCE_Mysql_STMT_Bind *bindresult);

    //设置SQL Command语句,TXT,BIN语句都可以,同时绑定参数,结果
    int set_stmt_command(const char *stmtcmd, size_t szsql,
                         ZCE_Mysql_STMT_Bind *bindparam,
                         ZCE_Mysql_STMT_Bind *bindresult);

    //得到SQL Command语句,TXT型
    const char *get_stmt_command() const;
    //得到SQL Command语句,为BIN型语句
    void get_stmt_command(char *, size_t &) const;
    //得到SQL Command语句
    void get_stmt_command(std::string &) const;


    /*!
    * @brief      执行SQL语句,不用输出结果集合的那种
    * @return     int
    * @param      num_affect  返回的影响记录条数
    * @param      lastid      返回的LASTID
    */
    int execute(unsigned int &num_affect, unsigned int &lastid);


    /*!
    * @brief      执行SQL语句,SELECT语句,转储结果集合的那种,
    *             注意这个函数条用的是mysql_stmt_store_result.
    * @return     int
    * @param      num_affect 返回的影响记录条数
    */
    int execute(unsigned int &num_affect);

    //从结果结合取出数据
    int fetch_row_next() const;
    //
    int seek_result_row(unsigned int nrow) const;

    //取得一个
    int  fetch_column(MYSQL_BIND *bind, unsigned int column, unsigned int offset) const;

    //返回结果集的行数目
    inline unsigned int get_num_of_result_rows() const;
    //返回结果集的列数目
    inline unsigned int get_num_of_result_fields() const;

    //将参数转化为MetaData,MySQL的结果集合
    void param_2_metadata(ZCE_Mysql_Result *) const;
    //将结果转化为MetaData,MySQL的结果集合
    void result_2_metadata(ZCE_Mysql_Result *) const;

    // 返回错误消息
    inline const char *get_error_message() const;
    // 返回错误号
    inline unsigned int get_error_no() const;

protected:

    /*!
    * @brief      预处理SQL,并且分析绑定的变量
    * @return     int
    * @param      bindparam    绑定的参数
    * @param      bindresult   绑定的结果
    * @note
    */
    int stmt_prepare_bind(ZCE_Mysql_STMT_Bind *bindparam,
                          ZCE_Mysql_STMT_Bind *bindresult);
    //SQL 执行命令，这个事一个基础函数，内部调用
    int _execute(unsigned int *num_affect, unsigned int *lastid);

protected:
    //命令缓冲buf的大小
    static const size_t SQL_INIT_BUFSIZE = 64 * 1024;

protected:

    ///联接
    ZCE_Mysql_Connect  *mysql_connect_;

    ///STMT SQL 命令
    std::string         stmt_command_;

    ///STMT 的Handle
    MYSQL_STMT         *mysql_stmt_;

    ///是否绑定结果了
    bool               is_bind_result_;

};

//得到connect 的句柄
inline ZCE_Mysql_Connect *ZCE_Mysql_STMT_Command::get_connection()
{
    return mysql_connect_;
}

//返回STMT Handle
inline MYSQL_STMT *ZCE_Mysql_STMT_Command::get_stmt_handle()
{
    return mysql_stmt_;
}

// 返回错误消息
inline const char *ZCE_Mysql_STMT_Command::get_error_message() const
{
    return mysql_stmt_error(mysql_stmt_);
}

// 返回错误号
inline unsigned int ZCE_Mysql_STMT_Command::get_error_no() const
{
    return mysql_stmt_errno(mysql_stmt_);
}

//返回结果集的行数目
inline unsigned int ZCE_Mysql_STMT_Command::get_num_of_result_rows() const
{
    return static_cast <unsigned int>(mysql_stmt_num_rows(mysql_stmt_));
}

//返回结果集的列数目
inline unsigned int ZCE_Mysql_STMT_Command::get_num_of_result_fields() const
{
    return static_cast <unsigned int>(mysql_stmt_field_count(mysql_stmt_));
}

#endif //MYSQL_VERSION_ID >= 40100

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_STMT_COMMAND_H_

