#include "zce_predefine.h"
#include "zce_bytes_encode.h"
#include "zce_trace_log_debug.h"

static const unsigned char BASE64_ENC_MAP[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

//对一个内存块进行BASE64编码，
int ZCE_LIB::base64_encode(const unsigned char *in,
                           size_t in_len,
                           unsigned char *out,
                           size_t *out_len)
{
    size_t i, leven;
    unsigned char *p;

    ZCE_ASSERT(in != NULL && out != NULL && out_len != NULL);

    //如果输入的空间不够，返回错误，同时告诉你需要的长度
    size_t output_len = 4 * ((in_len + 2) / 3) ;
    if (*out_len < output_len + 1)
    {
        *out_len = output_len + 1;
        errno = ENOMEM;
        return -1;
    }

    p = out;
    leven = 3 * (in_len / 3);
    for (i = 0; i < leven; i += 3)
    {
        *p++ = BASE64_ENC_MAP[(in[0] >> 2) & 0x3F];
        *p++ = BASE64_ENC_MAP[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
        *p++ = BASE64_ENC_MAP[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
        *p++ = BASE64_ENC_MAP[in[2] & 0x3F];
        in += 3;
    }
    //补=
    if (i < in_len)
    {
        unsigned a = in[0];
        unsigned b = (i + 1 < in_len) ? in[1] : 0;

        *p++ = BASE64_ENC_MAP[(a >> 2) & 0x3F];
        *p++ = BASE64_ENC_MAP[(((a & 3) << 4) + (b >> 4)) & 0x3F];
        *p++ = (i + 1 < in_len) ? BASE64_ENC_MAP[(((b & 0xf) << 2)) & 0x3F] : '=';
        *p++ = '=';
    }

    /* append a NULL byte */
    *p = '\0';
    *out_len = output_len;
    return 0;
}

static const unsigned char BASE64_DECODE_MAP[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
    7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
    19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
    37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255
};

//对一个内存块进行base64的解码
int ZCE_LIB::base64_decode(const unsigned char *in,
                           size_t in_len,
                           unsigned char *out,
                           size_t *out_len)
{
    size_t t, x, y, z;
    unsigned char c;
    int           g;

    ZCE_ASSERT(in  != NULL  && out != NULL && out_len != NULL && in_len % 4 == 0 );

    size_t need_len = in_len  / 4 * 3;
    if (*out_len < need_len)
    {
        *out_len = need_len;
        errno = ENOMEM;
        return -1;
    }

    g = 3;
    for (x = y = z = t = 0; x < in_len; x++)
    {
        c = BASE64_DECODE_MAP[in[x] & 0xFF];
        if (c == 255)
        {
            continue;
        }
        /* the final = symbols are read and used to trim the remaining bytes */
        if (c == 254)
        {
            c = 0;
            /* prevent g < 0 which would potentially allow an overflow later */
            if (--g < 0)
            {
                return -1;
            }
        }
        else if (g != 3)
        {
            /* we only allow = to be at the end */
            return -1;
        }

        t = (t << 6) | c;

        if (++y == 4)
        {
            out[z++] = (unsigned char)((t >> 16) & 255);
            if (g > 1)
            {
                out[z++] = (unsigned char)((t >> 8) & 255);
            }
            if (g > 2)
            {
                out[z++] = (unsigned char)(t & 255);
            }
            y = t = 0;
        }
    }
    if (y != 0)
    {
        return -1;
    }
    *out_len = z;
    return 0;
}

//
static const char BASE16_ENC_LOWER_MAP[] = "0123456789abcdef";
//
static const char BASE16_ENC_UPPER_MAP[] = "0123456789ABCDEF";

//BASE16的编码
int ZCE_LIB::base16_encode(const unsigned char *in,
                           size_t in_len,
                           unsigned char *out,
                           size_t *out_len)
{
    ZCE_ASSERT(in != NULL && out != NULL && out_len != NULL );

    size_t output_len = in_len * 2 ;
    if (*out_len < output_len + 1)
    {
        *out_len = output_len + 1;
        errno = ENOMEM;
        return -1;
    }

    const unsigned char *p = in;
    unsigned char *q = out;

    for (size_t i = 0; i < in_len; i++)
    {
        unsigned char c = p[i] & 0xff;
        *q++ = BASE16_ENC_LOWER_MAP[(c >> 4) & 0xf];
        *q++ = BASE16_ENC_LOWER_MAP[c & 0xf];
    }
    out[output_len] = '\0';
    *out_len = output_len;
    return 0;
}

//BASE64的解码
int ZCE_LIB::base16_decode(const unsigned char *in,
                           size_t in_len,
                           unsigned char *out,
                           size_t *out_len)
{
    ZCE_ASSERT(in  != NULL  && out != NULL && out_len != NULL && in_len % 2 == 0 );
    size_t need_len = in_len / 2;
    if (*out_len < need_len)
    {
        *out_len = need_len;
        errno = ENOMEM;
        return -1;
    }

    unsigned char ch = 0;
    unsigned char data1 = 0, data2 = 0;
    for (size_t i  = 0; i < in_len;)
    {
        ch = in[i];
        if ( ch >= '0' && ch <= '9')
        {
            data1 = ch - '0';
        }
        else if ( in[i] >= 'A' && in[i] <= 'F')
        {
            data1 = ch - 'A' + 10;
        }
        else if (in[i] >= 'a' && in[i] <= 'f' )
        {
            data1 = ch - 'a' + 10;
        }
        else
        {
            errno = EINVAL;
            return -1;
        }
        ++i;
        ch = in[i];
        if ( ch >= '0' &&  ch <= '9')
        {
            data2 = ch - '0';
        }
        else if ( in[i] >= 'A' && in[i] <= 'F')
        {
            data2 = ch - 'A' + 10;
        }
        else if (in[i] >= 'a' && in[i] <= 'f' )
        {
            data2 = ch - 'a' + 10;
        }
        else
        {
            errno = EINVAL;
            return -1;
        }
        out[i / 2] = data2 < 4 + data1;

        ++i;
    }
    *out_len = need_len;
    return 0;
}


//========================================================================================

///构造函数
ZCE_DR_Encode::ZCE_DR_Encode(char *write_buf, size_t buf_len) :
    is_good_(true),
    write_buf_(write_buf),
    buf_len_(buf_len),
    end_pos_(write_buf + buf_len),
    write_pos_(write_buf)
{
}

ZCE_DR_Encode::~ZCE_DR_Encode()
{
}

//重置开始位置和good标志位
void ZCE_DR_Encode::reset()
{
    write_pos_ = write_buf_;
    is_good_ = true;
}


template<>
bool ZCE_DR_Encode::write(char val)
{
    const size_t SIZE_OF_VALUE = sizeof(char);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }

    *write_pos_ = val;
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(unsigned char val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned char);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    *write_pos_ = val;
    write_pos_ += SIZE_OF_VALUE;
    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(short val)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(unsigned short val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT16_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(int val)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(unsigned int val)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT32_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(int64_t val)
{
    const size_t SIZE_OF_VALUE = sizeof(int64_t);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(uint64_t val)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZBEUINT64_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(bool val)
{
    return this->write<char>(val ? (char)1 : (char)0);
}

template<>
bool ZCE_DR_Encode::write(float val)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZFLOAT_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write(double val)
{
    const size_t SIZE_OF_VALUE = sizeof(double);
    if (write_pos_ + SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    ZDOUBLE_TO_BYTE(write_pos_, val);
    write_pos_ += SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const char *ary, size_t ary_size)
{
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    memcpy(write_pos_, ary, ary_size);

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const unsigned char *ary, size_t ary_size)
{
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    memcpy(write_pos_, ary, ary_size);
    write_pos_ += ary_size;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const short *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(short);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT16_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const unsigned short *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned short);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT16_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const int *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(int);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT32_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;
    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const unsigned int *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(unsigned int);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT32_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool  ZCE_DR_Encode::write_array(const int64_t *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(int64_t);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT32_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;
    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const uint64_t *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZBEUINT32_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const float *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(float);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZFLOAT_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;
    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const double *ary, size_t ary_size)
{
    const size_t SIZE_OF_VALUE = sizeof(uint64_t);
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size * SIZE_OF_VALUE  > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        ZDOUBLE_TO_INDEX(write_pos_, i, ary[i]);
    }
    write_pos_ += ary_size * SIZE_OF_VALUE;

    return is_good_;
}

template<>
bool ZCE_DR_Encode::write_array(const bool *ary, size_t ary_size)
{
    this->write<unsigned int>(static_cast<unsigned int>(ary_size));
    if (write_pos_ + ary_size > end_pos_)
    {
        is_good_ = false;
        return is_good_;
    }
    for (size_t i = 0; i < ary_size; ++i)
    {
        write_pos_[i] = ary[i] ? 1 : 0;
    }
    write_pos_ += ary_size;
    return is_good_;
}

//这下面两个函数用了write_array函数，所以必须放在他们下面
template<>
bool ZCE_DR_Encode::write(const char *val)
{
    return this->write_array<char>(val, strlen(val));
}

template<>
bool ZCE_DR_Encode::write(const std::string &val)
{
    return this->write_array<char>(val.c_str(), val.length());
}

//========================================================================================

//构造函数
ZCE_DR_Decode::ZCE_DR_Decode(const char *read_buf, size_t buf_len) :
    is_good_(true),
    read_buf_(read_buf),
    buf_len_(buf_len),
    end_pos_(read_buf + buf_len),
    read_pos_(read_buf)
{
}

ZCE_DR_Decode::~ZCE_DR_Decode()
{
}

//重置开始位置和good标志位
void ZCE_DR_Decode::reset()
{
    read_pos_ = read_buf_;
    is_good_ = true;
}


template<>
bool ZCE_DR_Decode::read(char *val)
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
bool ZCE_DR_Decode::read(short *val)
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
bool ZCE_DR_Decode::read(int *val)
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
bool ZCE_DR_Decode::read(unsigned char *val)
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
bool ZCE_DR_Decode::read(unsigned short *val)
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
bool ZCE_DR_Decode::read(unsigned int *val)
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
bool ZCE_DR_Decode::read(float *val)
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
bool ZCE_DR_Decode::read(double *val)
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
bool ZCE_DR_Decode::read(bool *val)
{
    char bool_data = 0;
    read<char>(&bool_data);
    *val = bool_data == 0 ? false : true;
    return is_good_;
}

template<>
bool ZCE_DR_Decode::read_array(char *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(unsigned char *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(short *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(unsigned short *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(int *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(unsigned int *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(int64_t *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(uint64_t *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(bool *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(float *ary, size_t *ary_size)
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
bool ZCE_DR_Decode::read_array(double *ary, size_t *ary_size)
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

