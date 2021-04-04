/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_helper.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��5��2��
* @brief      �ַ���������������ߣ�����Adapt�ȣ�������ͬ�����������
*             ������ʹ��ģ���ʱ��ͺܷ����ˡ�
*             Ҳ�ṩ��һ�����������
*             output_helper ����Ϊģ�庯���ṩͳһ����ڣ�
*             XXXXX_Out_Helper ��Щ��һ����Ϊ�˰����ṩ��ʽ�������
*
* @details    �������������ͣ�zce����Ļ����������ͣ������������
*
*             2012��5��
*             ǰ��������Ϊ�����𾴵ĺ����أ����飩Ůʿ����ϴ�ɽ�����˳��������׶�
*             ���ѣ��־壬ĬĬ�����������У����ǿ�����һ�����⣬
*             ��֪���Ƿ�����Ϊ����Ĵ��ڣ����ǿ�����ϣ��
*
*/

#ifndef ZCE_STRING_HELPER_H_
#define ZCE_STRING_HELPER_H_

#include "zce_string_format.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_socket.h"
#include "zce_time_value.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_uuid_generator.h"
#include "zce_uuid_generator.h"

namespace zce
{

//��׼�������͵��������Ĭ�ϵĸ�ʽ------------------------------------------------------------------------

//--------------------------------------------
///�������һ��bool ����ֵ
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const bool &out_data)
{
    if (out_data)
    {
        const size_t SIZE_OF_STR_TRUE = 4;
        use_len = std::min<size_t>(max_len, SIZE_OF_STR_TRUE);
        strncpy(buffer, "TRUE", use_len);
    }
    else
    {
        const size_t SIZE_OF_STR_FALSE = 5;
        use_len = std::min<size_t>(max_len, SIZE_OF_STR_FALSE);
        strncpy(buffer, "FALSE", max_len);
    }
}

inline void string_helper(std::string &stdstr,
                          const bool &out_data)
{
    if (out_data)
    {
        stdstr.append("TRUE");
    }
    else
    {
        stdstr.append("FALSE");
    }
}

//--------------------------------------------
///�������һ��CHAR
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const char &out_data)
{
    const size_t CHAR_NEEN_ROOM = 1;

    //ֻ���һ���ַ�
    if (max_len >= CHAR_NEEN_ROOM)
    {
        *buffer = out_data;
        use_len = CHAR_NEEN_ROOM;
    }
    else
    {
        //max_len == 0;
        use_len = 0;
    }
}

inline void string_helper(std::string &stdstr,
                          const char &out_data)
{
    stdstr.append(1, out_data);
}

//--------------------------------------------
///��������޷���CHAR
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned char &out_data)
{
    const size_t CHAR_NEEN_ROOM = 1;
    //ֻ���һ���ַ�
    if (max_len >= CHAR_NEEN_ROOM)
    {
        *buffer = out_data;
        use_len = CHAR_NEEN_ROOM;
    }
    else
    {
        //max_len == 0;
        use_len = 0;
    }
}

inline void string_helper(std::string &stdstr,
                          const unsigned char &out_data)
{
    stdstr.append(1, out_data);
}

//--------------------------------------------
///����������������ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const short &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

inline void string_helper(std::string &stdstr,
                          const short &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///��������޷��Ŷ�����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned short &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void string_helper(std::string &stdstr,
                          const unsigned short &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///�������int����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const int &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

inline void string_helper(std::string &stdstr,
                          const int &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///�޷���int����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned int &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void string_helper(std::string &stdstr,
                          const unsigned int &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///����long����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

inline void string_helper(std::string &stdstr,
                          const long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///��������޷���long����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void string_helper(std::string &stdstr,
                          const unsigned long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///��������з���long long����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const long long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

inline void string_helper(std::string &stdstr,
                          const long long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///��������޷���long long����
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const unsigned long long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   zce::FMT_UNSIGNED);
}

inline void string_helper(std::string &stdstr,
                          const unsigned long long &out_data)
{
    int64_t cvt_out_data = out_data;
    zce::fmt_int64(stdstr,
                   cvt_out_data,
                   zce::BASE_DECIMAL,
                   0,
                   0,
                   0);
}

//--------------------------------------------
///�������float�����ȸ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const float &out_data)
{
    double cvt_out_data = out_data;
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    cvt_out_data);
}

inline void string_helper(std::string &stdstr,
                          const float &out_data)
{
    double cvt_out_data = out_data;
    zce::fmt_double(stdstr,
                    cvt_out_data);
}

//--------------------------------------------
///�������double˫���ȸ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const double &out_data)
{
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    out_data);
}

