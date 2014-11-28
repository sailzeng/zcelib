#ifndef ZCE_LIB_SQLITE_STMT_HANDLER_H_
#define ZCE_LIB_SQLITE_STMT_HANDLER_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000

/*!
@brief      SQlite STMT的句柄
            用于SQL的处理等，STMT是个好东东，就是理解上麻烦一点。
*/
class ZCELIB_EXPORT SQLite_STMT_Handler
{
public:

    ///定义二进制数据结构，用于BLOB类型的数据
    struct BINARY
    {
        ///2进制数据的指针
        void         *binary_data_;
        ///二进制数据的长度
        int           binary_len_;
        //
        BINARY(void *binary_data, int binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BINARY()
        {
        }
    };

public:
    /*!
    * @brief      构造函数
    * @param      sqlite3_handler  SQlite3的DB封装句柄。
    */
    SQLite_STMT_Handler(SQLite_DB_Handler *sqlite3_handler);
    /*!
    * @brief      析构函数
    */
    ~SQLite_STMT_Handler();

public:

    /*!
    * @brief      预处理SQL语句
    * @return     int
    * @param      sql_string
    */
    int prepare_sql_string(const char *sql_string);

    /*!
    * @brief      重新初始化STMT的Handler
    * @return     int
    */
    int reset_stmt_handler();

    /*!
    * @brief      销毁SQLITE3的STMT HANDLER,恢复初始化值等。
    * @return     int
    */
    int finalize_stmt_handler();

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

    //
    /*!
    * @brief      得到当前返回列的长度
    * @return     unsigned int
    * @note
    */
    inline int column_bytes()
    {
        return ::sqlite3_column_bytes(sqlite3_stmt_handler_, current_col_);
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

    //SQLite STMT和MYSQL的API好像有一些本质区别，看看他的函数,下面没有引用,
    //SQLite在Bind函数调用的时候就取得了值？至少从函数的参数上可以这样分析
    //如需要bind blob数据，使用BINARY
    template <class bind_type>
    int bind(bind_type val);

    ///二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
    template <class value_type>
    void column(value_type val);


    ///导出结果
    template <class value_type>
    SQLite_STMT_Handler &operator >> (value_type val)
    {
        column<value_type>(val);
        return *this;
    }


protected:

    ///SQLite的DB句柄
    SQLite_DB_Handler    *sqlite_handler_;

    ///SQLite原声的STMT的句柄
    sqlite3_stmt          *sqlite3_stmt_handler_;

    ///当前绑定的,用于<<函数,从1开始
    int                    current_bind_;
    ///当前的列,用于>>函数,从0开始
    int                    current_col_;
};

#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE3_STMT_HANDLER_H_

