/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_helper.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年5月2日
* @brief      字符串的输出辅助工具，包括Adapt等，保持相同的输出函数，
*             这样在使用模版的时候就很方便了。
*             也提供了一组宏帮助输出。
*             output_helper 函数为模板函数提供统一的入口，
*             XXXXX_Out_Helper 这些类一般是为了帮助提供格式化输出，
*             ZCE_XXXX_YYYY_OUT 的宏一般是为了简化格式化输出的
* 
* @details    基本的数据类型，ZCE_LIB里面的基础数据类型，都有了输出，
* 
*             2012年5月
*             前几天有以为令人尊敬的何培蓉（珍珠）女士将光诚从山东救了出来，在凶恶，
*             背叛，恐惧，默默的云云众生中，我们看到了一朵奇葩，
*             正因为他们的存在，我们看到了希望
* 
*/

#ifndef ZCE_FMTSTR_HELPER_H_
#define ZCE_FMTSTR_HELPER_H_

#include "zce_fmtstr_format.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_socket.h"
#include "zce_time_value.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_uuid_128_generator.h"
#include "zce_uuid_64_generator.h"

namespace ZCE_LIB
{

//标准数据类型的输出，用默认的格式------------------------------------------------------------------------

///辅助输出一个bool 布尔值
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const bool &out_data)
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

///辅助输出一个CHAR
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const char &out_data)
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

///辅助输出无符号CHAR
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned char &out_data)
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

///辅助输出短整数的字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const short &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       0);
}

///辅助输出无符号短整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned short &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       ZCE_LIB::FMT_UNSIGNED);
}

///辅助输出int整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const int &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       0);
}

///无符号int整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned int &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       ZCE_LIB::FMT_UNSIGNED);
}

///符号long整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const long &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       0);
}

///辅助输出无符号long整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned long &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       ZCE_LIB::FMT_UNSIGNED);
}

///辅助输出有符号long long整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const long long &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       0);
}

///辅助输出无符号long long整数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned long long &out_data)
{
    int64_t cvt_out_data = out_data;
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       cvt_out_data,
                       BASE_DECIMAL,
                       0,
                       0,
                       ZCE_LIB::FMT_UNSIGNED);
}

///辅助输出float单精度浮点
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const float &out_data)
{
    double cvt_out_data = out_data;
    ZCE_LIB::fmt_double(buffer,
                        max_len,
                        use_len,
                        cvt_out_data);
}

///辅助输出double双精度浮点
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const double &out_data)
{
    double cvt_out_data = out_data;
    ZCE_LIB::fmt_double(buffer,
                        max_len,
                        use_len,
                        cvt_out_data);
}

///辅助输出const char字符串输出辅助函数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const char *out_data)
{
    const char *cvt_out_data = out_data;
    size_t str_len = strlen (cvt_out_data);
    ZCE_LIB::fmt_str(buffer,
                     max_len,
                     use_len,
                     cvt_out_data,
                     str_len);
}

///辅助输出std::string字符串输出
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const std::string &out_data)
{
    ZCE_LIB::fmt_str(buffer,
                     max_len,
                     use_len,
                     out_data.c_str(),
                     out_data.length()
                    );
}

//===========================================================================================
//一些C语言的结构的字符串输出

///输出Time时间
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          timeval &out_data)
{

    const char *ret_str = ZCE_OS::timestamp(&out_data,
                                            buffer,
                                            max_len);

    //返回成功
    if (ret_str)
    {
        use_len = ZCE_OS::LEN_OF_TIME_STAMP;
    }
    else
    {
        use_len = 0;
    }
}

