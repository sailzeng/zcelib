//
//Author:ZengXing
//用于ASNI string函数的补充
//2002-06-22
//


#include "zce_predefine.h"
#include "zce_os_adapt_process.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_string.h"












//==========================================================================================================
//取得一个唯一的名称,用于一些需要取唯一名字的地方，object一般选取一些指针考虑
char *ZCE_OS::object_unique_name (const void *object_ptr,
                                  char *name,
                                  size_t length)
{
    snprintf (name,
              length,
              "%u.%p",
              static_cast <int> (ZCE_OS::getpid ()),
              object_ptr
             );

    return name;
}


//通过前缀式，得到一个唯一的名称,
char *ZCE_OS::prefix_unique_name(const char *prefix_name,
                                 char *name,
                                 size_t length)
{
    static unsigned int uniqueid_builder = 0;

    ++uniqueid_builder;

    snprintf (name,
              length,
              "%s.%u.%x",
              prefix_name,
              static_cast <int> (ZCE_OS::getpid ()),
              uniqueid_builder
             );

    return name;
}



//==========================================================================================================
//将字符串全部转换为大写字符
char *ZCE_OS::strupr(char *str)
{

    assert(str);
    char *lstr = str;

    while (*lstr != '\0')
    {
        *lstr = static_cast<char> (::toupper(*lstr));
        ++lstr;
    }

    return str;
}

//将字符串全部转换为小写字符
char *ZCE_OS::strlwr(char *str)
{

    assert(str);
    char *lstr = str;

    while (*lstr++ != '\0')
    {
        *lstr = static_cast<char>(::tolower(*lstr));
    }

    return str;
}

//字符串比较，忽视大小写
//高效版本
int ZCE_OS::strcasecmp(const char *string1, const char *string2)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strcasecmp(string1, string2);
#elif defined (ZCE_OS_LINUX)
    return ::strcasecmp(string1, string2);
#endif

}

//字符串定长比较，忽视大小写
//高效版本
int ZCE_OS::strncasecmp(const char *string1, const char *string2, size_t maxlen)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strncasecmp(string1, string2, maxlen);
#elif defined (ZCE_OS_LINUX)
    return ::strncasecmp(string1, string2, maxlen);
#endif
}

//==========================================================================================================
//原来的代码中有人对ASCII 255的字符进行了判定处理，ASCII255是Non-breaking space，
//我暂时去掉了。感觉好像出现这种情况很特例。因为当时没有写注释，所以完全不记得场景了。

//左规整字符串，去掉字符串左边的空格，换行，回车，Tab
char *ZCE_OS::strtrimleft(char   *str)
{
    ZCE_ASSERT(str);

    char *lstr = str;

    for (; *lstr != '\0'; lstr++)
    {
        //注意isspace的参数是int,（WIN下）要求字符是0~255,
        if (::isspace(static_cast<unsigned char>(*lstr)) != 0 )
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
        memmove(str, lstr, strlen(lstr) + 1);
    }

    return str;
}

