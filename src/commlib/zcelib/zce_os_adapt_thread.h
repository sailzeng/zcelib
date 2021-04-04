/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_thread.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��16��
* @brief
*
* @details    Ŀ����վ��ACE���˵ļ���ϣ��õ�һ�����Ӽ򵥵ķ�װ
*             ACE���߳���������SUNƽ̨�Ļ�����ʵ�ֵģ���pthread����������
*             ���ⲿ�ֺ�����û�б�������ΪΪ�˼��ݣ����Һܶ����������ƽ̨��ͬ��
*             ��Щ��Ϊ��REAL TIMEϵͳ׼����
*
* @note       ע��WINDOWS�µ��ں���ʵʹ��HANDLE��ʶһ���̣߳��������߳�ID��
*
*/

#ifndef ZCE_LIB_OS_ADAPT_THREAD_H_
#define ZCE_LIB_OS_ADAPT_THREAD_H_

#include "zce_os_adapt_predefine.h"

//����
namespace zce
{

//------------------------------------------------------------------------------------------------------

/*!
* @brief      ��ʼ���߳�����
* @return     int       0�ɹ���-1ʧ��
* @param[in]  attr
*/
int pthread_attr_init(pthread_attr_t *attr);

/*!
* @brief      �����߳�����
* @return     int       0�ɹ���-1ʧ��
* @param[in]  attr
*/
int pthread_attr_destroy(pthread_attr_t *attr);

/*!
* @brief      �Ǳ�׼�����������߳����Ա������ԣ���������ã��̵߳ĵķ��룬JOIN���ԣ���ջ��С���̵߳ĵ������ȼ�
* @return     int                0�ɹ���-1ʧ��
* @param[out] attr
* @param[in]  detachstate    ��������� PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
* @param[in]  stacksize      ��ջ�Ĵ�С
* @param[in]  threadpriority �߳����ȼ�����0��Ĭ��ֵ������̫�����������������
* @note       WINDOWS ��LINUX���е����Բ�����
*/
int pthread_attr_setex(pthread_attr_t *attr,
                       int detachstate = PTHREAD_CREATE_DETACHED,
                       size_t stacksize = 0,
                       int threadpriority = 0
                      );

/*!
* @brief      �Ǳ�׼����������̼߳�������
* @return     int           0�ɹ���-1ʧ��
* @param      attr
* @param      detachstate
* @param      stacksize
* @param      threadpriority
*/
int pthread_attr_getex(const pthread_attr_t *attr,
                       int *detachstate,
                       size_t *stacksize,
                       int *threadpriority
                      );

/*!
* @brief      ����һ���߳�,
*             Ϊ�˷��㣬�����Ҳ���ṩ�Դ����͹���Ĺ��ܣ������󣬾Ϳ�ʼ������
*             ����ֵ����˵����WINDOWS�£�����ָ��ķ���ֵ������һ��DWORD��LINUX�£�����ָ�뷵��ֵ������void *
*             ��ƽ̨����Ҫѡ��ĳ�ֲ������ͣ����߷���ѡ����ʵ����д���������ʱ��ҡ���˼��Σ�����Ƿ�����,��Ϊ��������װ�������ܵ������ã�
*             ���������ôǿ�İ��ã������������飬���ڲ�ͬƽ̨�²�һ��,
*             ��ͷ����pthread WIN32ʵ�ֵ��뷨����һ������ѡ����void ��Ϊ����ֵ
* @return     int               0�ɹ���-1ʧ��
* @param[out] threadid      �߳�ID������WINDOWSҲ���߳�ID�������߳̾����
* @param[in]  attr          �߳����Բ���
* @param[in]  start_routine ���е��̵߳Ļص�����ָ�룬��������ķ���ֵ��void����û��Ǩ���κ�ƽ̨��
* @param[in]  arg           ����ָ��Ĳ���
*/
int pthread_create(ZCE_THREAD_ID *threadid,
                   const pthread_attr_t *attr,
                   void (*start_routine)(void *),
                   void *arg);

/*!
* @brief      �����̣߳���һ��ķ�װ��������ô���pthread_attr_t
*             �������POSIX�ķ�װ�����Ƽ�ʹ��
* @return     int                0�ɹ���-1ʧ��
* @param[in]  start_routine  ����ָ��
* @param[in]  arg            start_routine���� �Ĳ���
* @param[out] threadid      ���ص��߳�ID
* @param[in]  detachstate    ��������� PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
* @param[in]  stacksize      ��ջ��С
* @param[in]  threadpriority �߳����ȼ� = 0 ��ʾĬ��
*/
int pthread_createex(void (*start_routine)(void *),
                     void *arg,
                     ZCE_THREAD_ID *threadid,
                     int detachstate = PTHREAD_CREATE_DETACHED,
                     size_t stacksize = 0,
                     int threadpriority = 0
                    );

/*!
* @brief      �˳��̣߳�ע�����û���κβ���������Ϊ����ֵ��
*             ע�⣬WINDOWS��LINUX�·���ֵ��ͬ�������ҷ�����ʹ�÷���ֵ������
*/
void pthread_exit(void );

/*!
* @brief      �ȴ�ĳ��JOIN���߳̽���������᷵��ֵ
* @return     int      0�ɹ���-1ʧ��
* @param      threadid �ȴ��˳����߳�ID��
* @note       �Ǳ�׼������û��ȡ�ط���ֵ
*/
int pthread_join(ZCE_THREAD_ID threadid);

//---------------------------------------------------------------------------------------------------------------
//��������������������3�������������ƣ���������Ϊ�˾����Ŭ�����������ϵͳ������Ƶģ�������ʹ��

/*!
* @brief      ����һ���߳�,�����̺߳��������ƽ̨����ģʽ���룬�з���ֵ�������ڸ���ƽ̨���岻ͬ�Ļص�����
* @return     int           0�ɹ���-1ʧ��
* @param      threadid      �߳�ID������WINDOWSҲ���߳�ID�������߳̾����
* @param      attr          �߳�����
* @param      start_routine ���е��̵߳Ļص�����ָ��
* @param      arg           start_routine �ص������Ĳ���
* @note       �Ƽ�������ĺ��������������ṩ������Ҫ��Ϊ������һЩ���Ⱞ�ã���������д����Ҫ�ڲ�ͬ��ƽ̨д��ֵĴ��룬�α��أ�
*/
int pthread_create(ZCE_THREAD_ID *threadid,
                   const pthread_attr_t *attr,
                   ZCE_THR_FUNC_RETURN (* start_routine)(void *),
                   void *arg);

/*!
* @brief      �ȴ�ĳ��JOIN���߳̽���,���ҵõ��̻߳ص������ķ���ֵ
* @return     int
* @param      threadid �ȴ��˳����߳�ID��
* @param      ret_val  �̵߳ķ���ֵ����LINUX��WINDOWS����ʱ��������ͬ
*/
int pthread_join(ZCE_THREAD_ID threadid, ZCE_THR_FUNC_RETURN *ret_val);

/*!
* @brief      �˳�ĳ���̣߳�ͬʱ֪ͨ�߳��˳��ķ���ֵ
* @param      thr_ret ����ֵ
*/
void pthread_exit(ZCE_THR_FUNC_RETURN thr_ret );

//---------------------------------------------------------------------------------------------------------------

/*!
* @brief      �ȴ�ĳ��JOIN���߳̽���
* @return     int
* @param      threadid �ȴ��˳����߳�ID��
* @note       �Ǳ�׼������û��ȡ�ط���ֵ
*/
int pthread_join(ZCE_THREAD_ID threadid);

/*!
* @brief      �õ���ǰ�߳�ID
* @return     ZCE_THREAD_ID
*/
ZCE_THREAD_ID pthread_self(void);

/*!
* @brief      ȡ��һ���߳�
* @return     int       0�ɹ���-1ʧ��
* @param      threadid  �������߳�ID
* @note       ������������������ȫ��Ϊ�������ҵ�һ����װ��Ը����Ϊ��LINUXƽ̨����δ����ȡ��һ���߳�
*             ����WINDOWS������TerminateThread���������Ҳ�ǲ����Ƽ���
*/
int pthread_cancel(ZCE_THREAD_ID threadid);

/*!
* @brief      ��һ���߳̽����ɰ󣬲�����Ҫjoinȥ�ȴ��˳�
* @return     int        0�ɹ���-1ʧ��
* @param      threadid   �������߳�ID
*/
int pthread_detach(ZCE_THREAD_ID threadid);

#if defined ZCE_OS_WINDOWS
/*!
* @brief      ���߳�IDת��ΪHANDLE��ֻ��WIN2000�Ժ�����
* @return     ZCE_THREAD_HANDLE
* @param      threadid
* @note       �ڲ����õ���OpenThread��һ��WIN SERVER 2000����еĺ��� VC6Ӧ��û��
*/
ZCE_THREAD_HANDLE pthread_id2handle(ZCE_THREAD_ID threadid);
#endif //

int pthread_yield(void);

//------------------------------------------------------------------------------------------------------
//Thread Specific Data ���߳�˽�У�ר�У��洢 TSS
//Ҳ�� TLS Thread Local Storage

/*!
* @brief
* @return     int
* @param      key
* @param      (*destructor) �����������˲�����Windows ��û���ô����������ƽ̨��������ʹ��
*/
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

/*!
* @brief
* @return     int
* @param      key
* @note
*/
int pthread_key_delete(pthread_key_t key);

/*!
* @brief
* @return     void*
* @param      key
* @note
*/
void *pthread_getspecific(pthread_key_t key);

/*!
* @brief
* @return     int
* @param      key
* @param      value
* @note
*/
int pthread_setspecific(pthread_key_t key, const void *value);

};

#endif //ZCE_LIB_OS_ADAPT_THREAD_H_

