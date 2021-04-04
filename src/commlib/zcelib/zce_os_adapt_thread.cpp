#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_thread.h"

//Ϊʲô��������ACE�������ޣ��������ޣ��������������������������ޣ���������������

//----------------------------------------------------------------------------------------
//
int zce::pthread_attr_init(pthread_attr_t *attr)
{
    //����ʱֻ��ע�⼸������
#if defined (ZCE_OS_WINDOWS)
    attr->detachstate = PTHREAD_CREATE_DETACHED;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedparam.sched_priority = 0;
    attr->stacksize = 0;
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_attr_init (attr);
#endif

}

//
int zce::pthread_attr_destroy(pthread_attr_t *attr )
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_attr_destroy (attr);
#endif

}

//���ã����߻���߳����Ա�������
//��������ã��̵߳ĵķ��룬JOIN���ԣ���ջ��С���̵߳ĵ������ȼ�
int zce::pthread_attr_getex(const pthread_attr_t *attr,
                            int *detachstate,
                            size_t *stacksize,
                            int *threadpriority
                           )
{
#if defined (ZCE_OS_WINDOWS)

    *detachstate = attr->detachstate;
    *threadpriority = attr->schedparam.sched_priority;
    *stacksize = attr->stacksize;
    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_getdetachstate (attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    sched_param param;
    ret = ::pthread_attr_getschedparam (attr, &param);

    if (ret != 0)
    {
        return ret;
    }

    *threadpriority  = param.sched_priority;
    ret = ::pthread_attr_getstacksize (attr, stacksize);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif
}

int zce::pthread_attr_setex(pthread_attr_t *attr,
                            int detachstate,
                            size_t stacksize,
                            int threadpriority
                           )
{

#if defined (ZCE_OS_WINDOWS)
    assert( PTHREAD_CREATE_JOINABLE == detachstate || PTHREAD_CREATE_DETACHED == detachstate );
    attr->detachstate = detachstate;
    attr->stacksize = stacksize;
    //�޸ĵ��Ȳ��Լ̳з�ʽ
    attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr->schedparam.sched_priority = threadpriority;

    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_setdetachstate (attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    //ע�������Ҫ�������Ƿ����ز��ӷ���̵õ����ȷ���
    ret = ::pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);

    if (ret != 0)
    {
        return ret;
    }

    sched_param param;
    param.sched_priority = threadpriority;
    ret = ::pthread_attr_setschedparam (attr, &param);

    if (ret != 0)
    {
        return ret;
    }

    // �Ӹ��ж����С����Сֵ����Ϊ��Сֵ
    if (stacksize < PTHREAD_STACK_MIN)
    {
        stacksize = PTHREAD_STACK_MIN;
    }

    ret = ::pthread_attr_setstacksize (attr, stacksize);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif //#if defined (ZCE_OS_LINUX)
}

//�߳�����������������
class THREAD_START_FUN_ADAPT
{
protected:
    typedef void (*START_ROUTINE_FUN)(void *);

    //�߳���������
    START_ROUTINE_FUN   start_routine_;
    //�߳���������
    void               *arg_;

public:
    //���캯������������
    THREAD_START_FUN_ADAPT(void (*start_routine)(void *), void *arg):
        start_routine_(start_routine),
        arg_(arg)
    {
    }

    ~THREAD_START_FUN_ADAPT()
    {
    }
#if defined (ZCE_OS_WINDOWS)
    //
    static unsigned int __stdcall adapt_svc_run(void *adapt_svc)
    {
        THREAD_START_FUN_ADAPT *my_adapt = static_cast<THREAD_START_FUN_ADAPT *>(adapt_svc);
        my_adapt->start_routine_(my_adapt->arg_);
        delete my_adapt;
        unsigned int return_data = 0;
        return return_data;
    }
#elif defined (ZCE_OS_LINUX)
    //
    static void *adapt_svc_run(void *adapt_svc)
    {
        THREAD_START_FUN_ADAPT *my_adapt = static_cast<THREAD_START_FUN_ADAPT *>(adapt_svc);
        my_adapt->start_routine_(my_adapt->arg_);
        delete my_adapt;
        void *return_dword = 0;
        return return_dword;
    }
#endif //
};

//����һ��һ���߳�,Ϊ�˷��㣬�����Ҳ���ṩ�Դ����͹���Ĺ����ˣ�
//ע��start_routine���Һ�pthread_create�õĲ�һ��ม�
//Ϊʲôvoid (*start_routine)(void*) ������LINUX�µı�׼�أ���������ԥ���Σ����������ĸĴ��룬
//ACE��ʵ�ַ�ʽ�ǶԲ�ͬƽ̨��������Դ����ô��Ǵ��û�з���Ŀ��ܣ�ȱ�����޷�д��һ�µĴ��룬����������
//����ѡ��˭Ҳ��ȥǨ�ͣ����ṩ����ֵ���㣬�������Ҫ����ֵ�������߳���ķ�װ��������API
//��ΪWIN32�·���unsigned int��LINUX�·���void *����Ҫ����ƽ̨�����ԣ���˭ת����˭��������,�����ҷ���
//����ͳһû�з���ֵ���������Ŀ��ܣ����������Լ�
int zce::pthread_create(ZCE_THREAD_ID *threadid,
                        const pthread_attr_t *attr,
                        void (*start_routine)(void *),
                        void *arg)
{
#if defined (ZCE_OS_WINDOWS)

    //�������new��֮���ڴ������̺߳����delete,
    //����������ɾ�����ڣ�THREAD_START_FUN_ADAPT::adapt_svc_run����ط���
    THREAD_START_FUN_ADAPT *adapt_object = new THREAD_START_FUN_ADAPT(start_routine, arg);
    //��CRT���̴߳������������߳�
    HANDLE thread_handle = (HANDLE)::_beginthreadex(NULL,
                                                    static_cast<unsigned int>(attr->stacksize),
                                                    THREAD_START_FUN_ADAPT::adapt_svc_run,
                                                    adapt_object,
                                                    0,
                                                    threadid);

    //ע��_beginthreadex�ķ���ֵ0��ʾ���󣬺�_beginthread��һ��
    if (NULL == thread_handle)
    {
        delete adapt_object;
        return -1;
    }

    //�����߳����ȼ�
    if (PTHREAD_EXPLICIT_SCHED == attr->inheritsched &&
        0 != attr->schedparam.sched_priority )
    {
        //�߳�������ȼ���ȡֵ��Ӧ���£�
        BOOL bret = ::SetThreadPriority(thread_handle,
                                        attr->schedparam.sched_priority);

        if ( !bret )
        {
            //�Ƿ��¼һ�£��Ǻ�
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    THREAD_START_FUN_ADAPT *adapt_object = new THREAD_START_FUN_ADAPT(start_routine, arg);
    return ::pthread_create (threadid, attr,
                             THREAD_START_FUN_ADAPT::adapt_svc_run,
                             adapt_object);

#endif //#if defined (ZCE_OS_LINUX)
}

//�������POSIX�ķ�װ�����Ƽ�ʹ��
//�����ṩһ����һ��ķ�װ�ɣ�������ô���pthread_attr_t
int zce::pthread_createex(void (*start_routine)(void *),
                          void *arg,
                          ZCE_THREAD_ID *threadid,
                          int detachstate,
                          size_t stacksize,
                          int threadpriority
                         )
{
    int ret = 0;
    pthread_attr_t attr;
    ret = zce::pthread_attr_init(&attr);

    if (0 != ret)
    {
        return ret;
    }

    //�����̲߳���
    ret = zce::pthread_attr_setex(&attr,
                                  detachstate,
                                  stacksize,
                                  threadpriority
                                 );

    if (0 != ret)
    {
        zce::pthread_attr_destroy(&attr);
        return ret;
    }

    //�����߳�
    ret = zce::pthread_create(threadid,
                              &attr,
                              start_routine,
                              arg);

    if (0 != ret)
    {
        zce::pthread_attr_destroy(&attr);
        return ret;
    }

    return 0;
}

//�˳�������void������ƽ̨�����ݣ����Ըɴ�ʲô�������ݳ���
void zce::pthread_exit( void )
{
#if defined (ZCE_OS_WINDOWS)

    unsigned int return_data = 0;
    _endthreadex(return_data);
    return;

#elif defined (ZCE_OS_LINUX)
    void *return_data = NULL;
    return ::pthread_exit (return_data);
#endif //#if defined (ZCE_OS_LINUX)
}

//�ȴ�ĳ��JOIN���߳̽���,������᷵��ֵ
int zce::pthread_join(ZCE_THREAD_ID threadid)
{
    ZCE_THR_FUNC_RETURN ret_val;
    return zce::pthread_join (threadid, &ret_val);
}

//==========================================================================================
//ע�⣬WINDOWS��LINUX�·���ֵ��ͬ�������ҷǳ��ǳ�����������������ݷ���ֵ��
//WINDOWS ���Դ���unsigned int ��LINUX����void *��

#if defined (ZCE_OS_WINDOWS)

class WIN_THREAD_STARTFUN_ADAPT
{
protected:
    typedef unsigned int (*WIN_START_ROUTINE_FUN)(void *);

    //�߳���������
    WIN_START_ROUTINE_FUN start_routine_;
    //�߳���������
    void                 *arg_;

public:
    //���캯������������
    WIN_THREAD_STARTFUN_ADAPT(unsigned int (*start_routine)(void *), void *arg):
        start_routine_(start_routine),
        arg_(arg)
    {
    }

    ~WIN_THREAD_STARTFUN_ADAPT()
    {
    }

    //
    static unsigned int __stdcall adapt_svc_run(void *adapt_svc)
    {
        WIN_THREAD_STARTFUN_ADAPT *my_adapt = static_cast<WIN_THREAD_STARTFUN_ADAPT *>(adapt_svc);
        unsigned int return_data = my_adapt->start_routine_(my_adapt->arg_);
        return return_data;
    }

};

#endif //

//����һ���߳�,�����̺߳��������ƽ̨����ģʽ���룬�з���ֵ
int zce::pthread_create(ZCE_THREAD_ID *threadid,
                        const pthread_attr_t *attr,
                        ZCE_THR_FUNC_RETURN (* start_routine)(void *),
                        void *arg)
{
#if defined (ZCE_OS_WINDOWS)

    WIN_THREAD_STARTFUN_ADAPT adapt_object(start_routine, arg);
    //��CRT���̴߳������������߳�
    HANDLE thread_handle = (HANDLE)::_beginthreadex(NULL,
                                                    static_cast<unsigned int>(attr->stacksize),
                                                    THREAD_START_FUN_ADAPT::adapt_svc_run,
                                                    &adapt_object,
                                                    0,
                                                    threadid);
    //ע��_beginthreadex�ķ���ֵ0��ʾ���󣬺�_beginthread��һ��
    if (NULL == thread_handle)
    {
        return -1;
    }

    //�����߳����ȼ�
    if (PTHREAD_EXPLICIT_SCHED == attr->inheritsched &&
        0 != attr->schedparam.sched_priority )
    {
        //�߳�������ȼ���ȡֵ��Ӧ���£�
        BOOL bret = ::SetThreadPriority(thread_handle,
                                        attr->schedparam.sched_priority);

        if ( !bret )
        {
            //�Ƿ��¼һ�£��Ǻ�
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_create (threadid,
                             attr,
                             start_routine,
                             arg);

#endif //#if defined (ZCE_OS_LINUX)
}

//�˳�һ���̣߳����Եõ�����ֵ
void zce::pthread_exit(ZCE_THR_FUNC_RETURN thr_ret)
{
#if defined (ZCE_OS_WINDOWS)
    return _endthreadex(thr_ret);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_exit (thr_ret);
#endif //#if defined (ZCE_OS_LINUX)
}

//�ȴ�ĳ��JOIN���߳̽���,���ҵõ��̻߳ص������ķ���ֵ
int zce::pthread_join(ZCE_THREAD_ID threadid, ZCE_THR_FUNC_RETURN *ret_val)
{

#if defined (ZCE_OS_WINDOWS)

    //OpenThread��һ��WIN SERVER 2000����еĺ��� VC6Ӧ��û��
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
                                            );
    if (thr_handle == NULL)
    {
        return -1;
    }
    DWORD thread_ret;

    if (::WaitForSingleObject (thr_handle, INFINITE) == WAIT_OBJECT_0
        && ::GetExitCodeThread (thr_handle, &thread_ret) != FALSE)
    {
        *ret_val = thread_ret;
        //�˴���Ҫ�ر�handle��  ::CloseHandle (thr_handle);������Ϊ���ǵ��õ���_endthreadex �������߳�
        return 0;
    }

    return -1;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_join (threadid, ret_val);
#endif //#if defined (ZCE_OS_LINUX)
}

//�õ��߳�ID
ZCE_THREAD_ID zce::pthread_self(void)
{
#if defined (ZCE_OS_WINDOWS)
    return ::GetCurrentThreadId();
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_self ();
#endif //#if defined (ZCE_OS_LINUX)
}

//ȡ��һ���߳�
//������������������ȫ��Ϊ�������ҵ�һ����װ��Ը����Ϊ��LINUXƽ̨����δ����ȡ��һ���߳�
//����WINDOWS������TerminateThread���������Ҳ�ǲ����Ƽ���
int zce::pthread_cancel(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)

#pragma warning (push)
#pragma warning (disable:6258)
    //OpenThread��һ��WIN SERVER 2000����еĺ��� VC6Ӧ��û��
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
                                            );
    if (thr_handle == NULL)
    {
        errno = GetLastError();
        return -1;
    }
    //ǿ���˳�
    BOOL bret = ::TerminateThread(thr_handle, 0);
    if (!bret)
    {
        return -1;
    }

    ::CloseHandle(thr_handle);
    return 0;

#pragma warning (pop)

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_cancel (threadid);
#endif //#if defined (ZCE_OS_LINUX)
}

//��һ���߳̽����ɰ�
int zce::pthread_detach(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows�̱߳�������detach�ģ��Ǻ�
    ZCE_UNUSED_ARG(threadid);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_detach (threadid);
#endif //#if defined (ZCE_OS_LINUX)
}

//���߳�IDת��ΪHANDLE��ֻ��WIN2000������
#if defined ZCE_OS_WINDOWS
ZCE_THREAD_HANDLE pthread_id2handle(ZCE_THREAD_ID threadid)
{
    //OpenThread��һ��WIN SERVER 2000����еĺ��� VC6Ӧ��û��
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
                                            );
    return thr_handle;

}
#endif //

int zce::pthread_yield(void)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep(0);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_yield ();
#endif //#if defined (ZCE_OS_LINUX)
}

//=================================================================================================================

//destructor ��������WIN32ƽ̨û���ã����鲻�á�
//
int zce::pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(destructor);

    *key = ::TlsAlloc ();

    //�������FALSE��ʶʧ��
    if (TLS_OUT_OF_INDEXES == *key)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_key_create(key, destructor);
#endif
}

//
int zce::pthread_key_delete(pthread_key_t key)
{

#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret =  ::TlsFree (key);

    //�������FALSE��ʶʧ��
    if (!bool_ret)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_key_delete(key);
#endif
}

int zce::pthread_setspecific(pthread_key_t key, const void *data)
{

#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret =  ::TlsSetValue (key, (LPVOID)( data));

    //�������FALSE��ʶʧ��
    if (!bool_ret)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_setspecific (key, data);
#endif
}

void *zce::pthread_getspecific (pthread_key_t key)
{
#if defined (ZCE_OS_WINDOWS)

    LPVOID  data = ::TlsGetValue (key);

    if (data == 0 && ::GetLastError () != NO_ERROR)
    {
        errno = GetLastError();
        return NULL;
    }
    else
    {
        return data;
    }

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_getspecific (key);

#endif

}

