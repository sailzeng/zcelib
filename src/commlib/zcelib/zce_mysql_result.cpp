
#include "zce_predefine.h"
#include "zce_mysql_result.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

/*********************************************************************************
class ZCE_Mysql_Result
*********************************************************************************/

//构造函数
ZCE_Mysql_Result::ZCE_Mysql_Result():
    mysql_result_(NULL),
    current_row_(NULL),
    current_field_(0),
    fields_length_(NULL),
    num_result_row_(0),
    num_result_field_(0),
    mysql_fields_(NULL)
{
}

//构造函数
ZCE_Mysql_Result::ZCE_Mysql_Result(MYSQL_RES *sqlresult):
    mysql_result_(NULL),
    current_row_(NULL),
    current_field_(0),
    fields_length_(NULL),
    num_result_row_(0),
    num_result_field_(0),
    mysql_fields_(NULL)
{
    set_mysql_result(sqlresult);
}

//析构函数
ZCE_Mysql_Result::~ZCE_Mysql_Result()
{
    // 释放结果集合的内存资源
    if (mysql_result_ != NULL)
    {
        mysql_free_result(mysql_result_);
    }
}

//放入结果集合
void ZCE_Mysql_Result::set_mysql_result(MYSQL_RES *sqlresult)
{
    ZCE_ASSERT(sqlresult);

    //如果已经有结果集, 释放原有的结果集,
    if (NULL != mysql_result_)
    {
        mysql_free_result(mysql_result_);
        mysql_result_  = NULL;
    }

    //清0当前行,列以及当前行长度数组指针
    fields_length_ = NULL;
    current_row_   = NULL;
    current_field_ = 0;

    //行数目，列数目清0
    num_result_row_   = 0;
    num_result_field_ = 0;

    //列属性指针清0
    mysql_fields_        = 0;

    mysql_result_ = sqlresult;

    //如果不是一个空的结果集合
    if (mysql_result_)
    {
        //得到行数,列数
        num_result_row_   = (unsigned int)  mysql_num_rows(mysql_result_);
        num_result_field_ = mysql_num_fields(mysql_result_);

        //列属性指针,其实就是返回一个数组的指针,效率应该是有保障的
        mysql_fields_      = mysql_fetch_fields(mysql_result_);
    }

    return;
}

//如果已经有结果集, 释放原有的结果集,
void ZCE_Mysql_Result::free_result()
{
    //如果已经有结果集, 释放原有的结果集,
    if (NULL != mysql_result_)
    {
        mysql_free_result(mysql_result_);
        mysql_result_  = NULL;
    }
}

//检索一个结果集合的下一行,最开始从0行开始
bool ZCE_Mysql_Result::fetch_row_next()
{
    if (mysql_result_ == NULL)
    {
        return false;
    }

    //检索一个结果集合的下一行
    current_row_ = ::mysql_fetch_row(mysql_result_);

    //如果NEXT行为空,结束访问
    if (current_row_ == NULL )
    {
        return false;
    }

    //得到此行所有列的长度
    fields_length_ = ::mysql_fetch_lengths(mysql_result_);

    current_field_ = 0;
    return true;
}

//检索到row_id 行,
int ZCE_Mysql_Result::seek_row(unsigned int row_id)
{
    //检查结果集合为空,或者参数row错误
    if (mysql_result_ == NULL || row_id >= num_result_row_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    mysql_data_seek(mysql_result_, row_id);
    current_row_   = ::mysql_fetch_row(mysql_result_);
    fields_length_ = ::mysql_fetch_lengths(mysql_result_);
    current_field_ = 0;
    return 0;
}

//根据列序号ID得到字段值,data作为返回值
const char *ZCE_Mysql_Result::field_data(const char *fname) const
{
    //根据列的名字得到Field ID
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    if (ret == -1 || current_row_ == NULL)
    {
        ZCE_ASSERT(false);
        return NULL;
    }

    return current_row_[fid];
}

//根据字段列ID,得到字段值
int ZCE_Mysql_Result::field_data(const char *fname, char *pfdata) const
{
    //根据列的名字得到Field ID
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == NULL || pfdata == NULL)
    {
        ZCE_ASSERT(false);
        return ret;
    }

    //
    memcpy(pfdata , current_row_[fid], fields_length_[fid]);
    return 0;

}

