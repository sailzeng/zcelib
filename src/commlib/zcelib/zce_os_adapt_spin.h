/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_spin.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��9��6��
* @brief      pthread spin�����ķ�װ��
*             SPIN LOCK��������������CPU�л���ͬ������
*             �ҵ�ʵ�֣���WINDOWS�����ٽ����������ڣ�+SPIN ��ʵ�� ����Mutex�����̼䣩ģ
*             ��SPIN lock�� ��Ȼ������ʵû������ʵ��SPIN LOCK��Ч����
*
* @details    WINDOWS�º�LINUX����ʵ��ʵ����Spin lock����WINDOWS�£�������SPIN LOCK
*             ������Spin lockֻ����������ʹ�ã��û�����ʵ�޷�ʹ�ã�
*             ������һ���ٽ���ʹ��SPIN�����ƣ����ٽ����ҵ����������������
*             ��WINDOWS��ʵ�ֱȽ�����˼�����������Լ�����һ��SPIN Count����û�дﵽ���ֵ
*             ʱ�����Լӣ����ͷ�CPU��һ���������ֵ����û�л���ٽ�������ô�͵ȴ���
*             ���ֻ��һ�����ĵ�CPU����ô���˻�Ϊ���ǳ������ٽ�����
*
*             LINUX�£������ײ�ϵͣ�SPINҲ�ù�MUTEX��(ĳ�̶ֳ����������ΪSPIN��LINUX���ٽ���)
*
*             spin�Ͼ��������٣����Ƿż������³�������ҹ���һ�£�
*             ��ѩ������
*             http://bbs.pediy.com/showthread.php?t=74502
*
*             LINUX SPIN�����ܲ���
*             http://www.parallellabs.com/2010/01/31/pthreads-programming-spin-lock-vs-mutex-performance-analysis/
*
*             �����˵һ�¸Ľ�����ʷ������дSPIN�������棬�ر�����һֱ��ΪWINDOWS ���û�̬��
*             û��SPIN���ԣ�������������������������Ҳ����һ��MSDN�������������װ����Ҳ�ͼ���2
*             �д��룬�Ǻǣ�
*
* @note       SPIN�����ó���Ӧ���Ǿ��������ٽ����ĵط���ͬ������һ�δ��룬һЩ����
*             SPIN�����ó���Ӧ���ǳ�ײ�٣�����Ҫ��ʱ��ȴ��ĵط�����Ҳ����ΪɶSPIN���ṩ��ʱ��
*
*             ����Spin lock��������˵����һ���ö����������������������˶�����ʹ�õĶ�����
*             ����˵��������֪��������ʲô��ΪʲôҪ��spin lock����������Mutex���ٽ����ɡ�
*             �ٸ����ӣ���������Ҫ�ȴ�ĳ�����̵߳���Ϣ����������������ȡ����Mutex��Spin���ʺܶࡣ
*
*/
#ifndef ZCE_LIB_OS_ADAPT_SPIN_H_
#define ZCE_LIB_OS_ADAPT_SPIN_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      SPIN ���ĳ�ʼ��
* @return     int        0��ʶ�ɹ�����0��ʶʧ���Լ�����ԭ��
* @param[in]  lock       SPIN����
* @param[in]  pshared    ����ķ��ʣ��Ƕ��̹߳���PTHREAD_PROCESS_PRIVATE�����Ƕ���̹���
*/
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

/*!
* @brief      SPIN ���ĳ�ʼ����չ�汾���Ǳ�׼����������WIN�¶���̼�ʹ�ã�ʹ��������У������������֣�
* @return     int           0��ʾ�ɹ�����0��ʾʧ���Լ�����ԭ��
* @param[in]  lock          SPIN����
* @param[in]  process_share �Ƿ���Ҫ����̼乲��
* @param[in]  spin_name     ��WIN�£����Ҫ����̼乲��Ҫ����
*/
int pthread_spin_initex(pthread_spinlock_t *lock,
                        bool process_share = false,
                        const char *spin_name = NULL);

/*!
* @brief      SPIN ��������
* @return     int    0��ʾ�ɹ�����0��ʾʧ���Լ�����ԭ��
* @param[in]  lock   SPIN����
*/
int pthread_spin_destroy(pthread_spinlock_t *lock);

/*!
* @brief      SPIN ���ļ���
* @return     int   0��ʾ�ɹ�����0��ʾʧ���Լ�����ԭ��
* @param[in]  lock  SPIN����
*/
int pthread_spin_lock(pthread_spinlock_t *lock);

/*!
* @brief      SPIN ���ĳ��Լ���
* @return     int   0��ʾ�ɹ�����0��ʾʧ���Լ�����ԭ��
* @param[in]  lock  SPIN����
*/
int pthread_spin_trylock(pthread_spinlock_t *lock);

/*!
* @brief      SPIN ���Ľ���
* @return     int   0��ʾ�ɹ�����0��ʾʧ���Լ�����ԭ��
* @param[in]  lock  SPIN����
*/
int pthread_spin_unlock(pthread_spinlock_t *lock);

};

#endif //ZCE_LIB_OS_ADAPT_SPIN_H_

