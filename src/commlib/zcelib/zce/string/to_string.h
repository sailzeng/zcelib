/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/string/to_string.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年5月2日
* @brief      字符串的输出辅助工具，包括Adapt等，保持相同的输出函数，
*             这样在使用模版的时候就很方便了。
*             也提供了一组宏帮助输出。
*             to_str 函数为模板函数提供统一的入口，
*             XXXXX_Out_Helper 这些类一般是为了帮助提供格式化输出，
*
* @details    基本的数据类型，zce里面的基础数据类型，都有了输出，
*
*             2012年5月
*             前几天有以为令人尊敬的何培蓉（珍珠）女士将光诚从山东救了出来，在凶恶，
*             背叛，恐惧，默默的云云众生中，我们看到了一朵奇葩，
*             不知道是否是因为奇葩的存在，我们看到了希望
*
*/

#pragma once

#include "zce/string/format.h"

namespace zce
{
class time_value;
class UUID64;
class UUID128;

namespace skt
{
class addr_in;
class addr_in6;
}

//标准数据类型的输出，用默认的格式------------------------------------------------------------------------

//--------------------------------------------
///辅助输出一个bool 布尔值
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const bool& out_data)
{
    if (out_data)
    {
        const size_t SIZE_OF_STR_TRUE = 4;
        use_len = std::min<size_t>(max_len, SIZE_OF_STR_TRUE);
        strncpy(buffer, "TRUE", use_len);
    }
    else
    {
        const size_t SIZE_OF_STR_FALSE = 5;
        use_len = std::min<size_t>(max_len, SIZE_OF_STR_FALSE);
        strncpy(buffer, "FALSE", max_len);
    }
}

inline void to_string(std::string& stdstr,
                      const bool& out_data)
{
    if (out_data)
    {
        stdstr.append("TRUE");
    }
    else
    {
        stdstr.append("FALSE");
    }
}

//--------------------------------------------
///辅助输出一个CHAR
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const char& out_data)
{
    const size_t CHAR_NEEN_ROOM = 1;

    //只输出一个字符
    if (max_len >= CHAR_NEEN_ROOM)
    {
        *buffer = out_data;
        use_len = CHAR_NEEN_ROOM;
    }
    else
    {
        //max_len == 0;
        use_len = 0;
    }
}

inline void to_string(std::string& stdstr,
                      const char& out_data)
{
    stdstr.append(1, out_data);
}

//--------------------------------------------
///辅助输出无符号CHAR
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const unsigned char& out_data)
{
    const size_t CHAR_NEEN_ROOM = 1;
    //只输出一个字符
    if (max_len >= CHAR_NEEN_ROOM)
    {
        *buffer = out_data;
        use_len = CHAR_NEEN_ROOM;
    }
    else
    {
        //max_len == 0;
        use_len = 0;
    }
}

inline void to_string(std::string& stdstr,
                      const unsigned char& out_data)
{
    stdstr.append(1, out_data);
}

//--------------------------------------------
///辅助输出短整数的字符串
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const short& out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

inline void to_string(std::string& stdstr,
                      const short& out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出无符号短整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const unsigned short out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void to_string(std::string& stdstr,
                      const unsigned short out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出int整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const int out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

inline void to_string(std::string& stdstr,
                      const int out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///无符号int整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const unsigned int out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void to_string(std::string& stdstr,
                      const unsigned int out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///符号long整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

inline void to_string(std::string& stdstr,
                      const long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出无符号long整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const unsigned long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void to_string(std::string& stdstr,
                      const unsigned long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出有符号long long整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const long long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

inline void to_string(std::string& stdstr,
                      const long long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出无符号long long整数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const unsigned long long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void to_string(std::string& stdstr,
                      const unsigned long long out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_NUMBER::DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///辅助输出float单精度浮点
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const float out_data)
{
    double cvt_out_data = out_data;
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    cvt_out_data);
}

inline void to_string(std::string& stdstr,
                      const float out_data)
{
    double cvt_out_data = out_data;
    zce::fmt_double(stdstr,
                    cvt_out_data);
}

//--------------------------------------------
///辅助输出double双精度浮点
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const double out_data)
{
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    out_data);
}

inline void to_string(std::string& stdstr,
                      const double out_data)
{
    zce::fmt_double(stdstr,
                    out_data);
}

//--------------------------------------------
///辅助输出const char字符串输出辅助函数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const char* out_data)
{
    const char* cvt_out_data = out_data;
    size_t str_len = strlen(cvt_out_data);
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 cvt_out_data,
                 str_len);
}

inline void to_string(std::string& stdstr,
                      const char* out_data)
{
    stdstr.append(out_data);
}

///辅助输出std::string字符串输出
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const std::string& out_data)
{
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 out_data.c_str(),
                 out_data.length());
}