//右规整字符串，去掉字符串右边的空格，换行，回车，Tab
char *ZCE_OS::strtrimright(char *str)
{
    ZCE_ASSERT(str);
    char *lstr = str + strlen(str) - 1;

    for (; lstr >= str; lstr--)
    {
        //注意isspace的参数是int,（WIN下）要求字符是0~255,
        if (::isspace(static_cast<unsigned char> (*lstr)) != 0 )
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (lstr != str + strlen(str) - 1)
    {
        *++lstr = '\0';
    }

    return str;
}

//规整字符串，去掉字符串两边的空格，换行，回车，Tab
char *ZCE_OS::strtrim(char *str)
{
    ZCE_OS::strtrimleft(str);
    ZCE_OS::strtrimright(str);
    return str;
}


//-------------------------------------------------------------------------------------------------------------------
//跨越空白符，指空格、水平制表、垂直制表、换页、回车和换行符，这类字符都跨越，
const char *ZCE_OS::skip_whitespace (const char *str)
{
    while (::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    return str;
}

//跨越某个token
const  char *ZCE_OS::skip_token(const char *str)
{
    while (::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    while ( *str && 0 == ::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    //后面的空格要不要跳过？算了，留给下一个把
    //while (::isspace(static_cast<unsigned char>(*str)))
    //{
    //    ++str;
    //}
    return str;
}

//跨越一行
const char *ZCE_OS::skip_line(const char *str)
{
    while ( ('\n' != (*str)) && ('\0' != (*str)) )
    {
        ++str;
    }

    //如果是换行符，前进一个
    if ('\n' == (*str) )
    {
        ++str;
    }

    return str;
}



//跨越谋个分隔符号
const char *ZCE_OS::skip_separator(const char *str, char separator_char)
{
    while ( (separator_char != (*str)) && ('\0' != (*str)) )
    {
        ++str;
    }

    //如果是换行符，前进一个
    if (separator_char == (*str) )
    {
        ++str;
    }

    return str;
}




//==========================================================================================================

//调试打印内存信息，就是简单的内存翻译为16进制字符串
void ZCE_OS::memory_debug(FILE *stream, const unsigned char *mem, size_t mem_len)
{
    fprintf(stream, "DEBUG memory[%p][%lu]", mem, mem_len);
    for (size_t i = 0; i < mem_len; ++i)
    {
        fprintf(stream, "%02x", mem[i]);
    }
    fprintf(stream, "\n");
}


//用 11 02 03 0E E0         ..... 格式的输出，指针信息。调试打印内存信息
void ZCE_OS::memory_debug_ex(FILE *stream, const unsigned char *mem, size_t mem_len)
{
    //60个字符换行
    const unsigned int LINE_OUT_NUM = 60;

    unsigned char ascii_str[LINE_OUT_NUM + 1];
    ascii_str[LINE_OUT_NUM] = '\0';
    size_t j = 0;
    for (size_t i = 0; i < mem_len ; ++i, ++j)
    {
        //换行
        if (i % LINE_OUT_NUM == 0 && i != 0  )
        {
            fprintf(stream, "  %s\n", ascii_str);
            //从头开始记录
            j = 0;
        }
        unsigned char bytmp = *(mem + i);
        fprintf(stream, "%02X ", bytmp);


        //只考虑能显示的字符，特殊字符更换为'.'
        if (bytmp <= 0x20 || bytmp >= 0xFA )
        {
            bytmp = '.';
        }
        ascii_str [j] = bytmp;
    }

    //如果不是LINE_OUT_NUM 长度整除，要对齐，输出最后的字符串
    if (mem_len % LINE_OUT_NUM != 0 )
    {
        //为了对齐，打印空格
        for (size_t k = 0; k < LINE_OUT_NUM - mem_len % LINE_OUT_NUM; k++)
        {
            fprintf(stream, "%s", "   ");
        }

        ascii_str[j] = '\0';
        fprintf(stream, "  %s\n", ascii_str);
    }
}


//==========================================================================================================

//快速内存拷贝，当然其实他并不算块，
//这个纯属好玩的，经过测试，他其实并没有memcpy快，所以不建议使用
void *ZCE_OS::fast_memcpy(void *dst, const void *src, size_t sz)
{
    void *r = dst;

    //先进行uint64_t长度的拷贝，一般而言，内存地址都是对齐的，
    size_t n = sz & ~(sizeof(uint64_t) - 1);
    uint64_t *src_u64 = (uint64_t *) src;
    uint64_t *dst_u64 = (uint64_t *) dst;

    while (n)
    {
        *dst_u64++ = *src_u64++;
        n -= sizeof(uint64_t);
    }

    //将没有非8字节字长取整的部分copy
    n = sz & (sizeof(uint64_t) - 1);
    uint8_t *src_u8 = (uint8_t *) src;
    uint8_t *dst_u8 = (uint8_t *) dst;
    while (n-- )
    {
        (*dst_u8++ = *src_u8++);
    }

    return r;
}

//快速内存拷贝的第二个版本，其实就是在复制的时候增加了一次复制，更加优化一点
//这个也没有memcpy快
void *ZCE_OS::fast_memcpy2(void *dst, const void *src, size_t sz)
{
    void *r = dst;

    //先进行uint64_t长度的拷贝，一般而言，内存地址都是对齐的，
    size_t n = sz & ~((sizeof(uint64_t)<< 1)  - 1);
    uint64_t *src_u64 = (uint64_t *) src;
    uint64_t *dst_u64 = (uint64_t *) dst;

    while (n)
    {
        *dst_u64++ = *src_u64++;
        *dst_u64++ = *src_u64++;
        n -= sizeof(uint64_t)*2;
    }

    //讲没有非8字节字长的部分copy
    n = sz & ((sizeof(uint64_t)<<1) - 1);
    uint8_t *src_u8 = (uint8_t *) src;
    uint8_t *dst_u8 = (uint8_t *) dst;
    while (n-- )
    {
        (*dst_u8++ = *src_u8++);
    }
    return r;
}
