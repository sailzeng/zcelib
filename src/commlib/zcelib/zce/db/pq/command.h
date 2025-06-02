#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

#include "zce/os_adapt/string.h"
#include "zce/buffer/string_buffer.h"
#include "zce/db/pq/connect.h"
#include "zce/db/pq/result.h"

namespace zce::pq
{
class command;

///****************************************************************************************
/// class zce::pq::command  PQ 的绑定参数包装封装类，PQ只能绑定参数，不能绑定结果，
///****************************************************************************************
class bind
{
    friend class zce::pq::command;
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
    void terminate();
    /*!
    * @brief      绑定一个参数
    * @return     int
    * @param      id   参数的下标
    * @param      type 参数类型
    * @param      len  是否为nullptr,
    * @param      fmt  参数的数据的指针
    */
    int tie_one_param(size_t id,
        char* paramdata,
        Oid type,
        int len,
        int fmt);

    //! bind 函数，PQ绑定只能作为参数
    //! 绑定数值类型
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

    //string , string_view 作为文本绑定
    void tie(size_t id, const std::string& val);
    void tie(size_t id, const std::string_view& val);
    //char_buf,作为二进制数据绑定
    void tie(size_t id, const zce::string_buf& val);

    size_t num_bind() const
    {
        return num_bind_;
    }

protected:

    ///绑定的变量个数
    size_t      num_bind_ = 0;

    ///BIND MySQL的封装方式让我不能用vector,
    char** param_value_ = nullptr;
    //
    Oid* param_type_ = nullptr;
    //
    int* param_len_ = nullptr;
    //
    int* param_fmt_ = nullptr;
};

///****************************************************************************************
/// class zce::pq::command
///****************************************************************************************

class command
{
public:

    //构造函数,析构函数
    command(zce::pq::connect& conn);
    ~command() noexcept;

    //避免拷贝
    command(command&&) noexcept = delete;
    command& operator=(command&&) noexcept = delete;
    command(const command&) = delete;
    command& operator=(const command&) = delete;

    void reset(zce::pq::connect& conn)
    {
        conn_ = conn.get_handle();
        stmt_clear();
        stmt_name_[0] = 0;
    }

    const char* error_message()
    {
        return ::PQerrorMessage(conn_);
    }

    int error_no()
    {
        return 0;
    }

    //! 注意：默认情况下，PQ默认是自动提交事务的，
    //! 如果你要使用事务，你需要通过，BEGIN，这些SQL语句来明确要使用事务
    //! 开始一个事务，Begin Transaction，返回0标识成功
    int trans_begin();
    //! 提交事务Commit Transaction,返回0标识成功
    int trans_commit();
    //! 回滚事务Rollback Transaction,返回0标识成功
    int trans_rollback();

    //! 执行DDL SQL语句，什么都不管的那种，
    int execute(std::string_view sqlcmd);
    //! 执行DML SQL语句,不用输出结果集合的那种，INSERT,UPDATE语句等
    int execute(std::string_view sqlcmd,
        size_t& num_affect,
        uint64_t* last_id);
    //! 执行DQL SQL语句,SELECT语句,转储结果集合的那种,
    int execute(std::string_view sqlcmd,
        size_t& num_affect,
        zce::pq::result& pq_res);

    ///stmt 的函数=============================================================
    auto bind_param(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.bind_param_);
    }

    //! STMT 的清理
    void stmt_clear();

    //! 绑定 重置，可以重新绑定参数
    void bind_reset();

    //! STMT 的准备，分析SQL语句，绑定参数和结果
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
        stmt_clear();
        zce::unique_name("STMT",
            stmt_name_,
            sizeof(stmt_name_));
        size_t param_num = stmt_count_sql_param(sql_cmd);
        PGresult* res = ::PQprepare(conn_,
            stmt_name_,
            sql_cmd.data(),
            (int)param_num,
            nullptr);
        if (::PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            ZCE_LOG(RS_ERROR,
                "Failed to prepare SQL : %s : %s\n", sql_cmd.data(),
                ::PQerrorMessage(conn_));
            return -1;
        }
        ::PQclear(res);
        if (param_num > 0)
        {
            bind_param_.initialize(param_num);
            _tie_all_i(std::index_sequence_for<Args...>{}, args...);
        }

        return 0;
    }

    //! STMT 的执行，可以修改绑定参数，然后多次调用这个函数
    int stmt_execute(size_t& num_affect,
        size_t* last_id,
        int res_fmt = FMT_TEXT);

    int stmt_execute(size_t& num_affect,
        zce::pq::result& pq_res,
        int res_fmt = FMT_TEXT);

    template <class bind_type>
    command& operator << (bind_type val)
    {
        bind_param_.tie(current_bind_, val);
        ++current_bind_;
        return *this;
    }

    //! 获取当前STMT SQL绑定的参数个数
    static size_t stmt_count_sql_param(const std::string_view& sql);

protected:

    template<std::size_t... Is, typename... Args>
    void _tie_all_i(std::index_sequence<Is...>, Args && ...args)
    {
        //用,运算符展开参数 fold expression
        (bind_param_.tie(Is, std::forward<Args>(args)), ...);
    }

    int get_result(PGresult* res,
        size_t& num_affect,
        size_t* last_id,
        zce::pq::result* pq_res);
protected:
    //
    ::PGconn* conn_ = nullptr;
    //! stmt 的名字，唯一标示
    char stmt_name_[64] = { 0 };
    //! 绑定的参数
    zce::pq::bind bind_param_;
    //!当前bind绑定SQL语句参数的下标，用于<<函数,,从0开始
    int current_bind_ = 0;
};
}

#endif
