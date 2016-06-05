/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_string.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2002年06月22日
* @brief      用于ASNI string函数的补充
*
*
* @details    定义基本LIB函数，供大家一起使用
*
*
*
* @note       这里面有部分应该是ZCELIB(ZENLIB)最早的代码。
*             多年后整理这段代码，发现有自己的工号信息，哈哈，
*             2002年，我应该在北京出差回来，HW的商业网络部门刚刚成立的时候
*             当然后来，这里面的相当部分代码还是重写了，
*
*/


#ifndef ZCE_LIB_OS_ADAPT_STRING_H_
#define ZCE_LIB_OS_ADAPT_STRING_H_


#include "zce_trace_debugging.h"



namespace ZCE_LIB
{
//==========================================================================================================

//


/*!
* @brief      字符的功能的萃取类，用在兼容char，wchar_t字符串处理的时候还是挺有用的
*             yunfeiyang GG搞得一套东西，
* @tparam     char_type 字符类型，char 或者 wchar_t
* @note       char_traits里面还有 length,move等函数
*/
template <typename char_type>
struct yun_char_traits : public std::char_traits < char_type >
{
};

template <>
struct yun_char_traits < char > : public std::char_traits < char >
{
    typedef std::string string_type;

    //注意 int_type 就是std::char_traits<wchar_t>::int_type
    static int tolower(int_type c)
    {
        return ::tolower(c);
    }
    static int toupper(int_type c)
    {
        return ::toupper(c);
    }

    static int isspace(int_type c)
    {
        return (isascii(c) && ::isspace(c));
    }

    static char_type *strend(char_type *s)
    {
        return ::strchr(s, 0);
    }
    static const char_type *strend(const char_type *s)
    {
        return ::strchr(s, 0);
    }

};

template <>
struct yun_char_traits < wchar_t > : public std::char_traits < wchar_t >
{
    typedef std::wstring    string_type;

    //注意 int_type 就是std::char_traits<wchar_t>::int_type
    static int tolower(int_type c)
    {
        return ::towlower(c);
    }

    static int toupper(int_type c)
    {
        return ::towupper(c);
    }

    static int isspace(int_type c)
    {
        return ::iswspace(c);
    }

    static char_type *strend(char_type *s)
    {
        return ::wcschr(s, 0);
    }

