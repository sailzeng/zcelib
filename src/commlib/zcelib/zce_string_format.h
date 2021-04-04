/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_format.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��4��30��
* @brief      ��ʽ��ĳЩ���������int64��double��str��
*             �����ˣ�����Ҳ����3���и�ʽ����
*
* @details    ���ִ�����������openssh,�м��bsd-snprintf.c���룬ԭ���Ĵ���Ӧ����֧��BSDЭ���,
*             �ҽ�3�����������˳��������������Լ�����־�����Ҳ����һЩ����
*             bsd-snprintf.c���Ҽ����Ķ���snprintfʵ�ֱȽϸɾ��Ĵ��룬����Щ�ط�͵�����٣�
*             �����в��ٵ�bug����������double�������������,
*             ԭ����fmtstrҲ�����⣬�о����߶��ھ��ȺͿ�ȵĶ��������෴��
*             ���������ǵĴ���ҲӦ����ѭBSDЭ�飬���Ͼ�������Ϊ����־�����ģ��ҶԴ�������������
*             ������Ĵ����Ҵ���ʹ��Apache��Э�鿪Դ��Apache 2.0��Э���BSDЭ��ǳ��ӽ�
*             ͬʱ��Ҳ�е㻳�����Ǵ�GPL�Ĵ���Ĺ����ģ��Ǻǣ�
*             ΢���GLIBC���ⲿ�ִ���������ɶ�����Ȼ���ǿ��ǵ������ȷҪ���Ӻܶ�
*             fmt_double���������Լ������ĳ����ģ�ԭ���ĺ���fmtfp��������ģ�ֻ����������Χ��ʹ�ã�
*             �Ҳο���libc-5.4.29.tar.gz/libc/cvt/cvt_r.�ĺ���ecvt_r �� fcvt_r,����������
*             ecvt_r��fcvt_r���������Ҹ��˰���űȽ��������ǵ���ͼ��������Ƶĳ��ԣ����ܾ��Ƿ�����
*             �ǽ��и�ʽ��ת�������
*
*             ������Ĵ󲿷ִ��������������Լ���fmtstr�ڲ�ʹ�ã��ⲿ�������롣
*
*             ������������google�Ĵ���������Ȼֹͣ�����ˣ����ġ�
*
*/

#ifndef _ZCE_FORMAT_OUT_STRING_H_
#define _ZCE_FORMAT_OUT_STRING_H_

namespace zce
{

///��ʽ���ַ�������Ŀ��ƺ꣬����&һ������
enum FORMAT_STRING_FLAG
{
    ///����룬�൱��Format specifications �� flag ��"-",
    FMT_MINUS           = (1 << 0),
    FMT_LEFT_ALIGN      = (1 << 0),

