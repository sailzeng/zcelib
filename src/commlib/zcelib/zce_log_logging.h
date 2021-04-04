/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_loggging.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2010��7��11��
* @brief      ������ֵ��Ե���־�����ԣ��꣬���ڵ�����־���
*
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_LOG_LOGGING_H_
#define ZCE_LIB_LOG_LOGGING_H_

//==========================================================================================================

#include "zce_config.h"
#include "zce_os_adapt_error.h"


//������־���,��ʵ���ڲ��ĺ�����Ϊ�������
#if defined ZCE_USE_LOGMSG  && ZCE_USE_LOGMSG == 1

#include "zce_log_msg.h"

//�����
#define ZLOG_ENABLE           ZCE_Trace_LogMsg::instance()->enable_output(true)
//�ر����
#define ZLOG_DISABLE          ZCE_Trace_LogMsg::instance()->enable_output(false)
//���MASK����,С������������־��Ϣ�������
#define ZLOG_SET_OUTLEVEL     ZCE_Trace_LogMsg::instance()->set_log_priority


//���껹�ù�һ��ΪGCC2.9�����˫���ŵĺ죬�����ˣ����������������ô��汾������

//ʹ�õ��Լ��������־
#define ZCE_LOG               ZCE_Trace_LogMsg::debug_output

#if _MSC_VER <= 1300

//�ṩһЩ��д�ķ�ʽ����Ȼ��Ҳ���ò����ر��
#define ZLOG_TRACE            ZCE_Trace_LogMsg::debug_traceex
#define ZLOG_DEBUG            ZCE_Trace_LogMsg::debug_debugex
#define ZLOG_INFO             ZCE_Trace_LogMsg::debug_infoex
#define ZLOG_ERROR            ZCE_Trace_LogMsg::debug_errorex
#define ZLOG_ALERT            ZCE_Trace_LogMsg::debug_alertex
#define ZLOG_FATAL            ZCE_Trace_LogMsg::debug_fatalex

#else

#define ZLOG_TRACE(...)       ZCE_Trace_LogMsg::debug_output(RS_TRACE,__VA_ARGS__)
#define ZLOG_DEBUG(...)       ZCE_Trace_LogMsg::debug_output(RS_DEBUG,__VA_ARGS__)
#define ZLOG_INFO(...)        ZCE_Trace_LogMsg::debug_output(RS_INFO,__VA_ARGS__)
#define ZLOG_ERROR(...)       ZCE_Trace_LogMsg::debug_output(RS_ERROR,__VA_ARGS__)
#define ZLOG_ALERT(...)       ZCE_Trace_LogMsg::debug_output(RS_ALERT,__VA_ARGS__)
#define ZLOG_FATAL(...)       ZCE_Trace_LogMsg::debug_output(RS_FATAL,__VA_ARGS__)

#endif

//----------------------------------------------------------------------------------
//���ۣ�DEBUG�汾��REALSE�汾Ҳ�����õ�һЩ���ԣ���Щ�������а汾�������ã�����һЩ������ʱ��ҲҪ�жϵĶ���
//ALL���κ�ʱ��

//��ΪWindow��LINUX������ʵ��ʹ�õĺ�������ʵ�ڷ�DEBUG�汾��ʵ��û�б�©���������Ա�����������Щ�������ʹ�á�

#if defined (ZCE_OS_WINDOWS)

_ACRTIMP int __cdecl _CrtDbgReport(
    _In_       int         _ReportType,
    _In_opt_z_ char const *_FileName,
    _In_       int         _Linenumber,
    _In_opt_z_ char const *_ModuleName,
    _In_opt_z_ char const *_Format,
    ...);
#endif

#if defined (ZCE_OS_LINUX)
//__assert_fail���������extern�ģ����������extern���ã�linux�Լ�ֻ�ڷ�NDEBUG�汾�²���extern����
extern "C"  void __assert_fail(__const char *__assertion, __const char *__file,
                               unsigned int __line, __const char *__function)
