#include "zce_predefine.h"
#include "zce_bytes_data_represent.h"

//========================================================================================

///¹¹Ôìº¯Êý
ZCE_DR_Encode::ZCE_DR_Encode(char *write_buf, size_t buf_len) :
    write_buf_(write_buf),
    buf_len_(buf_len),
    write_pos_(write_buf),
    end_pos_(write_buf + buf_len),
    is_good_(true)
{
}

ZCE_DR_Encode::~ZCE_DR_Encode()
{

}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const char &val)
{
    *write_pos_ = val;
    ++write_pos_;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}

template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const short &val)
{
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += 2;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const int &val)
{
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += 4;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const int64_t &val)
{
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += 8;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const unsigned char &val)
{
    *write_pos_ = val;
    ++write_pos_;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}

template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const unsigned short &val)
{
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += 2;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const unsigned int &val)
{
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += 4;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}


template<>
ZCE_DR_Encode& ZCE_DR_Encode::write(const uint64_t &val)
{
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += 8;
    if (write_pos_ > end_pos_)
    {
        is_good_ = false;
    }
    return *this;
}




//========================================================================================


