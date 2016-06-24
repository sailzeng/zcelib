//
//Author:ZengXing
//用于ASNI string函数的补充
//始于2002-06-22，当然内部反复改写过
//


#include "zce_predefine.h"
#include "zce_os_adapt_process.h"
#include "zce_trace_debugging.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_time_value.h"
#include "zce_lock_thread_mutex.h"
#include "zce_os_adapt_string.h"


//==========================================================================================================
//取得一个唯一的名称,用于一些需要取唯一名字的地方，object一般选取一些指针考虑
char *ZCE_LIB::object_unique_name (const void *object_ptr,
                                   char *name,
                                   size_t length)
{
    snprintf (name,
              length,
              "%u.%p",
              static_cast <int> (ZCE_LIB::getpid ()),
              object_ptr
             );

    return name;
}


//通过前缀式，得到一个唯一的名称,
char *ZCE_LIB::prefix_unique_name(const char *prefix_name,
                                  char *name,
                                  size_t length)
{


    ZCE_Thread_Light_Mutex id_lock;
    ZCE_Lock_Guard<ZCE_Thread_Light_Mutex> id_guard(id_lock);

    static unsigned int uniqueid_builder = 0;
    ++uniqueid_builder;

    snprintf (name,
              length,
              "%s.%u.%x",
              prefix_name,
              static_cast <int> (ZCE_LIB::getpid ()),
              uniqueid_builder
             );

    return name;
}



//==========================================================================================================
//将字符串全部转换为大写字符
char *ZCE_LIB::strupr(char *str)
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
char *ZCE_LIB::strlwr(char *str)
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
int ZCE_LIB::strcasecmp(const char *string1, const char *string2)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strcasecmp(string1, string2);
#elif defined (ZCE_OS_LINUX)
    return ::strcasecmp(string1, string2);
#endif

}

//字符串定长比较，忽视大小写
//高效版本
int ZCE_LIB::strncasecmp(const char *string1, const char *string2, size_t maxlen)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strncasecmp(string1, string2, maxlen);
#elif defined (ZCE_OS_LINUX)
    return ::strncasecmp(string1, string2, maxlen);
#endif
}



//替换src字符串中的sub字符串为replace，保存到dst字符串中
const char *ZCE_LIB::str_replace(const char *src, char *dst, const char *sub, const char *replace)
{
    ZCE_ASSERT(src && dst && sub && replace);

    //记录当前指针位置,偏移
    size_t  dst_offset = 0, src_offset = 0;

    //求得各字符串长度
    size_t src_len = strlen(src);
    size_t sub_len = strlen(sub);
    size_t replace_len = strlen(replace);

    const char *find_pos = NULL;
    //strstr查找sub字符串出现的指针
    while ( 0 != (find_pos = strstr(src + src_offset, sub)) )
    {

        //拷贝src字符串，从首地址开始，pos个字符。
        memcpy(dst + dst_offset, src + src_offset, find_pos - (src + src_offset));

        dst_offset += find_pos - (src + src_offset);
        src_offset = find_pos - src + sub_len;

        memcpy(dst + dst_offset, replace, replace_len);
        dst_offset += replace_len;
    }

    //把sub字符串后面的字符串到dst中
    memcpy(dst + dst_offset, src + src_offset, src_len - src_offset);
    dst_offset += src_len - src_offset;

    //最后添加字符串结尾标记'\0'
    *(dst + dst_offset) = '\0';

    //返回dst
    return dst;
}


void str_split_into2(const std::string &src_str,
                     const std::string &search_str,
                     std::string &str_1,
                     std::string &str_2)
{
    // current position old_str in str
    auto pos = std::search(src_str.begin(),
                           src_str.end(),
                           search_str.begin(),
                           search_str.end());

    if (pos == src_str.end())
    {
        str_1.assign(src_str.begin(), src_str.end());
        return;
    }

    size_t old_size = search_str.length();

    if ( pos != src_str.end())
    {
        // append src string
        str_1.assign(src_str.begin(), pos );
    }

    if (pos + old_size != src_str.end())
    {
        str_2.assign(pos + old_size, src_str.end());
    }
}

