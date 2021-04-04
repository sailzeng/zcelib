/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_process_semaphore.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��15��
* @brief      ���̼�ͬ���õ��źŵƣ�
*
*/
#ifndef ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_
#define ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"

/*!
* @brief      ���̵��źŵ���
*             LINUX���������źŵƣ�
*             WINDOWS���ô����Ƶ��źŵƣ�
*/
class ZCE_Process_Semaphore  : public ZCE_Lock_Base
{

public:

    ///�߳�����GUARD
    typedef ZCE_Lock_Guard<ZCE_Process_Semaphore> LOCK_GUARD;

public:

    /*!
    * @brief      ���캯��,Ĭ�ϴ��������źŵƣ�
    * @param      init_value  �źŵƳ�ʼ����ֵ
    * @param      sem_name    �źŵƵ�����
    */
    ZCE_Process_Semaphore (unsigned int init_value,
                           const char *sem_name
                          );
    virtual ~ZCE_Process_Semaphore (void);

    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    //����,
    virtual void unlock();

    /*!
    * @brief      ����ʱ�䳬ʱ�ĵ���������ʱ�����
    * @return     bool      true���������false���޷������
    * @param      abs_time  ��ʱ�ľ���ʱ��
    */
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      ���ʱ��ĳ�ʱ��������ʱ�󣬽���
    * @return     bool           true���������false���޷������
    * @param      relative_time  ��ʱ�ľ���ʱ��
    */
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

protected:

    ///�źŵƶ���
    sem_t            *lock_;

    ///�źŵƵ����ƣ����ò���¼�����������ǹ����ڴ���ļ�����
    char              sema_name_[PATH_MAX + 1];

};

#endif //ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_

