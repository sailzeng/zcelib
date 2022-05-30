#include "zce/predefine.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/socket.h"
#include "zce/time/time_value.h"
#include "zce/socket/addr_in.h"
#include "zce/socket/addr_in6.h"
#include "zce/uuid/generator.h"
#include "zce/uuid/generator.h"
#include "zce/string/to_string.h"

//-----------------------------------------------------------------------------------------------------
namespace zce
{
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr_in& out_data)
{
    const char* ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                                buffer,
                                                max_len,
                                                use_len);
    //返回成功
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const sockaddr_in& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                         addr_str,
                                         max_len,
                                         use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///IPV6的socket地址
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr_in6& out_data)
{
    const char* ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                                buffer,
                                                max_len,
                                                use_len);
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const sockaddr_in6& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                         addr_str,
                                         max_len,
                                         use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///sockaddr *的指针类型
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const sockaddr* out_data)
{
    const char* ret_str = zce::sockaddr_ntop_ex(out_data,
                                                buffer,
                                                max_len,
                                                use_len);
    //返回成功
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const sockaddr* out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::sockaddr_ntop_ex(out_data,
                                         addr_str,
                                         max_len,
                                         use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助IPV4的socket地址字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const in_addr& out_data)
{
    const char* ret_str = zce::inet_ntop(AF_INET,
                                         (void*)(&out_data),
                                         buffer,
                                         max_len);
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const in_addr& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    auto ret_str = zce::inet_ntop(AF_INET,
                                  (void*)(&out_data),
                                  addr_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出IPV6的socket地址字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const in6_addr& out_data)
{
    const char* ret_str = zce::inet_ntop(AF_INET6,
                                         (void*)(&out_data),
                                         buffer,
                                         max_len);
    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const in6_addr& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    auto ret_str = zce::inet_ntop(AF_INET6,
                                  (void*)(&out_data),
                                  addr_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///输出zce::Time_Value的时间
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const zce::Time_Value& out_data)
{
    const char* ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);
    //返回成功
    if (ret_str)
    {
        use_len = zce::TIMESTR_ISO_USEC_LEN;
    }
    else
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const zce::Time_Value& out_data)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_string(time_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///输出zce::Sockaddr_In的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const zce::Sockaddr_In& out_data)
{
    const char* ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //返回成功
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const zce::Sockaddr_In& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_string(addr_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出Sockaddr_In6的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const Sockaddr_In6& out_data)
{
    const char* ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //返回成功
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const Sockaddr_In6& out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_string(addr_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出ZCE_UUID64的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const UUID64& out_data)
{
    const char* ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //返回成功
    if (ret_str)
    {
        use_len = UUID64::LEN_OF_ZCE_UUID64_STR;
    }
    else
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const UUID64& out_data)
{
    char uuid_str[UUID64::LEN_OF_ZCE_UUID64_STR + 1];
    uuid_str[UUID64::LEN_OF_ZCE_UUID64_STR] = '\0';
    size_t max_len = UUID64::LEN_OF_ZCE_UUID64_STR + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_string(uuid_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出ZCE_UUID128的字符串
void to_str(char* buffer,
            size_t max_len,
            size_t& use_len,
            const UUID128& out_data)
{
    const char* ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);
    if (!ret_str)
    {
        zce::to_str(buffer, max_len, use_len, "<ERROR>");
    }
}

void to_string(std::string& stdstr,
               const UUID128& out_data)
{
    char uuid_str[UUID128::LEN_OF_ZCE_UUID128_STR + 1];
    uuid_str[UUID128::LEN_OF_ZCE_UUID128_STR] = '\0';
    size_t max_len = UUID128::LEN_OF_ZCE_UUID128_STR + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_string(uuid_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

//=======================================================================================================

//DOUBLE格式化输出辅助
Double_Out_Helper::Double_Out_Helper(double out_data,
                                     size_t width,
                                     size_t precision,
                                     int flags) :
    out_data_(out_data),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

Double_Out_Helper::~Double_Out_Helper()
{
}

//String格式化输出辅助类
String_Out_Helper::String_Out_Helper(const char* out_str_ptr,
                                     size_t out_str_len,
                                     size_t width,
                                     size_t precision,
                                     int flags) :
    out_str_ptr_(out_str_ptr),
    out_str_len_(out_str_len),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

//
String_Out_Helper::String_Out_Helper(const std::string& out_str,
                                     size_t width,
                                     size_t precision,
                                     int flags) :
    out_str_ptr_(out_str.c_str()),
    out_str_len_(out_str.length()),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

String_Out_Helper::String_Out_Helper(const char* out_str_ptr,
                                     size_t width,
                                     size_t precision,
                                     int flags) :
    out_str_ptr_(out_str_ptr),
    out_str_len_(strlen(out_str_ptr)),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

String_Out_Helper::~String_Out_Helper()
{
}
}