///IPV6的socket地址
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr_in &out_data)
{
    const char *ret_str = ZCE_OS::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                                     buffer,
                                                     max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///IPV6的socket地址
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr_in6 &out_data)
{
    const char *ret_str = ZCE_OS::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                                     buffer,
                                                     max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///sockaddr *的指针类型
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr *out_data)
{
    const char *ret_str = ZCE_OS::socketaddr_ntop_ex(out_data,
                                                     buffer,
                                                     max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///辅助IPV4的socket地址字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const in_addr &out_data)
{
    const char *ret_str = ZCE_OS::inet_ntop(AF_INET,
                                            (void *)(&out_data),
                                            buffer,
                                            max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///辅助输出IPV6的socket地址字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const in6_addr &out_data)
{
    const char *ret_str = ZCE_OS::inet_ntop(AF_INET6,
                                            (void *)(&out_data),
                                            buffer,
                                            max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

//===========================================================================================
//一些ZCELIB常用的结构的字符串输出

///输出ZCE_Time_Value的时间
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Time_Value &out_data)
{
    const char *ret_str = out_data.timestamp(buffer,
                                             max_len);

    //返回成功
    if (ret_str)
    {
        use_len = ZCE_OS::LEN_OF_TIME_STAMP;
    }
    else
    {
        use_len = 0;
    }
}

///输出ZCE_Sockaddr_In的字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Sockaddr_In &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///辅助输出ZCE_Sockaddr_In6的字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Sockaddr_In6 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len);

    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        use_len = 0;
    }
}

///辅助输出ZCE_UUID64的字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_UUID64 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len);

    //返回成功
    if (ret_str)
    {
        use_len = ZCE_UUID64::LEN_OF_ZCE_UUID64_STR;
    }
    else
    {
        use_len = 0;
    }
}

///辅助输出ZCE_UUID128的字符串
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_UUID128 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len);

    //返回成功
    if (ret_str)
    {
        use_len = ZCE_UUID128::LEN_OF_ZCE_UUID128_STR;
    }
    else
    {
        use_len = 0;
    }
}

//==================================================================================================
//特殊的格式输出方式

///INT格式化输出辅助类，可以帮助进行一些格式化输出
class Int_Out_Helper
{
public:
    //辅助类的构造函数
    explicit Int_Out_Helper(int64_t out_data,
                            BASE_NUMBER_SYSTEM base = BASE_DECIMAL,
                            size_t width = -1,
                            size_t precision = size_t(-1),
                            int flags = 0);

    ~Int_Out_Helper();

public:

    ///输出的int数据
    int64_t            out_data_;
    ///输出的进制方式
    BASE_NUMBER_SYSTEM base_;
    ///输出宽度，默认0
    size_t             width_;
    ///输出精度，默认-1，最大值，
    size_t             precision_;
    ///输出格式控制标志
    int                flags_;
};

//帮助输出16进制INT
#ifndef ZCE_INT_HEX_OUT
#define ZCE_INT_HEX_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                     ZCE_LIB::BASE_HEXADECIMAL,\
                                                     0,\
                                                     size_t(-1),\
                                                     ZCE_LIB::FMT_NUM|ZCE_LIB::FMT_UP)
#endif

//帮助输出16进制UINT
#ifndef ZCE_UINT_HEX_OUT
#define ZCE_UINT_HEX_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                      ZCE_LIB::BASE_HEXADECIMAL,\
                                                      0,\
                                                      size_t(-1),\
                                                      ZCE_LIB::FMT_NUM|ZCE_LIB::FMT_UP|ZCE_LIB::FMT_UNSIGNED)
#endif

//帮助输出8进制INT
#ifndef ZCE_INT_OCT_OUT
#define ZCE_INT_OCT_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                     ZCE_LIB::BASE_OCTAL,\
                                                     0,\
                                                     size_t(-1),\
                                                     ZCE_LIB::FMT_NUM|ZCE_LIB::FMT_UP)
#endif

//帮助输出8进制UINT
#ifndef ZCE_UINT_OCT_OUT
#define ZCE_UINT_OCT_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                      ZCE_LIB::BASE_OCTAL,\
                                                      0,\
                                                      size_t(-1),\
                                                      ZCE_LIB::FMT_NUM|ZCE_LIB::FMT_UP|ZCE_LIB::FMT_UNSIGNED)
#endif

//帮助输出2进制INT
#ifndef ZCE_INT_BIN_OUT
#define ZCE_INT_BIN_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                     ZCE_LIB::BASE_BINARY,\
                                                     0,\
                                                     size_t(-1),\
                                                     ZCE_LIB::FMT_UP)
#endif

