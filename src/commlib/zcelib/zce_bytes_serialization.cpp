#include "zce_predefine.h"
#include "zce_bytes_serialization.h"
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

//重置开始位置和good标志位
void ZCE_Serialized_Save::reset()
{
    write_pos_ = write_buf_;
    is_good_ = true;
}


void ZCE_Serialized_Save::save_arithmetic(const char &val)
{
    const size_t SIZE_OF_VALUE = sizeof(char);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
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

//重置开始位置和good标志位
void ZCE_Serialized_Load::reset()
{
    read_pos_ = read_buf_;
    is_good_ = true;
}


template<>
bool ZCE_Serialized_Load::read(char *val)
{
    const size_t SIZE_OF_VALUE = sizeof(char);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = *read_pos_;
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(short *val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_BEUINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(int *val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_BEUINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(unsigned char *val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned char);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = *read_pos_;
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(unsigned short *val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_BEUINT16(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(unsigned int *val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_BEUINT32(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(float *val)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_FLOAT(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(double *val)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    if (read_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *val = ZBYTE_TO_DOUBLE(read_pos_);
    read_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read(bool *val)
{
    char bool_data = 0;
    read<char>(&bool_data);
    *val = bool_data == 0 ? false : true;
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(char *ary, size_t *ary_size)
{
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + read_arysize > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    memcpy(ary, read_pos_, read_arysize);
    read_pos_ += read_arysize;
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(unsigned char *ary, size_t *ary_size)
{
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + read_arysize > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    memcpy(ary, read_pos_, read_arysize);
    read_pos_ += read_arysize;
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(short *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + read_arysize * SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT16(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(unsigned short *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT16(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(int *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT32(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(unsigned int *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT32(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(int64_t *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(int64_t);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT64(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(uint64_t *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_BEUINT64(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(bool *ary, size_t *ary_size)
{
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + read_arysize > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = *(read_pos_ + i) == 0 ? false : true;
    }
    read_pos_ += read_arysize;
    return is_good_;
}


template<>
bool ZCE_Serialized_Load::read_array(float *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_FLOAT(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

template<>
bool ZCE_Serialized_Load::read_array(double *ary, size_t *ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    unsigned int read_arysize = 0;
    this->read<unsigned int>(&read_arysize);
    if (read_arysize > *ary_size || read_pos_ + (read_arysize * SIZE_OF_VALUE) > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *ary_size = read_arysize;
    for (unsigned int i = 0; i < read_arysize; ++i)
    {
        ary[i] = ZINDEX_TO_DOUBLE(read_pos_, i);
    }
    read_pos_ += (read_arysize * SIZE_OF_VALUE);
    return is_good_;
}

