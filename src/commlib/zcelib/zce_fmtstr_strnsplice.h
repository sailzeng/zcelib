/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_strnsplice.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月6日
* @brief
* 
* @details
* 
* @note
* 
*/

#ifndef ZCE_FMTSTR_STRN_SPLICE_H_
#define ZCE_FMTSTR_STRN_SPLICE_H_

#include "zce_fmtstr_helper.h"

namespace ZCE_LIB
{

//默认的分隔符号是'\0',表示不需要分隔符，你可以通过函数设置分割符号
static char STRNCPY_SEPARATOR_CHAR = '\0';

#define __ZCE_STRNSPLICE_BEGIN   foo_use_len = 0; \
    if ( 0 == foo_max_len ) \
    { \
        return foo_buffer; \
    } \
    size_t max_len = foo_max_len - 1; \
    size_t use_len = 0; \
    char *buffer = foo_buffer; \
    buffer[max_len] = '\0'

#define __ZCE_STRNSPLICE_REPEAT(x)   if (max_len > 0)  \
    { \
        ZCE_LIB::output_helper(buffer, max_len, use_len, out_data##x);\
        buffer += use_len; \
        max_len -= use_len; \
        foo_use_len += use_len; \
    } \
    if (STRNCPY_SEPARATOR_CHAR != '\0' && max_len > 0 ) \
    {  \
        *buffer = STRNCPY_SEPARATOR_CHAR;  \
        buffer += 1;   \
        max_len -= 1;  \
        foo_use_len += 1;  \
    }

#define __ZCE_STRNSPLICE_END   foo_buffer[foo_use_len] = '\0'; \
    return foo_buffer

template <class T1, class T2>
char *zce_strnsplice(char *foo_buffer,
                     size_t foo_max_len,
                     size_t &foo_use_len,
                     const T1 &out_data1,
                     const T2 &out_data2)
{
    foo_use_len = 0;
    if ( 0 == foo_max_len )
    {
        return foo_buffer;
    }

    size_t max_len = foo_max_len - 1;
    size_t use_len = 0;
    char *buffer = foo_buffer;
    buffer[max_len] = '\0';

    //如果还有空间容纳字符
    if (max_len > 0)
    {
        ZCE_LIB::output_helper(buffer, max_len, use_len, out_data1);

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
    }
    //如果需要分隔符，添加分割符号
    if (STRNCPY_SEPARATOR_CHAR != '\0' && max_len > 0 )
    {
        *buffer = STRNCPY_SEPARATOR_CHAR;
        buffer += 1;
        max_len -= 1;
        foo_use_len += 1;
    }

    if (max_len > 0)
    {
        ZCE_LIB::output_helper(buffer, max_len, use_len, out_data2);

        buffer += use_len;
        max_len -= use_len;
        foo_use_len += use_len;
    }

    //如果需要分隔符，添加分割符号
    if (STRNCPY_SEPARATOR_CHAR != '\0' && max_len > 0 )
    {
        *buffer = STRNCPY_SEPARATOR_CHAR;
        buffer += 1;
        max_len -= 1;
        foo_use_len += 1;
    }

    foo_buffer[foo_use_len] = '\0';
    //返回
    return foo_buffer;
}

template < class T1 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_END;
}

//template < class T1,class T2 >
//void zce_strnsplice(char *foo_buffer,
//    size_t foo_max_len,
//    size_t &foo_use_len,
//    const T1 &out_data1,
//    const T2 &out_data2 )
//{
//    __ZCE_STRNSPLICE_BEGIN;
//    __ZCE_STRNSPLICE_REPEAT(1);
//    __ZCE_STRNSPLICE_REPEAT(2);
//    __ZCE_STRNSPLICE_END;
//}

template < class T1, class T2, class T3 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30,
                    const T31 &out_data31 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_REPEAT(31);
    __ZCE_STRNSPLICE_END;
}

template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31, class T32 >
void zce_strnsplice(char *foo_buffer,
                    size_t foo_max_len,
                    size_t &foo_use_len,
                    const T1 &out_data1,
                    const T2 &out_data2,
                    const T3 &out_data3,
                    const T4 &out_data4,
                    const T5 &out_data5,
                    const T6 &out_data6,
                    const T7 &out_data7,
                    const T8 &out_data8,
                    const T9 &out_data9,
                    const T10 &out_data10,
                    const T11 &out_data11,
                    const T12 &out_data12,
                    const T13 &out_data13,
                    const T14 &out_data14,
                    const T15 &out_data15,
                    const T16 &out_data16,
                    const T17 &out_data17,
                    const T18 &out_data18,
                    const T19 &out_data19,
                    const T20 &out_data20,
                    const T21 &out_data21,
                    const T22 &out_data22,
                    const T23 &out_data23,
                    const T24 &out_data24,
                    const T25 &out_data25,
                    const T26 &out_data26,
                    const T27 &out_data27,
                    const T28 &out_data28,
                    const T29 &out_data29,
                    const T30 &out_data30,
                    const T31 &out_data31,
                    const T32 &out_data32 )
{
    __ZCE_STRNSPLICE_BEGIN;
    __ZCE_STRNSPLICE_REPEAT(1);
    __ZCE_STRNSPLICE_REPEAT(2);
    __ZCE_STRNSPLICE_REPEAT(3);
    __ZCE_STRNSPLICE_REPEAT(4);
    __ZCE_STRNSPLICE_REPEAT(5);
    __ZCE_STRNSPLICE_REPEAT(6);
    __ZCE_STRNSPLICE_REPEAT(7);
    __ZCE_STRNSPLICE_REPEAT(8);
    __ZCE_STRNSPLICE_REPEAT(9);
    __ZCE_STRNSPLICE_REPEAT(10);
    __ZCE_STRNSPLICE_REPEAT(11);
    __ZCE_STRNSPLICE_REPEAT(12);
    __ZCE_STRNSPLICE_REPEAT(13);
    __ZCE_STRNSPLICE_REPEAT(14);
    __ZCE_STRNSPLICE_REPEAT(15);
    __ZCE_STRNSPLICE_REPEAT(16);
    __ZCE_STRNSPLICE_REPEAT(17);
    __ZCE_STRNSPLICE_REPEAT(18);
    __ZCE_STRNSPLICE_REPEAT(19);
    __ZCE_STRNSPLICE_REPEAT(20);
    __ZCE_STRNSPLICE_REPEAT(21);
    __ZCE_STRNSPLICE_REPEAT(22);
    __ZCE_STRNSPLICE_REPEAT(23);
    __ZCE_STRNSPLICE_REPEAT(24);
    __ZCE_STRNSPLICE_REPEAT(25);
    __ZCE_STRNSPLICE_REPEAT(26);
    __ZCE_STRNSPLICE_REPEAT(27);
    __ZCE_STRNSPLICE_REPEAT(28);
    __ZCE_STRNSPLICE_REPEAT(29);
    __ZCE_STRNSPLICE_REPEAT(30);
    __ZCE_STRNSPLICE_REPEAT(31);
    __ZCE_STRNSPLICE_REPEAT(32);
    __ZCE_STRNSPLICE_END;
}

};

#endif //ZCE_FMTSTR_STRNCPY_H_

