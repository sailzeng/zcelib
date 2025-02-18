#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/result.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
//构造函数
result::result() noexcept :
    mysql_result_(nullptr),
    current_row_(nullptr),
    current_field_(0),
    fields_length_(nullptr),
    num_result_row_(0),
    num_result_field_(0),
    mysql_fields_(nullptr)
{
}

//构造函数
result::result(MYSQL_RES* sqlresult) noexcept :
    mysql_result_(nullptr),
    current_row_(nullptr),
    current_field_(0),
    fields_length_(nullptr),
    num_result_row_(0),
    num_result_field_(0),
    mysql_fields_(nullptr)
{
    set_mysql_result(sqlresult);
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

//放入结果集合
void result::set_mysql_result(MYSQL_RES* res)
{
    ZCE_ASSERT(res);

    //如果已经有结果集, 释放原有的结果集,
    if (nullptr != mysql_result_)
    {
        mysql_free_result(mysql_result_);
        mysql_result_ = nullptr;
    }

    //清0当前行,列以及当前行长度数组指针
    fields_length_ = nullptr;
    current_row_ = nullptr;
    current_field_ = 0;

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
        num_result_row_ = (unsigned int)mysql_num_rows(mysql_result_);
        num_result_field_ = mysql_num_fields(mysql_result_);

        //列属性指针,其实就是返回一个数组的指针,效率应该是有保障的
        mysql_fields_ = mysql_fetch_fields(mysql_result_);
    }

    return;
}

//如果已经有结果集, 释放原有的结果集,
void result::free_result()
{
    //如果已经有结果集, 释放原有的结果集,
    if (nullptr != mysql_result_)
    {
        mysql_free_result(mysql_result_);
        mysql_result_ = nullptr;
    }
}

//检索一个结果集合的下一行,最开始从0行开始
bool result::fetch_row()
{
    if (mysql_result_ == nullptr)
    {
        return false;
    }

    //检索一个结果集合的下一行
    current_row_ = ::mysql_fetch_row(mysql_result_);

    //如果NEXT行为空,结束访问
    if (current_row_ == nullptr)
    {
        return false;
    }

    //得到此行所有列的长度
    fields_length_ = ::mysql_fetch_lengths(mysql_result_);

    current_field_ = 0;
    return true;
}

//检索到row_id 行,
int result::seek_row(size_t row_id)
{
    //检查结果集合为空,或者参数row错误
    if (mysql_result_ == nullptr || row_id >= num_result_row_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    mysql_data_seek(mysql_result_, row_id);
    current_row_ = ::mysql_fetch_row(mysql_result_);
    fields_length_ = ::mysql_fetch_lengths(mysql_result_);
    current_field_ = 0;
    return 0;
}

//根据列序号ID得到字段值,data作为返回值
const char* result::field_data(const char* fname) const
{
    //根据列的名字得到Field ID
    size_t fid = 0;
    int ret = field_index(fname, fid);

    if (ret == -1 || current_row_ == nullptr)
    {
        ZCE_ASSERT(false);
        return nullptr;
    }

    return current_row_[fid];
}

//根据字段列ID,得到字段值
int result::field_data(const char* fname, char* pfdata) const
{
    //根据列的名字得到Field ID
    size_t fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == nullptr || pfdata == nullptr)
    {
        ZCE_ASSERT(false);
        return ret;
    }

    //
    memcpy(pfdata, current_row_[fid], fields_length_[fid]);
    return 0;
}

//
int result::get_field(const char* fname, zce::mysql::field& ffield) const
{
    //循环比较所有的列名,效率比较低下
    size_t fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ffield.set_field(current_row_[fid], fields_length_[fid], mysql_fields_[fid].type);
    return 0;
}

//根据字段名称得到字段表结构定义的类型,效率较低,
//返回-1 表示错误
int result::field_type(const char* fname, enum_field_types& ftype) const
{
    //循环比较所有的列名,效率比较低下
    size_t fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ftype = mysql_fields_[fid].type;
    return 0;
}

//根据Field Name 得到此列值的实际长度
int result::field_length(const char* fname, unsigned long& flength) const
{
    //根据列的名字得到Field ID
    size_t fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = fields_length_[fid];
    return 0;
}

