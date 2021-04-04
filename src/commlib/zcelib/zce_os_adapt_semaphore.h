/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_semaphore.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��10��6�� ����Jobs���ˣ������Լ���ȻҲ���˵��˿�ʼ�廳ǰ����ͣ�
* @brief      �źŵƵĿ�ƽ̨��װ����LINUX��POSIX�ķ�װ����
*
* @details    Windows �� Linux ���źŵ��ϱȽϽӽ���
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_SEMAPHORE_H_
#define ZCE_LIB_OS_ADAPT_SEMAPHORE_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      ��ʼ��������һ���������������źŵƣ��߳���һ���������źŵƾ��㹻��,
*             WINDOWS�µ������źŵ����޷�����̹���ģ�
* @return     int         0�ɹ���-1ʧ��
* @param[out] sem         �źŵƾ��
* @param[in]  pshared     �źŵƵ�SHARE��ʽ��WINDOWS�´˲�������
* @param[in]  init_value  �źŵƳ��Ե�ֵ
* @param[in]  max_val     max_val���Ǳ�׼��������Ĭ��ֵ�����ˣ�Windows�����У���������ģ��Mutex��
* @note       ��Ӧ�����ٺ����� @ref sem_destroy
*/
int sem_init(sem_t *sem,
             int pshared,
             unsigned int init_value,
             unsigned int max_val = 0x7FFFFFFF);

/*!
* @brief      ��������(����)�źŵ�
* @return     int      0�ɹ���-1ʧ��
* @param      sem      �źŵƾ��
*/
int sem_destroy(sem_t *sem);

/*!
* @brief      ��(����)�źŵ�,max_val���ֵ���Ǳ�׼������������Ĭ��ֻ�����ˣ������Ҫ���ڴ��������źŵ�,
*             �����źŵƣ����Կ���̷���
* @return     sem_t*     ����ɹ����أ��źŵƾ�� ���򷵻�NULL
* @param      name       �źŵƵ�����
* @param      oflag      ͬ�ļ�open�Ĵ���������
* @param      mode       ͬ�ļ�open�ķ���Ȩ�޲���
* @param      init_value �źŵƳ�ʼ����ֵ
* @param      max_val    max_val���Ǳ�׼��������Ĭ��ֵ�����ˣ�Windows�����У�
* @note       �򿪺�Ҫʹ��@ref sem_close��@ref sem_unlink
*/
sem_t *sem_open(const char *name,
                int oflag = O_CREAT,
                mode_t mode = ZCE_DEFAULT_FILE_PERMS,
                unsigned int init_value = 1,
                unsigned int max_val = 0x7FFFFFFF);

/*!
* @brief      �ر��źŵ�
* @return     int 0�ɹ���-1ʧ��
* @param      sem �źŵƶ���
*/
int sem_close(sem_t *sem);

/*!
* @brief      ɾ���źŵ�
* @return     int  0�ɹ���-1ʧ��
* @param      name �źŵƵ�����
* @note
*/
int sem_unlink(const char *name);

/*!
* @brief      �źŵƵ�V����,
* @return     int
* @param      sem
* @note
*/
int sem_post (sem_t *sem);

/*!
* @brief      �Ǳ�׼�������źŵƵ�release_count��V������
* @return     int
* @param      sem
* @param      release_count
* @note
*/
int sem_post (sem_t *sem,
              u_int release_count);

/*!
* @brief      �źŵ�,����������P������,
* @return     int
* @param      sem
* @note
*/
int sem_trywait (sem_t *sem);

/*!
* @brief      �źŵ�������P������,����źŵƵ�ֵС��0��
* @return     int
* @param      sem
*/
int sem_wait (sem_t *sem);

/*!
* @brief      �źŵƴ���ʱ�ȴ�������������P��������ʱ���Ǿ���ֵ
* @return     int              0�ɹ���-1ʧ�ܣ�����ǳ�ʱʧ�ܣ�errno��ETIMEDOUT
* @param      sem              �źŵƶ���
* @param      abs_timeout_spec �ȴ���ʱ��㣬����ʱ��
* @note
*/
int sem_timedwait(sem_t *sem, const ::timespec *abs_timeout_spec);

/*!
* @brief      �źŵƴ���ʱ�ȴ�������������P������,�Ǳ�׼ʵ��,ʹ��timeval�ṹ��ʱ���Ǿ���ֵ
* @return     int              0�ɹ���-1ʧ�ܣ�����ǳ�ʱʧ�ܣ�errno��ETIMEDOUT
* @param      sem              �źŵƶ���
* @param      abs_timeout_time �ȴ���ʱ��㣬����ʱ��  timeval�����ڲ���ʱ����ʾͨ������
*/
int sem_timedwait(sem_t *sem, const timeval *abs_timeout_time);

/*!
* @brief      ���ص�ǰ�źŵƵĵ�ǰֵ,���ã�WindowsĿǰ��֧�֣�ĳ�̶ֳ��Ϸ�ӳ����WINDOWS API����Ƶ����⣩
*             ΢��ų��Ժ��Ľ����API,ReleaseSemaphore,��WIN8û�п����ı�
* @return     int   0�ɹ���-1ʧ�ܣ�
* @param      sem   �źŵƶ���
* @param      sval  �źŵƵ�ֵ
*/
int sem_getvalue(sem_t *sem, int *sval);
};

#endif //ZCE_LIB_OS_ADAPT_SEMAPHORE_H_

