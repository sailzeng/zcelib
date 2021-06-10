/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_msg.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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

#include "zce/string/extend.h"
#include "zce/logger/log_basic.h"

/******************************************************************************************
class ZCE_Trace_LogMsg
******************************************************************************************/
class ZCE_Trace_LogMsg : public ZCE_LogTrace_Basic
{
public:

    ///构造函数
    ZCE_Trace_LogMsg();
    virtual ~ZCE_Trace_LogMsg();

protected:

    /*!
    @brief      实际写入日志的函数
    @param      outlevel    输出日志级别
    @param      lpszFormat  日志的格式化字符串
    @param      args        动态的日志参数列表组成的va_list
    */
    void vwrite_logmsg(zce::LOG_PRIORITY outlevel, const char* str_format, va_list args);

    template <typename... out_type >
    void foo_write_logmsg(zce::LOG_PRIORITY outlevel,
                          const char* str_format,
                          const out_type &...out_data)
    {
        //如果日志输出开关关闭
        if (if_output_log_ == false)
        {
            return;
        }

        //如果输出的日志级别低于Mask值
        if (permit_outlevel_ > outlevel)
        {
            return;
        }

        //得到当前时间
        timeval now_time_val(zce::gettimeofday());

        //我要保留一个位置放'\0'
        char log_tmp_buffer[LOG_TMP_BUFFER_SIZE + 1];
        log_tmp_buffer[LOG_TMP_BUFFER_SIZE] = '\0';

        //还是为\n考虑留一个空间
        size_t sz_buf_len = LOG_TMP_BUFFER_SIZE;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //得到打印信息,str_nprintf 为自己内部的函数，str_format使用{}作为输出控制符
        size_t sprt_use_len = 0;
        zce::str_nprintf(log_tmp_buffer + sz_use_len,
                         sz_buf_len,
                         sprt_use_len,
                         str_format,
                         out_data...);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

        //如果要自动增加换行符号，
        if (auto_new_line_)
        {
            log_tmp_buffer[sz_use_len] = '\n';
            ++sz_use_len;

            //注意sz_buf_len在这儿没有调整，因为'\n'的位置我前面为了安全扣除了
            //也不能直接用--sz_buf_len;因为sz_buf_len可能==0
        }

        output_log_info(now_time_val,
                        log_tmp_buffer,
                        sz_use_len);
    }
public:

    //实例的赋值
    static void instance(ZCE_Trace_LogMsg*);
    //实例的获得
    static ZCE_Trace_LogMsg* instance();
    //清除实例
    static void clean_instance();

    //利用单子对象，Aseert调试
    static void debug_assert(const char* file_name,
                             const int file_line,
                             const char* fuction_name,
                             const char* expression_name);

    ///利用单子对象，输出Aseert调试信息,增强版本函数
    static void debug_assert_ex(const char* file_name,
                                const int file_line,
                                const char* fuction_name,
                                const char* expression_name,
                                const char* out_string);

    ///利用单子对象，打印日志信息
    static void write_logmsg(zce::LOG_PRIORITY dbglevel,
                             const char* str_format,
                             ...);

    template <typename... out_type >
    static void write_logplus(zce::LOG_PRIORITY outlevel,
                              const char* str_format,
                              const out_type &...out_data)
    {
        log_instance_->foo_write_logmsg(outlevel,
                                        str_format,
                                        out_data...);
    }

protected:

    ///多行输出对象的最大长度
    static const size_t SIZE_OF_MULTILINE_BUF = 512 * 1024 - 1;

protected:

    ///多行术后出的
    char* multiline_buf_;

    ///同步锁
    ZCE_Thread_Light_Mutex   multiline_lock_;

protected:

    ///单子实例指针
    static ZCE_Trace_LogMsg* log_instance_;
};

#endif //ZCE_LIB_TRACE_LOG_MESSAGE_H_
