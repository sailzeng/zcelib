#ifndef ZCE_LIB_THREAD_WAIT_MANAGER_H_
#define ZCE_LIB_THREAD_WAIT_MANAGER_H_

#include "zce_boost_non_copyable.h"



/*!
* @brief      �̵߳ĵȴ�������
*
* @note       ���������߳�ZCE_Thread_Base �ڲ�����ģ�����Ƕ����࣬�����ô�����static ������
*             ҲӰ��ZCE_Thread_Base������
*/
class ZCE_Thread_Wait_Manager : public ZCE_NON_Copyable
{
protected:

    //��¼��Ҫ�ȴ����߳���Ϣ��ͨ������ʹ��
    struct MANAGE_WAIT_INFO
    {
    public:

        MANAGE_WAIT_INFO(ZCE_THREAD_ID wait_thr_id, int wait_group_id):
            wait_thr_id_(wait_thr_id),
            wait_group_id_(wait_group_id)
        {
        }
        ~MANAGE_WAIT_INFO()
        {
        }
    public:
        //�̵߳�ID
        ZCE_THREAD_ID     wait_thr_id_;
        //����ID
        int               wait_group_id_;
    };

protected:
    //��list�������ܲ����ر�ã������ǵ�Ҫ�м�ɾ�����صȵȣ�����
    typedef std::list <MANAGE_WAIT_INFO>   MANAGE_WAIT_THREAD_LIST;

    //����ʵ��
    static ZCE_Thread_Wait_Manager *instance_;

protected:

    //����ϣ���ȴ����̼߳�¼
    MANAGE_WAIT_THREAD_LIST   wait_thread_list_;

public:
    //���캯����������ӵ��ʵ�������Ƽ����õ�������
    ZCE_Thread_Wait_Manager();
    ~ZCE_Thread_Wait_Manager();

    //�����Ҫ������Ҫ�Լ��Ǽǣ�
    void record_wait_thread(ZCE_THREAD_ID wait_thr_id, int wait_group_id = 0 );
    //�Ǽ�һ��Ҫ���еȴ�����ȴ��߳�
    void record_wait_thread(const ZCE_Thread_Task *wait_thr_task);

    //�����е��߳��˳�
    void wait_all();

    //
    void wait_group(int group_id);

public:

    //���Ӻ���
    static ZCE_Thread_Wait_Manager *instance();
    //�����ӵĺ���
    static void clean_instance();
};

#endif //#ifndef ZCE_LIB_THREAD_WAIT_MANAGER_H_

