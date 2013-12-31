#ifndef ZCE_LIB_SQLITE3_STMT_HANDLER_H_
#define ZCE_LIB_SQLITE3_STMT_HANDLER_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3003000

/*!
@brief      SQlite STMT的句柄
            用于SQL的处理等，STMT是个好东东，就是理解上麻烦一点。
*/
class ZCELIB_EXPORT SQLite3_STMT_Handler
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
    SQLite3_STMT_Handler(SQLite3_DB_Handler *sqlite3_handler);
    /*!
    * @brief      析构函数
    */
    ~SQLite3_STMT_Handler();

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
    * @brief      执行SQL
    * @return     int         0成功，否则失败
    * @param[out] hash_reuslt 返回值,是否有结果
    */
    int execute_stmt_sql(bool &hash_reuslt);

    /*!
    * @brief      取得列的数量
    * @param      num_col
    */
    void get_column_count(int &num_col);

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
    const char *get_dbret_errstr()
    {
        return sqlite3_db_handler_->get_dbret_errstr();
    }
    ///DB返回的错误ID
    unsigned int get_dbret_errid()
    {
        return sqlite3_db_handler_->get_dbret_errid();
    }

    //得到当前返回列的长度
    unsigned int get_cur_field_length();

    //SQLite STMT和MYSQL的API好像有一些本质区别，看看他的函数,下面没有引用,
    //SQLite在Bind函数调用的时候就取得了值？至少从函数的参数上可以这样分析

    SQLite3_STMT_Handler &operator << (char );
    SQLite3_STMT_Handler &operator << (short );
    SQLite3_STMT_Handler &operator << (int );
    SQLite3_STMT_Handler &operator << (long );
    SQLite3_STMT_Handler &operator << (long long );

    SQLite3_STMT_Handler &operator << (unsigned char );
    SQLite3_STMT_Handler &operator << (unsigned short );
    SQLite3_STMT_Handler &operator << (unsigned int );
    SQLite3_STMT_Handler &operator << (unsigned long );
    SQLite3_STMT_Handler &operator << (unsigned long long );

    SQLite3_STMT_Handler &operator << (float );
    SQLite3_STMT_Handler &operator << (double );

    SQLite3_STMT_Handler &operator << (const char *);
    SQLite3_STMT_Handler &operator << (const std::string &);

    //
    SQLite3_STMT_Handler &operator << (const BINARY &);

    SQLite3_STMT_Handler &operator >> (char &);
    SQLite3_STMT_Handler &operator >> (short &);
    SQLite3_STMT_Handler &operator >> (int &);
    SQLite3_STMT_Handler &operator >> (long &);
    SQLite3_STMT_Handler &operator >> (long long &);

    SQLite3_STMT_Handler &operator >> (unsigned char &);
    SQLite3_STMT_Handler &operator >> (unsigned short &);
    SQLite3_STMT_Handler &operator >> (unsigned int &);
    SQLite3_STMT_Handler &operator >> (unsigned long &);
    SQLite3_STMT_Handler &operator >> (unsigned long long &);

    SQLite3_STMT_Handler &operator >> (float &);
    SQLite3_STMT_Handler &operator >> (double &);

    SQLite3_STMT_Handler &operator >> (char *);
    SQLite3_STMT_Handler &operator >> (std::string &);

    ///二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
    SQLite3_STMT_Handler &operator >> (BINARY &);

protected:

    ///SQLite的DB句柄
    SQLite3_DB_Handler    *sqlite3_db_handler_;

    ///SQLite原声的STMT的句柄
    sqlite3_stmt          *sqlite3_stmt_handler_;

    ///当前绑定的,用于<<函数,从1开始
    int                    current_bind_;
    ///当前的列,用于>>函数,从0开始
    int                    current_col_;
};

#endif //SQLITE_VERSION_NUMBER >= 3003000

#endif //ZCE_LIB_SQLITE3_STMT_HANDLER_H_

