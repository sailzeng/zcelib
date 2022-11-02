//用于ASNI string函数的补充
//始于2002-06-22，当然内部反复改写过
//

#include "zce/predefine.h"
#include "zce/os_adapt/process.h"
#include "zce/logger/logging.h"
#include "zce/lock/thread_mutex.h"
#include "zce/os_adapt/string.h"

//==========================================================================================================
//取得一个唯一的名称,用于一些需要取唯一名字的地方，object一般选取一些指针考虑
char* zce::object_unique_name(const void* object_ptr,
                              char* name,
                              size_t length)
{
    snprintf(name,
             length,
             "%u.%p",
             static_cast <int> (zce::getpid()),
             object_ptr
    );

    return name;
}

//通过前缀式，得到一个唯一的名称,
char* zce::prefix_unique_name(const char* prefix_name,
                              char* name,
                              size_t length)
{
    thread_light_mutex id_lock;
    lock_guard<thread_light_mutex> id_guard(id_lock);

    static unsigned int uniqueid_builder = 0;
    ++uniqueid_builder;

    snprintf(name,
             length,
             "%s.%u.%x",
             prefix_name,
             static_cast <int> (zce::getpid()),
             uniqueid_builder
    );

    return name;
}

//==========================================================================================================
//将字符串全部转换为大写字符
char* zce::strupr(char* str)
{
    assert(str);
    char* lstr = str;

    while (*lstr != '\0')
    {
        *lstr = static_cast<char> (::toupper(*lstr));
        ++lstr;
    }

    return str;
}

//将字符串全部转换为小写字符
char* zce::strlwr(char* str)
{
    assert(str);
    char* lstr = str;

    while (*lstr++ != '\0')
    {
        *lstr = static_cast<char>(::tolower(*lstr));
    }

    return str;
}

//字符串比较，忽视大小写
//高效版本
int zce::strcasecmp(const char* string1, const char* string2)
{
#if defined (ZCE_OS_WINDOWS)  //原来Windows是stricmp
    return ::strcasecmp(string1, string2);
#elif defined (ZCE_OS_LINUX)
    return ::strcasecmp(string1, string2);
#endif
}

//字符串定长比较，忽视大小写
//高效版本
int zce::strncasecmp(const char* string1, const char* string2, size_t maxlen)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strncasecmp(string1, string2, maxlen);
#elif defined (ZCE_OS_LINUX)
    return ::strncasecmp(string1, string2, maxlen);
#endif
}

//替换src字符串中的sub字符串为replace，保存到dst字符串中
const char* zce::str_replace(const char* src, char* dst, const char* sub, const char* replace)
{
    ZCE_ASSERT(src && dst && sub && replace);

    //记录当前指针位置,偏移
    size_t  dst_offset = 0, src_offset = 0;

    //求得各字符串长度
    size_t src_len = strlen(src);
    size_t sub_len = strlen(sub);
    size_t replace_len = strlen(replace);

    const char* find_pos = nullptr;
    //strstr查找sub字符串出现的指针
    while (0 != (find_pos = strstr(src + src_offset, sub)))
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

void str_split_into2(const std::string& src_str,
                     const std::string& search_str,
                     std::string& str_1,
                     std::string& str_2)
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

    if (pos != src_str.end())
    {
        // append src string
        str_1.assign(src_str.begin(), pos);
    }

    if (pos + old_size != src_str.end())
    {
        str_2.assign(pos + old_size, src_str.end());
    }
}

