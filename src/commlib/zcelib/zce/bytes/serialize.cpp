#include "zce/predefine.h"
#include "zce/bytes/serialize.h"
#include "zce/logger/logging.h"

//========================================================================================
namespace zce::serialize
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

void encode::write_i(const char& val)
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

void encode::write_i(const unsigned char& val)
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

void encode::write_i(const short& val)
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

void encode::write_i(const unsigned short& val)
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

void encode::write_i(const int& val)
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
void encode::write_i(const unsigned int& val)
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
void encode::write_i(const float& val)
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
void encode::write_i(const double& val)
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

void encode::write_i(const long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write_i(const unsigned long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    ZBEUINT32_TO_BYTE(write_pos_, val);
#elif defined ZCE_OS_LINUX
    ZBEUINT64_TO_BYTE(write_pos_, val);
#endif
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write_i(const long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    ZBEUINT32_TO_BYTE(write_pos_, val);
#elif defined ZCE_OS_LINUX
    ZBEUINT64_TO_BYTE(write_pos_, val);
#endif
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write_i(const unsigned long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long long);
    if (!is_good_ || write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;
    return;
}

void encode::write_i(const bool& val)
{
    return this->write_i(val ? (char)1 : (char)0);
}

//
void en_class_help<std::string>::write_help(encode* ssave,
                                            const std::string& val)
{
    ssave->write_array(val.c_str(), val.length());
}

void en_class_help<std::string_view>::write_help(encode* ssave,
                                                 const std::string_view& val)
{
    ssave->write_array(val.data(), val.length());
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

void decode::read_i(bool& val)
{
    char bool_data = 0;
    read_i(bool_data);
    val = bool_data == 0 ? false : true;
    return;
}

void decode::read_i(char& val)
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

void decode::read_i(unsigned char& val)
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

void decode::read_i(short& val)
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

void decode::read_i(int& val)
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

void decode::read_i(unsigned short& val)
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

void decode::read_i(unsigned int& val)
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

void decode::read_i(long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    val = ZBYTE_TO_BEUINT32(read_pos_);
#elif defined ZCE_OS_LINUX
    val = ZBYTE_TO_BEUINT64(read_pos_);
#endif
    read_pos_ += SIZE_OF_VALUE;
    return;
}
void decode::read_i(unsigned long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
#if defined ZCE_OS_WINDOWS
    val = ZBYTE_TO_BEUINT32(read_pos_);
#elif defined ZCE_OS_LINUX
    val = ZBYTE_TO_BEUINT64(read_pos_);
#endif
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read_i(long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(long long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}
void decode::read_i(unsigned long long& val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned long long);
    if (!is_good_ || read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return;
    }
    val = ZBYTE_TO_BEUINT64(read_pos_);
    read_pos_ += SIZE_OF_VALUE;
    return;
}

void decode::read_i(float& val)
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

void decode::read_i(double& val)
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

void de_class_help<std::string>::read_help(decode* sload,
                                           std::string& val)
{
    unsigned int v_size = 0;
    sload->read_i(v_size);

    val.assign(sload->read_pos_, v_size);
    sload->read_pos_ += v_size;
}
}
