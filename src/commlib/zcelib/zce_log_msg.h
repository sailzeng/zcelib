/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_msg.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Monday, December 30, 2013
* @brief
*
*
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_LOG_MESSAGE_H_
#define ZCE_LIB_LOG_MESSAGE_H_

#include "zce_log_basic.h"

/******************************************************************************************
class ZCE_Trace_LogMsg
******************************************************************************************/
class ZCE_Trace_LogMsg : public ZCE_LogTrace_Basic
{

public:

    ///���캯��
    ZCE_Trace_LogMsg();
    virtual ~ZCE_Trace_LogMsg();

    ///ΪC������׼���ķ�װ���Ҽǲ�Ҫ����,���һ��(��)��־��¼.
    void write_logmsg(ZCE_LOG_PRIORITY outlevel, const char *str_format, ...);

protected:

    /*!
    @brief      ʵ��д����־�ĺ���
    @param      outlevel    �����־����
    @param      lpszFormat  ��־�ĸ�ʽ���ַ���
    @param      args        ��̬����־�����б���ɵ�va_list
    */
    void vwrite_logmsg(ZCE_LOG_PRIORITY outlevel, const char *str_format, va_list args);

public:

    //ʵ���ĸ�ֵ
    static void instance(ZCE_Trace_LogMsg *);
    //ʵ���Ļ��
    static ZCE_Trace_LogMsg *instance();
    //���ʵ��
    static void clean_instance();

    //���õ��Ӷ���Aseert����
    static void debug_assert(const char *file_name,
                             const int file_line,
                             const char *fuction_name,
                             const char *expression_name);

    ///���õ��Ӷ������Aseert������Ϣ,��ǿ�汾����
    static void debug_assert_ex(const char *file_name,
                                const int file_line,
                                const char *fuction_name,
                                const char *expression_name,
                                const char *out_string);

    ///���õ��Ӷ��󣬴�ӡ��־��Ϣ
    static void debug_output(ZCE_LOG_PRIORITY dbglevel,
                             const char *str_format,
                             ... );

    //ΪʲôҪ�����鷳,�������ڲ����ú�(__VA_ARGS__)�������(���ֱ�������֧��,VS2003),
#if _MSC_VER <= 1300

    //ʹ�� RS_TRACE ���Լ������
    static void debug_traceex(const char *, ... );
    //ʹ�� RS_DEBUG ���Լ������
    static void debug_debugex(const char *str_format, ... );
    //ʹ�� RS_INFO ���Լ������
    static void debug_infoex(const char *str_format, ... );
    //ʹ�� RS_ERROR ���󼶱����
    static void debug_errorex(const char *str_format, ... );
    //ʹ�� RS_ALERT ���ش��󼶱����
    static void debug_alertex(const char *str_format, ... );
    //ʹ�� RS_FATAL �������󼶱����
    static void debug_fatalex(const char *str_format, ... );

#endif //#if _MSC_VER <= 1300

protected:

    ///��������������󳤶�
    static const size_t SIZE_OF_MULTILINE_BUF = 512 * 1024 - 1;

protected:

    ///�����������
    char                    *multiline_buf_;

    ///ͬ����
    ZCE_Thread_Light_Mutex   multiline_lock_;

protected:

    ///����ʵ��ָ��
    static ZCE_Trace_LogMsg *log_instance_;

};

#endif //ZCE_LIB_TRACE_LOG_MESSAGE_H_

