#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
class result;

/*!
* @brief MYSQL_BIND 的包装封装累，
*
* @note  STMT_Bind里面bind的变量数据，是否为nullptr，返回长度，都是指针，
*        外部的保存生命周期，请慎重处理。
*/
class bind_data
{
public:

    bind_data() = default;
    bind_data(size_t num_bind);
    ~bind_data();
    //拷贝构造，拷贝赋值，和移动构造，移动赋值
    bind_data(bind_data&& bind) noexcept;
    bind_data& operator=(bind_data&& bind) noexcept;
    bind_data(const bind_data& bind);
    bind_data& operator=(const bind_data& bind);

    void initialize(size_t num_bind);
    void clear();
    /*!
    * @brief      绑定一个参数
    * @return     int
    * @param      id   参数的下标
    * @param      type 参数类型
    * @param      len  是否为nullptr,
    * @param      fmt  参数的数据的指针
    */
    int bind_one_param(size_t id,
                       char* paramdata,
                       Oid type,
                       int len,
                       int fmt);

    template <class T>
    void bind(size_t id, T& val);

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
/// class command
///****************************************************************************************

class command
{
public:

    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                uint64_t* last_id);

    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                zce::pq::result* pq_res);

protected:
    //
    ::PGconn* conn_ = nullptr;
};
}

#endif