//-------------------------------------------------------------------------------------------------------------------
//跨越空白符，指空格、水平制表、垂直制表、换页、回车和换行符，这类字符都跨越，
const char* zce::skip_whitespace(const char* str)
{
    while (::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    return str;
}

//跨越某个token
const  char* zce::skip_token(const char* str)
{
    while (::isspace(static_cast<unsigned char>(*str)))
    {
        ++str;
    }

    while (*str && 0 == ::isspace(static_cast<unsigned char>(*str)))
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
const char* zce::skip_line(const char* str)
{
    while (('\n' != (*str)) && ('\0' != (*str)))
    {
        ++str;
    }

    //如果是换行符，前进一个
    if ('\n' == (*str))
    {
        ++str;
    }

    return str;
}

//跨越谋个分隔符号
const char* zce::skip_separator(const char* str, char separator_char)
{
    while ((separator_char != (*str)) && ('\0' != (*str)))
    {
        ++str;
    }

    //如果是换行符，前进一个
    if (separator_char == (*str))
    {
        ++str;
    }

    return str;
}

//==========================================================================================================

//用 11 02 03 0E E0         ..... 格式的输出，指针信息。调试打印内存信息
void zce::memory_debug(const unsigned char* mem_ptr,
                       size_t mem_len,
                       std::vector<std::string>& str_ary)
{
    //一行输出的字符数量
    const size_t LINE_OUT_NUM = 32;
    const size_t SPEARATOR_LEN = 2;
    char ascii_str[LINE_OUT_NUM + SPEARATOR_LEN + 1] = { 0 };
    ascii_str[LINE_OUT_NUM + SPEARATOR_LEN] = '\0';
    const size_t HEX_STR_LEN = 4;
    char hex_str[HEX_STR_LEN] = { 0 };
    const size_t ADDR_STR_LEN = 32;
    char addr_str[ADDR_STR_LEN] = { 0 };
    //头部用4个空格作为分割富豪

    ascii_str[0] = ' ';
    ascii_str[1] = ' ';

    std::string line_string;
    line_string.reserve(LINE_OUT_NUM * 4 + 64);
    str_ary.reserve(mem_len / LINE_OUT_NUM + 1);

    size_t j = 0;
    for (size_t i = 0; i < mem_len; ++i, ++j)
    {
        //换行
        if (i % LINE_OUT_NUM == 0 && i != 0)
        {
            line_string += ascii_str;
            str_ary.push_back(line_string);
            line_string.clear();
            //从头开始记录
            j = 0;
        }
        if (j == 0)
        {
            snprintf(addr_str, ADDR_STR_LEN, "%p ", mem_ptr + i);
            line_string += addr_str;
        }
        unsigned char bytmp = *(mem_ptr + i);
        snprintf(hex_str, HEX_STR_LEN, "%02X ", bytmp);
        line_string += hex_str;

        //只考虑能显示的字符，特殊字符更换为'.',扩展ASICII码就不考虑了
        if (bytmp <= 0x20 || bytmp >= 0x7F)
        {
            bytmp = '.';
        }
        ascii_str[SPEARATOR_LEN + j] = bytmp;
    }

    //如果不是LINE_OUT_NUM 长度整除，要对齐，输出最后的字符串
    if (mem_len % LINE_OUT_NUM != 0)
    {
        //为了对齐，打印空格
        for (size_t k = 0; k < LINE_OUT_NUM - mem_len % LINE_OUT_NUM; k++)
        {
            snprintf(hex_str, HEX_STR_LEN, "   ");
            line_string += hex_str;
        }
        ascii_str[SPEARATOR_LEN + j] = '\0';
        line_string += ascii_str;
        str_ary.push_back(line_string);
    }
}

//调试打印内存信息，就是简单的内存翻译为16进制字符串
void zce::memory_debug(const unsigned char* mem_ptr, size_t mem_len, FILE* stream)
{
    //%zu不知道VC从什么年代支持的
    fprintf(stream, "DEBUG memory[%p][%zu] \n", mem_ptr, mem_len);
    std::vector<std::string> str_ary;
    zce::memory_debug(mem_ptr, mem_len, str_ary);
    for (std::string& out : str_ary)
    {
        fprintf(stream, "%s\n", out.c_str());
    }
}

//辅助打印一个指针内部数据的函数，用16进制的方式打印日志
void zce::memory_debug(zce::LOG_PRIORITY dbg_lvl,
                       const char* dbg_info,
                       const unsigned char* mem_ptr,
                       size_t mem_len)
{
    ZCE_LOG(dbg_lvl, "[DEBUG_POINTER] out pointer address[%p] [%s].", mem_ptr, dbg_info);
    std::vector<std::string> str_ary;
    zce::memory_debug(mem_ptr, mem_len, str_ary);
    for (std::string& out : str_ary)
    {
        ZCE_LOG(dbg_lvl, "[DEBUG_POINTER] %s.", out.c_str());
    }

    return;
}

//==========================================================================================================

//快速内存拷贝，当然其实他并不算块，
//这个纯属好玩的，经过测试，他其实并没有memcpy快，所以不建议使用
void* zce::fast_memcpy(void* dst, const void* src, size_t sz)
{
    void* r = dst;

    //先进行uint64_t长度的拷贝，一般而言，内存地址都是对齐的，
    size_t n = sz & ~(sizeof(uint64_t) - 1);
    uint64_t* src_u64 = (uint64_t*)src;
    uint64_t* dst_u64 = (uint64_t*)dst;

    while (n)
    {
        *dst_u64++ = *src_u64++;
        n -= sizeof(uint64_t);
    }

    //将没有非8字节字长取整的部分copy
    n = sz & (sizeof(uint64_t) - 1);
    uint8_t* src_u8 = (uint8_t*)src;
    uint8_t* dst_u8 = (uint8_t*)dst;
    while (n--)
    {
        (*dst_u8++ = *src_u8++);
    }

    return r;
}

//快速内存拷贝的第二个版本，其实就是在复制的时候增加了一次复制，更加优化一点
//这个也没有memcpy快
void* zce::fast_memcpy2(void* dst, const void* src, size_t sz)
{
    void* r = dst;

    //先进行uint64_t长度的拷贝，一般而言，内存地址都是对齐的，
    size_t n = sz & ~((sizeof(uint64_t) << 1) - 1);
    uint64_t* src_u64 = (uint64_t*)src;
    uint64_t* dst_u64 = (uint64_t*)dst;

    while (n)
    {
        *dst_u64++ = *src_u64++;
        *dst_u64++ = *src_u64++;
        n -= sizeof(uint64_t) * 2;
    }

    //讲没有非8字节字长的部分copy
    n = sz & ((sizeof(uint64_t) << 1) - 1);
    uint8_t* src_u8 = (uint8_t*)src;
    uint8_t* dst_u8 = (uint8_t*)dst;
    while (n--)
    {
        (*dst_u8++ = *src_u8++);
    }
    return r;
}