#include "zce/predefine.h"
#include "zce/socket/addr_in.h"
#include "zce/socket/addr_in6.h"
#include "zce/time/time_value.h"
#include "zce/string/from_string.h"

//==========================================================================================================

//GCC 编译的时候不能处理template<> int8_t zce::from_str(const char *str)
//的写法，只能把名字空间放到外面来了，估计是和class容易混淆
namespace zce
{
//将c str 转换为很多数值类型，作为返回值返回
template<>
int8_t from_str(const char* str)
{
    //10表示10进制，
    char* stopstring = NULL;
    return static_cast<int8_t>(::strtol(str, &stopstring, 10));
}

template<>
uint8_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<uint8_t>(::strtol(str, &stopstring, 10));
}

template<>
int16_t from_str(const char* str)
{
    //10表示10进制，
    char* stopstring = NULL;
    return static_cast<int16_t>(::strtol(str, &stopstring, 10));
}

template<>
uint16_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<uint16_t>(::strtol(str, &stopstring, 10));
}

template<>
int32_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<int32_t>(::strtol(str, &stopstring, 10));
}

template<>
uint32_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<uint32_t>(::strtoul(str, &stopstring, 10));
}

template<>
int64_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<int64_t>(::strtoll(str, &stopstring, 10));
}

template<>
uint64_t from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<uint64_t>(::strtoull(str, &stopstring, 10));
}

template<>
double from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<double>(::strtod(str, &stopstring));
}

template<>
float from_str(const char* str)
{
    char* stopstring = NULL;
    return static_cast<float>(::strtod(str, &stopstring));
}

template<>
bool from_str(const char* str)
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
zce::Sockaddr_In from_str(const char* str)
{
    zce::Sockaddr_In val;
    val.set(str);
    return val;
}

template<>
zce::Sockaddr_In6 from_str(const char* str)
{
    zce::Sockaddr_In6 val;
    val.set(str);
    return val;
}

template<>
zce::Time_Value from_str(const char* str)
{
    zce::Time_Value val;
    val.from_string(str, false, zce::TIME_STR_FORMAT::ISO_USEC);
    return val;
}

template<>
std::string from_str(const char* str)
{
    return std::string(str);
}
}