    static const char_type *strend(const char_type *s)
    {
        return ::wcschr(s, 0);
    }

};


//==========================================================================================================
//原来的代码中有人对ASCII 255的字符进行了判定处理，ASCII255是Non-breaking space，
//我暂时去掉了。感觉好像出现这种情况很特例。因为当时没有写注释，所以完全不记得场景了。

/*!
* @brief      左规整字符串，去掉字符串左边的空格，换行，回车，Tab
* @tparam     char_type 字符类型，char or wchar_t
* @return     char_type *
* @param      str
*/
template <typename char_type>
char_type *strtrimleft(char_type *str)
{
    ZCE_ASSERT(str);

    char_type *lstr = str;
    for (; *lstr != '\0'; lstr++)
    {
        //注意isspace的参数是int,（WIN下）要求字符是0~255,
        if (yun_char_traits<char_type>::isspace(*lstr) != 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (lstr != str)
    {
        yun_char_traits<char_type>::move(str, lstr,
                                         yun_char_traits<char_type>::length(lstr) + 1);
    }

    return str;
}


/*!
* @brief      右规整字符串，去掉字符串右边的空格，换行，回车，Tab
* @return     char*
* @param      str
*/
template <typename char_type>
char *strtrimright(char_type *str)
{
    ZCE_ASSERT(str);
    char_type *lstr = str + yun_char_traits<char_type>::length(str) - 1;

    for (; lstr >= str; lstr--)
    {
        //注意isspace的参数是int,（WIN下）要求字符是0~255,
        if (yun_char_traits<char_type>::isspace(*lstr) != 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (lstr != str + yun_char_traits<char_type>::length(str) - 1)
    {
        *++lstr = 0;
    }

    return str;
}


/*!
* @brief      规整字符串，去掉字符串两边的空格，换行，回车，Tab
* @return     char*
* @param      str
*/
template <typename char_type>
inline char *strtrim(char_type *str)
{
    ZCE_LIB::strtrimleft<char_type>(str);
    ZCE_LIB::strtrimright<char_type>(str);
    return str;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
void trimleft_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    typename std::basic_string<char_type, char_traits_type, allocator_typ>::iterator it = str.begin();

    while (it != str.end() && yun_char_traits<char_type>::isspace(*it))
    {
        ++it;
    }

    str.erase(str.begin(), it);
    return;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
void trimright_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    typename std::basic_string<char_type, char_traits_type, allocator_typ>::iterator it = str.end();

    if (it == str.begin())
    {
        return;
    }

    while (it != str.begin() && yun_char_traits<char_type>::isspace(*--it));

    str.erase(it + 1, str.end());

    return;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
inline void trim_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    trimright_stdstr(str);
    trimleft_stdstr(str);
    return;
}

//==========================================================================================================
//下面这几个函数目前的平台可能有，但当时的某个平台可能缺失，是HP？还是IBM？SUN？无法确认了，
//暂时保留了。


/*!
* @brief      将字符串全部转换为大写字符
*/
char *strupr(char *str);


/*!
* @brief      将字符串全部转换为小写字符
*/
char *strlwr(char *str);


/*!
* @brief      字符串比较，忽视大小写
* @return     int
* @param      string1
* @param      string2
*/
int strcasecmp(const char *string1, const char *string2);

/*!
* @brief      字符串定长比较，忽视大小写
* @return     int
* @param      string1
* @param      string2
* @param      maxlen
* @note
*/
int strncasecmp(const char *string1, const char *string2, size_t maxlen);




//==========================================================================================================
//产生唯一名字的一些函数


/*!
* @brief      通过对象指针，取得一个唯一的名称,用于一些需要名字的地方,保证传入的空间有48个字节，
* @return     char*           返回的名字
* @param[in]  object_ptr  对象指针，某个对象的指针地址，通过这个指针地址转换得到唯一名字
* @param[out] name        名字的buffer
* @param[in]  length      buffer的长度
*/
char *object_unique_name (const void *object_ptr,
                          char *name,
                          size_t length);


/*!
* @brief      通过前缀式，得到一个唯一的名称,唯一名称包括，前缀+进程ID+内部计数器
* @return     char*        返回的名字
* @param[in]  prefix_name  前缀的字符串
* @param[out] name         名字的buffer
* @param[in]  length       buffer的长度
*/
char *prefix_unique_name(const char *prefix_name,
                         char *name,
                         size_t length);

//==========================================================================================================

/*!
* @brief      分割字符串，
* @tparam     iter_type1  被分割数据的迭代器类型
* @tparam     iter_type2  分割标示的迭代器类型
* @tparam     compare_type 比较方式
* @tparam     container_type 容器类型
* @param[in]  fs 被分割数据开始位置的迭代器
* @param[in]  ls 被分割数据结束位置的迭代器
* @param[in]  fo 分割标示的开始位置迭代器
* @param[in]  lo 分割标示的结束位置迭代器
* @param[in]  pr 比较方式
* @param[out] v  输出的容器，一般是vector<string>
*/
template < typename iter_type1, typename iter_type2, typename compare_type, typename container_type>
void _str_split(iter_type1 fs, iter_type1 ls, iter_type2 fo, iter_type2 lo, compare_type pr, container_type &v)
{

    //使用尾部插入的迭代器
    std::back_insert_iterator<container_type> o = std::back_inserter(v);
    if (fo == lo)
    {
        *o =typename container_type::value_type(fs, ls);
        return;
    }

    // current position old_str in str
    iter_type1 pos = std::search(fs, ls, fo, lo, pr);

    if (pos == ls)
    {
        if (fs < ls)
        {
            *o = typename container_type::value_type(fs, ls);
        }

        return;
    }

    size_t old_size = std::distance(fo, lo);

    for (; pos != ls;)
    {
        // append src string
        *o = typename container_type::value_type(fs, pos);
        // ignore old_str
        fs = pos + old_size;
        // find next
        pos = std::search(fs, ls, fo, lo, pr);
    }

    if (fs != ls)
    {
        *o = typename container_type::value_type(fs, ls);
    }

    return;
}


/*!
* @brief      根据分割符，将一个字符串，分割成若干个字符串，放入容器v
* @tparam     char_type      字符类型
* @tparam     container_type 容器类型，
* @param[in]  str            处理的字符串，
* @param[in]  separator      分割字符串，注意如果是单个字符，也要用""括起来
* @param[out] v              容器，用于存放分隔的结果,一般是vector<string>
*/
template < typename char_type, typename container_type >
inline void str_split(const char_type *str, const char_type *separator, container_type &v)
{
    _str_split(str,
               yun_char_traits<char_type>::strend(str),
               separator,
               yun_char_traits<char_type>::strend(separator),
               std::equal_to<char_type>(),
               v);
    return;
}

template < typename container_type >
inline void string_split(const std::string &source_str,
                         const std::string &separator,
                         container_type &v)
{
    _str_split(source_str.begin(),
               source_str.end(),
               separator.begin(),
               separator.end(),
               std::equal_to<std::string::value_type>(),
               v);
    return;
}


/*!
* @brief      将一个字符串根据分割符（也可以是一个字符串）分成2个字符串。
* @param      src_str 原字符串
* @param      search_str 分割的字符串
* @param      str_1 分割的第一个字符串
* @param      str_2 分割得到的第二个字符串
*/
void str_split_into2(const std::string &src_str,
                     const char *search_str,
                     std::string &str_1,
                     std::string &str_2);


/*!
* @brief
* @return     const char*
* @param      src
* @param      dst
* @param      sub
* @param      replace
*/
const char *str_replace(const char *src,
                        char *dst,
                        const char *sub,
                        const char *replace);

//==========================================================================================================
//用于一些表格字符串分析函数

/*!
* @brief      跨越空白符，指空格、水平制表、垂直制表、换页、回车和换行符，这类字符都跨越，
* @return     const char* 跳过后的地址
* @param      str         参数字符串
* @note       为了分析一些表格信息，增加的一组函数，一致一些类top的代码时翻译的
*/
const char *skip_whitespace(const char *str);

//
/*!
* @brief      跨越某个token，跳过某个单词
* @return     const char* 跳过后的地址
* @param      str         参数字符串
*/
const char *skip_token(const char *str);


/*!
* @brief      跳过一行
* @return     const char* 跳过后的地址
* @param[in]  str         参数字符串
* @note
*/
const char *skip_line(const char *str);


/*!
* @brief      跨越某个token,直到分隔符后（或者结束）
* @return     const char*    跳过后的地址
* @param[in]  str            参数字符串
* @param[in]  separator_char 分割字符，要找到这个字符为止，（跳到这个字符后面）
*/
const char *skip_separator(const char *str, char separator_char);



//==========================================================================================================


/*!
* @brief      一个简单的打印输出某个内存地址信息的函数，就是简单的内存翻译为16进制字符串
* @param      stream  文件流对象，stdout,stderr也都可以
* @param      mem     调试的内存指针
* @param      mem_len 指针长度，
* @note       如果是日志输出提供了方便的辅助类，这仅仅用于不需要使用日志的地方
*/
void memory_debug(FILE *stream, const unsigned char *mem, size_t mem_len);



/*!
* @brief      用 11 02 03 0E E0         ..... 格式的输出，指针信息。调试打印内存信息
*             格式信息更加丰富，和清晰，当然打印也更加慢
* @param      stream  文件流对象，stdout,stderr也都可以
* @param      mem     调试的内存指针
* @param      mem_len 指针长度
*/
void memory_debug_ex(FILE *stream, const unsigned char *mem, size_t mem_len);


//==========================================================================================================

/*!
* @brief      快速内存拷贝函数，主要就是利用字节对齐，8字节的拷贝快过单字节进行，
*             主要是受到文章  http://blog.chinaunix.net/uid-20043340-id-154852.html 的影响
*             但其实这文章可信度不大，我自己实际测试仍然是memcpy更快，作者可能是在非优化条件下的测试。
*             有兴趣的参考阅读。
*             http://www.cnblogs.com/fullsail/p/3160098.html
*             里面参考的阅读的部分也都可以读一下。
* @return     void*  返回dst
* @param      dst    目标地址
* @param      src    源数据地址
* @param      sz     拷贝的长度
* @note       任何时候都，都请优先选择 C运行库的 memcpy,
*/
void *fast_memcpy(void *dst, const void *src, size_t sz);


void *fast_memcpy2(void *dst, const void *src, size_t sz);

//==========================================================================================================


/*!
* @brief      将c str 转换为很多数值类型，作为返回值返回
* @tparam     ret_type 返回值类型
* @return     ret_type
* @param[in]  str  c string 字符串参数
*/
template<typename ret_type>
ret_type str_to_value(const char *str);


/*!
* @brief      将c string 转换为很多数值类型，作为指针参数返回
* @tparam     ptr_type 指针参数类型
* @param[in]  str c string 字符串参数
* @param[out] ptr 指针参数
*/
template<typename ptr_type>
void str_to_ptr(const char *str, ptr_type *ptr);


/*!
* @brief      从std string字符串转换得到数据类型
* @tparam     ret_type  返回的类型
* @return     ret_type  返回的数据类型
* @param[in]  str       字符串
*/
template<typename ret_type>
ret_type string_to_value(const std::string &stdstr)
{
    return str_to_value<ret_type>(stdstr.c_str());
}

/*!
* @brief      将std string 转换为很多数值类型，作为指针参数返回
* @tparam     ptr_type  指针参数类型
* @param      stdstr
* @param      ptr
*/
template<typename ptr_type>
void string_to_ptr(const std::string &stdstr, ptr_type *ptr)
{
    str_to_ptr<ptr_type>(stdstr.c_str(), ptr);
}

};














#endif //ZCE_LIB_STRING_EX_H_

