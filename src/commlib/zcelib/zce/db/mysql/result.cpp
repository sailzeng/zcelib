#include "zce/predefine.h"
#include "zce/db/mysql/result.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
//=====================================================================================
//根据字段列ID,得到字段值
const char* cursor::field_data(size_t colum) const
{
    // Check if cursor_row_ is null before dereferencing
    ZCE_ASSERT(cursor_row_ && colum < num_field_);
    if (cursor_row_ == nullptr || colum >= num_field_)
    {
        ZCE_ASSERT(false);
        return nullptr;
    }
    return cursor_row_[colum];
}

//根据字段列ID,得到字段值的指针，长度你自己保证
int cursor::field_data(size_t colum, char* pfdata) const
{
    if (cursor_row_ == nullptr || colum >= num_field_ || pfdata == nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    memcpy(pfdata, cursor_row_[colum], fields_len_[colum]);
    return 0;
}

//根据字段顺序ID,得到字段表结构定义的类型
enum_field_types cursor::field_type(size_t colum) const
{
    ZCE_ASSERT(cursor_row_ && colum < num_field_);
    if (cursor_row_ == nullptr || colum >= num_field_)
    {
        return MYSQL_TYPE_NULL;
    }

    return mysql_fields_[colum].type;
}

//根据Field ID 得到此列值的实际长度
size_t cursor::field_length(size_t colum) const
{
    ZCE_ASSERT(cursor_row_ && colum < num_field_);
    if (cursor_row_ == nullptr || colum >= num_field_)
    {
        ZCE_ASSERT(false);
        return (size_t)-1;
    }
    return (size_t)fields_len_[colum];
}

//根据字段的序列值得到字段值
zce::mysql::field  cursor::get_field(size_t colum) const
{
    ZCE_ASSERT(cursor_row_ && colum < num_field_);
    if (cursor_row_ == nullptr || colum >= num_field_)
    {
        return zce::mysql::field();
    }
    return zce::mysql::field(cursor_row_[colum],
                             fields_len_[colum],
                             mysql_fields_[colum].type);
}

//! 清理
void cursor::clear()
{
    cursor_rowid_ = (size_t)-1;
    cursor_row_ = nullptr;
    fields_len_ = nullptr;
    num_field_ = 0;
    mysql_fields_ = nullptr;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
template<>
int cursor::field(size_t colum, char*& val) const
{
    ZCE_ASSERT(nullptr != cursor_row_ && colum <= num_field_ && nullptr != val);
    //长度不包括结束符号
    memcpy(val, cursor_row_[colum], fields_len_[colum]);
    val[fields_len_[colum]] = '\0';
    return 0;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
//考虑过对于unsigned char *做一些特别处理，后来还是算了,用BINARY去考虑了
template<>
int cursor::field(size_t colum, unsigned char*& val) const
{
    ZCE_ASSERT(nullptr != cursor_row_ && colum <= num_field_ && nullptr != val);

    //长度不包括结束符号
    memcpy(val, cursor_row_[colum], fields_len_[colum]);
    val[fields_len_[colum]] = '\0';
    return 0;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
template<>
int cursor::field(size_t colum, zce::string_buf& val) const
{
    ZCE_ASSERT(nullptr != cursor_row_ && colum < num_field_
               && val.capacity() >= fields_len_[colum]);

    //长度不包括结束符号
    val.assign(cursor_row_[colum], fields_len_[colum]);
    return 0;
}

template<>
int cursor::field(size_t colum, zce::ztm& val) const
{
    //为什么不直接用ztm的字符串转换函数呢，因为MYSQL_TIME的字符串转换有一些特殊地方，比如TIME
    //可能是HHH:MM:SS,而不是HH:MM:SS,所以直接用MYSQL_TIME的转换函数
    MYSQL_TIME mt;
    int ret = zce::from_str<MYSQL_TIME>(cursor_row_[colum], mt);
    if (ret != 0)
    {
        return ret;
    }
    val = zce::make_ztm(&mt);
    return 0;
}

//=====================================================================================
//构造函数
result::result(MYSQL_RES* sqlresult) noexcept
{
    save_result(sqlresult);
}

//析构函数
result::~result() noexcept
{
    // 释放结果集合的内存资源
    if (mysql_result_ != nullptr)
    {
        ::mysql_free_result(mysql_result_);
    }
}

result::result(result&& res) noexcept
{
    //移动资源
    move_result(std::move(res));
}

result& result::operator=(result&& res) noexcept
{
    //清理
    free_result();
    move_result(std::move(res));
    return *this;
}

void result::move_result(result&& res) noexcept
{
    mysql_result_ = res.mysql_result_;
    num_result_row_ = res.num_result_row_;
    num_result_field_ = res.num_result_field_;
    mysql_fields_ = res.mysql_fields_;
    res.mysql_result_ = nullptr;
    res.num_result_row_ = 0;
    res.num_result_field_ = 0;
    res.mysql_fields_ = nullptr;
}

//放入结果集合
void result::save_result(MYSQL_RES* res)
{
    ZCE_ASSERT(res);

    //如果已经有结果集, 释放原有的结果集,
    if (nullptr != mysql_result_)
    {
        mysql_free_result(mysql_result_);
        mysql_result_ = nullptr;
    }

    //清0当前行,列以及当前行长度数组指针
    cursor_.clear();

    //行数目，列数目清0
    num_result_row_ = 0;
    num_result_field_ = 0;

    //列属性指针清0
    mysql_fields_ = 0;

    mysql_result_ = res;

    //如果不是一个空的结果集合
    if (mysql_result_)
    {
        //得到行数,列数
        num_result_row_ = (size_t) ::mysql_num_rows(mysql_result_);
        num_result_field_ = (size_t) ::mysql_num_fields(mysql_result_);

        //列属性指针,其实就是返回一个数组的指针,效率应该是有保障的
        mysql_fields_ = mysql_fetch_fields(mysql_result_);

        cursor_.num_field_ = num_result_field_;
        cursor_.mysql_fields_ = mysql_fields_;
    }

    return;
}

//如果已经有结果集, 释放原有的结果集,
void result::free_result()
{
    //如果已经有结果集, 释放原有的结果集,
    if (nullptr != mysql_result_)
    {
        ::mysql_free_result(mysql_result_);
        mysql_result_ = nullptr;
    }
}

//检索一个结果集合的下一行,最开始从0行开始
bool result::cursor_fetch()
{
    if (mysql_result_ == nullptr)
    {
        return false;
    }

    //检索一个结果集合的下一行
    cursor_.cursor_row_ = ::mysql_fetch_row(mysql_result_);
    //如果NEXT行为空,结束访问
    if (cursor_.cursor_row_ == nullptr)
    {
        return false;
    }
    if (cursor_.cursor_rowid_ == (size_t)-1)
    {
        cursor_.cursor_rowid_ = 0;
    }
    else
    {
        ++cursor_.cursor_rowid_;
    }
    //得到此行所有列的长度
    cursor_.fields_len_ = ::mysql_fetch_lengths(mysql_result_);

    return true;
}

//检索到row_id 行,
bool result::cursor_seek(size_t row_id)
{
    //检查结果集合为空,或者参数row错误
    if (mysql_result_ == nullptr || row_id >= num_result_row_)
    {
        ZCE_ASSERT(false);
        return false;
    }

    ::mysql_data_seek(mysql_result_, row_id);
    cursor_.cursor_row_ = ::mysql_fetch_row(mysql_result_);
    if (cursor_.cursor_row_ == nullptr)
    {
        return false;
    }
    cursor_.cursor_rowid_ = row_id;
    cursor_.fields_len_ = ::mysql_fetch_lengths(mysql_result_);

    return true;
}

//根据字段顺序ID,得到表结构定义的字段长度
size_t result::field_def_size(size_t colum) const
{
    //检查结果集合为空,或者参数colum错误
    if (mysql_result_ == nullptr && colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return (size_t)-1;
    }

    return (size_t)mysql_fields_[colum].length;
}

//根据字段顺序ID,得到字段表结构定义的类型
enum_field_types result::field_type(size_t colum) const
{
    //检查结果集合为空,或者参数nfield错误
    if (mysql_fields_ == nullptr || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return MYSQL_TYPE_NULL;
    }

    return mysql_fields_[colum].type;
}

//根据列名得到列ID,从0开始排序
//循环比较,效率比较低
size_t result::field_index(const char* fname) const
{
    //循环比较所有的列名,效率比较低下
    for (size_t i = 0; i < num_result_field_; ++i)
    {
        //MYSQL列名字是不区分大小写的
        if (!strcasecmp(fname, mysql_fields_[i].name))
        {
            return i;
        }
    }
    return (size_t)-1;
}

//根据列Field ID 返回表定义列域名,列域名字,可能为空
//计算得到的列的列名字也可能是空,
const char* result::field_name(size_t colum) const
{
    //检查结果集合为空,或者参数nfield错误
    if (mysql_result_ == nullptr || colum >= num_result_field_)
    {
        return nullptr;
    }

    //直接得到列域的名字
    return mysql_fields_[colum].name;
}

//根据字段列ID,得到字段值
const char* result::field_data(size_t row, size_t colum)
{
    if (row >= num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return nullptr;
    }
    if (row != cursor_.cursor_rowid_)
    {
        if (cursor_seek(row))
        {
            return nullptr;
        }
    }
    return cursor_.cursor_row_[colum];
}

//根据字段列ID,得到字段值的指针，长度你自己保证
int result::field_data(size_t row,
                       size_t colum,
                       char* pfdata)
{
    if (row >= num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }
    if (row != cursor_.cursor_rowid_)
    {
        if (cursor_seek(row))
        {
            return -1;
        }
    }
    memcpy(pfdata,
           cursor_.cursor_row_[colum],
           cursor_.fields_len_[colum]);
    return 0;
}

//根据Field ID 得到此列值的实际长度
size_t result::field_length(size_t row, size_t colum)
{
    if (row >= num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return (size_t)-1;
    }
    if (row != cursor_.cursor_rowid_)
    {
        if (cursor_seek(row))
        {
            return 0;
        }
    }
    return (size_t)cursor_.fields_len_[colum];
}

//根据字段的序列值得到字段值
zce::mysql::field  result::get_field(size_t row, size_t colum)
{
    if (row >= num_result_row_ || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return zce::mysql::field();
    }
    if (row != cursor_.cursor_rowid_)
    {
        if (cursor_seek(row))
        {
            return zce::mysql::field();
        }
    }

    return zce::mysql::field(cursor_.cursor_row_[colum],
                             cursor_.fields_len_[colum],
                             mysql_fields_[colum].type);
}
}

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