__THROW __attribute__((__noreturn__));
#endif //#if defined (ZCE_OS_LINUX)


#ifndef ZCE_ASSERT_ALL
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL


#ifndef ZCE_ASSERT_ALL_EX
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL_EX


//���û�ж���ʵ��ZCE�ڲ�����־�����ʹ��printf��Ϊ���������
#else

#include "zce_log_priority.h"

//==========================================================================================================
class ZCE_Trace_Printf
{
public:

    //���va_list�Ĳ�����Ϣ
    void vwrite_logmsg(ZCE_LOG_PRIORITY outlevel,
                       const char *str_format,
                       va_list args)
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

        //�õ���ӡ��Ϣ,_vsnprintfΪ���⺯��
        vfprintf(stderr, str_format, args);
        fprintf(stderr, "\n");
    }

    //д��־
    void write_logmsg(ZCE_LOG_PRIORITY outlevel, const char *str_format, ...)
    {
        va_list args;
        va_start(args, str_format);
        vwrite_logmsg(outlevel, str_format, args);
        va_end(args);

    }

    //����־�������
    void enable_output(bool enable_out)
    {
        if_output_log_ = enable_out;
    }

    //������־���Level
    ZCE_LOG_PRIORITY set_log_priority(ZCE_LOG_PRIORITY outlevel)
    {
        ZCE_LOG_PRIORITY oldlevel = permit_outlevel_;
        permit_outlevel_ = outlevel;
        return oldlevel;
    }

    //!���õ��Ӷ��󣬴�ӡ��־��Ϣ
    static void debug_output(ZCE_LOG_PRIORITY dbglevel,
                             const char *str_format,
                             ...)
    {
        va_list args;
        va_start(args, str_format);

        ZCE_Trace_Printf::instance()->vwrite_logmsg(dbglevel, str_format, args);

        va_end(args);
    }

    //ʵ���Ļ��
    static ZCE_Trace_Printf *instance()
    {
        static ZCE_Trace_Printf log_instance;
        return &log_instance;
    }

    //
protected:

    //!�����־��Ϣ��Maskֵ,С�������Ϣ����Ϣ���������
    ZCE_LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    //!�Ƿ������־��Ϣ,����������ʱ����
    bool                  if_output_log_ = true;

};

#define ZLOG_ENABLE           ZCE_Trace_Printf::instance()->enable_output(true)
#define ZLOG_DISABLE          ZCE_Trace_Printf::instance()->enable_output(false)
#define ZLOG_SET_OUTLEVEL     ZCE_Trace_Printf::instance()->set_log_priority
#define ZCE_LOG               ZCE_Trace_Printf::debug_output

#define ZLOG_TRACE(...)       ZCE_Trace_Printf::debug_output(RS_TRACE,__VA_ARGS__)
#define ZLOG_DEBUG(...)       ZCE_Trace_Printf::debug_output(RS_DEBUG,__VA_ARGS__)
#define ZLOG_INFO(...)        ZCE_Trace_Printf::debug_output(RS_INFO,__VA_ARGS__)
#define ZLOG_ERROR(...)       ZCE_Trace_Printf::debug_output(RS_ERROR,__VA_ARGS__)
#define ZLOG_ALERT(...)       ZCE_Trace_Printf::debug_output(RS_ALERT,__VA_ARGS__)
#define ZLOG_FATAL(...)       ZCE_Trace_Printf::debug_output(RS_FATAL,__VA_ARGS__)

#ifndef ZCE_ASSERT_ALL
#define ZCE_ASSERT_ALL(expr) assert(expr)
#endif

#ifndef ZCE_ASSERT_ALL_EX
#define ZCE_ASSERT_ALL_EX(expr,str) assert(expr)
#endif

#endif //#if !defined ZCE_USE_LOGMSG 


//==========================================================================================================
//���Եĺ�Ķ���

//��REALSE�汾��һЩ��
#if !defined NDEBUG

#ifndef ZCE_ASSERT
#define ZCE_ASSERT(expr)   ZCE_ASSERT_ALL(expr)
#endif