    ///���ӷ���λ���൱��Format specifications �� flag ��"+"
    FMT_PLUS            = (1 << 1),
    ///����λ�������-,�ÿո����,�൱��Format specifications �� flag ��" "
    FMT_SPACE           = (1 << 2),
    ///����ǰ׺0,0x,0X���൱��Format specifications �� flag ��"#"
    FMT_PREFIX             = (1 << 3),
    ///�Ҷ��룬��߶���Ĳ�����0��䣬�൱��Format specifications �� flag ��"0"
    FMT_ZERO            = (1 << 4),
    ///��д��x==>X,e==>E
    FMT_UP              = (1 << 5),
    ///�����޷����������
    FMT_UNSIGNED        = (1 << 6),
    ///���ָ�����൱��"e",���ͬʱ��FMT_UP���൱��"E"
    FMT_EXPONENT        = (1 << 7),

};

///����Ľ���ϵͳ��Ĭ����10����
enum BASE_NUMBER_SYSTEM
{
    ///������
    BASE_BINARY        = 2,
    ///�˽���
    BASE_OCTAL         = 8,
    ///ʮ����
    BASE_DECIMAL       = 10,
    ///ʮ������
    BASE_HEXADECIMAL   = 16,
};

///Ĭ�ϵĸ��㾫�ȳ���
static const size_t DEFAULT_DOUBLE_PRECISION = 6;

/*!
* @brief      �����ַ����ĸ�ʽ�������ע�����������BUFFERĩβ�����\0,
* @param[out] buffer    ������ַ���BUFFER
* @param[in]  max_len   BUFFER�������ó��ȣ�
* @param[out] use_len   ���ز�������ʽ����ʹ����BUFFER�ĳ���
* @param[in]  value     Ҫ���и�ʽ�������Դ�ַ���
* @param[in]  str_len   Ҫ���д����Դ�ַ�������
* @param[in]  width     ����ַ����Ŀ�ȣ�Ϊ0��ʾ�������ȣ��൱��printf������ʽ������%32.16s�м��32��
* @param[in]  precision ���ȣ�������������ַ����൱��printf������ʽ������%32.16s�м��16��
* @param[in]  flags     �ο� @ref zce::FORMAT_STRING_FLAG
*/
void fmt_str(char *buffer,
             size_t max_len,
             size_t &use_len,
             const char *value,
             size_t str_len,
             size_t width = 0,
             size_t precision = -1,
             int flags = 0);

///@brief ���ڽ��ַ�����ʽ�����������ӣ��� @stdstr���棨ĩβ�����������������ĺ���
void fmt_str(std::string &stdstr,
             const char *value,
             size_t str_len,
             size_t width = 0,
             size_t precision = -1,
             int flags = 0);


/*!
* @brief      ����int64�ĸ�ʽ�������ע�����������BUFFERĩβ�����\0,��BSD��openssh snprintf������ֲ
*             �ҵ����ţ������һ�����һ��int32�Ĵ��룬������int64��˵�������ȫ�����࣬
*             �����Ҫ����int32��Ψһ��Ŀ�ľ�����32λ�����ϵ��ٶȿ��Ը��ӿ�һ�㣬������ô���Ҫ�𣿷������Ĵ���󲿷ֶ�����64λ����������
*             ������һ�£���ɾ����int32�Ĵ��룬
* @param      buffer      ������ص��ַ���BUFFER
* @param      max_len     BUFFER�������ó��ȣ�
* @param      use_len     ���ز�������ʽ����ʹ����BUFFER�ĳ���
* @param      value       Ҫ���������int64ֵ
* @param      base        ����Ľ��� �ο��� @ref zce::BASE_NUMBER_SYSTEM
* @param      width       ����Ŀ�ȣ�Ϊ0��ʾ�������ȣ��൱��printf������ʽ������%32.16d�м��32��
* @param      precision   ���ȣ�������������ַ����൱��printf������ʽ������%32.16d�м��16�������������ض�
* @param      flags       �ο� @ref zce::FORMAT_STRING_FLAG
*/
void fmt_int64(char *buffer,
               size_t max_len,
               size_t &use_len,
               int64_t value,
               BASE_NUMBER_SYSTEM base = BASE_DECIMAL,
               size_t width = 0,
               size_t precision = 0,
               int flags = 0);

///���ڽ�int64��ʽ�����������ӣ��� @stdstr���棨ĩβ�����������������ĺ���
void fmt_int64(std::string &stdstr,
               int64_t value,
               BASE_NUMBER_SYSTEM base,
               size_t width = 0,
               size_t precision = 0,
               int flags = 0);

/*!
* @brief      �������ֵ����,Ĭ����ָ����ʽ�������Լ�ռ�,���������Ǹ�������˼�Ķ�����
* @param      buffer     ������ص��ַ���BUFFER
* @param      max_len    BUFFER�������ó��ȣ�
* @param      use_len    ���ز�������ʽ����ʹ����BUFFER�ĳ���
* @param      fvalue     Ҫ���������doubleֵ
* @param      width      ����Ŀ�ȣ�Ϊ0��ʾ�������ȣ��൱��printf������ʽ������%32.16d�м��32��
* @param      precision  ���ȣ����С������漸λ
* @param      flags      �ο� @ref zce::FORMAT_STRING_FLAG
* @note
*/
void fmt_double(char *buffer,
                size_t max_len,
                size_t &use_len,
                double fvalue,
                size_t width = 0,
                size_t precision = DEFAULT_DOUBLE_PRECISION,
                int flags = zce::FMT_EXPONENT | zce::FMT_UP);

///���ڽ�double��ʽ�����������ӣ��� @stdstr���棨ĩβ�����������������ĺ���
void fmt_double(std::string &stdstr,
                double fvalue,
                size_t width = 0,
                size_t precision = DEFAULT_DOUBLE_PRECISION,
                int flags = zce::FMT_EXPONENT | zce::FMT_UP);

}

#endif
