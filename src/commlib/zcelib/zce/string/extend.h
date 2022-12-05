/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/string/extend.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年2月6日
* @brief      实现一个C++的snprintf，类型无关的，速度可以是普通snprintf快30-50%，
*             而且是类型安全的，
*
* @details    在此再次感谢唐声福小伙子，fastformat的门是他给我打开的，我一直认为C++
*             的格式化输出的输出天生慢过C的。
*             唐声福告知了我fastformat这个库，以及他的部分代码。打开门这扇门后，我发现
*             我原来的认知其实有误。
*             std::iostream C++的慢其实慢在实现。
*
* @note       我这儿有两套实现。一套是用重载，多个函数，内部用宏，
*             一套是用variadic 变参实现的。必须有C++ 11以上的支持才能使用
*             用FOO_FMT_STRING_USE_VARIADIC
*             速度测试比较（都是是REALSE版本）
*             Windows 下：
*             variadic版本sformat_n     : 0.509 sec .
*             重载版本sformat_n          : 0.448 sec .
*             snprintf                 : 1.705 sec .
*             iostream                 : 2.477 sec .
*             Linux 下：(同一台机器，但是是虚拟机,VirtualBox)
*             variadic版本sformat_n     : 0.882927 sec .
*             重载版本sformat_n          : 0.820313 sec .
*             snprintf                 : 0.711378 sec .
*             iostream                 : 0.949645 sec .
*
*             因为C++20的出现，std::format 肯定要成为新的标杆。
*             查询历史记录，你可以发现一些不是CPP11就能跑的一些代码。
*             我已经不愿意看到老的代码。虽然我心血很多。
*
*/

#pragma once

#include "zce/string/to_string.h"

