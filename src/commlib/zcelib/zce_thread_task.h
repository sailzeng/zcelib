/**
* @copyright 2004-2012  Apache License, Version 2.0 FULLSAIL
* @filename  zce_thread_task.h
* @author    Sailzeng <sailerzeng@gmail.com>
* @version
* @date      2011��6��18��
* @brief     ��ƽ̨���̶߳����װ��Ϊ�˷�װ�ĵķ��㣬���ʵ����һ���߳�һ������
*            �߳̿����ø�һ��GROUP ID�ٴΰ�װһ��
*
* @details   �ڲ���װ�õ����Լ�OS���pthread_XXX�ĺ�����һ������join��detach
*            �Ҳο���ACE��ʵ�֣�����ʵ�ڲ���ȷACEΪɶҪʵ����һ��TASK����������
             ����߳���ɶ�ô�������Ϊ�����������д�Ļ�ɬ�˺ܶ�
*
*
* @note
*
*/

#ifndef ZCE_LIB_THREAD_TASK_H_
#define ZCE_LIB_THREAD_TASK_H_

#include "zce_boost_non_copyable.h"

/*!
* @brief      ���Լ���װ��pthread����������TASK���ͣ�ÿ���߳�һ������
*
* @note       ���󲻿ɿ������ƣ�
*/
class ZCE_Thread_Task : public ZCE_NON_Copyable
{

public:

    ///���캯��
    ZCE_Thread_Task();
    ///��������
    virtual ~ZCE_Thread_Task();

public:

    /*!
    * @brief      ����һ���̣߳�������߳̿�ʼ����
    * @return     int ==0��ʶ�ɹ�����0ʧ��
    * @param[in]  group_id �̹߳��������Զ���ͬGROUP ID���߳̽���һЩ����
    * @param[out] threadid ���ص��߳�ID��
    * @param[in]  detachstate ����������̻߳���JOIN���߳� PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE
    * @param[in]  stacksize ��ջ��С Ϊ0��ʾĬ�ϣ��������Ҫʹ�úܶ��̣߳�����������С��WIN һ����1M��LINUXһ����10M(8M)
    * @param[in]  threadpriority ���ȼ���Ϊ0��ʾĬ��
    * @note
    */
    int activate(int group_id,
                 ZCE_THREAD_ID *threadid,
                 int detachstate = PTHREAD_CREATE_JOINABLE,
                 size_t stacksize = 0,
                 int threadpriority = 0);

    ///�߳̽�����ķ���ֵint ����
    int thread_return();

    ///�õ�group id
    int group_id() const;

    ///�õ�����̶߳���������߳�ID
    ZCE_THREAD_ID thread_id() const;

    ///���룬���ٽ��а�
    int detach();

    ///�ȴ��߳��˳���join
    int wait_join();

    ///�߳��ó�CPU��ʱ��
    int yield();

protected:

    ///��Ҫ�̳еĴ���ĺ���,������������һ��������OK
    virtual int svc (void);

protected:

    ///��̬������Ҳ����Ҫִ�еĺ������������svc
    static void svc_run (void *args);

public:

    ///��Ч����ID
    static const int INVALID_GROUP_ID = -1;

protected:

    ///�̵߳�GROUP ID,
    int                     group_id_;

    ///�̵߳�ID
    ZCE_THREAD_ID           thread_id_;

    ///�̵߳ķ���ֵ
    int                     thread_return_;

};

#endif //#ifndef ZCE_LIB_THREAD_TASK_H_

