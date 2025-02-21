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
int from_str(const char* str, signed char &to_val)
{
    //10表示10进制，
    char* stopstring = nullptr;
    to_val = static_cast<signed char>(::strtol(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, unsigned char &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<unsigned char>(::strtoul(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, char &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<char>(::strtol(str, &stopstring, 10));
    return 0;
}

template<>
int from_str(const char* str, signed short &to_val)
{
    //10表示10进制，
    char* stopstring = nullptr;
    to_val = static_cast<signed short>(::strtol(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, unsigned short &to_val)
{
    //10表示10进制，
    char* stopstring = nullptr;
    to_val = static_cast<unsigned short>(::strtoul(str, &stopstring, 10));
    return 0;
}

template<>
int from_str(const char* str, signed int &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<signed int>(::strtol(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, unsigned int &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<unsigned int>(::strtoul(str, &stopstring, 10));
    return 0;
}

template<>
int from_str(const char* str, signed long &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<signed long>(::strtol(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, unsigned long &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<unsigned long>(::strtoul(str, &stopstring, 10));
    return 0;
}

template<>
int from_str(const char* str, signed long long &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<signed long long>(::strtoll(str, &stopstring, 10));
    return 0;
}
template<>
int from_str(const char* str, unsigned long long &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<unsigned long long>(::strtoull(str, &stopstring, 10));
    return 0;
}

template<>
int from_str(const char* str, float &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<float>(::strtod(str, &stopstring));
    return 0;
}
template<>
int from_str(const char* str, double &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<double>(::strtod(str, &stopstring));
    return 0;
}
template<>
int from_str(const char* str, long double &to_val)
{
    char* stopstring = nullptr;
    to_val = static_cast<long double>(::strtold(str, &stopstring));
    return 0;
}

template<>
int from_str(const char* str, bool &to_val)
{
    if (0 == strcasecmp("TRUE", str))
    {
        to_val = true;
    }
    else if (1 == ::atoi(str))
    {
        to_val = true;
    }
    return -1;
}

template<>
int from_str(const char* str, zce::skt::addr_in &to_val)
{
    return to_val.from_str(str);
}

template<>
int from_str(const char* str, zce::skt::addr_in6 &to_val)
{
    return to_val.from_str(str);
}

template<>
int from_str(const char* str, zce::time_value &to_val)
{
    return to_val.from_str(str, false, zce::TS_FMT::ISO_USEC);
}

template<>
int from_str(const char* str, std::string &to_val)
{
    to_val = std::string(str);
    return 0;
}

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL ==1

template<>
int from_str(const char* str, MYSQL_TIME& mytm)
{
    return zce::str_to_MYSQL_TIME(str, &mytm);
}
#endif
}