namespace zce
{
//----------------------------------------------------------------------------------------------------------

//格式化字符串的标识，出现一个{}，标识进行一次参数输出
static const char SNRPINTF_FMT_IDENTIFY[] = "{}";
//格式化字符串的长度
static const size_t LEN_OF_FMT_IDENTIFY = 2;

//转义字符串
static const char SNRPINTF_ESCAPE_IDENTIFY[] = "{}%";
//转义字符串长度
static const size_t LEN_OF_ESCAPE_IDENTIFY = 3;
//转义字符，如果前面出现{}再出现%，就标识转义
static const char SNRPINTF_FMT_ESCAPE_CHAR = '%';

// 下面代码推荐使用的是 sformat_n string_format，如果使用，你未来应该很容易切换的std::format

template <typename out_type >
void _string_format_outdata(std::string& foo_string,
                            const char*& foo_fmt_spec,
                            out_type&& out_data)
{
    const char* id_pos = nullptr;

    //处理第一个数据的输出，
    while (*foo_fmt_spec != '\0')
    {
        //找到{}，
        id_pos = strstr(foo_fmt_spec, zce::SNRPINTF_FMT_IDENTIFY);
        //将{}前面的字符串输出
        zce::fmt_str(foo_string, foo_fmt_spec,
                     (id_pos == nullptr) ? strlen(foo_fmt_spec) : (id_pos - foo_fmt_spec));
        if (id_pos == nullptr)
        {
            return;
        }
        else
        {
            foo_fmt_spec = id_pos;
        }

        //检查后面字符是否是%，用于判断是否是转义
        if (*(id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::embed_string(out_data, foo_string);
            foo_fmt_spec += LEN_OF_FMT_IDENTIFY;
            break;
        }
        //{}%将转义为{}输出
        else
        {
            zce::fmt_str(foo_string, SNRPINTF_FMT_IDENTIFY, LEN_OF_FMT_IDENTIFY);
            foo_fmt_spec += LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }
}

void _string_format_outlist(std::string& foo_string,
                            const char*& foo_fmt_spec);

//vanic 递归展开
template <typename out_type, typename... out_tlist >
void _string_format_outlist(std::string& foo_string,
                            const char*& foo_fmt_spec,
                            const out_type& out_data,
                            out_tlist&& ... out_datalist)
{
    _string_format_outdata(foo_string, foo_fmt_spec, out_data);
    _string_format_outlist(foo_string, foo_fmt_spec, out_datalist...);
}

/*!
* @brief
* @tparam     out_type
* @return     std::string&
* @param      foo_string
* @param      foo_fmt_spec
* @param      ...out_data
* @note
*/
template <typename... out_type >
std::string& string_format(std::string& foo_string,
                           const char* foo_fmt_spec,
                           out_type&&...out_data)
{
    const char* fmt_spec = foo_fmt_spec;
    _string_format_outlist(foo_string, fmt_spec, out_data...);
    _string_format_outlist(foo_string, fmt_spec);
    return foo_string;
}

//--------------------------------------------------------------------------------------------------------------------------------

//如果没有参数的特殊处理
void _sformat_outlist(char*& foo_buffer,
                      size_t& foo_max_len,
                      size_t& foo_use_len,
                      const char*& foo_fmt_spec);

template <typename T >
void _sformat_outdata(char*& foo_buffer,
                      size_t& foo_max_len,
                      size_t& foo_use_len,
                      const char*& foo_fmt_spec,
                      T&& out_data
)
{
    size_t use_len = 0;
    const char* id_pos = nullptr;

    //处理第一个数据的输出，
    while (foo_max_len > 0)
    {
        //找到{}，
        id_pos = ::strstr(foo_fmt_spec, zce::SNRPINTF_FMT_IDENTIFY);
        //将{}前面的字符串输出
        zce::fmt_str(foo_buffer,
                     foo_max_len,
                     use_len,
                     foo_fmt_spec,
                     (id_pos == nullptr) ? strlen(foo_fmt_spec) : (id_pos - foo_fmt_spec));

        foo_buffer += use_len;
        foo_max_len -= use_len;
        foo_use_len += use_len;
        foo_fmt_spec += use_len;

        if (0 == foo_max_len || id_pos == nullptr)
        {
            foo_buffer[0] = '\0';
            return;
        }

        //检查后面字符是否是?，用于判断是否是转义
        if (*(id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::embed_str(out_data, foo_buffer, foo_max_len, use_len);
            foo_buffer += use_len;
            foo_max_len -= use_len;
            foo_use_len += use_len;
            foo_fmt_spec += LEN_OF_FMT_IDENTIFY;
            break;
        }
        //{}%将转义为{}输出
        else
        {
            zce::fmt_str(foo_buffer,
                         foo_max_len,
                         use_len,
                         SNRPINTF_FMT_IDENTIFY,
                         LEN_OF_FMT_IDENTIFY);
            foo_buffer += use_len;
            foo_max_len -= use_len;
            foo_use_len += use_len;
            foo_fmt_spec += LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }
}

template <typename T, typename... Args >
void _sformat_outlist(char*& foo_buffer,
                      size_t& foo_max_len,
                      size_t& foo_use_len,
                      const char*& foo_fmt_spec,
                      T&& out_data,
                      Args&& ... out_datalist)
{
    _sformat_outdata(foo_buffer,
                     foo_max_len,
                     foo_use_len,
                     foo_fmt_spec,
                     out_data);
    _sformat_outlist(foo_buffer,
                     foo_max_len,
                     foo_use_len,
                     foo_fmt_spec,
                     out_datalist...);
}

/*!
* @brief      类似snprintf的格式化字符串函数，但用C++11方式优化处理。{}作为输出点。
*             格式控制通过辅助类完成。
* @tparam     out_type     输出的参数类型
* @return     char*        格式化后的字符串指针
* @param      foo_buffer   用于格式化字符串
* @param      foo_max_len  字符串最大长度
* @param      foo_use_len  使用的字符串长度
* @param      foo_fmt_spec 格式化的格式字符串
* @param      ...out_data  输出数据，插入格式化字符串中
* @note
*/
template <typename... Args >
char* sformat_n(char* foo_buffer,
                size_t foo_max_len,
                size_t& foo_use_len,
                const char* foo_fmt_spec,
                Args&& ...out_data)
{
    foo_use_len = 0;

    if (0 == foo_max_len)
    {
        return foo_buffer;
    }

    size_t max_len = foo_max_len - 1;
    char* buffer = foo_buffer;
    buffer[max_len] = '\0';
    const char* fmt_spec = foo_fmt_spec;

    _sformat_outlist(buffer, max_len, foo_use_len, fmt_spec, out_data...);
    //尾巴上的那点东西也要输出，
    _sformat_outlist(buffer, max_len, foo_use_len, fmt_spec);
    foo_buffer[foo_use_len] = '\0';
    //返回
    return foo_buffer;
}

//--------------------------------------------------------------------------------------------------------------------------------

template <typename T >
void _splice_outdata(char*& foo_buffer,
                     size_t foo_max_len,
                     size_t& foo_use_len,
                     char separator_char,
                     T&& out_data)
{
    size_t max_len = foo_max_len - 1;
    size_t use_len = 0;
    //如果还有空间容纳字符
    if (foo_max_len > 0)
    {
        zce::embed_str(out_data, foo_buffer, foo_max_len, use_len);
        foo_buffer += use_len;
        foo_max_len -= use_len;
        foo_use_len += use_len;
    }
    //如果需要分隔符，添加分割符号
    if (separator_char != '\0' && max_len > 0)
    {
        *foo_buffer = separator_char;
        foo_buffer += 1;
        max_len -= 1;
        foo_use_len += 1;
    }
}

///啥也不干,就是完成变参递归
void _splice_outlist(char*& foo_buffer,
                     size_t foo_max_len,
                     size_t& foo_use_len,
                     char separator_char);

template <typename T, typename... Args >
void _splice_outlist(char*& foo_buffer,
                     size_t foo_max_len,
                     size_t& foo_use_len,
                     char separator_char,
                     T&& out_data,
                     Args&& ... out_datalist)
{
    _splice_outdata(foo_buffer, foo_max_len, foo_use_len, separator_char, out_data);
    _splice_outlist(foo_buffer, foo_max_len, foo_use_len, separator_char, out_datalist...);
}

/*!
* @brief      函数用于讲多个参数拼接成一个字符串，可以使用风格符
* @return     char*    返回字符串
* @param      foo_buffer  存放拼接结果的字符串
* @param      foo_max_len 字符串长度
* @param      foo_use_len 字符串使用的长度
* @param      separator_char 拼接的分割符号，=0表示不需要
* @param      ...out_data 用于拼接的类型 多个参数
*/
template <typename... Args >
char* ssplice_n(char* foo_buffer,
                size_t foo_max_len,
                size_t& foo_use_len,
                char separator_char,
                Args&& ...out_data)
{
    foo_use_len = 0;
    if (0 == foo_max_len)
    {
        return foo_buffer;
    }

    size_t max_len = foo_max_len - 1;
    char* buffer = foo_buffer;
    buffer[max_len] = '\0';

    _splice_outlist(buffer,
                    max_len, foo_use_len, separator_char, out_data...);

    foo_buffer[foo_use_len] = '\0';
    //返回
    return foo_buffer;
}

//--------------------------------------------------------------------------------------------------------------------------------

template <typename T >
void _string_splice_data(std::string& foo_string,
                         char separator_char,
                         T&& out_data)
{
    zce::embed_string(foo_string, out_data);
    if (separator_char != '\0')
    {
        foo_string.append(1, separator_char);
    }
}

void _string_splice_list(std::string& foo_string,
                         char separator_char);

template <typename T, typename... Args >
void _string_splice_list(std::string& foo_string,
                         char separator_char,
                         T&& out_data,
                         Args&&... out_datalist)
{
    _string_splice_data(foo_string, separator_char, out_data);
    _string_splice_list(foo_string, separator_char, out_datalist...);
}

/*!
 * @brief
 * @tparam ...out_type 合并的类型列表
 * @param foo_string     合并输出的string
 * @param separator_char 分割字符
 * @param ...out_data    合并的数据
*/
template <typename... Args >
std::string& splice_string(std::string& foo_string,
                           char separator_char,
                           Args&&...out_data)
{
    _foo_c11_string_splice(foo_string, separator_char, out_data...);
    return foo_string;
}
};