inline void to_string(std::string& stdstr,
                      const std::string& out_data)
{
    stdstr.append(out_data);
}

///辅助输出void *指针地址输出
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const void* out_data)
{
    ptrdiff_t save_point = (const char*)out_data - (const char*)0;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   static_cast<int64_t>(save_point),
                   BASE_NUMBER::HEXADECIMAL,
                   0,
                   sizeof(save_point) == 4 ? 8 : 16,
                   zce::FMT_PREFIX | zce::FMT_UP | zce::FMT_ZERO);
}

inline void to_string(std::string& stdstr,
                      const void* out_data)
{
    ptrdiff_t save_point = (const char*)out_data - (const char*)0;
    zce::fmt_int64(stdstr,
                   static_cast<int64_t>(save_point),
                   BASE_NUMBER::HEXADECIMAL,
                   0,
                   sizeof(size_t) == 4 ? 8 : 16,
                   zce::FMT_PREFIX | zce::FMT_UP | zce::FMT_ZERO);
}

//===========================================================================================
//一些C语言的结构的字符串输出

///输出Time时间
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   timeval& out_data)
{
    const char* ret_str = zce::timestamp(&out_data,
                                         buffer,
                                         max_len);
    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void to_string(std::string& stdstr,
                      timeval& out_data)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    auto ret_str = zce::timestamp(&out_data,
                                  time_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///IPV4的socket地址
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr_in& out_data);

void to_string(std::string& stdstr,
               const sockaddr_in& out_data);

///IPV6的socket地址
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr_in6& out_data);

void to_string(std::string& stdstr,
               const sockaddr_in6& out_data);

///sockaddr *的指针类型
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr* out_data);

void to_string(std::string& stdstr,
               const sockaddr* out_data);

///辅助IPV4的socket地址字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const in_addr& out_data);

void to_string(std::string& stdstr,
               const in_addr& out_data);

///辅助输出IPV6的socket地址字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const in6_addr& out_data);

void to_string(std::string& stdstr,
               const in6_addr& out_data);

//-----------------------------------------------------------------------------------------------
//一些ZCELIB常用的结构的字符串输出

///输出zce::Time_Value的时间
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const zce::time_value& out_data);

inline void to_string(std::string& stdstr,
                      const zce::time_value& out_data);

///输出zce::skt::addr_in的字符串
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const zce::skt::addr_in& out_data);

void to_string(std::string& stdstr,
               const zce::skt::addr_in& out_data);

///辅助输出zce::skt::addr_in6的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const zce::skt::addr_in6& out_data);

void to_string(std::string& stdstr,
               const zce::skt::addr_in6& out_data);

///辅助输出ZCE_UUID64的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const UUID64& out_data);

void to_string(std::string& stdstr,
               const UUID64& out_data);

///辅助输出ZCE_UUID128的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const UUID128& out_data);

void to_string(std::string& stdstr,
               const UUID128& out_data);
//-----------------------------------------------------------------------------------------------
//特殊的格式输出方式

///INT格式化输出辅助类，可以帮助进行一些格式化输出
namespace aidout
{
class o_int
{
public:
    //辅助类的构造函数，跟进有符号和没有符号的整数类型进行了区分处理
    //is_unsigned 不光对整数生效，对浮点也有作用
    template <typename int_type, typename std::enable_if<(std::is_integral<int_type>::value&&
                                                          std::is_unsigned<int_type>::value), int >::type = 0>
    o_int(int_type out_data,
          size_t width = 0,
          int flags = 0,
          BASE_NUMBER base = BASE_NUMBER::DECIMAL,
          size_t precision = 0) :
        out_data_(out_data),
        width_(width),
        precision_(precision),
        base_(base),
        flags_(flags)
    {
        flags_ |= zce::FMT_UNSIGNED;
    }

    template <typename int_type, typename std::enable_if<std::is_integral<int_type>::value,
        typename std::enable_if<std::is_signed<int_type>::value, int>::type>::type = 0>
    o_int(int_type out_data,
          size_t width = 0,
          int flags = 0,
          BASE_NUMBER base = BASE_NUMBER::DECIMAL,
          size_t precision = 0) :
        out_data_(out_data),
        width_(width),
        precision_(precision),
        base_(base),
        flags_(flags)
    {
    }

    ~o_int() = default;

public:

    ///输出的int数据
    int64_t            out_data_;
    ///输出宽度，默认0
    size_t             width_;
    ///输出精度，默认-1，最大值，
    size_t             precision_;
    ///输出的进制方式
    BASE_NUMBER base_;
    ///输出格式控制标志
    int                flags_;
};

class  o_hexint : public o_int
{
public:
    template <typename int_type>
    o_hexint(int_type out_data,
             size_t width = 0,
             int flags = zce::FMT_PREFIX | zce::FMT_UP,
             size_t precision = 0) :
        zce::aidout::o_int(out_data, width, flags, BASE_NUMBER::HEXADECIMAL, precision)
    {
        flags_ |= zce::FMT_UNSIGNED;
    }
};
}

//INT格式化输出辅助
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const zce::aidout::o_int& out_data)
{
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   out_data.out_data_,
                   out_data.base_,
                   out_data.width_,
                   out_data.precision_,
                   out_data.flags_);
}

inline void to_string(std::string& stdstr,
                      const zce::aidout::o_int& out_data)
{
    zce::fmt_int64(stdstr,
                   out_data.out_data_,
                   out_data.base_,
                   out_data.width_,
                   out_data.precision_,
                   out_data.flags_);
}

namespace aidout
{
///DOUBLE格式化输出辅助类,帮助做一些格式化操作，
class o_double
{
public:
    explicit o_double(double out_data,
                      size_t width = -1,
                      size_t precision = size_t(-1),
                      int flags = 0);
    ~o_double() = default;

public:
    //要输出的浮点
    double             out_data_;
    //输出的宽度
    size_t             width_;
    //输出的精度，小数点进度
    size_t             precision_;
    //输出标志
    int                flags_;
};
}

///帮助浮点以小数方式输出，名字有点误解，但double和float都可以，
#ifndef ZCE_DOUBLE_DECIMAL_OUT
#define ZCE_DOUBLE_DECIMAL_OUT(x)   zce::aidout::o_double((x),\
                                                           0,\
                                                           size_t(-1),\
                                                           0)
#endif

///帮助浮点以指数方式输出
#ifndef ZCE_DOUBLE_EXPONENT_OUT
#define ZCE_DOUBLE_EXPONENT_OUT(x)  zce::aidout::o_double((x),\
                                                           0,\
                                                           size_t(-1),\
                                                           zce::FMT_UP|zce::FMT_EXPONENT)
#endif

///DOUBLE格式化输出辅助
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const zce::aidout::o_double& out_data)
{
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    out_data.out_data_,
                    out_data.width_,
                    out_data.precision_,
                    out_data.flags_);
}

inline void to_string(std::string& stdstr,
                      const zce::aidout::o_double& out_data)
{
    zce::fmt_double(stdstr,
                    out_data.out_data_,
                    out_data.width_,
                    out_data.precision_,
                    out_data.flags_);
}

namespace aidout
{
///String格式化输出辅助类，帮助做一些格式化操作
class o_string
{
public:
    //String格式化输出辅助类
    o_string(const char* out_str_ptr,
             size_t out_str_len,
             size_t width,
             size_t precision,
             int flags) :
        out_str_ptr_(out_str_ptr),
        out_str_len_(out_str_len),
        width_(width),
        precision_(precision),
        flags_(flags)
    {
    }

    //
    o_string(const std::string& out_str,
             size_t width = size_t(-1),
             size_t precision = size_t(-1),
             int flags = 0) :
        out_str_ptr_(out_str.c_str()),
        out_str_len_(out_str.length()),
        width_(width),
        precision_(precision),
        flags_(flags)
    {
    }

    o_string(const char* out_str_ptr,
             size_t width = size_t(-1),
             size_t precision = size_t(-1),
             int flags = 0) :
        out_str_ptr_(out_str_ptr),
        out_str_len_(strlen(out_str_ptr)),
        width_(width),
        precision_(precision),
        flags_(flags)
    {
    }

    ~o_string() = default;

public:
    ///字符串
    const char*        out_str_ptr_;
    ///字符串的长度
    size_t             out_str_len_;
    ///输出的宽度
    size_t             width_;
    ///输出的精度，小数点进度
    size_t             precision_;
    ///输出标志
    int                flags_;
};
}

///const char *字符串输出辅助函数
inline void to_str(char* buffer,
                   size_t max_len,
                   size_t& use_len,
                   const zce::aidout::o_string& out_data)
{
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 out_data.out_str_ptr_,
                 out_data.out_str_len_,
                 out_data.width_,
                 out_data.precision_,
                 out_data.flags_
    );
}

inline void to_string(std::string& stdstr,
                      const zce::aidout::o_string& out_data)
{
    zce::fmt_str(stdstr,
                 out_data.out_str_ptr_,
                 out_data.out_str_len_,
                 out_data.width_,
                 out_data.precision_,
                 out_data.flags_);
}
};//zce
