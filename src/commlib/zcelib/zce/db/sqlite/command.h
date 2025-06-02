/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   command.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月4日
* @brief
*
*
* @details    STMT = prepared statement
*             参数预先声明，
*
*
* @note
*
*/
#pragma once

#include "zce/db/sqlite/connect.h"
#include "zce/buffer/string_buffer.h"

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

namespace zce::sqlite
{
class stmt_result;
class result;

/*!
@brief      SQlite 的命令类，主要用于SQL的预处理和绑定参数
@note       这个类是对sqlite3_stmt的封装，主要用于SQL的预处理和绑定参数，
            用于SQL的处理等，STMT是个好东东，就是理解上麻烦一点。
*/
class command
{
public:
    /*!
    * @brief      构造函数
    * @param      sqlite3_handler  SQlite3的DB封装句柄。
    */
    command(connect& cnn);
    ~command();

    //避免拷贝
    command(command&&) noexcept = delete;
    command& operator=(command&&) noexcept = delete;
    command(const command&) = delete;
    command& operator=(const command&) = delete;

public:

    void reset(connect* sqlite3);

    /*!
    * @brief      销毁SQLITE3的STMT HANDLER,恢复初始化值等。
    * @return     int
    */
    int terminate();

    //!
    sqlite3* get_sqlite3_handler()
    {
        return sqlite3_;
    };
    //!
    sqlite3_stmt* get_sqlite3_stmt()
    {
        return statement_;
    }

    const char* error_message()
    {
        return ::sqlite3_errmsg(sqlite3_);
    }

    //DB返回的错误ID
    int error_no()
    {
        return ::sqlite3_errcode(sqlite3_);
    }

    //! 开始一个事务，Begin Transaction，返回0标识成功
    int trans_begin();
    //! 提交事务Commit Transaction,返回0标识成功
    int trans_commit();
    //! 回滚事务Rollback Transaction,返回0标识成功
    int trans_rollback();

    //! 执行SQL语句，什么都不管的那种，DDL
    int execute(std::string_view sqlcmd);

    //! 执行SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                uint64_t* last_id);

    /*!
    * @brief     执行SQL语句,SELECT语句,转储结果集合的那种,,（二进制的不行）
    * @return     int 返回0表示成功，
    * @param      sqlcmd SQL语句
    * @param      num_affect 查询得到的条数
    * @param      sqlite_res 执行的结果，返回值
    * @note       内部会调用sqlite3_get_table,sqlite3_free_table，
    *             这个函数在SQLite中不是被推荐的函数，建议使用时考虑一下，虽然其
    *             执行查询，确实比sqlite3_exec，方便
    *             另外，这个函数应该不能处理二进制数据，因为你无法得知结果长度
    */
    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                zce::sqlite::result& sqlite_res);

    //! 关闭同步
    int turn_off_synch();

    ///stmt 的函数=============================================================

    /*!
    * @brief      对于SQL语句的?参数，进行绑定，
    * @tparam     T 绑定的参数类型
    * @return     int       返回 0 表示成功，
    * @param      id        绑定的下标，从0开始,注意从0开始是为了内部统一，以及和C语言的习惯，
                            而SQLite的sqlite3_bind_xxx函数都是从1开始的，我们内部统一+1
    * @param      val       SQL语句绑定的参数  id 0->$1 , id 1->$2
    * @note       SQLite STMT和MYSQL的API好像有一些本质区别，看看他的函数,下面没有引用,
    *             SQLite在Bind函数调用的时候就取得了值？至少从函数的参数上可以这样分析
    *             如需要bind blob数据，使用string_buf
    */
    int bind(size_t id, const bool& val);
    int bind(size_t id, const char& val);
    int bind(size_t id, const short& val);
    int bind(size_t id, const int& val);
    int bind(size_t id, const long& val);
    int bind(size_t id, const long long& val);
    int bind(size_t id, const unsigned char& val);
    int bind(size_t id, const unsigned short& val);
    int bind(size_t id, const unsigned int& val);
    int bind(size_t id, const unsigned long& val);
    int bind(size_t id, const unsigned long long& val);
    int bind(size_t id, const float& val);
    int bind(size_t id, const double& val);
    int bind(size_t id, const char* val);
    //string , string_view 作为文本绑定
    int bind(size_t id, const std::string& val);
    int bind(size_t id, const std::string_view& val);
    //char_buf,作为二进制数据绑定
    int bind(size_t id, const zce::string_buf& val);

    //!
    void bind_reset();

    //!我没有提供sqlite3_clear_bindings 的封装，我觉得这个函数不需要用到，使用stmt_reset替代
    //! void clear_bindings(); //清理绑定的参数

    /*!
    * @brief      预处理SQL语句
    * @return     int
    * @param      sqlcmd
    */
    int stmt_prepare(std::string_view sqlcmd);

    /*!
    * @brief      STMT 分析SQL，绑定参数和结果，
    * @return     int
    * @param      sql_cmd 执行的SQL
    * @param      bind_data 绑定的参数和结果
    * @note
    */
    template <typename... Args>
    int stmt_prepare(std::string_view sql_cmd,
                     Args && ...args)
    {
        int ret = stmt_prepare(sql_cmd);
        if (ret != 0)
        {
            return ret;
        }

        size_t args_num = sizeof...(Args);
        assert(args_num == num_bind_);
        if (args_num != num_bind_)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] sqlite3_stmt prepare error, args_num[%zu] != num_bind_[%zu].",
                    args_num,
                    num_bind_);
            return -1;
        }
        if (num_bind_ > 0)
        {
            _tie_all_i(std::index_sequence_for<Args...>{}, args...);
        }
        return 0;
    }

    //!bind绑定参数,列号自动++
    template <class bind_type>
    command& operator << (bind_type& val)
    {
        bind(current_bind_, val);
        ++current_bind_;
        return *this;
    }

    int stmt_execute(size_t& num_affect,
                     uint64_t* last_id);

    int stmt_execute(size_t& num_affect,
                     zce::sqlite::stmt_result& sq_stmt_res);

protected:

    template<std::size_t... Is, typename... Args>
    void _tie_all_i(std::index_sequence<Is...>, Args && ...args)
    {
        //用,运算符展开参数 fold expression
        (this->bind(Is, std::forward<Args>(args)), ...);
    }

    //!* @brief      执行一次stmt SQL，，如果执行成功，返回0，
    //!* @param[out] has_result 返回值,如果有结果返回，置为true
    int stmt_execute(bool& has_result);

protected:

    //! SQLite的DB句柄
    sqlite3* sqlite3_ = nullptr;

    //! SQLite原声的STMT的句柄
    sqlite3_stmt* statement_ = nullptr;

    //! 绑定的变量个数
    size_t      num_bind_ = 0;

    //!当前bind绑定SQL语句参数的下标，用于<<函数,,从0开始
    int current_bind_ = 0;
};
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
