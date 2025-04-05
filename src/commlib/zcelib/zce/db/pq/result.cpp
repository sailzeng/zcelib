#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/connect.h"
#include "zce/db/pq/result.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
namespace zce::pq
{
result::result(::PGresult* res) noexcept
{
    set_result(res);
}

result::~result() noexcept
{
    clear();
}

result::result(result&& res) noexcept
{
    pq_result_ = res.pq_result_;
    num_result_row_ = res.num_result_row_;
    num_result_field_ = res.num_result_field_;

    res.pq_result_ = nullptr;
    res.num_result_row_ = 0;
    res.num_result_field_ = 0;
}
result& result::operator=(result&& res) noexcept
{
    clear();
    pq_result_ = res.pq_result_;
    num_result_row_ = res.num_result_row_;
    num_result_field_ = res.num_result_field_;

    res.pq_result_ = nullptr;
    res.num_result_row_ = 0;
    res.num_result_field_ = 0;
    return *this;
}

//放入结果集合
void result::set_result(::PGresult* res)
{
    ZCE_ASSERT(res);

    //如果已经有结果集, 释放原有的结果集,
    clear();

    pq_result_ = res;
    //如果不是一个空的结果集合
    if (pq_result_)
    {
        //得到行数,列数
        num_result_row_ = (size_t)::PQntuples(res);
        num_result_field_ = (size_t)::PQnfields(res);
    }
    return;
}

void result::clear()
{
    if (nullptr != pq_result_)
    {
        ::PQclear(pq_result_);
        pq_result_ = nullptr;
    }
    //行数目，列数目清0
    num_result_row_ = 0;
    num_result_field_ = 0;
}

//! @brief 根据colum返回表定义列域名,注意计算得到的列的名字也可能是空
const char* result::field_name(size_t colum) const
{
    if (colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return nullptr;
    }
    return ::PQfname(pq_result_, (int)colum);
}

//! @brief 根据Field Name得到Field ID,列号 返回-1表示没有找到
size_t result::field_index(const char* fname) const
{
    return (size_t)::PQfnumber(pq_result_, fname);
}

size_t result::field_def_size(size_t colum) const
{
    if (colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return (size_t)-1;
    }
    return (size_t)::PQfsize(pq_result_, (int)colum);
}

//! 根据列号取得其格式，返回0文本，1二进制
int result::field_format(size_t colum) const
{
    return ::PQfformat(pq_result_, (int)colum);
}

//! 根据列号取得类型Oid
::Oid result::field_type(size_t colum) const
{
    if (colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return InvalidOid;
    }
    return ::PQftype(pq_result_, (int)colum);
}

//取得字段的（实际）长度
size_t result::field_length(size_t row, size_t colum) const
{
    if (row > num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return (size_t)-1;
    }
    return (size_t)::PQgetlength(pq_result_, (int)row, (int)colum);
}

//取得字段的数据
const char* result::field_data(size_t row, size_t colum) const
{
    if (row > num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return nullptr;
    }
    return ::PQgetvalue(pq_result_, (int)row, (int)colum);
}
}
#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
