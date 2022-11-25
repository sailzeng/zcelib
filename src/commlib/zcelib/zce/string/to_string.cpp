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
//=======================================================================================================
namespace aidout
{
//DOUBLE格式化输出辅助
o_double::o_double(double out_data,
                   size_t width,
                   size_t precision,
                   int flags) :
    out_data_(out_data),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}
}//namespace outer

//=================================================================================================

const char *to_str(::timeval& out_data,
                   char* buffer,
                   size_t buf_len)
{
    return zce::timestamp(&out_data,
                          buffer,
                          buf_len);
}

///输出Time时间
void embed_str(::timeval& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = zce::timestamp(&out_data,
                                         buffer,
                                         max_len);
    //返回成功
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(::timeval& out_data,
                  std::string& stdstr)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    auto ret_str = zce::timestamp(&out_data,
                                  time_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

//IP的socket地址
const char *to_str(::sockaddr_in& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                 buffer,
                                 buf_len,
                                 use_len);
}

void embed_str(const ::sockaddr_in& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                                buffer,
                                                max_len,
                                                use_len);
    //返回成功
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const ::sockaddr_in& out_data,
                  std::string& stdstr)
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
const char *to_str(::sockaddr_in6& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                 buffer,
                                 buf_len,
                                 use_len);
}

void embed_str(const ::sockaddr_in6& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                                buffer,
                                                max_len,
                                                use_len);
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const ::sockaddr_in6& out_data,
                  std::string& stdstr)
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

//!sockaddr *的指针类型
const char *to_str(const ::sockaddr* out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return zce::sockaddr_ntop_ex(out_data,
                                 buffer,
                                 buf_len,
                                 use_len);
}

void embed_str(const ::sockaddr* out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = zce::sockaddr_ntop_ex(out_data,
                                                buffer,
                                                max_len,
                                                use_len);
    //返回成功
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const ::sockaddr* out_data,
                  std::string& stdstr)
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
const char *to_str(const ::in_addr* out_data,
                   char* buffer,
                   size_t buf_len)
{
    return zce::inet_ntop(AF_INET,
                          (void*)(&out_data),
                          buffer,
                          buf_len);
}

void embed_str(const ::in_addr& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
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
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const ::in_addr& out_data,
                  std::string& stdstr)
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
const char *to_str(const ::in6_addr* out_data,
                   char* buffer,
                   size_t buf_len)
{
    return zce::inet_ntop(AF_INET6,
                          (void*)(&out_data),
                          buffer,
                          buf_len);
}

void embed_str(const ::in6_addr& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
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
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const ::in6_addr& out_data,
                  std::string& stdstr)
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

//=================================================================================================
//一些zce常用的结构的字符串输出
const char *to_str(const zce::time_value& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return out_data.to_str(buffer,
                           buf_len,
                           use_len);
}
///输出zce::time_value的时间
void embed_str(const zce::time_value& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = out_data.to_str(buffer,
                                          max_len,
                                          use_len);
    //返回成功
    if (ret_str)
    {
        use_len = zce::TIMESTR_ISO_USEC_LEN;
    }
    else
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const zce::time_value& out_data,
                  std::string& stdstr)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_str(time_str,
                                          max_len,
                                          use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

//zce::sockaddr_any  万能的处理sockaddr_in，sockaddr_in6的类
const char *to_str(zce::sockaddr_any& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return zce::sockaddr_ntop_ex((const sockaddr*)(&out_data),
                                 buffer,
                                 buf_len,
                                 use_len);
}

void embed_str(const zce::sockaddr_any& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = zce::sockaddr_ntop_ex((const sockaddr*)(out_data),
                                                buffer,
                                                max_len,
                                                use_len);
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const zce::sockaddr_any& out_data,
                  std::string& stdstr)
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

///输出zce::skt::addr_in的字符串
const char *to_str(zce::skt::addr_in& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return out_data.to_str(buffer,
                           buf_len,
                           use_len);
}

void embed_str(const zce::skt::addr_in& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = out_data.to_str(buffer,
                                          max_len,
                                          use_len);

    //返回成功
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const zce::skt::addr_in& out_data,
                  std::string& stdstr)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_str(addr_str,
                                          max_len,
                                          use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出zce::skt::addr_in6的字符串
const char *to_str(zce::skt::addr_in6& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return out_data.to_str(buffer,
                           buf_len,
                           use_len);
}

void embed_str(const zce::skt::addr_in6& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = out_data.to_str(buffer,
                                          max_len,
                                          use_len);

    //返回成功
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const zce::skt::addr_in6& out_data,
                  std::string& stdstr)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_str(addr_str,
                                          max_len,
                                          use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出uuid64的字符串
const char *to_str(zce::uuid64& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return out_data.to_str(buffer,
                           buf_len,
                           use_len);
}

void embed_str(const uuid64& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = out_data.to_str(buffer,
                                          max_len,
                                          use_len);

    //返回成功
    if (ret_str)
    {
        use_len = uuid64::LEN_OF_ZCE_UUID64_STR;
    }
    else
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const uuid64& out_data,
                  std::string& stdstr)
{
    char uuid_str[uuid64::LEN_OF_ZCE_UUID64_STR + 1];
    uuid_str[uuid64::LEN_OF_ZCE_UUID64_STR] = '\0';
    size_t max_len = uuid64::LEN_OF_ZCE_UUID64_STR + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_str(uuid_str,
                                          max_len,
                                          use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///辅助输出uuid128的字符串
const char *to_str(zce::uuid128& out_data,
                   char* buffer,
                   size_t buf_len)
{
    size_t use_len = 0;
    return out_data.to_str(buffer,
                           buf_len,
                           use_len);
}

void embed_str(const uuid128& out_data,
               char* buffer,
               size_t max_len,
               size_t& use_len)
{
    const char* ret_str = out_data.to_str(buffer,
                                          max_len,
                                          use_len);
    if (!ret_str)
    {
        zce::embed_str("<ERROR>", buffer, max_len, use_len);
    }
}

void embed_string(const uuid128& out_data,
                  std::string& stdstr)
{
    char uuid_str[uuid128::LEN_OF_ZCE_UUID128_STR + 1];
    uuid_str[uuid128::LEN_OF_ZCE_UUID128_STR] = '\0';
    size_t max_len = uuid128::LEN_OF_ZCE_UUID128_STR + 1;
    size_t use_len = 0;
    const char* ret_str = out_data.to_str(uuid_str,
                                          max_len,
                                          use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}
}//namespace zce