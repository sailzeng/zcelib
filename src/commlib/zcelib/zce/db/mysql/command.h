///
#pragma once

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
//如果你要用MYSQL的库

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 26812)
#endif

//STMT都是4.1.2后的版本功能

class command;
class result;
class connect;

//======================================================================
/*!
* @brief MYSQL_BIND 的包装封装累，
*
* @note  STMT_Bind里面bind的变量数据，是否为nullptr，返回长度，都是指针，
*        外部的保存生命周期，请慎重处理。
*/
class bind
{
    friend class zce::mysql::command;
public:
    /*!
    * @brief      仅仅是为了适配 stmt_bind 的绑定操作
    *             绑定文本，2进制参数数据，用于 mysql_stmt_bind_param
    */
    class blob
    {
        friend class zce::mysql::bind;
    public:
        /*!
        * @brief
        * @param[in] data_type 数据类型，是MYSQL_TYPE_VARCHAR MYSQL_TYPE_BLOB or MYSQL_TYPE_TINY_BLOB
        *                      MYSQL_TYPE_MEDIUM_BLOB MYSQL_TYPE_LONG_BLOB MYSQL_TYPE_VAR_STRING
        *                      MYSQL_TYPE_STRING
        * @param[in] blob_ptr 数据指针，就是是写入的(读取)存放的地方数据，
        * @param[in/out] blob_len 数据长度的指针，传入参数表示数据长度，使用后保存是表示写入的数据长度
        */
        blob(enum_field_types data_type,
             void* blob_ptr,
             unsigned long* blob_len) :
            bind_type_(data_type),
            blob_ptr_(blob_ptr),
            blob_len_(blob_len)
        {
            assert(MYSQL_TYPE_VARCHAR == bind_type_
                   || MYSQL_TYPE_BLOB == bind_type_
                   || MYSQL_TYPE_TINY_BLOB == bind_type_
                   || MYSQL_TYPE_MEDIUM_BLOB == bind_type_
                   || MYSQL_TYPE_LONG_BLOB == bind_type_
                   || MYSQL_TYPE_VAR_STRING == bind_type_
                   || MYSQL_TYPE_STRING == bind_type_);
        };
        //
        ~blob() = default;
    protected:
        //
        enum_field_types  bind_type_ = MYSQL_TYPE_BLOB;
        //
        void* blob_ptr_ = nullptr;
        //
        unsigned long* blob_len_ = 0;
    };

    /*!
    @brief      仅仅是为了适配 zce::mysql::bind_data 的bind
    */
    class time
    {
        friend class zce::mysql::bind;
    public:
        //
        time(enum_field_types timetype, MYSQL_TIME* pstmttime) :
            stmt_timetype_(timetype),
            stmt_ptime_(pstmttime)
        {
            assert(stmt_timetype_ == MYSQL_TYPE_TIMESTAMP ||
                   stmt_timetype_ == MYSQL_TYPE_DATE ||
                   stmt_timetype_ == MYSQL_TYPE_DATETIME ||
                   stmt_timetype_ == MYSQL_TYPE_TIME);
        };
        //
        ~time() = default;

    protected:
        //
        enum_field_types  stmt_timetype_ = MYSQL_TYPE_TIMESTAMP;
        //
        MYSQL_TIME* stmt_ptime_ = nullptr;
    };

    /*!
    * @brief      仅仅是为了适配 zce::mysql::bind_data 的bind
    *             绑定一个空参数
    * @note
    */
    class null
    {
        friend class zce::mysql::bind;
    public:
        null(my_bool* is_null) :
            is_null_(is_null)
        {
        };
        ~null() = default;
    protected:
        my_bool* is_null_ = nullptr;
    };

public:

    bind() = default;
    bind(size_t num_bind);
    ~bind();
    //拷贝构造，拷贝赋值，和移动构造，移动赋值
    bind(bind&& bind) noexcept;
    bind& operator=(bind&& bind) noexcept;
    bind(const bind& bind);
    bind& operator=(const bind& bind);

    void initialize(size_t num_bind);
    void clear();
    /*!
    * @brief      绑定一个参数
    * @return     int
    * @param      id        参数的下标
    * @param      paramtype 参数类型
    * @param      bisnull   是否为nullptr,
    * @param      paramdata 参数的数据的指针
    * @param      szparam   参数的长度
    */
    int tie_one_param(size_t id,
                      ::enum_field_types paramtype,
                      my_bool* is_null,
                      void* paramdata,
                      unsigned long szparam = 0);

    /*!
    * @brief
    * @return     int
    * @param[in]     id        参数的下标
    * @param[in]     paramtype
    * @param[in]     resdata
    * @param[in,out] szres
    */
    int tie_one_result(size_t id,
                       ::enum_field_types res_type,
                       void* resdata,
                       unsigned long* szres);

