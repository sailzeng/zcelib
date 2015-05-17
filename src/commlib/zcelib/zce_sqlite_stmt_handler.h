#ifndef ZCE_LIB_SQLITE_STMT_HANDLER_H_
#define ZCE_LIB_SQLITE_STMT_HANDLER_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000

/*!
@brief      SQlite STMT的句柄
            用于SQL的处理等，STMT是个好东东，就是理解上麻烦一点。
*/
class ZCE_SQLite_STMTHdl
{
public:

    ///定义二进制数据结构，用于辅助绑定BLOB类型的参数数据
    struct BIN_Param
    {
        /*!
        * @brief      
        * @param      binary_data 二进制数据BUFFER
        * @param      binary_len  数据长度，
        */
        BIN_Param(void *binary_data, int binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BIN_Param()
        {
        }

        ///2进制数据的指针
        void   *binary_data_;
        ///二进制数据的长度,
        int    binary_len_;
    };

    ///定义二进制数据结构，用于辅助绑定BLOB类型的数据结果
    struct BIN_Result
    {
        /*!
        * @brief
        * @param      binary_data 二进制数据BUFFER
        * @param      binary_len  数据长度，初始化为数据长度，使用后记录数据结果长度
        */
        BIN_Result(void *binary_data, int *binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BIN_Result()
        {
        }

        ///2进制数据的指针
        void   *binary_data_;
        ///二进制数据的长度,注意绑定结果时，这个数值座位结果绑定的时候，会辅助返回长度
        int    *binary_len_;
    };


public:
    /*!
    * @brief      构造函数
    * @param      sqlite3_handler  SQlite3的DB封装句柄。
    */
    ZCE_SQLite_STMTHdl(ZCE_SQLite_DB_Handler *sqlite3_handler);
    /*!
    * @brief      析构函数
    */
    ~ZCE_SQLite_STMTHdl();

public:

    /*!
    * @brief      预处理SQL语句
    * @return     int
    * @param      sql_string
    */
    int prepare(const char *sql_string);

    /*!
    * @brief      重新初始化STMT的Handler
    * @return     int
    */
    int reset();

    /*!
    * @brief      销毁SQLITE3的STMT HANDLER,恢复初始化值等。
    * @return     int
    */
    int finalize();

    /*!
    * @brief      执行SQL，第一次是执行SQL，后面移动游标
    * @return     int         0成功，否则失败
    * @param[out] hash_reuslt 返回值,是否有结果
    * note        要执行多次，第一次得到结果集合，后面移动游标。
    */
    int execute_stmt_sql(bool &hash_reuslt);

    ///开始一个事务
    int begin_transaction();
    ///提交一个事务
    int commit_transction();
    ///将同步选项关闭，建议不要使用
    int turn_off_synch();

    ///
    sqlite3_stmt *get_sqlite3_stmt_handler()
    {
        return sqlite3_stmt_handler_;
    }

    ///错误语句Str
    inline const char *error_message()
    {
        return sqlite_handler_->error_message();
    }
    ///DB返回的错误ID
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
        return ::sqlite3_column_bytes(sqlite3_stmt_handler_, result_col);
    }

    /*!
    * @brief      取得列的数量
    * @return     int
    * @param      num_col
    */
    int column_count()
    {
        return ::sqlite3_column_count(sqlite3_stmt_handler_);
    }

    ///当前column的数据长度
    inline int cur_column_bytes()
    {
        return ::sqlite3_column_bytes(sqlite3_stmt_handler_, current_col_);
    }

    /*!
    * @brief      对于SQL语句的?参数，进行绑定，
    * @tparam     bind_type 绑定的参数类型
    * @return     int       返回 0 表示成功，
    * @param      bind_index 绑定的下标，从1开始
    * @param      val       SQL语句绑定的参数
    * @note       SQLite STMT和MYSQL的API好像有一些本质区别，看看他的函数,下面没有引用,
    *             SQLite在Bind函数调用的时候就取得了值？至少从函数的参数上可以这样分析
    *             如需要bind blob数据，使用BINARY
    */
    template <class bind_type>
    int bind(int bind_col, bind_type val);

    /*!
    * @brief      取得列的结果
    * @tparam     value_type 结果的类型
    * @param      result_col 列号，从0开始
    * @param      val 取出的结果
    * @note       二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
    */
    template <class value_type>
    void column(int result_col, value_type val);


    ///导出结果,列号自动++
    template <class value_type>
    ZCE_SQLite_STMTHdl &operator >> (value_type &val)
    {
        column<value_type &>(current_col_, val);
        ++current_col_;
        return *this;
    }



    ///bind绑定参数,列号自动++
    template <class bind_type>
    ZCE_SQLite_STMTHdl &operator << (bind_type val)
    {
        bind<bind_type>(current_bind_, val);
        ++current_bind_;
        return *this;
    }

    //这两个类型的<<函数使用的是引用，所以重载一下，
    ZCE_SQLite_STMTHdl &operator << (const ZCE_SQLite_STMTHdl::BIN_Param &val);
    ZCE_SQLite_STMTHdl &operator << (const std::string &val);


protected:

    ///SQLite的DB句柄
    ZCE_SQLite_DB_Handler *sqlite_handler_;

    ///SQLite原声的STMT的句柄
    sqlite3_stmt *sqlite3_stmt_handler_;

    ///当前取结果的列,用于>>函数,从0开始
    int current_col_;

    ///当前bind绑定SQL语句参数的下标，用于>>函数,,从1开始
    int current_bind_;
};

#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE3_STMT_HANDLER_H_

