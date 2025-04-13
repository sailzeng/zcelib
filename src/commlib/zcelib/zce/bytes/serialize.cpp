#include "zce/predefine.h"
#include "zce/bytes/serialize.h"
#include "zce/logger/logging.h"

//========================================================================================
namespace zce::ser
{
///构造函数
encode::encode(char* write_buf, size_t buf_len) :
    is_good_(true),
    write_buf_(write_buf),
    buf_len_(buf_len),
    end_pos_(write_buf + buf_len),
    write_pos_(write_buf)
{
}

void encode::write(const char& val)
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

void encode::write(const unsigned char& val)
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

void encode::write(const short& val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT16_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const unsigned short& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT16_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const int& val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT32_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}
void encode::write(const unsigned int& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT32_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}
void encode::write(const float& val)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZFLOAT_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return;
}
void encode::write(const double& val)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZDOUBLE_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT64_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const unsigned long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    ZUINT32_TO_BEBYTE(write_pos_, val);
#elif defined ZCE_OS_LINUX
    ZUINT64_TO_BEBYTE(write_pos_, val);
#endif
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }

    ZUINT64_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const unsigned long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZUINT64_TO_BEBYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write(const bool& val)
{
    return this->write(val ? (char)1 : (char)0);
}

//
void encode::write(const std::string& val)
{
    write_array(val.c_str(), val.length());
}

void encode::write(const std::string_view& val)
{
    write_array(val.data(), val.length());
}

//========================================================================================

//构造函数
decode::decode(const char* read_buf, size_t buf_len) :
    is_good_(true),
    read_buf_(read_buf),
    buf_len_(buf_len),
    end_pos_(read_buf + buf_len),
    read_pos_(read_buf)
{
}

void decode::read(bool& val)
{
    char bool_data = 0;
    read(bool_data);
    val = bool_data == 0 ? false : true;
    return;
}

void decode::read(char& val)
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

void decode::read(unsigned char& val)
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

void decode::read(short& val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(int& val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(unsigned short& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(unsigned int& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    val = ZBEBYTE_TO_UINT32(read_pos_);
#elif defined ZCE_OS_LINUX
    val = ZBEBYTE_TO_UINT64(read_pos_);
#endif
    read_pos_ += SIZE_OF_VALUE;
    return;
}
void decode::read(unsigned long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    val = ZBEBYTE_TO_UINT32(read_pos_);
#elif defined ZCE_OS_LINUX
    val = ZBEBYTE_TO_UINT64(read_pos_);
#endif
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}
void decode::read(unsigned long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBEBYTE_TO_UINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read(float& val)
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

void decode::read(double& val)
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

void decode::read(std::string& val)
{
    const size_t SIZE_OF_STRINGLEN = sizeof(uint32_t);
    if (!is_good_ || read_pos_ + SIZE_OF_STRINGLEN > end_pos_)
    {
        is_good_ = false;
        return;
    }
    uint32_t v_size = ZBYTE_TO_UINT32(read_pos_);
    read_pos_ += SIZE_OF_STRINGLEN;
    if (!is_good_ || read_pos_ + v_size > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val.assign(read_pos_, v_size);
    read_pos_ += v_size;
}
}