//根据字段顺序ID,得到表结构定义的字段长度
int result::field_define_size(unsigned int colum, unsigned int& flength) const
{
    //检查结果集合为空,或者参数colum错误
    if (mysql_result_ == nullptr && colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = mysql_fields_[colum].length;
    return 0;
}

//根据字段名称得到表结构定义的字段长度,效率较低
int result::field_define_size(const char* fname, unsigned int& fdefsz) const
{
    //循环比较所有的列名,效率比较低下
    size_t fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空
    if (ret == -1 || mysql_result_ != nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    fdefsz = mysql_fields_[fid].length;
    return 0;
}
//>>操作是给C++的爱好者准备的，但是其在发生问题是无法报错(参数限制),除非你用异常
//用于结果集合中的当前行，当前列数据输出，输出晚后列值加+1
template<>
void result::field(size_t colum, bool& val) const
{
    val = false;

    int fields = sscanf(current_row_[colum], "%c", (char*)&val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, char& val) const
{
    val = 0;

    int fields = sscanf(current_row_[colum], "%c", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, short& val) const
{
    val = 0;

    int fields = sscanf(current_row_[colum], "%hd", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, int& val) const
{
    val = 0;

    int fields = sscanf(current_row_[colum], "%d", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, long& val) const
{
    val = 0;
    //如果结果集为空
    int fields = sscanf(current_row_[colum], "%ld", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, long long& val) const
{
    val = 0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%lld", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, unsigned char& val) const
{
    val = 0;
    //如果结果集为空
    int fields = sscanf(current_row_[colum], "%c", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, unsigned short& val) const
{
    val = 0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%hu", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, unsigned long& val) const
{
    val = 0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%lu", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, unsigned int& val) const
{
    val = 0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%u", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, unsigned long long& val) const
{
    val = 0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%llu", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, float& val) const
{
    val = 0.0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%f", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

template<>
void result::field(size_t colum, double& val) const
{
    val = 0.0;
    //转换以及检查
    int fields = sscanf(current_row_[colum], "%lf", &val);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
template<>
void result::field(size_t colum, char* &val) const
{
    ZCE_ASSERT((nullptr != val) && (nullptr != current_row_[current_field_]));

    //长度不包括结束符号
    memcpy(val, current_row_[colum], fields_length_[colum]);
    val[fields_length_[colum]] = '\0';

    return;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
//考虑过对于unsigned char *做一些特别处理，后来还是算了,用BINARY去考虑了
template<>
void result::field(size_t colum, unsigned char* &val) const
{
    ZCE_ASSERT((nullptr != val) && (nullptr != current_row_[current_field_]));

    //长度不包括结束符号
    memcpy(val, current_row_[colum], fields_length_[colum]);
    val[fields_length_[colum]] = '\0';

    return;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
template<>
void result::field(size_t colum, result::BINARY*& val) const
{
    ZCE_ASSERT((nullptr != val) && (nullptr != current_row_[colum]));

    //长度不包括结束符号
    memcpy(val, current_row_[colum], fields_length_[colum]);

    return;
}

template<>
void result::field(size_t colum, std::string& val) const
{
    if (current_row_[current_field_])
    {
        val.assign(current_row_[colum], fields_length_[colum]);
    }
    else
    {
        val = "";
    }
    return;
}

//根据列名得到列ID,从0开始排序
//循环比较,效率比较低
inline int result::field_index(const char* fname, size_t& field_id) const
{
    //循环比较所有的列名,效率比较低下
    for (unsigned int i = 0; i < num_result_field_; ++i)
    {
        //MYSQL列名字是不区分大小写的
        if (!strcasecmp(fname, mysql_fields_[i].name))
        {
            field_id = i;
            return 0;
        }
    }
    return -1;
}

//根据列Field ID 返回表定义列域名,列域名字,可能为空
//计算得到的列的列名字也可能是空,
char* result::field_name(size_t colum) const
{
    //检查结果集合为空,或者参数nfield错误
    if (mysql_result_ == nullptr || colum >= num_result_field_)
    {
        return nullptr;
    }

    //直接得到列域的名字
    return mysql_fields_[colum].name;
}

//返回结果集的行数目,num_result_row_ 结果在execute函数中也可以得到
unsigned int result::num_of_rows() const
{
    return num_result_row_;
}

//返回结果集的列数目
unsigned int result::num_of_fields() const
{
    return num_result_field_;
}

//根据字段列ID,得到字段值
const char* result::field_data(size_t colum) const
{
    if (current_row_ == nullptr || colum >= num_result_field_)
    {
        return nullptr;
    }

    return current_row_[colum];
}

//根据字段列ID,得到字段值的指针，长度你自己保证
int result::field_data(size_t colum, char* pfdata) const
{
    //检查结果集合的当前行为空(可能没有fetch_row_next),或者参数colum错误
    if (current_row_ == nullptr || colum >= num_result_field_ || pfdata == nullptr)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    memcpy(pfdata, current_row_[colum], fields_length_[colum]);
    return 0;
}

//根据字段顺序ID,得到字段表结构定义的类型
int result::field_type(size_t colum, enum_field_types& ftype) const
{
    //检查结果集合为空,或者参数nfield错误
    if (current_row_ == nullptr || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ftype = mysql_fields_[colum].type;
    return 0;
}

//根据Field ID 得到此列值的实际长度
int result::field_length(size_t colum, unsigned long& flength) const
{
    //检查结果集合的当前行为空(可能没有fetch_row_next),或者参数colum错误
    if (current_row_ == nullptr && colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = fields_length_[colum];
    return 0;
}

unsigned long result::get_cur_field_length()
{
    return static_cast<unsigned long>(fields_length_[current_field_]);
}

//根据字段的序列值得到字段值
int result::get_field(size_t colum, zce::mysql::field& ffield) const
{
    //进行安全检查，如果错误返回
    if (current_row_ == nullptr || colum >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ffield.set_field(current_row_[colum], fields_length_[colum], mysql_fields_[colum].type);
    return 0;
}

//根据列序号ID得到字段FIELD，
//[]操作符号函数不检查检查列ID,自己保证参数
zce::mysql::field result::operator[](size_t colum) const
{
    zce::mysql::field ffield(current_row_[colum],
                             fields_length_[colum],
                             mysql_fields_[colum].type);
    return ffield;
}
}

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1