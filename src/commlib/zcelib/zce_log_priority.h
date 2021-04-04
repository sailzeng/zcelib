#pragma once

namespace zce
{

//!��־�������,��־��¼ʱʹ�õ��Ǽ���,
enum class LOG_PRIORITY
{
    //!������Ϣ,Ĭ��Mask�������,Ĭ��ȫ�����
    LP_TRACE = 1,
    //!������Ϣ
    LP_DEBUG = 2,
    //!��ͨ��Ϣ
    LP_INFO = 3,
    //!������Ϣ
    LP_ERROR = 4,
    //!�澯���͵Ĵ���
    LP_ALERT = 5,
    //!��������
    LP_FATAL = 6,
};

//!RS����д�����ڼ���Richard steven
#ifndef RS_TRACE
#define RS_TRACE zce::LOG_PRIORITY::LP_TRACE
#endif

#ifndef RS_DEBUG
#define RS_DEBUG zce::LOG_PRIORITY::LP_DEBUG
#endif

#ifndef RS_INFO
#define RS_INFO zce::LOG_PRIORITY::LP_INFO
#endif

#ifndef RS_ERROR
#define RS_ERROR zce::LOG_PRIORITY::LP_ERROR
#endif

#ifndef RS_ALERT
#define RS_ALERT zce::LOG_PRIORITY::LP_ALERT
#endif

#ifndef RS_FATAL
#define RS_FATAL zce::LOG_PRIORITY::LP_FATAL
#endif

};

