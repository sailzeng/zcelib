#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/connect.h"
#include "zce/db/pq/result.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
namespace zce::pq
{
//================================================================================
//! @brief 将PG的时间戳转换为zce::ztm
int result::time::parse_date(int32_t date, zce::ztm* pztm)
{
    ZCE_ASSERT(pztm != nullptr);
    if (pztm == nullptr)
    {
        return -1;
    }
    pztm->fmt_ = TMS_FMT::ISO_DATE_USEC;
    time_t epoch = PG_EPOCH + date * 86400;
    tm mytm = { 0 };
    zce::gmtime_r(&epoch, &mytm);
    pztm->year_ = mytm.tm_year + 1900;
    pztm->mon_ = mytm.tm_mon + 1;
    pztm->day_ = mytm.tm_mday;
    return 0;
}

int result::time::parse_time(double time, zce::ztm* pztm)
{
    ZCE_ASSERT(pztm != nullptr);
    if (pztm == nullptr)
    {
        return -1;
    }
    pztm->fmt_ = TMS_FMT::ISO_TIME_USEC;
    pztm->hour_ = static_cast<int>(time / 3600);
    pztm->min_ = static_cast<int>(fmod(time, 3600) / 60);
    pztm->sec_ = static_cast<int>(fmod(time, 60));
    pztm->usec_ = static_cast<int>((time - floor(time)) * 1'000'000);
    return 0;
}

int result::time::parse_timestamp(int64_t timestamp, zce::ztm* pztm)
{
    ZCE_ASSERT(pztm != nullptr);
    if (pztm == nullptr)
    {
        return -1;
    }
    pztm->fmt_ = TMS_FMT::ISO_TIME_USEC;
    time_t epoch = PG_EPOCH + timestamp / USEC_PER_SEC;
    tm mytm = { 0 };
    zce::gmtime_r(&epoch, &mytm);
    pztm->year_ = mytm.tm_year + 1900;
    pztm->mon_ = mytm.tm_mon + 1;
    pztm->day_ = mytm.tm_mday;
    pztm->usec_ = static_cast<time_t>(timestamp % USEC_PER_SEC);
    return 0;
}

int result::time::parse_interval(interval intvl_val, zce::ztm* pztm)
{
    ZCE_ASSERT(pztm != nullptr);
    if (pztm == nullptr)
    {
        return -1;
    }
    pztm->fmt_ = TMS_FMT::ISO_TIME_USEC;
    pztm->usec_ = intvl_val.time_usec;
    pztm->day_ = intvl_val.days;
    pztm->mon_ = intvl_val.months;
    return 0;
}

//================================================================================
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

template <>
int result::field(size_t row, size_t colum, zce::ztm& val) const
{
    //返回字段为文本类型
    auto ffmt = field_format(colum) == 0;
    if (ffmt == FMT_TEXT)
    {
        return zce::from_str(::PQgetvalue(pq_result_, (int)row, (int)colum), val);
    }
    else if (ffmt == FMT_BINARY)
    {
        zce::ser::decode dc(::PQgetvalue(pq_result_, (int)row, (int)colum),
                            (size_t)::PQgetlength(pq_result_, (int)row, (int)colum));
        PG_OID_TYPE pg_oid = (PG_OID_TYPE)field_type(colum);

        if (pg_oid == PG_DATE)
        {
            int32_t date_val = 0;
            dc.read(date_val);
            return time::parse_date(date_val, &val);
        }
        else if (pg_oid == PG_TIME)
        {
            double time_val = 0;
            dc.read(time_val);
            return time::parse_time(time_val, &val);
        }
        else if (pg_oid == PG_TIMESTAMP || pg_oid == PG_TIMESTAMPTZ)
        {
            int64_t timestamp_val = 0;
            dc.read(timestamp_val);
            return time::parse_timestamp(timestamp_val, &val);
        }
        else if (pg_oid == PG_INTERVAL)
        {
            interval intvl_val;
            dc.read(intvl_val.time_usec);
            dc.read(intvl_val.days);
            dc.read(intvl_val.months);
            return time::parse_interval(intvl_val, &val);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
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
    cursor_row_ = (size_t)-1;
}

bool result::cursor_fetch()
{
    if (pq_result_ == nullptr || cursor_row_ + 1 >= num_result_row_)
    {
        return false;
    }
    ++cursor_row_;
    return true;
}

bool result::cursor_seek(size_t row_id)
{
    if (pq_result_ == nullptr || row_id >= num_result_row_)
    {
        return false;
    }
    cursor_row_ = row_id;
    return true;
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
