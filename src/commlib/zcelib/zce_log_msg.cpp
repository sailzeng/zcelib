#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_log_basic.h"
#include "zce_log_msg.h"

//ZengXing 22503

//
ZCE_Trace_LogMsg *ZCE_Trace_LogMsg::log_instance_ = NULL;

/******************************************************************************************
class ZCE_Trace_LogMsg
******************************************************************************************/
//��������
ZCE_Trace_LogMsg::ZCE_Trace_LogMsg():
    multiline_buf_(NULL)
{
}

//��������
ZCE_Trace_LogMsg::~ZCE_Trace_LogMsg()
{
    if (multiline_buf_)
    {
        delete [] multiline_buf_;
    }
}

//���va_list�Ĳ�����Ϣ
void ZCE_Trace_LogMsg::vwrite_logmsg(ZCE_LOG_PRIORITY outlevel,
                                     const char *str_format,
                                     va_list args)
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
    char log_tmp_buffer[LOG_TMP_BUFFER_SIZE + 1];
    log_tmp_buffer[LOG_TMP_BUFFER_SIZE] = '\0';

    //����Ϊ\n������һ���ռ�
    size_t sz_buf_len = LOG_TMP_BUFFER_SIZE ;
    size_t sz_use_len = 0;

    //���ͷ����Ϣ
    stringbuf_loghead(outlevel,
                      now_time_val,
                      log_tmp_buffer,
                      sz_buf_len,
                      sz_use_len);
    sz_buf_len -= sz_use_len;

    //�õ���ӡ��Ϣ,_vsnprintfΪ���⺯��
    int len_of_out = vsnprintf(log_tmp_buffer + sz_use_len, sz_buf_len, str_format, args);

    //���������ַ���������ĳ�
    if (len_of_out >= static_cast<int>( sz_buf_len) || len_of_out < 0)
    {
        sz_use_len = LOG_TMP_BUFFER_SIZE;
        sz_buf_len = 0;
    }
    else
    {
        sz_use_len += len_of_out;
        sz_buf_len -= len_of_out;
    }

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

//д��־
void ZCE_Trace_LogMsg::write_logmsg(ZCE_LOG_PRIORITY outlevel, const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);
    vwrite_logmsg(outlevel, str_format, args);
    va_end(args);

}

//ZASSERT����չ���壬
void ZCE_Trace_LogMsg::debug_assert(const char *file_name,
                                    const int file_line,
                                    const char *function_name,
                                    const char *expression_name)
{
    debug_output(RS_FATAL, "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],EXPRESSION:[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name);
}

//Aseert����,��ǿ�汾����
void ZCE_Trace_LogMsg::debug_assert_ex(const char *file_name,
                                       const int file_line,
                                       const char *function_name,
                                       const char *expression_name,
                                       const char *out_string)
{
    debug_output(RS_FATAL, "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],EXPRESSION:[%s] OutString[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name,
                 out_string);
}

//����vwrite_logmsg���ʵ�����
void ZCE_Trace_LogMsg::debug_output(ZCE_LOG_PRIORITY dbglevel,
                                    const char *str_format, ... )
{
    va_list args;

    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(dbglevel, str_format, args);
    }

    va_end(args);
}

//�õ�Ψһ�ĵ���ʵ��
ZCE_Trace_LogMsg *ZCE_Trace_LogMsg::instance()
{
    if (log_instance_ == NULL)
    {
        log_instance_ = new ZCE_Trace_LogMsg();
    }

    return log_instance_;
}

//��ֵΨһ�ĵ���ʵ��
void ZCE_Trace_LogMsg::instance(ZCE_Trace_LogMsg *instatnce)
{
    clean_instance();
    log_instance_ = instatnce;
    return;
}

//�������ʵ��
void ZCE_Trace_LogMsg::clean_instance()
{
    if (log_instance_)
    {
        delete log_instance_;
    }

    log_instance_ = NULL;
    return;
}

//ΪʲôҪ�����鷳,�������ڲ����ú�(__VA_ARGS__)�������(���ֱ�������֧��,VS2003),
#if _MSC_VER <= 1300

//�ò��õ���־�������
void ZCE_Trace_LogMsg::debug_traceex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_TRACE, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_debugex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_DEBUG, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_infoex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_INFO, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_errorex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_ERROR, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_alertex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_ALERT, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_fatalex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_FATAL, str_format, args);
    }

    va_end(args);
}

#endif //#if _MSC_VER <= 1300