    ///得到STMT HANDLE
    inline MYSQL_BIND* get_stmt_bind()
    {
        return stmt_bind_;
    }

    inline MYSQL_BIND* operator[](size_t id) const
    {
        return &stmt_bind_[id];
    }

    //! bind 函数，如果没有特殊说明，既可以用于绑定参数，也可以用于绑定结果,
    //! 绑定数值类型 id 从0开始，
    void tie(size_t id, bool& val);
    void tie(size_t id, char& val);
    void tie(size_t id, short& val);
    void tie(size_t id, int& val);
    void tie(size_t id, long& val);
    void tie(size_t id, long long& val);
    void tie(size_t id, unsigned char& val);
    void tie(size_t id, unsigned short& val);
    void tie(size_t id, unsigned int& val);
    void tie(size_t id, unsigned long& val);
    void tie(size_t id, unsigned long long& val);
    void tie(size_t id, float& val);
    void tie(size_t id, double& val);

    //! 绑定文本，二进制
    void tie(size_t id, zce::mysql::bind::blob& val);
    //! 绑定MYSQL_TIME
    void tie(size_t id, zce::mysql::bind::time& val);
    void tie(size_t id, zce::mysql::bind::null& val);

    //! 绑定string 作为参数，只能作为参数绑定，不能作为结果绑定
    void tie(size_t id, const std::string& val);
    //! 绑定string_view 作为参数，只能作为参数绑定，不能作为结果绑定
    void tie(size_t id, const std::string_view& val);

    size_t num_bind() const
    {
        return num_bind_;
    }
protected:

    ///绑定的变量个数
    size_t      num_bind_ = 0;

    ///BIND MySQL的封装方式让我不能用vector,
    MYSQL_BIND* stmt_bind_ = nullptr;
};

///****************************************************************************************
/// class command
///****************************************************************************************

/*!
* @brief      MYSQL的Handle,负责连接，命令执行等
*/
class command
{
public:

    //构造函数,析构函数
    command(zce::mysql::connect& conn);
    ~command() noexcept;

    //避免拷贝
    command(command&&) noexcept = delete;
    command& operator=(command&&) noexcept = delete;
    command(const command&) = delete;
    command& operator=(const command&) = delete;

    //!得到STMT的句柄
    inline MYSQL_STMT* get_stmt()
    {
        return stmt_;
    }

    //! 注意：默认情况下，MySQL默认是自动提交事务的，如果你要使用事务，你需要通过，
    //! ::mysql_autocommit，关闭自动提交
    //! 开始一个事务，Begin Transaction，返回0标识成功
    int trans_begin();
    //! 提交事务Commit Transaction,返回0标识成功
    int trans_commit();
    //! 回滚事务Rollback Transaction,返回0标识成功
    int trans_rollback();