#ifndef ZCE_ASSERT_EX
#define ZCE_ASSERT_EX(expr,str)   ZCE_ASSERT_ALL_EX(expr,str)
#endif

#ifndef ZCE_VERIFY
#define ZCE_VERIFY(expr)       ZCE_ASSERT(expr)
#endif

//#if !defined NDEBUG
#else

//ע������ZCE_ASSERT��VERIFYZCE_VERIFY������,VERIFY�ڲ��ı��ʽʽ��Ȼ������
#ifndef ZCE_ASSERT
#define ZCE_ASSERT(...)             ((void)0)
#endif

#ifndef ZCE_ASSERT_EX
#define ZCE_ASSERT_EX(...)          ((void)0)
#endif


#ifndef ZCE_VERIFY
#define ZCE_VERIFY(expr)            ((void)(expr))
#endif

#endif


//==========================================================================================================
//����һ���ṹ�Ĺ���������������к�������
class ZCE_Trace_Function
{
public:
    //!��������
    const char        *func_name_;
    //!�ļ�����
    const char        *codefile_name_;
    //!�ļ����кţ��к��Ǻ������ڲ���λ�ã����Ǻ�����������ʼλ�ã������ֺη�
    int                code_line_;
    //!�������־����
    ZCE_LOG_PRIORITY   log_priority_;

    //!�����Ҫ���ٷ���ֵ���ѷ���ֵ�ı�����ָ����Ϊһ������
    int               *ret_ptr_ = NULL;

public:
    //���ù��캯����ʾ���뺯�������
    ZCE_Trace_Function(const char *func_name,
                       const char *file_name,
                       int file_line,
                       ZCE_LOG_PRIORITY   log_priority) :
        func_name_(func_name),
        codefile_name_(file_name),
        code_line_(file_line),
        log_priority_(log_priority),
        ret_ptr_(NULL)
    {
        ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE]%s entry,File %s|%u ", func_name_, codefile_name_, code_line_);
    }

    //���ù��캯����ʾ���뺯�������
    ZCE_Trace_Function(const char *func_name,
                       const char *file_name,
                       int file_line,
                       ZCE_LOG_PRIORITY   log_priority,
                       int *ret_ptr) :
        func_name_(func_name),
        codefile_name_(file_name),
        code_line_(file_line),
        log_priority_(log_priority),
        ret_ptr_(ret_ptr)
    {
        ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] entry,code file [%s|%u] default ret = [%d].",
                func_name_,
                codefile_name_,
                code_line_,
                *ret_ptr_);
    }

    //��������������ʾ���뺯�������
    ~ZCE_Trace_Function()
    {
        //�����Ƿ��ע����ֵ���в�ͬ�����
        if (ret_ptr_)
        {
            //����ط�����ĳɹ�ʧ������ֻ����Բο����塣
            ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] exit,code file [%s|%u] "
                    "ret = [%d],return %s.",
                    func_name_,
                    codefile_name_,
                    code_line_,
                    *ret_ptr_,
                    (*ret_ptr_ == 0) ? "success" : "fail");
        }
        else
        {
            ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] exit,code file [%s|%u] .",
                    func_name_,
                    codefile_name_,
                    code_line_);
        }
    }

};

//ZCE_FUNCTION_TRACE(RS_DEBUG)�����ڸ��ٺ����Ľ���
//���ں����Ŀ�ʼʹ��ZCE_FUNCTION_TRACE(RS_DEBUG)����꣬�������ӷֺ�
#ifndef ZCE_TRACE_FUNCTION
#define ZCE_TRACE_FUNCTION(x) ZCE_Trace_Function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x))
#endif


#ifndef ZCE_TRACE_FUNC_RETURN
#define ZCE_TRACE_FUNC_RETURN(x,y) ZCE_Trace_Function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x),(y))
#endif


