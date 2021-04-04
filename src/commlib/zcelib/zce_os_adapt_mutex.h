/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��8��13��
* @brief      ��ƽ̨���������ֵĴ��룬�ӿڻ���ģ����pthread�ĺ�����
*
* @details    ��Windows�����Ƿֳɼ���ģʽ,����TIMEOUT�������ݹ������ǵݹ���
*             ����TIMEOUT��MUTEX���������ٽ���ģ��ʵ�ֵģ����ٽ����ǵݹ��ѽ��
*             �ݹ�����WINDOWS�Լ���MUTEX�ǵݹ�ģ�
*             �ǵݹ�������ร���������źŵ�ģ��
*             ��Щ�������������ķ�װ����@ref zce_lock_thread_mutex.h
*             @ref zce_lock_process_mutex.h,
*             ��������Ķ������������ˣ�д������Ա��á���������C���ÿ�
*             pthread_mutex�ڲ����Ѿ������˷ǵݹ������ܣ����źŵ�ģ��
*
* @note       pthread_mutex_XXX�����ķ���ֵ���Ƿ���0��ʾ�ɹ��������Ǵ���ID
*             �����POSIX�󲿷ֺ�����̫һ�����󲿷�����������return -1;Ȼ�����
*             ������errno�У�
*             ��СС�Ĵ��⣬�������Ϊ�������Ϊ���ڶ��̻߳���������ֵ�ȱ�����errno
*             �������״���
*             ����ط�������pthread_mutex_XXX�������룬return����ֵ���汣�����
*             ID��
*             ����ע�ⷵ��ֵ�Ĵ���
*/

#ifndef ZCE_LIB_OS_ADAPT_MUTEX_H_
#define ZCE_LIB_OS_ADAPT_MUTEX_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

//Pthread Mutex�����Բ��ֲ����������е���Ų��ĸо�

/*!
* @brief      ��ʼ��MUTEX�����Բ���
* @return     int  ����0��ʶ�ɹ�����ʵ��ʵ�ڿ��������ֺ�����ô����ʧ�ܣ�
* @param      attr ��ʼ��������
*/
int pthread_mutexattr_init (pthread_mutexattr_t *attr);

/*!
* @brief      ��������������
* @return     int  ����0��ʶ�ɹ�
* @param      attr ���ٵ�����
*/
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

/*!
* @brief      �������������ù�������
* @return     int      ����0��ʶ�ɹ�
* @param      attr     ����������
* @param      pshared  ������Ч��PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE����������̼乲��Ҫ����PTHREAD_PROCESS_SHARED
*/
int pthread_mutexattr_setpshared (pthread_mutexattr_t *attr,
                                  int pshared);
//
/*!
* @brief      ȡ���̵߳Ĺ�������
* @return     int      ����0��ʶ�ɹ�
* @param[in]  attr
* @param[out] pshared ���ع�������
*/
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
                                 int *pshared);

/*!
* @brief      �����̵߳�����
* @return     int    ����0��ʶ�ɹ�
* @param      attr   ���õ�����
* @param      type   PTHREAD_MUTEX_XXXX�ļ���ֵ��������|һ�����ö������
*/
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

/*!
* @brief      ȡ���̵߳�����
* @return     int    ����0��ʶ�ɹ�
* @param      attr   ȡ����Ϥ
* @param      type   ���ص�type
* @note
*/
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr,
                              int *type);

/*!
* @brief      pthread mutex ��ʼ��
* @return     int    ����0��ʾ�ɹ�,���򷵻ش���ID
* @param      mutex  MUTEX����
* @param      attr   MUTEX������
*/
int pthread_mutex_init (pthread_mutex_t *mutex,
                        const pthread_mutexattr_t *attr);

#if defined (ZCE_OS_WINDOWS)
/*!
* @brief      ��ʼ�������̵߳����ԣ���ͬ��ƽ̨����ͬ��Ĭ�϶��壬
* @return     int             ����0��ʾ�ɹ�
* @param      mutex           PTHREAD MUTEX ����
* @param      process_share   �Ƿ���Ҫ���̼乲��Ĭ�϶�FALSE
* @param      recursive       �Ƿ���Ҫ�ݹ飬Ĭ��WIN��LINUX��TURE
* @param      need_timeout    �Ƿ���Ҫ��ʱ����WIN��Ĭ��ΪFALSE����Ϊ���������óɱ����͵��ٽ�����
* @param      mutex_name      �����������֣�WIN�����Ҫ����̹����ͱ����и����֣����Ҫ����̹����鷳���ϸ������֣�����û����
* @note       �Ǳ�׼�����ǽ�����ʹ�ã��򵥶���,
*/
int pthread_mutex_initex(pthread_mutex_t *mutex,
                         bool process_share = false,
                         bool recursive = true,
                         bool need_timeout = false,
                         const char *mutex_name = NULL);
#elif defined (ZCE_OS_LINUX)
int pthread_mutex_initex(pthread_mutex_t *mutex,
                         bool process_share = false,
                         bool recursive = true,
                         bool need_timeout = true,
                         const char *mutex_name = NULL);
#endif

/*!
* @brief      pthread mutex ����
* @return     int   ����0��ʾ�ɹ�
* @param      mutex MUTEX����
*/
int pthread_mutex_destroy (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex ����
* @return     int     ����0��ʾ�ɹ�
* @param      mutex   MUTEX����
*/
int pthread_mutex_lock (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex ��ʱ������
* @return     int              ����0��ʾ�ɹ������򷵻ش���ID
* @param      mutex            MUTEX����
* @param      abs_timeout_spec ��ʱ��ʱ�䣬Ϊtimespec���ͣ������Ǿ���ʱ�䣬
*/
int pthread_mutex_timedlock (pthread_mutex_t *mutex,
                             const ::timespec *abs_timeout_spec);

/*!
* @brief      pthread mutex ��ʱ�������Ǳ�׼ʵ��,�����ڲ��Ƚ�ͨ�õ�ʱ�����timeval
* @return     int             ����0��ʾ�ɹ������򷵻ش���ID
* @param      mutex           MUTEX����
* @param      abs_timeout_val ��ʱ��ʱ�䣬timeval���ͣ�����ʱ�䣬
*                             ���ϣ�����ֵ��дNULL������������ܺ�����ĺ�����ͻ��
*                             ���������� timeval*time_out =NULL,�����time_out��Ϊ������
*/
int pthread_mutex_timedlock (pthread_mutex_t *mutex,
                             const timeval *abs_timeout_val);

/*!
* @brief      pthread mutex ���Լ���
* @return     int
* @param      mutex
*/
int pthread_mutex_trylock (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex ����
* @return     int    ����0��ʾ�ɹ�
* @param      mutex  MUTEX����
*/
int pthread_mutex_unlock (pthread_mutex_t *mutex);

};

#endif //ZCE_LIB_OS_ADAPT_TIME_H_

