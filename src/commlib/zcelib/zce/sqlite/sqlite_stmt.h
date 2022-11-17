/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   sqlite_stmt.h
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

#include "zce/sqlite/sqlite_hdl.h"

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000

namespace zce
{
/*!
@brief      SQlite STMT的句柄
            用于SQL的处理等，STMT是个好东东，就是理解上麻烦一点。
*/
class sqlite_stmt
{
public:

    //!定义二进制数据结构，用于辅助绑定BLOB类型的参数数据
    struct BLOB_bind
    {
        /*!
        * @brief
        * @param      bind_data 二进制数据BUFFER,const 常量数据
        * @param      bind_size  数据长度，
        */
        BLOB_bind(const void* bind_data, int bind_size) :
            bind_data_(bind_data)
            , bind_size_(bind_size)
        {
        }
        ~BLOB_bind() = default;

        //!2进制数据的指针
        const void* bind_data_;
        //!二进制数据的长度,
        int   bind_size_;
    };

    //!定义二进制数据结构，用于辅助绑定BLOB类型的数据结果
    struct BLOB_column
    {
        /*!
        * @brief
        * @param      binary_data 二进制数据BUFFER
        * @param      binary_len  数据长度，初始化为数据长度，使用后记录数据结果长度
        */
        BLOB_column(void* binary_data, int* binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BLOB_column()
        {
        }

        //!2进制数据的指针
        void* binary_data_;
        //!二进制数据的长度,注意绑定结果时，这个数值座位结果绑定的时候，会辅助返回长度
        int* binary_len_;
    };

public:
    /*!
    * @brief      构造函数
    * @param      sqlite3_handler  SQlite3的DB封装句柄。
    */
    sqlite_stmt(sqlite_hdl* sqlite3_handler);
    /*!
    * @brief      析构函数
    */
    ~sqlite_stmt();

public:

    /*!
    * @brief      预处理SQL语句
    * @return     int
    * @param      sql_string
    */
    int prepare(const char* sql_string);

    /*!
    * @brief      重新初始化STMT的Handler
    * @return     int
    */
    int reset();

    /*!
    * @brief      销毁SQLITE3的STMT HANDLER,恢复初始化值等。
    * @return     int
    */
    int terminate();

    /*!
    * @brief      执行一次stmt SQL，，如果执行成功，返回0，
    * @return     int  0成功，否则失败
    * @param[out] has_result 返回值,如果有结果返回，置为true
    * note        要执行多次，第一次得到结果集合，后面移动游标。
    */
    int step(bool& has_result);

    //!
    sqlite3_stmt* get_sqlite3_stmt_handler()
    {
        return prepared_statement_;
    }

    //!错误语句Str
    inline const char* error_message()
    {
        return sqlite_handler_->error_message();
    }
    //!DB返回的错误ID
    inline  unsigned int error_code()
    {
        return sqlite_handler_->error_code();
    }

    /*!
    * @brief      得到当前返回列的长度
    * @return     int 长度
    * @param[in]  result_col
    * @note
    */
    inline int column_bytes(int result_col)
    {
        return ::sqlite3_column_bytes(prepared_statement_, result_col);
    }

    /*!
    * @brief      取得列的数量
    * @return     int
    * @param      num_col
    */
    int column_count()
    {
        return ::sqlite3_column_count(prepared_statement_);
    }

    //!当前column的数据长度
    inline int cur_column_bytes()
    {
        return ::sqlite3_column_bytes(prepared_statement_, current_col_);
    }

    /*!
    * @brief      对于SQL语句的?参数，进行绑定，
    * @tparam     bind_type 绑定的参数类型
    * @return     int       返回 0 表示成功，
    * @param      bind_index 绑定的下标，从1开始
    * @param      val       SQL语句绑定的参数
    * @note       SQLite STMT和MYSQL的API好像有一些本质区别，看看他的函数,下面没有引用,
    *             SQLite在Bind函数调用的时候就取得了值？至少从函数的参数上可以这样分析
    *             如需要bind blob数据，使用BLOB_bind
    */
    template <class bind_type>
    int bind(int bind_col, bind_type val);

    /*!
    * @brief      取得列的结果
    * @tparam     column_type 结果的类型
    * @param      result_col 列号，从0开始
    * @param      val 取出的结果
    * @note       文本数据自己看看末尾的'\0',而二进制数据不要这样考虑
    *             如需要column blob数据,使用BLOB_column
    */
    template <class column_type>
    void column(int result_col, column_type val);

    //!导出结果,列号自动++
    template <class value_type>
    sqlite_stmt& operator >> (value_type& val)
    {
        column<value_type&>(current_col_, val);
        ++current_col_;
        return *this;
    }

    //!bind绑定参数,列号自动++
    template <class bind_type>
    sqlite_stmt& operator << (bind_type val)
    {
        bind<bind_type>(current_bind_, val);
        ++current_bind_;
        return *this;
    }

    //这两个类型的<<函数使用的是引用，所以重载一下，
    sqlite_stmt& operator << (const sqlite_stmt::BLOB_bind& val);
    sqlite_stmt& operator << (const std::string& val);

protected:

    //!SQLite的DB句柄
    sqlite_hdl* sqlite_handler_ = nullptr;

    //!SQLite原声的STMT的句柄
    sqlite3_stmt* prepared_statement_ = nullptr;

    //!当前取结果的列,用于>>函数,从0开始
    int current_col_ = 0;

    //!当前bind绑定SQL语句参数的下标，用于>>函数,,从1开始
    int current_bind_ = 1;
};
}

#endif //SQLITE_VERSION_NUMBER >= 3005000