//
int ZCE_Mysql_Result::get_field(const char *fname, ZCE_Mysql_Field &ffield) const
{
    //循环比较所有的列名,效率比较低下
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ffield.set_field(current_row_[fid], fields_length_[fid], mysql_fields_[fid].type);
    return 0;
}

//根据字段名称得到字段表结构定义的类型,效率较低,
//返回-1 表示错误
int ZCE_Mysql_Result::field_type(const char *fname, enum_field_types &ftype) const
{

    //循环比较所有的列名,效率比较低下
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ftype = mysql_fields_[fid].type;
    return 0;
}

//根据Field Name 得到此列值的实际长度
int ZCE_Mysql_Result::field_length(const char *fname, unsigned int &flength ) const
{
    //根据列的名字得到Field ID
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空,或者没有找到相关的列ID
    if (ret == -1 || current_row_ == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = fields_length_[fid];
    return 0;
}

//根据字段顺序ID,得到表结构定义的字段长度
int ZCE_Mysql_Result::field_define_size(unsigned int fieldid, unsigned int &flength) const
{

    //检查结果集合为空,或者参数fieldid错误
    if ( mysql_result_ == NULL && fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = mysql_fields_[fieldid].length;
    return 0;
}

//根据字段名称得到表结构定义的字段长度,效率较低
int ZCE_Mysql_Result::field_define_size(const char *fname, unsigned int &fdefsz) const
{

    //循环比较所有的列名,效率比较低下
    unsigned int fid = 0;
    int ret = field_index(fname, fid);

    //如果结果集为空
    if (ret == -1 || mysql_result_ != NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    fdefsz = mysql_fields_[fid].length;
    return 0;
}

//>>操作是给C++的爱好者准备的，但是其在发生问题是无法报错(参数限制),除非你用异常
//用于结果集合中的当前行，当前列数据输出，输出晚后列值加+1
ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (char &val)
{
    val = 0;

    sscanf(current_row_[current_field_], "%c", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (short &val)
{
    val = 0;

    sscanf(current_row_[current_field_], "%hd", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (int &val)
{
    val = 0;

    sscanf(current_row_[current_field_], "%d", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (long &val)
{
    val = 0;
    //如果结果集为空
    sscanf(current_row_[current_field_], "%ld", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (long long &val)
{
    val = 0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%lld", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned char &val)
{
    val = 0;
    //如果结果集为空
    sscanf(current_row_[current_field_], "%c", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned short &val)
{
    val = 0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%hu", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned long &val)
{
    val = 0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%lu", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned int &val)
{
    val = 0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%u", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned long long &val)
{
    val = 0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%llu", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (float &val)
{
    val = 0.0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%f", &val);
    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (double &val)
{
    val = 0.0;
    //转换以及检查
    sscanf(current_row_[current_field_], "%lf", &val);
    ++current_field_;
    return *this;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (char *val)
{
    ZCE_ASSERT((NULL != val) &&  (NULL != current_row_[current_field_] ));

    //长度不包括结束符号
    memcpy(val , current_row_[current_field_], fields_length_[current_field_]);
    val[fields_length_[current_field_]] = '\0';

    ++current_field_;
    return *this;
}

//对于char *,默认当作是一个字符串,所以末尾增加一个'\0'
//考虑过对于unsigned char *做一些特别处理，后来还是算了,用BINARY去考虑了
ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (unsigned char *val)
{
    ZCE_ASSERT((NULL != val) &&  (NULL != current_row_[current_field_] ));

    //长度不包括结束符号
    memcpy(val , current_row_[current_field_], fields_length_[current_field_] );
    val[fields_length_[current_field_]] = '\0';

    ++current_field_;
    return *this;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (ZCE_Mysql_Result::BINARY *val)
{
    ZCE_ASSERT((NULL != val) &&  (NULL != current_row_[current_field_] ));

    //长度不包括结束符号
    memcpy(val , current_row_[current_field_], fields_length_[current_field_]);

    ++current_field_;
    return *this;
}

ZCE_Mysql_Result &ZCE_Mysql_Result::operator >> (std::string &val)
{
    if (current_row_[current_field_])
    {
        val.assign(current_row_[current_field_], fields_length_[current_field_]) ;
    }
    else
    {
        val = "";
    }

    ++current_field_;
    return *this;
}

//如果你要用MYSQL的库
#endif //#if defined MYSQL_VERSION_ID

