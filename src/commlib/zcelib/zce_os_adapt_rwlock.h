/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_rwlock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��4��19��
* @brief      ��д������������-��ռ������ģ�⣬��OS���䣬WIN��ʹ����������ģ�⣬
*             д���ʼ�Ļ�������WIN SVR 2008��VISTA���Ժ��ϵͳ��ʹ�ã���Ϊ
*             ģ��Ķ���������ʵԶ�Ȳ����ٽ�������MS�Լ�ʵ�ֵĶ�д��������ʵͦ�õģ�
*             WIN2008�Ժ���һ���̼߳����д����MS�Լ�ʵ���ˣ�������UNLOCK����
*             �Ƿֳ������ģ�д���⿪����ȡ���⿪��Ҳ�����ݴ�����һЩ�鷳��
*             �������汾���Ҫʹ��ֻ��ģ���ˣ�
*             ��WINDOWS�µ�ģ��ֻ֧���̰߳汾�������㿼�Ƕ���̣�
*
*             ���Լ��Ĳ��ԣ�WIN�£�ģ���RW�ٶȴ�Լ����ʵ��RW LOCK��9��������
*             ���Ĳ��Ƽ���ģ��ģ���2��2д������������ݺ�MS�Ĳ��Խ�����ƣ�
*
* @details    �����Ĵ����Ǵ�W.Richard stevens��UNP V2�����ϸ�д�ģ�
*             ������˼�ķ�����Щ������Ĳο��Ļ���ACE�����ģ�
*             Ϊ����WINDOWS�ϸ������ף������˼򻯵�EVENTģ�����������
*
*             ���ʹ��Windows 2008 VISTA ֮����е��Ǹ�������������һ������Ҫ�����
*             ��ΪWindows��API��Ƶĺ�LINUX PTHREAD����Ʋ�һ����WINDOWS��API
*             �ṩ��2���������������ڹ��������������ֱ�һ������PTHREADֵ�ṩ��һ����
*             ������N��ѡ��
*             1.��unlock�������棬�ѹ��������������Ľ���������һ�Σ���Windows����
*             ��Ϊ�ҵĶ�Σ����󣩵��ã�������ʲô�鷳��
*             2.�ṩ�������������ķ�װ��BOOST����������˼·��
*             3.�Լ���TLS�������������䵽���õ���ʲô����������������õĽ���������ʱ��
*             �����еķ�ʸ��
*
*/

#ifndef ZCE_LIB_OS_ADAPT_RWLOCK_H_
#define ZCE_LIB_OS_ADAPT_RWLOCK_H_

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_condi.h"



namespace zce
{



/*!
* @brief      ��д���Ķ���ĳ�ʼ��
* @return     int
* @param      rwlock
* @param      attr
* @note
*/
int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr);


/*!
* @brief      ���ٶ�д���Ķ���
* @return     int
* @param      rwlock
*/
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

/*!
* @brief      ��д���Ķ���ĵĳ�ʼ�����Ǳ�׼�������飬ʹ��
* @return     int               ����0��ʶ�ɹ���
* @param      rwlock            ��д������
* @param      use_win_slim      �Ƿ�ʹ��WIN2008�Ժ�ӵ�е�Slim RWlock��ע��Slim RWlockû�г�ʱ����
*                               ��Ȼǰ�����ϵͳ����֧�֣�
* @param      priority_to_write д�����Ȼ��Ƕ�ȡ���ȣ���ֻ��Windows��ģ�����ã���ȫ��Ϊ���Լ�����,�����ʲô��MSһ�����Ʋ��ԣ�
*/
int pthread_rwlock_initex(pthread_rwlock_t *rwlock,
                          bool  priority_to_write = true);

/*!
* @brief      ��ȡ��ȡ���������ȡ�������еȴ�
* @return     int     0�ɹ���������ֵ��ʾ����ID
* @param      rwlock  ��д������
*/
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

/*!
* @brief      ���Ի�ȡ�����������ȡ�������򷵻ش��󣬲�����
* @return     int     0�ɹ�������ʧ��
* @param      rwlock  ��д������
* @note
*/
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

/*!
* @brief      ��ȡ��ȡ���ģ�ͬʱ�ȴ�ֱ����ʱ
* @return     int              0�ɹ�������ʧ�ܣ�����ֵ��ʾ����ID
* @param      rwlock           ��д������
* @param      abs_timeout_spec �ȴ���ʱ��㣨ʱ��㣩
* @note
*/
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                               const ::timespec *abs_timeout_spec);

/*!
* @brief      �Ǳ�׼����ȡ���ĳ�ʱ������ʱ�����������timeval��
* @return     int              0�ɹ�������ʧ�ܣ�����ֵ��ʾ����ID
* @param      rwlock           ��д������
* @param      abs_timeout_val  �ȴ���ʱ��㣨ʱ��㣩
*/
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                               const timeval *abs_timeout_val);

/*!
* @brief      ��ȡд������ռ�����������ȡ��������һֱ�ȴ�
* @return     int     0�ɹ�������ʧ�ܣ�����ֵ��ʾ����ID
* @param      rwlock  ��д������
*/
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

/*!
* @brief      ���Ի�ȡд������ռ�����������ȡ���������ش��󣬲�����
* @return     int     0�ɹ�������ʧ�ܣ�����ֵ��ʾ����ID
* @param      rwlock  ��д������
*/
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);

/*!
* @brief      ��ȡд������ռ�����������ȡ��������ȴ�һ��ʱ��
* @return     int         0�ɹ�������ʧ�ܣ�����ֵ��ʾ����ID
* @param      rwlock           ��д������
* @param      abs_timeout_spec ��ʱʱ��㣬����ʱ��,timespec���͵ģ��ͱ�׼һ��
*/
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                               const ::timespec *abs_timeout_spec);

/*!
@param      abs_timeout_val ��ʱʱ��㣬����ʱ��,timeval���͵�,���ϣ�����ֵ��дNULL������������ܺ�����ĺ�����ͻ��
* @brief      �Ǳ�׼��дȡ���ĳ�ʱ������ʱ�����������timeval��
* @return     int         0�ɹ�������ʧ��
* @param      rwlock          ��д������
*                             ���������� timeval*time_out =NULL,�����time_out��Ϊ������
*/
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                               const timeval *abs_timeout_val);

/*!
* @brief      �������Զ�����д������һ������������ط���WINDOWS SVR 20008�Ժ�ʵ�ֵĶ�д��ʵ�ֲ�һ��
* @return     int     0�ɹ�������ʧ��
* @param      rwlock  ��д������
*/
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);


};

#endif //ZCE_LIB_OS_ADAPT_RWLOCK_H_