inline void string_helper(std::string &stdstr,
                          const double &out_data)
{
    zce::fmt_double(stdstr,
                    out_data);
}

//--------------------------------------------
///�������const char�ַ��������������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const char *out_data)
{
    const char *cvt_out_data = out_data;
    size_t str_len = strlen (cvt_out_data);
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 cvt_out_data,
                 str_len);
}

inline void string_helper(std::string &stdstr,
                          const char *out_data)
{
    stdstr.append(out_data);
}

///�������std::string�ַ������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const std::string &out_data)
{
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 out_data.c_str(),
                 out_data.length());
}

inline void string_helper(std::string &stdstr,
                          const std::string &out_data)
{
    stdstr.append(out_data);
}

///�������void *ָ���ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const void *out_data)
{
    ptrdiff_t save_point = (const char *)out_data - (const char *)0;
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   static_cast<int64_t>(save_point),
                   zce::BASE_HEXADECIMAL,
                   0,
                   sizeof(save_point) == 4 ? 8 : 16,
                   zce::FMT_PREFIX | zce::FMT_UP | zce::FMT_ZERO);
}

inline void string_helper(std::string &stdstr,
                          const void *out_data)
{
    ptrdiff_t save_point = (const char *)out_data - (const char *)0;
    zce::fmt_int64(stdstr,
                   static_cast<int64_t>(save_point),
                   zce::BASE_HEXADECIMAL,
                   0,
                   sizeof(size_t) == 4 ? 8 : 16,
                   zce::FMT_PREFIX | zce::FMT_UP | zce::FMT_ZERO);
}

//===========================================================================================
//һЩC���ԵĽṹ���ַ������

///���Timeʱ��
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          timeval &out_data)
{

    const char *ret_str = zce::timestamp(&out_data,
                                         buffer,
                                         max_len);
    //���سɹ�
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          timeval &out_data)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    auto ret_str = zce::timestamp(&out_data,
                                  time_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///IPV4��socket��ַ
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr_in &out_data)
{
    const char *ret_str = zce::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                                  buffer,
                                                  max_len,
                                                  use_len);
    //���سɹ�
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const sockaddr_in &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                           addr_str,
                                           max_len,
                                           use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///IPV6��socket��ַ
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr_in6 &out_data)
{
    const char *ret_str = zce::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                                  buffer,
                                                  max_len,
                                                  use_len);
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const sockaddr_in6 &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::socketaddr_ntop_ex((const sockaddr *)(&out_data),
                                           addr_str,
                                           max_len,
                                           use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///sockaddr *��ָ������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const sockaddr *out_data)
{
    const char *ret_str = zce::socketaddr_ntop_ex(out_data,
                                                  buffer,
                                                  max_len,
                                                  use_len);
    //���سɹ�
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const sockaddr *out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    auto ret_str = zce::socketaddr_ntop_ex(out_data,
                                           addr_str,
                                           max_len,
                                           use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///����IPV4��socket��ַ�ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const in_addr &out_data)
{
    const char *ret_str = zce::inet_ntop(AF_INET,
                                         (void *)(&out_data),
                                         buffer,
                                         max_len);
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const in_addr &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    auto ret_str = zce::inet_ntop(AF_INET,
                                  (void *)(&out_data),
                                  addr_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///�������IPV6��socket��ַ�ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const in6_addr &out_data)
{
    const char *ret_str = zce::inet_ntop(AF_INET6,
                                         (void *)(&out_data),
                                         buffer,
                                         max_len);
    //���سɹ�
    if (ret_str)
    {
        use_len = strlen(buffer);
    }
    else
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const in6_addr &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    auto ret_str = zce::inet_ntop(AF_INET6,
                                  (void *)(&out_data),
                                  addr_str,
                                  max_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

//-----------------------------------------------------------------------------------------------
//һЩZCELIB���õĽṹ���ַ������

///���ZCE_Time_Value��ʱ��
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Time_Value &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);
    //���سɹ�
    if (ret_str)
    {
        use_len = zce::TIMESTR_ISO_USEC_LEN;
    }
    else
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const ZCE_Time_Value &out_data)
{
    char time_str[MAX_TIMEVAL_STRING_LEN + 1];
    time_str[MAX_TIMEVAL_STRING_LEN] = '\0';
    size_t max_len = MAX_TIMEVAL_STRING_LEN + 1;
    size_t use_len = 0;
    const char *ret_str = out_data.to_string(time_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///���ZCE_Sockaddr_In���ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Sockaddr_In &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //���سɹ�
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const ZCE_Sockaddr_In &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char *ret_str = out_data.to_string(addr_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///�������ZCE_Sockaddr_In6���ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_Sockaddr_In6 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //���سɹ�
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const ZCE_Sockaddr_In6 &out_data)
{
    char addr_str[MAX_SOCKETADDR_STRING_LEN + 1];
    addr_str[MAX_SOCKETADDR_STRING_LEN] = '\0';
    size_t max_len = MAX_SOCKETADDR_STRING_LEN + 1;
    size_t use_len = 0;
    const char *ret_str = out_data.to_string(addr_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///�������ZCE_UUID64���ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_UUID64 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);

    //���سɹ�
    if (ret_str)
    {
        use_len = ZCE_UUID64::LEN_OF_ZCE_UUID64_STR;
    }
    else
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const ZCE_UUID64 &out_data)
{
    char uuid_str[ZCE_UUID64::LEN_OF_ZCE_UUID64_STR + 1];
    uuid_str[ZCE_UUID64::LEN_OF_ZCE_UUID64_STR] = '\0';
    size_t max_len = ZCE_UUID64::LEN_OF_ZCE_UUID64_STR + 1;
    size_t use_len = 0;
    const char *ret_str = out_data.to_string(uuid_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

///�������ZCE_UUID128���ַ���
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const ZCE_UUID128 &out_data)
{
    const char *ret_str = out_data.to_string(buffer,
                                             max_len,
                                             use_len);
    if (!ret_str)
    {
        zce::output_helper(buffer, max_len, use_len, "<ERROR>");
    }
}

inline void string_helper(std::string &stdstr,
                          const ZCE_UUID128 &out_data)
{
    char uuid_str[ZCE_UUID128::LEN_OF_ZCE_UUID128_STR + 1];
    uuid_str[ZCE_UUID128::LEN_OF_ZCE_UUID128_STR] = '\0';
    size_t max_len = ZCE_UUID128::LEN_OF_ZCE_UUID128_STR + 1;
    size_t use_len = 0;
    const char *ret_str = out_data.to_string(uuid_str,
                                             max_len,
                                             use_len);
    stdstr.append(ret_str ? ret_str : "<ERROR>");
}

//-----------------------------------------------------------------------------------------------
//����ĸ�ʽ�����ʽ

///INT��ʽ����������࣬���԰�������һЩ��ʽ�����
class Int_Out_Helper
{
public:
    //������Ĺ��캯���������з��ź�û�з��ŵ��������ͽ��������ִ���
    //is_unsigned �����������Ч���Ը���Ҳ������
    template <typename int_type, typename std::enable_if<(std::is_integral<int_type>::value &&
                                                         std::is_unsigned<int_type>::value),int >::type = 0>
    Int_Out_Helper(int_type out_data,
                   size_t width = 0,
                   int flags = 0,
                   BASE_NUMBER_SYSTEM base = BASE_DECIMAL,
                   size_t precision = 0) :
        out_data_(out_data),
        width_(width),
        precision_(precision),
        base_(base),
        flags_(flags)
    {
        flags_ |= zce::FMT_UNSIGNED;
    }

    template <typename int_type, typename std::enable_if<std::is_integral<int_type>::value,
                                                         typename std::enable_if<std::is_signed<int_type>::value, int>::type>::type = 0>
    Int_Out_Helper(int_type out_data,
                   size_t width = 0,
                   int flags = 0,
                   BASE_NUMBER_SYSTEM base = BASE_DECIMAL,
                   size_t precision = 0) :
        out_data_(out_data),
        width_(width),
        precision_(precision),
        base_(base),
        flags_(flags)
    {
    }

    ~Int_Out_Helper() = default;

public:

    ///�����int����
    int64_t            out_data_;
    ///�����ȣ�Ĭ��0
    size_t             width_;
    ///������ȣ�Ĭ��-1�����ֵ��
    size_t             precision_;
    ///����Ľ��Ʒ�ʽ
    BASE_NUMBER_SYSTEM base_;
    ///�����ʽ���Ʊ�־
    int                flags_;
};

class  Int_HexOut_Helper : public Int_Out_Helper
{
public:
    template <typename int_type>
    Int_HexOut_Helper(int_type out_data,
                      size_t width = 0,
                      int flags = zce::FMT_PREFIX | zce::FMT_UP,
                      size_t precision = 0) :
        Int_Out_Helper(out_data, width, flags, zce::BASE_HEXADECIMAL, precision)
    {
        flags_ |= zce::FMT_UNSIGNED;
    }
};


//INT��ʽ���������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const Int_Out_Helper &out_data)
{
    zce::fmt_int64(buffer,
                   max_len,
                   use_len,
                   out_data.out_data_,
                   out_data.base_,
                   out_data.width_,
                   out_data.precision_,
                   out_data.flags_);
}

inline void string_helper(std::string &stdstr,
                          const Int_Out_Helper &out_data)
{
    zce::fmt_int64(stdstr,
                   out_data.out_data_,
                   out_data.base_,
                   out_data.width_,
                   out_data.precision_,
                   out_data.flags_);
}

///DOUBLE��ʽ�����������,������һЩ��ʽ��������
class Double_Out_Helper
{
public:
    explicit Double_Out_Helper(double out_data,
                               size_t width = -1,
                               size_t precision = size_t(-1),
                               int flags = 0);
    ~Double_Out_Helper();

public:
    //Ҫ����ĸ���
    double             out_data_;
    //����Ŀ��
    size_t             width_;
    //����ľ��ȣ�С�������
    size_t             precision_;
    //�����־
    int                flags_;
};

///����������С����ʽ����������е���⣬��double��float�����ԣ�
#ifndef ZCE_DOUBLE_DECIMAL_OUT
#define ZCE_DOUBLE_DECIMAL_OUT(x)   zce::Double_Out_Helper((x),\
                                                           0,\
                                                           size_t(-1),\
                                                           0)
#endif

///����������ָ����ʽ���
#ifndef ZCE_DOUBLE_EXPONENT_OUT
#define ZCE_DOUBLE_EXPONENT_OUT(x)  zce::Double_Out_Helper((x),\
                                                           0,\
                                                           size_t(-1),\
                                                           zce::FMT_UP|zce::FMT_EXPONENT)
#endif

///DOUBLE��ʽ���������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const Double_Out_Helper &out_data)
{
    zce::fmt_double(buffer,
                    max_len,
                    use_len,
                    out_data.out_data_,
                    out_data.width_,
                    out_data.precision_,
                    out_data.flags_);
}

inline void string_helper(std::string &stdstr,
                          const Double_Out_Helper &out_data)
{
    zce::fmt_double(stdstr,
                    out_data.out_data_,
                    out_data.width_,
                    out_data.precision_,
                    out_data.flags_);
}

///String��ʽ����������࣬������һЩ��ʽ������
class String_Out_Helper
{
public:
    //
    explicit String_Out_Helper(const char *out_str_ptr,
                               size_t out_str_len,
                               size_t width,
                               size_t precision,
                               int flags);
    //
    explicit String_Out_Helper(const std::string &out_str,
                               size_t width = size_t(-1),
                               size_t precision = size_t(-1),
                               int flags = 0);
    //
    explicit String_Out_Helper(const char *out_str_ptr,
                               size_t width = size_t(-1),
                               size_t precision = size_t(-1),
                               int flags = 0);

    ~String_Out_Helper();

public:
    ///�ַ���
    const char        *out_str_ptr_;
    ///�ַ����ĳ���
    size_t             out_str_len_;
    ///����Ŀ��
    size_t             width_;
    ///����ľ��ȣ�С�������
    size_t             precision_;
    ///�����־
    int                flags_;
};


///const char *�ַ��������������
inline void output_helper(char *buffer,
                          size_t max_len,
                          size_t &use_len,
                          const String_Out_Helper &out_data)
{
    zce::fmt_str(buffer,
                 max_len,
                 use_len,
                 out_data.out_str_ptr_,
                 out_data.out_str_len_,
                 out_data.width_,
                 out_data.precision_,
                 out_data.flags_
                );
}

inline void string_helper(std::string &stdstr,
                          const String_Out_Helper &out_data)
{
    zce::fmt_str(stdstr,
                 out_data.out_str_ptr_,
                 out_data.out_str_len_,
                 out_data.width_,
                 out_data.precision_,
                 out_data.flags_);
}



};//zce

#endif