//!���ڳ������е��ĵط���
#ifndef ZCE_TRACE_FILELINE
#define ZCE_TRACE_FILELINE(log_priority)   ZCE_LOG((log_priority),"[FILELINE TRACE]goto File %s|%d,function:%s.",\
                                                   __FILE__,__LINE__,__ZCE_FUNC__)
#endif

//!
#ifndef ZCE_TRACE_FAIL_RETURN
#define ZCE_TRACE_FAIL_RETURN(log_priority,fail_str,ret_int) ZCE_LOG((log_priority),"[FAIL RETRUN]Fail in file [%s|%d],function:%s," \
                                                                     "api fail info:%s,return %d,last error %d.",__FILE__,__LINE__,__ZCE_FUNC__,(fail_str),(ret_int),zce::last_error())
#endif

#ifndef ZCE_TRACE_FAIL_INFO
#define ZCE_TRACE_FAIL_INFO(log_priority,fail_str)     ZCE_LOG((log_priority),\
                                                               "[API FAIL ]API fail in file [%s|%d],function:%s,fail api:%s,last error %d.",\
                                                               __FILE__,__LINE__,__ZCE_FUNC__,(fail_str),zce::last_error())
#endif


//��־��ӡ��ջ��Ϣ�ĺ꣬x����־���������
#ifndef ZCE_BACKTRACE_STACK
#define ZCE_BACKTRACE_STACK(x) zce::backtrace_stack(x,__ZCE_FUNC__)
#endif

//��־��ӡ��ջ��Ϣ�ĺ꣬x����־���������,y��ָ�룬z��ָ�볤��
#ifndef ZCE_TRACE_POINTER_DATA
#define ZCE_TRACE_POINTER_DATA(x,y,z)     zce::memory_debug((x),__ZCE_FUNC__,(y),(z))
#endif

//-----------------------------------------------------------------------------------------------
//DEBUG �汾���е�һЩ��
//ZCE_ASSERTԭ����д���� while(!(f)){ do something,��������WINDOWS�»��������и澯�����ȸ���

#if defined DEBUG || defined _DEBUG

#ifndef ZCE_LOGMSG_DEBUG
#define ZCE_LOGMSG_DEBUG        ZCE_LOG
#endif

#ifndef ZCE_ASSERT_DEBUG
#define ZCE_ASSERT_DEBUG         ZCE_ASSERT
#endif

#ifndef ZCE_FUNCTION_TRACE_DEBUG
#define ZCE_FUNCTION_TRACE_DEBUG   ZCE_TRACE_FUNCTION
#endif

#ifndef ZCE_FILELINE_TRACE_DEBUG
#define ZCE_FILELINE_TRACE_DEBUG   ZCE_TRACE_FILELINE
#endif

#ifndef ZCE_BACKTRACE_STACK_DEBUG
#define ZCE_BACKTRACE_STACK_DEBUG    ZCE_BACKTRACE_STACK
#endif

//�ǵ��԰汾�ĺ궨�壬
#else  //#if defined DEBUG || defined _DEBUG

//������ǵ��Ա���,����Щ����Ϊ��


#ifndef ZCE_LOGMSG_DEBUG
#if defined ZCE_OS_WINDOWS
#define ZCE_LOGMSG_DEBUG             __noop
#else
#define ZCE_LOGMSG_DEBUG(...)          do  {} while (0)
#endif
#endif

#ifndef ZCE_ASSERT_DEBUG
#define ZCE_ASSERT_DEBUG(...)       ((void)0)
#endif

#ifndef ZCE_FUNCTION_TRACE_DEBUG
#define ZCE_FUNCTION_TRACE_DEBUG(...) ((void)0)
#endif


#ifndef ZCE_FILELINE_TRACE_DEBUG
#define ZCE_FILELINE_TRACE_DEBUG(...) ((void)0)
#endif

#ifndef ZCE_BACKTRACE_STACK_DEBUG
#define ZCE_BACKTRACE_STACK_DEBUG(...) ((void)0)
#endif


#endif //#if defined DEBUG || defined _DEBUG

//-----------------------------------------------------------------------------------------------




#endif
