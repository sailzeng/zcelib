/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_format.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年4月30日
* @brief      格式化某些输出，包括int64，double，str，
*             搞完了，发现也就这3个有格式化，
*
* @details    部分代码最早来自openssh,中间的bsd-snprintf.c代码，原来的代码应该是支持BSD协议的,
*             我将3个函数整理了出来，打算用于自己的日志输出，也做过一些修正
*             bsd-snprintf.c是我见过的对于snprintf实现比较干净的代码，但有些地方偷懒不少，
*             甚至有不少的bug，比如他的double输出基本不可用,
*             原来的fmtstr也有问题，感觉作者对于精度和宽度的定义正好相反？
*             理论上我们的代码也应该遵循BSD协议，但毕竟代码是为了日志库打算的，我对代码做了修正，
*             而整体的代码我打算使用Apache的协议开源，Apache 2.0的协议和BSD协议非常接近
*             同时我也有点怀疑他是从GPL的代码改过来的，呵呵，
*             微软和GLIBC的这部分代码基本不可读，虽然他们考虑的情况的确要复杂很多
*             fmt_double函数是我自己费力改出来的，原来的函数fmtfp是有问题的，只能在整数范围内使用，
*             我参考了libc-5.4.29.tar.gz/libc/cvt/cvt_r.的函数ecvt_r 和 fcvt_r,进行了修正
*             ecvt_r和fcvt_r两个函数我搞了半天才比较明白他们的意图，他们设计的初衷，可能就是方便我
*             们进行格式化转换输出的
*
*             这里面的大部分代码是用于我们自己的fmtstr内部使用，外部非请误入。
*
*             后来，后来，google的代码搜索居然停止服务了，伤心。
*
*/

#ifndef _ZCE_FORMAT_OUT_STRING_H_
#define _ZCE_FORMAT_OUT_STRING_H_

namespace zce
{

///格式化字符串输出的控制宏，可以&一起试用
enum FORMAT_STRING_FLAG
{
    ///左对齐，相当于Format specifications 中 flag 的"-",
    FMT_MINUS           = (1 << 0),
    FMT_LEFT_ALIGN      = (1 << 0),

    ///增加符号位，相当于Format specifications 中 flag 的"+"
    FMT_PLUS            = (1 << 1),
    ///符号位如果不是-,用空格替代,相当于Format specifications 中 flag 的" "
    FMT_SPACE           = (1 << 2),
    ///增加前缀0,0x,0X，相当于Format specifications 中 flag 的"#"
    FMT_PREFIX             = (1 << 3),
    ///右对齐，左边多出的部分用0填充，相当于Format specifications 中 flag 的"0"
    FMT_ZERO            = (1 << 4),
    ///大写，x==>X,e==>E
    FMT_UP              = (1 << 5),
    ///当作无符号数据输出
    FMT_UNSIGNED        = (1 << 6),
    ///输出指数，相当于"e",如果同时用FMT_UP，相当于"E"
    FMT_EXPONENT        = (1 << 7),

};

///输出的进制系统，默认是10进制
enum class BASE_NUMBER
{
    ///二进制
    BINARY        = 2,
    ///八进制
    OCTAL         = 8,
    ///十进制
    DECIMAL       = 10,
    ///十六进制
    HEXADECIMAL   = 16,
};

///默认的浮点精度长度
static const size_t DEFAULT_DOUBLE_PRECISION = 6;

/*!
* @brief      用于字符串的格式化输出，注意这个函数在BUFFER末尾不添加\0,
* @param[out] buffer    输出的字符串BUFFER
* @param[in]  max_len   BUFFER的最大可用长度，
* @param[out] use_len   返回参数，格式化后使用了BUFFER的长度
* @param[in]  value     要进行格式化处理的源字符串
* @param[in]  str_len   要进行处理的源字符串长度
* @param[in]  width     输出字符串的宽度，为0表示不处理宽度，相当于printf函数格式化参数%32.16s中间的32。
* @param[in]  precision 精度，就是输出几个字符，相当于printf函数格式化参数%32.16s中间的16。
* @param[in]  flags     参考 @ref zce::FORMAT_STRING_FLAG
*/
void fmt_str(char *buffer,
             size_t max_len,
             size_t &use_len,
             const char *value,
             size_t str_len,
             size_t width = 0,
             size_t precision = -1,
             int flags = 0);

///@brief 用于将字符串格式化后输出（添加）到 @stdstr里面（末尾），参数含义见上面的函数
void fmt_str(std::string &stdstr,
             const char *value,
             size_t str_len,
             size_t width = 0,
             size_t precision = -1,
             int flags = 0);


/*!
* @brief      用于int64的格式化输出，注意这个函数在BUFFER末尾不添加\0,从BSD的openssh snprintf代码移植
*             我的困扰，本来我还搞了一个int32的代码，但他和int64如此的相像，完全的冗余，
*             如果我要保留int32，唯一的目的就是在32位机器上的速度可以更加快一点，但有那么大必要吗？服务器的代码大部分都会在64位机器上运行
*             考虑了一下，我删除了int32的代码，
* @param      buffer      输出返回的字符串BUFFER
* @param      max_len     BUFFER的最大可用长度，
* @param      use_len     返回参数，格式化后使用了BUFFER的长度
* @param      value       要进行输出的int64值
* @param      base        输出的进制 参考， @ref zce::BASE_NUMBER
* @param      width       输出的宽度，为0表示不处理宽度，相当于printf函数格式化参数%32.16d中间的32。
* @param      precision   精度，就是输出几个字符，相当于printf函数格式化参数%32.16d中间的16。整数输出不会截断
* @param      flags       参考 @ref zce::FORMAT_STRING_FLAG
*/
void fmt_int64(char *buffer,
               size_t max_len,
               size_t &use_len,
               int64_t value,
               BASE_NUMBER base = BASE_NUMBER::DECIMAL,
               size_t width = 0,
               size_t precision = 0,
               int flags = 0);

///用于将int64格式化后输出（添加）到 @stdstr里面（末尾），参数含义见上面的函数
void fmt_int64(std::string &stdstr,
               int64_t value,
               BASE_NUMBER base,
               size_t width = 0,
               size_t precision = 0,
               int flags = 0);

/*!
* @brief      浮点数字的输出,默认用指数方式输出，节约空间,浮点的输出是个很有意思的东东，
* @param      buffer     输出返回的字符串BUFFER
* @param      max_len    BUFFER的最大可用长度，
* @param      use_len    返回参数，格式化后使用了BUFFER的长度
* @param      fvalue     要进行输出的double值
* @param      width      输出的宽度，为0表示不处理宽度，相当于printf函数格式化参数%32.16d中间的32。
* @param      precision  精度，输出小数点后面几位
* @param      flags      参考 @ref zce::FORMAT_STRING_FLAG
* @note
*/
void fmt_double(char *buffer,
                size_t max_len,
                size_t &use_len,
                double fvalue,
                size_t width = 0,
                size_t precision = DEFAULT_DOUBLE_PRECISION,
                int flags = zce::FMT_EXPONENT | zce::FMT_UP);

///用于将double格式化后输出（添加）到 @stdstr里面（末尾），参数含义见上面的函数
void fmt_double(std::string &stdstr,
                double fvalue,
                size_t width = 0,
                size_t precision = DEFAULT_DOUBLE_PRECISION,
                int flags = zce::FMT_EXPONENT | zce::FMT_UP);

}

#endif