//帮助输出2进制UINT
#ifndef ZCE_UINT_BIN_OUT
#define ZCE_UINT_BIN_OUT(x)   ZCE_LIB::Int_Out_Helper((x),\
                                                      ZCE_LIB::BASE_BINARY,\
                                                      0,\
                                                      size_t(-1),\
                                                      ZCE_LIB::FMT_UP|ZCE_LIB::FMT_UNSIGNED)
#endif

//INT格式化输出辅助
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const Int_Out_Helper &out_data)
{
    ZCE_LIB::fmt_int64(buffer,
                       max_len,
                       use_len,
                       out_data.out_data_,
                       out_data.base_,
                       out_data.width_,
                       out_data.precision_,
                       out_data.flags_
                      );
}

///DOUBLE格式化输出辅助类,帮助做一些格式化操作，
class Double_Out_Helper
{
public:
    explicit Double_Out_Helper(double out_data,
                               size_t width = -1,
                               size_t precision = size_t(-1),
                               int flags = 0);
    ~Double_Out_Helper();

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

///帮助浮点以小数方式输出，名字有点误解，但double和float都可以，
#ifndef ZCE_DOUBLE_DECIMAL_OUT
#define ZCE_DOUBLE_DECIMAL_OUT(x)   ZCE_LIB::Double_Out_Helper((x),\
                                                               0,\
                                                               size_t(-1),\
                                                               0)
#endif

///帮助浮点以指数方式输出
#ifndef ZCE_DOUBLE_EXPONENT_OUT
#define ZCE_DOUBLE_EXPONENT_OUT(x)  ZCE_LIB::Double_Out_Helper((x),\
                                                               0,\
                                                               size_t(-1),\
                                                               ZCE_LIB::FMT_UP|ZCE_LIB::FMT_EXPONENT)
#endif

///DOUBLE格式化输出辅助
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const Double_Out_Helper &out_data)
{
    ZCE_LIB::fmt_double(buffer,
                        max_len,
                        use_len,
                        out_data.out_data_,
                        out_data.width_,
                        out_data.precision_,
                        out_data.flags_
                       );
}

///String格式化输出辅助类，帮助做一些格式化操作
class String_Out_Helper
{
public:
    //
    explicit String_Out_Helper(const char *out_str_ptr,
                               size_t out_str_len,
                               size_t width ,
                               size_t precision,
                               int flags);
    //
    explicit String_Out_Helper(const std::string &out_str,
                               size_t width = size_t(-1),
                               size_t precision = size_t(-1),
                               int flags = 0);
    //
    explicit String_Out_Helper(const char *out_str_ptr,
                               size_t width = size_t(-1),
                               size_t precision = size_t(-1),
                               int flags = 0);

    ~String_Out_Helper();

public:
    ///字符串
    const char        *out_str_ptr_;
    ///字符串的长度
    size_t             out_str_len_;
    ///输出的宽度
    size_t             width_;
    ///输出的精度，小数点进度
    size_t             precision_;
    ///输出标志
    int                flags_;
};

///const char *字符串输出辅助函数
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const String_Out_Helper &out_data)
{
    ZCE_LIB::fmt_str(buffer,
                     max_len,
                     use_len,
                     out_data.out_str_ptr_,
                     out_data.out_str_len_,
                     out_data.width_,
                     out_data.precision_,
                     out_data.flags_
                    );
}

///帮助字符串以左对齐的方式输出
#ifndef ZCE_STRING_LEFTALIGN_OUT
#define ZCE_STRING_LEFTALIGN_OUT(x,y)   ZCE_LIB::String_Out_Helper((x),\
                                                                   (y),\
                                                                   size_t(-1),\
                                                                   ZCE_LIB::FMT_LEFT_ALIGN)
#endif

///帮助字符串以右对齐的方式输出
#ifndef ZCE_STRING_RIGHTALIGN_OUT
#define ZCE_STRING_RIGHTALIGN_OUT(x,y)  ZCE_LIB::String_Out_Helper((x),\
                                                                   (y),\
                                                                   size_t(-1),\
                                                                   0)
#endif

};//ZCE_LIB

#endif
