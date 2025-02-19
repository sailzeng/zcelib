/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/stmtbind.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月17日
* @brief
*
* @details
*
* @note
*
*/

#pragma once

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 26812)
#endif

//这些函数都是4.1.2后的版本功能

namespace zce::mysql
{
/*!
* @brief MYSQL_BIND 的包装封装累，
*
* @note  STMT_Bind里面bind的变量数据，是否为nullptr，返回长度，都是指针，
*        外部的保存生命周期，请慎重处理。
*/
class stmt_bind
{
public:

    /*!
    * @brief      仅仅是为了适配 stmt_bind << 的操作符号
    *             绑定2进制参数数据，用于 mysql_stmt_bind_param
    */
    class bindata
    {
        friend class stmt_bind;

    public:

        /*!
        * @brief
        * @param[in] data_type 数据类型，只能是MYSQL_TYPE_BLOB or MYSQL_TYPE_STRING
        * @param[in] pdata 数据指针，就是是写入的(读取)存放的地方数据，
        * @param[in] data_len 数据长度的指针，传入参数表示数据长度，使用后保存是表示写入的数据长度
        */
        bindata(enum_field_types data_type, void* pdata, unsigned long data_len) :
            stmt_data_type_(data_type),
            stmt_pdata_(pdata),
            stmt_data_length_(data_len)
        {
            assert(MYSQL_TYPE_BLOB == stmt_data_type_
                   || MYSQL_TYPE_STRING == stmt_data_type_);
        };
        //
        ~bindata() = default;
    protected:
        //
        enum_field_types  stmt_data_type_ = MYSQL_TYPE_BLOB;
        //
        void* stmt_pdata_ = nullptr;
        //
        unsigned long  stmt_data_length_ = 0;
    };

    /*!
    @brief      仅仅是为了适配 stmt_bind << 的操作符号

    */
    class timedata
    {
        friend class stmt_bind;
    public:
        //
        timedata(enum_field_types timetype, MYSQL_TIME* pstmttime) :
            stmt_timetype_(timetype),
            stmt_ptime_(pstmttime)
        {
            assert(stmt_timetype_ == MYSQL_TYPE_TIMESTAMP ||
                   stmt_timetype_ == MYSQL_TYPE_DATE ||
                   stmt_timetype_ == MYSQL_TYPE_DATETIME ||
                   stmt_timetype_ == MYSQL_TYPE_TIME);
        };
        //
        ~timedata() = default;

    protected:
        //
        enum_field_types  stmt_timetype_ = MYSQL_TYPE_TIMESTAMP;
        //
        MYSQL_TIME* stmt_ptime_ = nullptr;
    };

    /*!
    * @brief      仅仅是为了适配 stmt_bind << 的操作符号
    *             绑定一个空参数
    * @note
    */
    class nulldata
    {
        friend class stmt_bind;
    public:
        nulldata(my_bool* is_null) :
            is_null_(is_null)
        {
        };

        ~nulldata() = default;
    protected:
        my_bool* is_null_ = nullptr;
    };

public:

    /*!
    * @brief      构造函数
    * @param      numbind  要绑定变量,结果的个数
    */
    stmt_bind(size_t numbind);
    ~stmt_bind();
    //定义出来不实现,让你无法用,有很多地方有我分配的指针,不能给你浅度复制
    stmt_bind& operator=(const stmt_bind& others) = delete;

    ///
    void set_stmt(MYSQL_STMT* stmt)
    {
        stmt_ = stmt;
    }

    /*!
    * @brief      绑定一个参数
    * @return     int
    * @param      col       参数的列号
    * @param      paramtype 参数类型
    * @param      bisnull   是否为nullptr,
    * @param      paramdata 参数的数据的指针
    * @param      szparam   参数的长度
    */
    int bind_one_param(size_t col,
                       ::enum_field_types paramtype,
                       my_bool* is_null,
                       void* paramdata,
                       unsigned long szparam = 0);

    /*!
    * @brief
    * @return     int
    * @param[in]     col
    * @param[in]     paramtype
    * @param[in]     paramdata
    * @param[in,out] szparam
    */
    int bind_one_result(size_t col,
                        ::enum_field_types res_type,
                        void* paramdata,
                        unsigned long* szparam);

    ///得到STMT HANDLE
    inline MYSQL_BIND* get_stmt_bind()
    {
        return stmt_bind_;
    }

    inline MYSQL_BIND* operator[](size_t paramno) const
    {
        return &stmt_bind_[paramno];
    }

    ///重新设置
    void reset();

    template <class T>
    void bind(size_t bind_col, T& val);

    template <typename T>
    stmt_bind& operator << (T&& val)
    {
        bind(current_bind_, std::forward<T>(val));
        ++current_bind_;
        return *this;
    }

    //!从STMT结果取出下一行数据
    int fetch_next_row() const;
    //!从STMT结果取某数据
    int seek_result_row(size_t nrow) const;

    //取得一个列的MYSQL_BIND
    int  fetch_column_bind(size_t column, size_t offset, MYSQL_BIND* bind) const;

    //返回结果集的行数目
    size_t result_rows_num() const
    {
        return static_cast <size_t>(::mysql_stmt_num_rows(stmt_));
    }
    //返回结果集的列数目
    size_t result_fields_num() const
    {
        return static_cast <size_t>(::mysql_stmt_field_count(stmt_));
    }

protected:

    ///绑定的变量个数
    size_t      num_bind_ = 0;

    ///当前使用的绑定参数序号,用于<<
    size_t      current_bind_ = 0;

    ///STMT 的Handle
    MYSQL_STMT* stmt_ = nullptr;

    ///BIND MySQL的封装方式让我不能用vector,
    MYSQL_BIND* stmt_bind_ = nullptr;
};

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif
}

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
