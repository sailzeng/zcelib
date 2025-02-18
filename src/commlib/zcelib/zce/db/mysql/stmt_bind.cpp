#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/stmt_bind.h"

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
//构造函数
stmt_bind::stmt_bind(size_t numbind) :
    num_bind_(numbind),
    current_bind_(0),
    stmt_bind_(nullptr)
{
    stmt_bind_ = new MYSQL_BIND[num_bind_];
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND) * num_bind_);
}

//析构函数
stmt_bind::~stmt_bind()
{
    if (stmt_bind_)
    {
        delete[] stmt_bind_;
    }
}

//重新设置
void stmt_bind::reset()
{
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND) * num_bind_);
    current_bind_ = 0;
}

//绑定一个参数
int stmt_bind::bind_one_param(size_t col,
                              ::enum_field_types paramtype,
                              my_bool* is_null,
                              void* paramdata,
                              unsigned long szparam)
{
    ZCE_ASSERT(col < num_bind_);
    if (col >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[col].buffer_type = paramtype;
    stmt_bind_[col].buffer = paramdata;

    stmt_bind_[col].is_null = is_null;
    stmt_bind_[col].length = nullptr;
    stmt_bind_[col].buffer_length = szparam;
    return 0;
}

int stmt_bind::bind_one_result(size_t col,
                               ::enum_field_types paramtype,
                               void* paramdata,
                               unsigned long* szparam)
{
    ZCE_ASSERT(col < num_bind_);
    if (col >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[col].buffer_type = paramtype;
    stmt_bind_[col].buffer = paramdata;

    stmt_bind_[col].buffer_length = *szparam;
    //长度指针保存返回值
    stmt_bind_[col].length = szparam;
    return 0;
}

template<>
void stmt_bind::bind(size_t col, bool& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(char);

    return;
}

//绑定一个char
template<>
void stmt_bind::bind(size_t col, char& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(char);

    return;
}

template<>
void stmt_bind::bind(size_t col, short& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(short);
    return;
}

template<>
void stmt_bind::bind(size_t col, int& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);

    return;
}

template<>
void stmt_bind::bind(size_t col, long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);

    return;
}

template<>
void stmt_bind::bind(size_t col, long long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[col].buffer = (void*)(&val);

    return;
}

template<>
void stmt_bind::bind(size_t col, unsigned char& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void stmt_bind::bind(size_t col, unsigned short& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void stmt_bind::bind(size_t col, unsigned int& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void stmt_bind::bind(size_t col, unsigned long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(unsigned long);
    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void stmt_bind::bind(size_t col, unsigned long long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);
    stmt_bind_[col].buffer_length = sizeof(unsigned long long);
    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void stmt_bind::bind(size_t col, float& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_FLOAT;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);

    return;
}

template<>
void stmt_bind::bind(size_t col, double& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_DOUBLE;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);

    return;
}

template<>
void stmt_bind::bind(size_t col, stmt_bind::bindata& bin_data)
{
    stmt_bind_[col].buffer_type = bin_data.stmt_data_type_;
    stmt_bind_[col].buffer = bin_data.stmt_pdata_;

    //这个可能既是绑定参数,也是绑定结果
    stmt_bind_[col].buffer_length = bin_data.stmt_data_length_;
    stmt_bind_[col].length = nullptr;

    return;
}

template<>
void stmt_bind::bind(size_t col, stmt_bind::timedata& val)
{
    stmt_bind_[col].buffer_type = val.stmt_timetype_;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(val.stmt_ptime_);

    stmt_bind_[col].buffer_length = sizeof(MYSQL_TIME);
    stmt_bind_[col].length = nullptr;

    return;
}

//绑定一个空参数
template<>
void stmt_bind::bind(size_t col, stmt_bind::nulldata& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_NULL;
    stmt_bind_[col].is_null = val.is_null_;

    return;
}

//
int stmt_bind::fetch_next_row() const
{
    int tmpret = ::mysql_stmt_fetch(stmt_);
    if (tmpret != 0)
    {
        return -1;
    }
    return 0;
}

//
int stmt_bind::fetch_column_bind(size_t column,
                                 size_t offset,
                                 MYSQL_BIND* bind) const
{
    int tmpret = ::mysql_stmt_fetch_column(stmt_,
                                           bind,
                                           static_cast<unsigned int>(column),
                                           static_cast<unsigned long>(offset));
    if (0 != tmpret)
    {
        return -1;
    }

    return 0;
}

//
int stmt_bind::seek_result_row(size_t nrow) const
{
    //检查结果集合为空,或者参数row错误
    ::mysql_stmt_data_seek(stmt_, nrow);
    int tmpret = ::mysql_stmt_fetch(stmt_);
    if (0 != tmpret)
    {
        return -1;
    }

    return 0;
}
}

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1