/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_plus.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004��8��1��
* @brief
*
* @details    ow��ow���ֿ�����������һ�����ʷ�ˣ�һ��㣬
*             �ʼд������ʱ���Ҹոն�C++��stream��һЩ�˽⡣����һ��������
*             �����¶���һ���������Ϊ�¶����������ģ��ڵ�һ������stream�ķ�ʽʵ����
*             ��־�������Ȼ���ò�˵��Ҳ����ͦ��ģ�
*             ��˵����C++ stream���㲻��һ����������Ķ���
*             1.��ʽ������ʹ�࣬��������ڸ�ʽ����������ԶԶ����printf���ֺ�����
*             2.����������Լ��snprintf���ຯ������50%.(GCC7.1���°汾�о�ϲ)
*             3.���߳���������ÿ��ƣ��ر��Ƕ��̻߳����£�������޷�������
*             ����C++ stream����һ�����������ȴ����������ʹ�á�
*
*             ���Ե�Ҫ�����ع���ʱ���Ҿ����ⶨ����steam�Ĵ��룬�ر��ǵ��ҷ���
*             fast format��Щ���������ַ�����ʽ�����Ѿ���ԽC��snprintf������
*             �����������ҡ��Ҿ����Լ�ʵ��һ�£���ȻC��log����Ѹ��������ϣ�
*             C++�İ汾ȴ��Ȼ�ȴ��˺ܳ�һ��ʱ�䣬��ʹ�ҵ�zce::foo_snprintf
*             �Ѿ���ɺ���Ϊ�ҷ��֣����û�б�͵�ģ�棬ÿһ����ģ��ʵ���ַ�����ʽ
*             ������һ�μ尾����ֻ���ú���ɴ����滻��
*
*             ����C++ 20����㵽���ѡ��ڴ�C++ format��ʵ�֡�
*
* @note
*
*/

#ifndef ZCE_LIB_TRACE_LOG_PLUS_H_
#define ZCE_LIB_TRACE_LOG_PLUS_H_

#include "zce_string_extend.h"
#include "zce_log_basic.h"



/******************************************************************************************
class ZCE_LogTrace_Plus ֻ��ΪC++������׼���ķ�װ,
******************************************************************************************/
class ZCE_LogTrace_Plus : public ZCE_LogTrace_Basic
{

public:

    ZCE_LogTrace_Plus();
    //��������
    ~ZCE_LogTrace_Plus();


#if ZCE_SUPPORT_CPP11 == 1

    template <typename... out_type >
    void foo_write_logmsg(ZCE_LOG_PRIORITY outlevel,
                          const char *str_format,
                          const out_type &...out_data)
    {
        //�����־������عر�
        if (if_output_log_ == false)
        {
            return;
        }

        //����������־�������Maskֵ
        if (permit_outlevel_ > outlevel)
        {
            return;
        }

        //�õ���ǰʱ��
        timeval now_time_val(zce::gettimeofday());

        //��Ҫ����һ��λ�÷�'\0'
        char log_tmp_buffer[LOG_TMP_BUFFER_SIZE + 1];
        log_tmp_buffer[LOG_TMP_BUFFER_SIZE] = '\0';

        //����Ϊ\n������һ���ռ�
        size_t sz_buf_len = LOG_TMP_BUFFER_SIZE;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //�õ���ӡ��Ϣ,foo_snprintf Ϊ�Լ��ڲ��ĺ�����str_formatʹ��%?��Ϊ������Ʒ�
        size_t sprt_use_len = 0;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data...);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

        //���Ҫ�Զ����ӻ��з��ţ�
        if (auto_new_line_)
        {
            log_tmp_buffer[sz_use_len] = '\n';
            ++sz_use_len;

            //ע��sz_buf_len�����û�е�������Ϊ'\n'��λ����ǰ��Ϊ�˰�ȫ�۳���
            //Ҳ����ֱ����--sz_buf_len;��Ϊsz_buf_len����==0
        }

        output_log_info(now_time_val,
                        log_tmp_buffer,
                        sz_use_len);
    }

#else

#define __ZCE_LOGPP_WRITE_BEGIN        if (if_output_log_ == false)\
    { \
        return ;\
    }\
    if (permit_outlevel_ > outlevel ) \
    { \
        return; \
    } \
    timeval now_time_val (zce::gettimeofday()); \
    char log_tmp_buffer[LOG_TMP_BUFFER_SIZE +1 ]; \
    log_tmp_buffer[LOG_TMP_BUFFER_SIZE ] = '\0'; \
    size_t sz_buf_len = LOG_TMP_BUFFER_SIZE; \
    size_t sz_use_len = 0; \
    stringbuf_loghead(outlevel,now_time_val,log_tmp_buffer,sz_buf_len,sz_use_len); \
    sz_buf_len -= sz_use_len; \
    size_t sprt_use_len = 0

#define __ZCE_LOGPP_WRITE_END        if (if_output_log_ == false) \
        sz_use_len += sprt_use_len; \
    sz_buf_len -= sprt_use_len; \
    if (auto_new_line_) \
    { \
        log_tmp_buffer[sz_use_len] = '\n'; \
        ++sz_use_len; \
    } \
    output_log_info(now_time_val,log_tmp_buffer,sz_use_len)

    //
    template <class T1, class T2>
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2)
    {
        //�����־������عر�
        if (if_output_log_ == false)
        {
            return ;
        }

        //����������־�������Maskֵ
        if (permit_outlevel_ > outlevel )
        {
            return;
        }

        //�õ���ǰʱ��
        timeval now_time_val (zce::gettimeofday());

        //��Ҫ����һ��λ�÷�'\0'
        char log_tmp_buffer[LOG_TMP_BUFFER_SIZE ];
        log_tmp_buffer[LOG_TMP_BUFFER_SIZE - 1] = '\0';

        //����Ϊ\n������һ���ռ�
        size_t sz_buf_len = LOG_TMP_BUFFER_SIZE - 2;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //�õ���ӡ��Ϣ,_vsnprintfΪ���⺯��
        size_t sprt_use_len = 0;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

        //���Ҫ�Զ����ӻ��з��ţ�
        if (auto_new_line_)
        {
            log_tmp_buffer[sz_use_len] = '\n';
            ++sz_use_len;

            //ע��sz_buf_len�����û�е�������Ϊ'\n'��λ����ǰ��Ϊ�˰�ȫ�۳���
            //Ҳ����ֱ����--sz_buf_len;��Ϊsz_buf_len����==0
        }

        output_log_info(now_time_val,
                        log_tmp_buffer,
                        sz_use_len);

    }

    template < class T1 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30, out_data31);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31, class T32 >
    void write_logmsg(ZCE_LOG_PRIORITY outlevel,
                      const char *str_format,
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
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30, out_data31, out_data32);
        __ZCE_LOGPP_WRITE_END;
    }

#endif //

public:

    //ʵ���ĸ�ֵ
    static void instance(ZCE_LogTrace_Plus *);
    //ʵ���Ļ��
    static ZCE_LogTrace_Plus *instance();
    //���ʵ��
    static void clean_instance();

protected:
    //
    static ZCE_LogTrace_Plus *lpp_instance_;

};

#endif //ZCE_LIB_TRACE_LOG_PLUS_H_

