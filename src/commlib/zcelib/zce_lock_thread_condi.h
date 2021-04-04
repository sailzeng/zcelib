/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_condi.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��2��1��
* @brief
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_THREAD_CONDI_H_
#define ZCE_LIB_LOCK_THREAD_CONDI_H_

#include "zce_log_logging.h"
#include "zce_lock_base.h"
#include "zce_os_adapt_condi.h"

//�̵߳�����������,Ϊ�˷�������ģ���࣬������ֱ��������typedef
template <class MUTEX>
class ZCE_Thread_Condition  : public ZCE_Condition_Base
{
public:

    //���캯��
    ZCE_Thread_Condition ()
    {
        int ret = 0;

        ret = zce::pthread_cond_initex(&lock_, false);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_initex", ret);
            return;
        }

    }

    //��������
    virtual ~ZCE_Thread_Condition (void)
    {
        //������������
        int ret =  zce::pthread_cond_destroy(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_destroy", ret);
            return;
        }
    }

    //�Ҹ���ZCE_Thread_Light_Mutex��ZCE_Thread_Recursive_Mutex�����ػ�ʵ��

    //�ȴ�
    virtual void wait (MUTEX *external_mutex);

    //����ʱ�䳬ʱ�ĵĵȴ�����ʱ�����
    virtual bool systime_wait(MUTEX *external_mutex, const ZCE_Time_Value &abs_time);

    //���ʱ��ĳ�ʱ�����ȴ�����ʱ�󣬽���
    virtual bool duration_wait(MUTEX *external_mutex, const ZCE_Time_Value &relative_time);

    // ��һ���ȴ��̷߳����ź� Signal one waiting thread.
    virtual void signal (void)
    {
        //
        int ret = zce::pthread_cond_signal(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_signal", ret);
            return ;
        }
    }

    ///�����еĵȴ��̹߳㲥�ź� Signal *all* waiting threads.
    virtual void broadcast (void)
    {
        //
        int ret = zce::pthread_cond_broadcast(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_broadcast", ret);
            return;
        }
    }

protected:

    ///������������
    pthread_cond_t    lock_;

};

//�����ֱ�����������ػ�����

///ʹ���߳�MUTEX
typedef ZCE_Thread_Condition<ZCE_Thread_Light_Mutex>        ZCE_Thread_Condition_Mutex;
///ʹ�ÿɵݹ��MUTEX����
typedef ZCE_Thread_Condition<ZCE_Thread_Recursive_Mutex>    ZCE_Thread_Condition_Recursive_Mutex;

#endif //ZCE_LIB_LOCK_THREAD_CONDI_H_

