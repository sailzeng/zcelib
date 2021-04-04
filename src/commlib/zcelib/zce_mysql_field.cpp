#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_mysql_field.h"

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

//High Speed �õ�String
void ZCE_Mysql_Field::get_string(std::string &tmpstr) const
{
    tmpstr.reserve(field_length_);
    tmpstr.assign(field_data_, field_length_);
}

//�õ��ַ���
ZCE_Mysql_Field::operator char() const
{
    char  tmp_tinyint = 0;

    short tmpshort = 0;
    //�õ�������
    int fields = sscanf(field_data_, "%hd", &tmpshort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //ͨ�����õ�TINY INT ����char
    tmp_tinyint = static_cast<char>(tmpshort & 0xFF);
    return tmp_tinyint;
}

//�õ�������
ZCE_Mysql_Field::operator short() const
{
    short tmp_short = 0;

    //�õ�������,����ֵ=0 ,EOF��ʾʧ��
    int fields = sscanf(field_data_, "%hd", &tmp_short);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //ת�����ɹ�����
    return tmp_short;
}

//�õ�������
ZCE_Mysql_Field::operator long() const
{
    long tmp_long = 0;

    //�õ�������,
    int fields = sscanf(field_data_, "%ld", &tmp_long);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_long;
}

//�õ�int����
ZCE_Mysql_Field::operator int() const
{
    int tmp_int = 0;

    //�õ�������,
    int fields = sscanf(field_data_, "%d", &tmp_int);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_int;
}

//�õ�int����
ZCE_Mysql_Field::operator unsigned int() const
{
    unsigned int tmp_uint = 0;

    //�õ�������,
    int fields = sscanf(field_data_, "%u", &tmp_uint);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_uint;
}

//ת��ΪINT64����
inline ZCE_Mysql_Field::operator long long() const
{
    long long tmp_longlong = 0;

    //�õ�64λ������
    int fields = sscanf(field_data_, "%lld", &tmp_longlong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_longlong;
}

//ת��Ϊ8λ�޷�������
ZCE_Mysql_Field::operator unsigned char() const
{
    unsigned char  tmp_utinyint = 0;
    unsigned short tmpshort = 0;
    //�õ�������
    int fields = sscanf(field_data_, "%hu", &tmpshort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //ͨ�����õ�TINY INT ����char
    tmp_utinyint = static_cast<unsigned char>(tmpshort & 0xFF);
    return tmp_utinyint;
}

//ת��Ϊ16λ���޷�������
ZCE_Mysql_Field::operator unsigned short() const
{
    unsigned short tmp_ushort = 0;

    //�õ�������,����ֵ=0 ,EOF��ʾʧ��
    int fields = sscanf(field_data_, "%hu", &tmp_ushort);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    //ת�����ɹ�����
    return tmp_ushort;
}
//ת��Ϊ32λ32�޷��ų�����
ZCE_Mysql_Field::operator unsigned long() const
{
    long tmp_ulong = 0;

    //�õ�������,
    int fields = sscanf(field_data_, "%lu", &tmp_ulong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_ulong;
}

//ת��Ϊ64λ�޷��ų�����
ZCE_Mysql_Field::operator unsigned long long() const
{
    unsigned long long tmp_ulonglong = 0;

    //�õ�64λ������
    int fields = sscanf(field_data_, "%llu", &tmp_ulonglong);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmp_ulonglong;
}

//ת��ΪFLOAT����
ZCE_Mysql_Field::operator float() const
{
    float tmpfloat = 0.0;
    //ת���õ�FLOAT������
    int fields = sscanf(field_data_, "%f", &tmpfloat);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmpfloat;
}

//ת��ΪDOUBLE����
ZCE_Mysql_Field::operator double() const
{
    double tmpdouble = 0.0;
    //ת���õ�DOBULE������
    int fields = sscanf(field_data_, "%lf", &tmpdouble);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    return tmpdouble;
}


//�����Ҫ��MYSQL�Ŀ�
#endif //#if defined ZCE_USE_MYSQL

