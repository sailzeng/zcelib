#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_mysql_field.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

//High Speed 得到String
void ZCE_Mysql_Field::get_string(std::string &tmpstr) const
{
    tmpstr.reserve(field_length_);
    tmpstr.assign(field_data_, field_length_);
}

//得到字符型
ZCE_Mysql_Field::operator char() const
{
    char  tmp_tinyint = 0;

    short tmpshort = 0;
    //得到短整型
    int fields = sscanf(field_data_, "%hd", &tmpshort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //通过并得到TINY INT 就是char
    tmp_tinyint = static_cast<char>(tmpshort & 0xFF);
    return tmp_tinyint;
}

//得到短整型
ZCE_Mysql_Field::operator short() const
{
    short tmp_short = 0;

    //得到短整型,返回值=0 ,EOF表示失败
    int fields = sscanf(field_data_, "%hd", &tmp_short);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //转换不成功或者
    return tmp_short;
}

//得到长整型
ZCE_Mysql_Field::operator long() const
{
    long tmp_long = 0;

    //得到长整型,
    int fields = sscanf(field_data_, "%ld", &tmp_long);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_long;
}

//得到int整型
ZCE_Mysql_Field::operator int() const
{
    int tmp_int = 0;

    //得到长整型,
    int fields = sscanf(field_data_, "%d", &tmp_int);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_int;
}

//得到int整型
ZCE_Mysql_Field::operator unsigned int() const
{
    unsigned int tmp_uint = 0;

    //得到长整型,
    int fields = sscanf(field_data_, "%u", &tmp_uint);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_uint;
}

//转换为INT64类型
inline ZCE_Mysql_Field::operator long long() const
{
    long long tmp_longlong = 0;

    //得到64位的整数
    int fields = sscanf(field_data_, "%lld", &tmp_longlong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_longlong;
}

//转换为8位无符号整型
ZCE_Mysql_Field::operator unsigned char() const
{
    unsigned char  tmp_utinyint = 0;
    unsigned short tmpshort = 0;
    //得到短整型
    int fields = sscanf(field_data_, "%hu", &tmpshort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //通过并得到TINY INT 就是char
    tmp_utinyint = static_cast<unsigned char>(tmpshort & 0xFF);
    return tmp_utinyint;
}

//转换为16位的无符号整型
ZCE_Mysql_Field::operator unsigned short() const
{
    unsigned short tmp_ushort = 0;

    //得到短整型,返回值=0 ,EOF表示失败
    int fields = sscanf(field_data_, "%hu", &tmp_ushort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //转换不成功或者
    return tmp_ushort;
}
//转换为32位32无符号长整型
ZCE_Mysql_Field::operator unsigned long() const
{
    long tmp_ulong = 0;

    //得到长整型,
    int fields = sscanf(field_data_, "%lu", &tmp_ulong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_ulong;
}

//转换为64位无符号长整型
ZCE_Mysql_Field::operator unsigned long long() const
{
    unsigned long long tmp_ulonglong = 0;

    //得到64位的整数
    int fields = sscanf(field_data_, "%llu", &tmp_ulonglong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_ulonglong;
}

//转换为FLOAT类型
ZCE_Mysql_Field::operator float() const
{
    float tmpfloat = 0.0;
    //转换得到FLOAT浮点型
    int fields = sscanf(field_data_, "%f", &tmpfloat);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmpfloat;
}

//转换为DOUBLE类型
ZCE_Mysql_Field::operator double() const
{
    double tmpdouble = 0.0;
    //转换得到DOBULE浮点型
    int fields = sscanf(field_data_, "%lf", &tmpdouble);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmpdouble;
}


//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL

