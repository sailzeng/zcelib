#include "zce_predefine.h"
#include "zce_bytes_serialize.h"
#include "zce_trace_log_debug.h"




//========================================================================================

///构造函数
ZCE_Serialize_Write::ZCE_Serialize_Write(char *write_buf, size_t buf_len) :
    is_good_(true),
    write_buf_(write_buf),
    buf_len_(buf_len),
    end_pos_(write_buf + buf_len),
    write_pos_(write_buf)
{
}

ZCE_Serialize_Write::~ZCE_Serialize_Write()
{
}

void ZCE_Serialize_Write::write_arithmetic(const char &val)
{
    const size_t SIZE_OF_VALUE = sizeof(char);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }

    *write_pos_ = val;
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const unsigned char &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned char);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    *write_pos_ = val;
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const short &val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const unsigned short &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const int &val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}
void ZCE_Serialize_Write::write_arithmetic(const unsigned int &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}
void ZCE_Serialize_Write::write_arithmetic(const float &val)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZFLOAT_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return;
}
void ZCE_Serialize_Write::write_arithmetic(const double &val)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZDOUBLE_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return ;
}


void ZCE_Serialize_Write::write_arithmetic(const int64_t &val)
{
    const size_t SIZE_OF_VALUE = sizeof(int64_t);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const uint64_t &val)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Write::write_arithmetic(const bool &val)
{
    return this->write_arithmetic(val ? (char)1 : (char)0);
}

//
void ZCE_ClassSerialize_WriteHelp<std::string>::write_help(ZCE_Serialize_Write *ssave,
    const std::string &val)
{
    ssave->write_array(val.c_str(), val.length());
}

//========================================================================================

//构造函数
ZCE_Serialize_Read::ZCE_Serialize_Read(const char *read_buf, size_t buf_len) :
    is_good_(true),
    read_buf_(read_buf),
    buf_len_(buf_len),
    end_pos_(read_buf + buf_len),
    read_pos_(read_buf)
{
}

ZCE_Serialize_Read::~ZCE_Serialize_Read()
{
}

void ZCE_Serialize_Read::read_arithmetic(bool &val)
{
    char bool_data = 0;
    read_arithmetic(bool_data);
    val = bool_data == 0 ? false : true;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(char &val)
{
    const size_t SIZE_OF_VALUE = sizeof(char);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = *read_pos_;
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(unsigned char &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned char);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = *read_pos_;
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(short &val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(int &val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(unsigned short &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(unsigned int &val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(int64_t &val)
{
    const size_t SIZE_OF_VALUE = sizeof(int64_t);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}
void ZCE_Serialize_Read::read_arithmetic(uint64_t &val)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(float &val)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_FLOAT(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_Serialize_Read::read_arithmetic(double &val)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_DOUBLE(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void ZCE_ClassSerialize_ReadHelp<std::string>::read_help(ZCE_Serialize_Read *sload,
    std::string &val)
{
    unsigned int v_size = 0;
    sload->read_arithmetic(v_size);

    val.assign(sload->read_pos_, v_size);
    sload->read_pos_ += v_size;
}
