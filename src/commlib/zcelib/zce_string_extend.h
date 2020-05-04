/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_string_extend.h
* @author     Sailzeng <sailerzeng@gmail.com>
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
*             variadic版本foo_snprintf : 0.509 sec .
*             重载版本foo_snprintf      : 0.448 sec .
*             snprintf                 : 1.705 sec .
*             iostream                 : 2.477 sec .
*             Linux 下：(同一台机器，但是是虚拟机,VirtualBox)
*             variadic版本foo_snprintf : 0.882927 sec .
*             重载版本foo_snprintf      : 0.820313 sec .
*             snprintf                 : 0.711378 sec .
*             iostream                 : 0.949645 sec .
*
*
*
*/

#ifndef ZCE_STRING_EXTEND_H_
#define ZCE_STRING_EXTEND_H_

#include "zce_string_helper.h"

namespace zce
{

//格式化字符串的标识，出现一个%?，标识进行一次参数输出
static const char SNRPINTF_FMT_IDENTIFY[] = "%?";
//格式化字符串的长度
static const size_t LEN_OF_FMT_IDENTIFY = 2;

//转义字符串
static const char SNRPINTF_ESCAPE_IDENTIFY[] = "%??";
//转义字符串长度
static const size_t LEN_OF_ESCAPE_IDENTIFY = 3;
//转义字符，如果前面出现%?再出现%，就标识转义
static const char SNRPINTF_FMT_ESCAPE_CHAR  = '?';

#ifndef FOO_FMT_STRING_USE_VARIADIC
#define FOO_FMT_STRING_USE_VARIADIC  1
#endif


//没有C++ 11的特性，没有任何用处
//#if ZCE_SUPPORT_CPP11 == 0
//#define FOO_FMT_STRING_USE_VARIADIC 0
//#endif


#if defined FOO_FMT_STRING_USE_VARIADIC && FOO_FMT_STRING_USE_VARIADIC ==1

//--------------------------------------------------------------------------------------------------------------------------------



//如果没有参数的特殊处理
inline static void _foo_c11_outdata(char *&foo_buffer,
                                    size_t &foo_max_len,
                                    size_t &foo_use_len,
                                    const char *&foo_fmt_spec)
{
    size_t len_of_str = strlen(foo_fmt_spec);
    if (len_of_str > foo_max_len)
    {
        foo_max_len = 0;
        foo_use_len += foo_max_len;
        memcpy(foo_buffer, foo_fmt_spec, foo_max_len);
    }
    else
    {
        foo_max_len -= len_of_str;
        foo_use_len += len_of_str;
        memcpy(foo_buffer, foo_fmt_spec, len_of_str);
    }
}


template <typename out_type >
static void _foo_c11_outdata(char *&foo_buffer,
                             size_t &foo_max_len,
                             size_t &foo_use_len,
                             const char *&foo_fmt_spec,
                             const out_type &out_data
                            )
{
    size_t use_len = 0;
    const char *id_pos = NULL;

    //处理第一个数据的输出，
    while (foo_max_len > 0)
    {
        //找到%?，
        id_pos = strstr(foo_fmt_spec, zce::SNRPINTF_FMT_IDENTIFY);
        //将%?前面的字符串输出
        zce::fmt_str(foo_buffer, foo_max_len, use_len, foo_fmt_spec, (id_pos == NULL) ? strlen(foo_fmt_spec) : (id_pos - foo_fmt_spec));

        foo_buffer += use_len;
        foo_max_len -= use_len;
        foo_use_len += use_len;
        foo_fmt_spec += use_len;

        if (0 == foo_max_len || id_pos == NULL)
        {
            foo_buffer[0] = '\0';
            return;
        }

        //检查后面字符是否是?，用于判断是否是转义
        if (*(id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::output_helper(foo_buffer, foo_max_len, use_len, out_data);
            foo_buffer += use_len;
            foo_max_len -= use_len;
            foo_use_len += use_len;
            foo_fmt_spec += LEN_OF_FMT_IDENTIFY;
            break;
        }
        //%??将转义为%?输出
        else
        {
            zce::fmt_str(foo_buffer, foo_max_len, use_len, SNRPINTF_FMT_IDENTIFY, LEN_OF_FMT_IDENTIFY);
            foo_buffer += use_len;
            foo_max_len -= use_len;
            foo_use_len += use_len;
            foo_fmt_spec += LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }
}

template <typename out_type, typename... out_tlist >
static void _foo_c11_outdata(char *&foo_buffer,
                             size_t &foo_max_len,
                             size_t &foo_use_len,
                             const char *&foo_fmt_spec,
                             const out_type &out_data,
                             out_tlist ... out_datalist)
{
    _foo_c11_outdata(foo_buffer, foo_max_len, foo_use_len, foo_fmt_spec, out_data);
    _foo_c11_outdata(foo_buffer, foo_max_len, foo_use_len, foo_fmt_spec, out_datalist...);
}

/*!
* @brief      类似snprintf的格式化字符串函数，但用C++11方式优化处理。%?作为输出点。
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
template <typename... out_type >
char* foo_snprintf(char* foo_buffer,
                   size_t foo_max_len,
                   size_t& foo_use_len,
                   const char* foo_fmt_spec,
                   const out_type& ...out_data)
{
    foo_use_len=0;

    if(0==foo_max_len)
    {
        return foo_buffer;
    }

    size_t max_len=foo_max_len-1,use_len=0;
    char* buffer=foo_buffer;
    buffer[max_len]='\0';
    const char* fmt_spec=foo_fmt_spec;

    _foo_c11_outdata(buffer,max_len,foo_use_len,fmt_spec,out_data...);

    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 fmt_spec,
                 strlen(fmt_spec));
    foo_use_len+=use_len;
    buffer[use_len]='\0';
    //返回
    return foo_buffer;
}
//--------------------------------------------------------------------------------------------------------------------------------

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
std::string &foo_string_format(std::string &foo_string,
                               const char *foo_fmt_spec,
                               const out_type &...out_data)
{
    const char *fmt_spec = foo_fmt_spec;
    _foo_c11_outstring(foo_string, fmt_spec, out_data...);
    zce::fmt_str(foo_string,
                 fmt_spec,
                 strlen(fmt_spec));
    return foo_string;
}

inline static void _foo_c11_outstring(std::string& foo_string,
                                      const char*& foo_fmt_spec)
{
    foo_string.append(foo_fmt_spec);
}

template <typename out_type >
void _foo_c11_outstring(std::string& foo_string,
                        const char*& foo_fmt_spec,
                        const out_type& out_data)
{
    const char* id_pos=NULL;

    //处理第一个数据的输出，
    while(*foo_fmt_spec!='\0')
    {
        //找到%?，
        id_pos=strstr(foo_fmt_spec,zce::SNRPINTF_FMT_IDENTIFY);
        //将%?前面的字符串输出
        zce::fmt_str(foo_string,foo_fmt_spec,(id_pos==NULL)?strlen(foo_fmt_spec):(id_pos-foo_fmt_spec));
        if(id_pos==NULL)
        {
            return;
        }
        else
        {
            foo_fmt_spec=id_pos;
        }

        //检查后面字符是否是?，用于判断是否是转义
        if(*(id_pos+LEN_OF_FMT_IDENTIFY)!=zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::string_helper(foo_string,out_data);
            foo_fmt_spec+=LEN_OF_FMT_IDENTIFY;
            break;
        }
        //%??将转义为%?输出
        else
        {
            zce::fmt_str(foo_string,SNRPINTF_FMT_IDENTIFY,LEN_OF_FMT_IDENTIFY);
            foo_fmt_spec+=LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }
}

//vanic 递归展开
template <typename out_type, typename... out_tlist >
static void _foo_c11_outstring(std::string &foo_string,
                               const char *&foo_fmt_spec,
                               const out_type &out_data,
                               out_tlist ... out_datalist)
{
    _foo_c11_outstring(foo_string, foo_fmt_spec, out_data);
    _foo_c11_outstring(foo_string, foo_fmt_spec, out_datalist...);
}





//--------------------------------------------------------------------------------------------------------------------------------


template <typename out_type >
void _foo_c11_splice(char *&foo_buffer,
                     size_t foo_max_len,
                     size_t &foo_use_len,
                     char separator_char,
                     const out_type &out_data)
{
    size_t max_len = foo_max_len - 1;
    size_t use_len = 0;
    //如果还有空间容纳字符
    if (foo_max_len > 0)
    {
        zce::output_helper(foo_buffer, foo_max_len, use_len, out_data);
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

template <typename out_type, typename... out_tlist >
void _foo_c11_splice(char *&foo_buffer,
                     size_t foo_max_len,
                     size_t &foo_use_len,
                     char separator_char,
                     const out_type &out_data,
                     out_tlist ... out_datalist)
{
    _foo_c11_splice(foo_buffer, foo_max_len, foo_use_len, separator_char, out_data);
    _foo_c11_splice(foo_buffer, foo_max_len, foo_use_len, separator_char, out_datalist...);
}

//--------------------------------------------------------------------------------------------------------------------------------
template <typename... out_type >
std::string &foo_string_splice(std::string &foo_string,
                               char separator_char,
                               const out_type &...out_data)
{
    _foo_c11_string_splice(foo_string, separator_char, out_data...);
    return foo_string;
}

template <typename out_type >
static void _foo_c11_string_splice(std::string &foo_string,
                                   char separator_char,
                                   const out_type &out_data)
{
    zce::string_helper(foo_string, out_data);
    foo_string.append(1, separator_char);
}

template <typename out_type, typename... out_tlist >
static void _foo_c11_string_splice(std::string &foo_string,
                                   char separator_char,
                                   const out_type &out_data,
                                   out_tlist ... out_datalist)
{
    _foo_c11_string_splice(foo_string, separator_char, out_data);
    _foo_c11_string_splice(foo_string, separator_char, out_datalist...);
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
template <typename... out_type >
char* foo_strnsplice(char* foo_buffer,
                     size_t foo_max_len,
                     size_t& foo_use_len,
                     char separator_char,
                     const out_type& ...out_data)
{
    foo_use_len=0;
    if(0==foo_max_len)
    {
        return foo_buffer;
    }

    size_t max_len=foo_max_len-1;
    char* buffer=foo_buffer;
    buffer[max_len]='\0';

    _foo_c11_splice(buffer,max_len,foo_use_len,separator_char,out_data...);

    foo_buffer[foo_use_len]='\0';
    //返回
    return foo_buffer;
}

#else

//我估计这辈子也不会再用这种方式增加这类函数了。估计不久就会废弃这些代码。我辛辛苦苦用python生成的
//因为模版函数写在.h文件中，所以宏也卸载了.h文件里面，过年回家看看能否用,

#define __ZCE_SNPRINTF_BEGIN   foo_use_len = 0; \
    if ( 0 == foo_max_len ) \
    { \
        return foo_buffer; \
    } \
    size_t max_len = foo_max_len-1; \
    size_t use_len = 0; \
    char *buffer = foo_buffer; \
    buffer[max_len] = '\0';   \
    const char *fmt_spec = foo_fmt_spec; \
    const char *id_pos = NULL

#define __ZCE_SNPRINTF_REPEAT(x)     while( max_len > 0) \
    {  \
        id_pos = strstr(fmt_spec,zce::SNRPINTF_FMT_IDENTIFY);  \
        zce::fmt_str(buffer,max_len,use_len,fmt_spec,(id_pos == NULL)?strlen(fmt_spec):(id_pos-fmt_spec)); \
        buffer += use_len; \
        max_len -= use_len; \
        foo_use_len += use_len; \
        fmt_spec += use_len;  \
        if (0 == max_len || id_pos == NULL ) \
        { \
            buffer[0] = '\0';  \
            return foo_buffer; \
        }  \
        if (* (id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR) \
        { \
            zce::output_helper(buffer,max_len,use_len,out_data##x);  \
            buffer += use_len; \
            max_len -= use_len; \
            foo_use_len += use_len; \
            fmt_spec += LEN_OF_FMT_IDENTIFY; \
            break; \
        } \
        else  \
        { \
            zce::fmt_str(buffer,max_len,use_len,SNRPINTF_FMT_IDENTIFY,LEN_OF_FMT_IDENTIFY); \
            buffer += use_len; \
            max_len -= use_len; \
            foo_use_len += use_len; \
            fmt_spec += LEN_OF_ESCAPE_IDENTIFY;  \
            continue;  \
        } \
    }

#define __ZCE_SNPRINTF_END zce::fmt_str(buffer, \
                                        max_len, \
                                        use_len, \
                                        fmt_spec, \
                                        strlen(fmt_spec)); \
foo_use_len += use_len; \
buffer[use_len] = '\0'; \
return foo_buffer

/*!
@brief      两个输出参数的snprintf函数，
            任何输出的位置，都用%?替代，这个函数不用宏定义，主要就是为了帮助我们调试，
            调试宏的代码，你不要他该有自信。
@tparam     T1  out_data1 的类型
@tparam     T2  out_data2 的类型
@return     char*        返回的BUFFER指针，方便你用于一些调用
@param[out] foo_buffer   输出字符串的buffer
@param[in]  foo_max_len  buffer的最大长度
@param[out] foo_use_len  返回格式化后使用的字符串长度
@param[in]  foo_fmt_spec 格式化字符串的format语句，里面用%?标识一个替代位置
@param      out_data1    输出的数据1
@param      out_data2    输出的数据2
*/
template <class T1, class T2>
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2)
{
    foo_use_len = 0;

    if ( 0 == foo_max_len )
    {
        return foo_buffer;
    }

    size_t max_len = foo_max_len - 1;
    size_t use_len = 0;
    char *buffer = foo_buffer;
    buffer[max_len] = '\0';
    const char *fmt_spec = foo_fmt_spec;

    const char *id_pos = NULL;

    //处理第一个数据的输出，
    while ( max_len > 0)
    {
        //找到%?，
        id_pos = strstr(fmt_spec, zce::SNRPINTF_FMT_IDENTIFY);
        //将%?前面的字符串输出
        zce::fmt_str(buffer, max_len, use_len, fmt_spec, (id_pos == NULL) ? strlen(fmt_spec) : (id_pos - fmt_spec));

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
        fmt_spec += use_len;

        if (0 == max_len || id_pos == NULL )
        {
            buffer[0] = '\0';
            return foo_buffer;
        }

        //检查后面字符是否是?，用于判断是否是转义
        if (* (id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::output_helper(buffer, max_len, use_len, out_data1);
            buffer += use_len;
            max_len -= use_len;
            foo_use_len += use_len;
            fmt_spec += LEN_OF_FMT_IDENTIFY;
            break;
        }
        //%??将转义为%?输出
        else
        {
            zce::fmt_str(buffer, max_len, use_len, SNRPINTF_FMT_IDENTIFY, LEN_OF_FMT_IDENTIFY);
            buffer += use_len;
            max_len -= use_len;
            foo_use_len += use_len;
            fmt_spec += LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }

    //处理第二个数据的输出，
    while ( max_len > 0)
    {
        //找到%?，
        id_pos = strstr(fmt_spec, zce::SNRPINTF_FMT_IDENTIFY);
        //将%?前面的字符串输出
        zce::fmt_str(buffer, max_len, use_len, fmt_spec, (id_pos == NULL) ? strlen(fmt_spec) : (id_pos - fmt_spec));

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
        fmt_spec += use_len;

        if (0 == max_len || id_pos == NULL )
        {
            buffer[0] = '\0';
            return foo_buffer;
        }

        //检查后面字符是否是?，用于判断是否是转义
        if (* (id_pos + LEN_OF_FMT_IDENTIFY) != zce::SNRPINTF_FMT_ESCAPE_CHAR)
        {
            zce::output_helper(buffer, max_len, use_len, out_data2);
            buffer += use_len;
            max_len -= use_len;
            foo_use_len += use_len;
            fmt_spec += LEN_OF_FMT_IDENTIFY;
            break;
        }
        //%??将转义为%?输出
        else
        {
            zce::fmt_str(buffer, max_len, use_len, SNRPINTF_FMT_IDENTIFY, LEN_OF_FMT_IDENTIFY);
            buffer += use_len;
            max_len -= use_len;
            foo_use_len += use_len;
            fmt_spec += LEN_OF_ESCAPE_IDENTIFY;
            continue;
        }
    }

    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 fmt_spec,
                 strlen(fmt_spec));
    foo_use_len += use_len;
    buffer[use_len] = '\0';
    //返回
    return foo_buffer;
}

template < class T1 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27,
                   const T28 &out_data28 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_REPEAT(28);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27,
                   const T28 &out_data28,
                   const T29 &out_data29 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_REPEAT(28);
    __ZCE_SNPRINTF_REPEAT(29);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27,
                   const T28 &out_data28,
                   const T29 &out_data29,
                   const T30 &out_data30 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_REPEAT(28);
    __ZCE_SNPRINTF_REPEAT(29);
    __ZCE_SNPRINTF_REPEAT(30);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27,
                   const T28 &out_data28,
                   const T29 &out_data29,
                   const T30 &out_data30,
                   const T31 &out_data31 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_REPEAT(28);
    __ZCE_SNPRINTF_REPEAT(29);
    __ZCE_SNPRINTF_REPEAT(30);
    __ZCE_SNPRINTF_REPEAT(31);
    __ZCE_SNPRINTF_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31, class T32 >
char *foo_snprintf(char *foo_buffer,
                   size_t foo_max_len,
                   size_t &foo_use_len,
                   const char *foo_fmt_spec,
                   const T1 &out_data1,
                   const T2 &out_data2,
                   const T3 &out_data3,
                   const T4 &out_data4,
                   const T5 &out_data5,
                   const T6 &out_data6,
                   const T7 &out_data7,
                   const T8 &out_data8,
                   const T9 &out_data9,
                   const T10 &out_data10,
                   const T11 &out_data11,
                   const T12 &out_data12,
                   const T13 &out_data13,
                   const T14 &out_data14,
                   const T15 &out_data15,
                   const T16 &out_data16,
                   const T17 &out_data17,
                   const T18 &out_data18,
                   const T19 &out_data19,
                   const T20 &out_data20,
                   const T21 &out_data21,
                   const T22 &out_data22,
                   const T23 &out_data23,
                   const T24 &out_data24,
                   const T25 &out_data25,
                   const T26 &out_data26,
                   const T27 &out_data27,
                   const T28 &out_data28,
                   const T29 &out_data29,
                   const T30 &out_data30,
                   const T31 &out_data31,
                   const T32 &out_data32 )
{
    __ZCE_SNPRINTF_BEGIN;
    __ZCE_SNPRINTF_REPEAT(1);
    __ZCE_SNPRINTF_REPEAT(2);
    __ZCE_SNPRINTF_REPEAT(3);
    __ZCE_SNPRINTF_REPEAT(4);
    __ZCE_SNPRINTF_REPEAT(5);
    __ZCE_SNPRINTF_REPEAT(6);
    __ZCE_SNPRINTF_REPEAT(7);
    __ZCE_SNPRINTF_REPEAT(8);
    __ZCE_SNPRINTF_REPEAT(9);
    __ZCE_SNPRINTF_REPEAT(10);
    __ZCE_SNPRINTF_REPEAT(11);
    __ZCE_SNPRINTF_REPEAT(12);
    __ZCE_SNPRINTF_REPEAT(13);
    __ZCE_SNPRINTF_REPEAT(14);
    __ZCE_SNPRINTF_REPEAT(15);
    __ZCE_SNPRINTF_REPEAT(16);
    __ZCE_SNPRINTF_REPEAT(17);
    __ZCE_SNPRINTF_REPEAT(18);
    __ZCE_SNPRINTF_REPEAT(19);
    __ZCE_SNPRINTF_REPEAT(20);
    __ZCE_SNPRINTF_REPEAT(21);
    __ZCE_SNPRINTF_REPEAT(22);
    __ZCE_SNPRINTF_REPEAT(23);
    __ZCE_SNPRINTF_REPEAT(24);
    __ZCE_SNPRINTF_REPEAT(25);
    __ZCE_SNPRINTF_REPEAT(26);
    __ZCE_SNPRINTF_REPEAT(27);
    __ZCE_SNPRINTF_REPEAT(28);
    __ZCE_SNPRINTF_REPEAT(29);
    __ZCE_SNPRINTF_REPEAT(30);
    __ZCE_SNPRINTF_REPEAT(31);
    __ZCE_SNPRINTF_REPEAT(32);
    __ZCE_SNPRINTF_END;
}


//默认的分隔符号是'\0',表示不需要分隔符，你可以通过函数设置分割符号

#define __ZCE_STRNSPLICE_BEGIN   foo_use_len = 0; \
    if ( 0 == foo_max_len ) \
    { \
        return foo_buffer; \
    } \
    size_t max_len = foo_max_len - 1; \
    size_t use_len = 0; \
    char *buffer = foo_buffer; \
    buffer[max_len] = '\0'

#define __ZCE_STRNSPLICE_REPEAT(x)   if (max_len > 0)  \
    { \
        zce::output_helper(buffer, max_len, use_len, out_data##x);\
        buffer += use_len; \
        max_len -= use_len; \
        foo_use_len += use_len; \
    } \
    if (separator_char != '\0' && max_len > 0 ) \
    {  \
        *buffer = separator_char;  \
        buffer += 1;   \
        max_len -= 1;  \
        foo_use_len += 1;  \
    }

#define __ZCE_STRNSPLICE_END   foo_buffer[foo_use_len] = '\0'; \
    return foo_buffer

template <class T1, class T2>
char *foo_strnsplice(char *foo_buffer,
                     size_t foo_max_len,
                     size_t &foo_use_len,
                     char separator_char,
                     const T1 &out_data1,
                     const T2 &out_data2)
{
    foo_use_len = 0;
    if (0 == foo_max_len)
    {
        return foo_buffer;
    }

    size_t max_len = foo_max_len - 1;
    size_t use_len = 0;
    char *buffer = foo_buffer;
    buffer[max_len] = '\0';

    //如果还有空间容纳字符
    if (max_len > 0)
    {
        zce::output_helper(buffer, max_len, use_len, out_data1);

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
    }
    //如果需要分隔符，添加分割符号
    if (separator_char != '\0' && max_len > 0)
    {
        *buffer = separator_char;
        buffer += 1;
        max_len -= 1;
        foo_use_len += 1;
    }

    if (max_len > 0)
    {
        zce::output_helper(buffer, max_len, use_len, out_data2);

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
    }

    //如果需要分隔符，添加分割符号
    if (separator_char != '\0' && max_len > 0)
    {
        *buffer = separator_char;
        buffer += 1;
        max_len -= 1;
        foo_use_len += 1;
    }

    foo_buffer[foo_use_len] = '\0';
    //返回
    return foo_buffer;
}

template < class T1 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_END;
}

//template < class T1,class T2 >
//void foo_strnsplice(char *foo_buffer,
//    size_t foo_max_len,
//    size_t &foo_use_len,
//    const T1 &out_data1,
//    const T2 &out_data2 )
//{
//    __ZCE_STRNSPLICE_BEGIN;
//    __ZCE_STRNSPLICE_REPEAT(1);
//    __ZCE_STRNSPLICE_REPEAT(2);
//    __ZCE_STRNSPLICE_END;
//}

template < class T1, class T2, class T3 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30,
                    const T31 &out_data31)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_REPEAT(31);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31, class T32 >
void foo_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    char separator_char,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30,
                    const T31 &out_data31,
                    const T32 &out_data32)
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_REPEAT(31);
    __ZCE_STRNSPLICE_REPEAT(32);
    __ZCE_STRNSPLICE_END;
}

#endif

};

#endif