    /*!
    * @brief      执行SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      lastid      插入ID等，对于有自增字段的时，(UINT32也许，还不够用，呵呵)
    */
    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                uint64_t* last_id);

    /*!
    * @brief      执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
    * @return     int         0成功，-1失败
    * @param      num_affect  查询得到的条数
    * @param      sqlresult   返回的结果集合
    */
    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                zce::mysql::result& my_res);

    //! 执行SQL语句，什么都不管的那种，DDL
    int execute(std::string_view sqlcmd);

    /*!
    * @brief 使用mysql_use_result取回结果集合，一次取一行
    * @param res 返回的MySQL结果集合
    * @return int       0表示成功，否则标识失败
    */
    int fetch_next_row(zce::mysql::result& res);

    /*!
    * @brief      如果一次执行多行SQL语句，这个方法用于取回结果集合
    * @return     int       0表示成功，否则标识失败
    * @param[out] res 返回的MySQL结果集合
    * @param[out] use_result 是否使用mysql_use_result取回结果集合，默认false，使用mysql_store_result
    */
    int fetch_next_result(zce::mysql::result& res,
                          bool use_result = false);

    /*!
    * @brief      编码转换，得到Real Escape String ,Real表示根据
    *             当前的MYSQL Connet的字符集,得到Escape String
    *             Escape String 为将字符传中的相关字符进行转义后的语
    *             句,比如',",\等字符
    * @return     size_t 编码后字符串的长度
    * @param      tostr        转换得到的字符串,最好保证有fromlen *2的长度
    * @param      fromstr      进行转换的字符串
    * @param      fromlen      转换的字符串长度
    */
    size_t real_escape_string(char* tostr,
                              const char* fromstr,
                              unsigned int fromlen);

    ///stmt 的函数-----------------------------------------------------

    /*!
    * @brief      预处理SQL,并且分析绑定的变量
    * @return     int
    * @param      bind_param    绑定的参数
    * @param      bind_result   绑定的结果
    * @note
    */
    int stmt_prepare(std::string_view sqlcmd,
                     const zce::mysql::bind* bind_param,
                     zce::mysql::bind* bind_result = nullptr);

    /*!
    * @brief      STMT 分析SQL，绑定参数和结果，然后执行
    * @return     int
    * @param      sql_cmd 执行的SQL
    * @param      param_num 绑定参数数量，bind_data的数量必须大于等于param_num，
    *                       多出部分视为绑定的结果
    * @param      bind_data 绑定的参数和结果
    * @note
    */
    template <typename... Args>
    int stmt_prepare(std::string_view sqlcmd,
                     size_t param_num,
                     Args && ...args)
    {
        ZCE_ASSERT(param_num <= sizeof...(Args));
        size_t result_num = sizeof...(Args) - param_num;
        int ret = ::mysql_stmt_prepare(stmt_,
                                       sqlcmd.data(),
                                       static_cast<unsigned long>(sqlcmd.size()));
        if (ret != 0)
        {
            return ret;
        }
        bind_param_.initialize(param_num);
        bind_result_.initialize(result_num);
        _tie_all_i(param_num, std::index_sequence_for<Args...>{}, args...);
        //绑定的参数
        if (param_num > 0)
        {
            ret = ::mysql_stmt_bind_param(stmt_,
                                          bind_param_.get_stmt_bind());
            if (ret != 0)
            {
                return ret;
            }
        }

        //绑定的结果
        if (result_num > 0)
        {
            ret = ::mysql_stmt_bind_result(stmt_,
                                           bind_result_.get_stmt_bind());
            //出错返回,或者处理
            if (ret != 0)
            {
                return ret;
            }
            is_bind_result_ = true;
        }

        return 0;
    }

    /*!
    * @brief      STMT 执行，并取回结果，
    *             在stmt_prepare调用这个函数，可以在修改bind参数后，再次调用
    * @return     int
    * @param      num_affect  返回的影响记录条数
    * @param      lastid      返回的LASTID
    */
    int stmt_execute(size_t* num_affect,
                     size_t* last_id);

    //! 从STMT 返回的结果取出下一行数据，因为已经bind了结果，相当于将结果集的
    //! 数据写入bind_data
    int stmt_fetch_next_row() const;

    //!从STMT结果取某数据
    int stmt_seek_result_row(size_t nrow) const;

    //取得一个列的MYSQL_BIND
    int  stmt_fetch_column(size_t column,
                           size_t offset,
                           zce::mysql::bind* bind_colum) const;

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

    //
    void stmt_param_2_metadata(result* res) const
    {
        MYSQL_RES* myres = ::mysql_stmt_param_metadata(stmt_);
        res->save_result(myres);
        return;
    }

    //
    void stmt_result_2_metadata(result* res) const
    {
        MYSQL_RES* myres = ::mysql_stmt_result_metadata(stmt_);
        res->save_result(myres);
        return;
    }

    //! STMT 的清理
    void stmt_clear();

protected:

    /*!
    * @brief      执行SQL语句,内部的基础函数,让大家共同调用的基础函数
    * @return     int         int  0成功，-1失败
    * @param[out] num_affect  影响的数据条数，或者返回结果的条数
    * @param[out] lastid      最后的插入ID是什么，
    * @param[out] my_res      SQL执行后的结果集合
    * @param[out] use_result  是否使用mysql_use_result 取结果，默认为false，（需要多次交互）
    *                         默认使用store mysql_store_result 方式使用什么方式获得结果，,
    */
    int get_result(size_t& num_affect,
                   size_t* last_id,
                   zce::mysql::result* my_res,
                   bool use_result = false);
public:

    /*!
    * @brief      得到转意后的Escaple String ,没有根据当前的字符集合进行操作,
    *             Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    *             为什么采用这样的奇怪参数顺序,因为mysql_escape_string
    * @return     unsigned int 编码后字符串的长度
    * @param      tostr        转换得到的字符串,最好保证有fromlen *2的长度
    * @param      fromstr      进行转换的字符串
    * @param      fromlen      转换的字符串长度
    */
    static size_t escape_string(char* tostr,
                                const char* fromstr,
                                unsigned int fromlen);

protected:

    template<std::size_t... Is, typename... Args>
    void _tie_all_i(size_t param_num, std::index_sequence<Is...>, Args && ...args)
    {
        //用,运算符展开参数 fold expression
        //在展开过程，Is如果小于param_num,就绑定参数，否则绑定结果
        ((Is <= param_num ? bind_param_.tie(Is, std::forward<Args>(args)) :
         bind_result_.tie(Is - param_num, std::forward<Args>(args))), ...);
    }

private:

    ///MYSQL的句柄
    MYSQL* mysql_;

    ///STMT 的Handle
    MYSQL_STMT* stmt_ = nullptr;

    ///
    zce::mysql::bind bind_param_;
    ///
    zce::mysql::bind bind_result_;

    ///
    bool is_bind_result_ = false;
};
}

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
