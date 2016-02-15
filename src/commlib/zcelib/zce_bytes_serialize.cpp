#include "zce_predefine.h"
#include "zce_bytes_serialize.h"
#include "zce_trace_log_debug.h"




//========================================================================================

///构造函数
ZCE_Serialized_Save::ZCE_Serialized_Save(char *write_buf, size_t buf_len) :
    is_good_(true),
    write_buf_(write_buf),
    buf_len_(buf_len),
    end_pos_(write_buf + buf_len),
    write_pos_(write_buf)
{
}

ZCE_Serialized_Save::~ZCE_Serialized_Save()
{
}

void ZCE_Serialized_Save::save_arithmetic(const char &val)
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

void ZCE_Serialized_Save::save_arithmetic(const unsigned char &val)
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

void ZCE_Serialized_Save::save_arithmetic(const short &val)
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

void ZCE_Serialized_Save::save_arithmetic(const unsigned short &val)
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

void ZCE_Serialized_Save::save_arithmetic(const int &val)
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
void ZCE_Serialized_Save::save_arithmetic(const unsigned int &val)
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
void ZCE_Serialized_Save::save_arithmetic(const float &val)
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
void ZCE_Serialized_Save::save_arithmetic(const double &val)
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


void ZCE_Serialized_Save::save_arithmetic(const int64_t &val)
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

void ZCE_Serialized_Save::save_arithmetic(const uint64_t &val)
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

void ZCE_Serialized_Save::save_arithmetic(const bool &val)
{
    return this->save_arithmetic(val ? (char)1 : (char)0);
}

//========================================================================================

//构造函数
ZCE_Serialized_Load::ZCE_Serialized_Load(const char *read_buf, size_t buf_len) :
    is_good_(true),
    read_buf_(read_buf),
    buf_len_(buf_len),
    end_pos_(read_buf + buf_len),
    read_pos_(read_buf)
{
}

ZCE_Serialized_Load::~ZCE_Serialized_Load()
{
}

void ZCE_Serialized_Load::load_arithmetic(bool &val)
{
    char bool_data = 0;
    load_arithmetic(bool_data);
    val = bool_data == 0 ? false : true;
    return;
}

void ZCE_Serialized_Load::load_arithmetic(char &val)
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

void ZCE_Serialized_Load::load_arithmetic(unsigned char &val)
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

void ZCE_Serialized_Load::load_arithmetic(short &val)
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

void ZCE_Serialized_Load::load_arithmetic(int &val)
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

void ZCE_Serialized_Load::load_arithmetic(unsigned short &val)
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

void ZCE_Serialized_Load::load_arithmetic(unsigned int &val)
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

void ZCE_Serialized_Load::load_arithmetic(float &val)
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

void ZCE_Serialized_Load::load_arithmetic(double &val)
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