//-------------------------------------------------------------------------------------------------------------------
//跨越空白符，指空格、水平制表、垂直制表、换页、回车和换行符，这类字符都跨越，
const char *ZCE_LIB::skip_whitespace (const char *str)
{
    while (::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    return str;
}

//跨越某个token
const  char *ZCE_LIB::skip_token(const char *str)
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
const char *ZCE_LIB::skip_line(const char *str)
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
const char *ZCE_LIB::skip_separator(const char *str, char separator_char)
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
void ZCE_LIB::memory_debug(FILE *stream, const unsigned char *mem, size_t mem_len)
{
    fprintf(stream, "DEBUG memory[%p][%lu]", mem, mem_len);
    for (size_t i = 0; i < mem_len; ++i)
    {
        fprintf(stream, "%02x", mem[i]);
    }
    fprintf(stream, "\n");
}


//用 11 02 03 0E E0         ..... 格式的输出，指针信息。调试打印内存信息
void ZCE_LIB::memory_debug_ex(FILE *stream, const unsigned char *mem, size_t mem_len)
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
void *ZCE_LIB::fast_memcpy(void *dst, const void *src, size_t sz)
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
void *ZCE_LIB::fast_memcpy2(void *dst, const void *src, size_t sz)
{
    void *r = dst;

    //先进行uint64_t长度的拷贝，一般而言，内存地址都是对齐的，
    size_t n = sz & ~((sizeof(uint64_t) << 1)  - 1);
    uint64_t *src_u64 = (uint64_t *) src;
    uint64_t *dst_u64 = (uint64_t *) dst;

    while (n)
    {
        *dst_u64++ = *src_u64++;
        *dst_u64++ = *src_u64++;
        n -= sizeof(uint64_t) * 2;
    }

    //讲没有非8字节字长的部分copy
    n = sz & ((sizeof(uint64_t) << 1) - 1);
    uint8_t *src_u8 = (uint8_t *) src;
    uint8_t *dst_u8 = (uint8_t *) dst;
    while (n-- )
    {
        (*dst_u8++ = *src_u8++);
    }
    return r;
}

//==========================================================================================================

//GCC 编译的时候不能处理template<> int8_t ZCE_LIB::str_to_value(const char *str)
//的写法，只能把名字空间放到外面来了，估计是和class容易混淆
namespace ZCE_LIB
{ 

//将c str 转换为很多数值类型，作为返回值返回
template<>
int8_t str_to_value(const char *str)
{
    //10表示10进制，
    char   *stopstring = NULL;
    return static_cast<int8_t>(::strtol(str, &stopstring, 10));
}

template<>
uint8_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<uint8_t>(::strtol(str, &stopstring, 10));
}

template<>
int16_t str_to_value(const char *str)
{
    //10表示10进制，
    char   *stopstring = NULL;
    return static_cast<int16_t>(::strtol(str, &stopstring, 10));
}

template<>
uint16_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<uint16_t>(::strtol(str, &stopstring, 10));
}

template<>
int32_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<int32_t>(::strtol(str, &stopstring, 10));
}


template<>
uint32_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<uint32_t>(::strtoul(str, &stopstring, 10));
}


template<>
int64_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<int64_t>(::strtoll(str, &stopstring, 10));
}


template<>
uint64_t str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<uint64_t>(::strtoull(str, &stopstring, 10));
}

template<>
double str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<double>(::strtod(str, &stopstring));
}


template<>
float str_to_value(const char *str)
{
    char   *stopstring = NULL;
    return static_cast<float>(::strtod(str, &stopstring));
}


template<>
bool str_to_value(const char *str)
{
    if (0 == strcasecmp("TRUE", str))
    {
        return true;
    }
    else if (1 == ::atoi(str))
    {
        return true;
    }
    return false;
}

//
template<>
ZCE_Sockaddr_In str_to_value(const char *str)
{
    ZCE_Sockaddr_In val;
    val.set(str);
    return val;
}

template<>
ZCE_Sockaddr_In6 str_to_value(const char *str)
{
    ZCE_Sockaddr_In6 val;
    val.set(str);
    return val;
}

template<>
ZCE_Time_Value str_to_value(const char *str)
{
    ZCE_Time_Value val;
    val.from_string(str, false, ZCE_LIB::TIME_STRFMT_US_SEC);
    return val;
}


//将c str 转换为很多数值类型，作为指针参数返回
template<>
void str_to_ptr(const char *str, int8_t *ptr)
{
    //10表示10进制，
    char   *stopstring = NULL;
    *ptr = static_cast<int8_t>(::strtol(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, uint8_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<uint8_t>(::strtol(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, int16_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<int16_t>(::strtol(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, uint16_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<uint16_t>(::strtol(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, int32_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<int32_t>(::strtol(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, uint32_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<uint32_t>(::strtoul(str, &stopstring, 10));
}


template<>
void str_to_ptr(const char *str, int64_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<int64_t>(::strtoll(str, &stopstring, 10));
}


template<>
void str_to_ptr(const char *str, uint64_t *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<uint64_t>(::strtoull(str, &stopstring, 10));
}

template<>
void str_to_ptr(const char *str, double *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<double>(::strtod(str, &stopstring));
}


template<>
void str_to_ptr(const char *str, float *ptr)
{
    char   *stopstring = NULL;
    *ptr = static_cast<float>(::strtod(str, &stopstring));
}


template<>
void str_to_ptr(const char *str, bool *ptr)
{
    if (0 == strcasecmp("TRUE", str))
    {
        *ptr = true;
    }
    else if (1 == ::atoi(str))
    {
        *ptr = true;
    }
    *ptr = false;
}


template<>
void str_to_ptr(const char *str, ZCE_Sockaddr_In *ptr)
{
    ptr->set(str);
}

template<>
void str_to_ptr(const char *str, ZCE_Sockaddr_In6 *ptr)
{
    ptr->set(str);
}

template<>
void str_to_ptr(const char *str, ZCE_Time_Value *ptr)
{
    ptr->from_string(str, false, ZCE_LIB::TIME_STRFMT_US_SEC);
}

};
