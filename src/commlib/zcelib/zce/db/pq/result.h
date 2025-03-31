#pragma once
#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
namespace zce::pq
{
class result
{
public:

    //!构造函数,析构函数
    result() noexcept = default;
    result(::PGresult* res) noexcept;
    ~result() noexcept;

    //避免拷贝
    result(const result&) = delete;
    result& operator=(const result&) = delete;
    result(result&&) noexcept = delete;
    result& operator=(result&&) = delete;

    //! @brief   设置PQ的结果集
    void set_pq_result(::PGresult* res);

    ///结果集合是否为空
    inline bool is_null()
    {
        return pq_result_ ? true : false;
    }

    //! @brief 根据colum返回表定义列域名,注意计算得到的列的名字也可能是空
    const char* field_name(size_t colum) const;

    //! @brief 根据Field Name得到Field ID,列号 返回-1表示没有找到
    size_t field_index(const char* fname) const;

    //! 根据列ID （colum）或者列名称（fname）取得字段定义长度
    size_t field_define_size(size_t colum) const;

    //! 根据列ID （colum）或者列名称（fname）取得字段的（实际）长度
    size_t field_length(size_t row, size_t colum) const;

public:

    //
    ::PGresult* pq_result_ = nullptr;

    ///结果集的行数
    size_t  num_result_row_ = 0;

    ///结果集的列数
    size_t  num_result_field_ = 0;
};
}
#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
