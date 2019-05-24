#include "zealot_predefine.h"

ZCE_Thread_Win_RW_Mutex light_rw;

int g_daomei_foo = 0;

//SRWLOCK srw_lock;
//pthread_rwlock_t prw_lock;

size_t TEST_NUMBER = 10000000;

class Task_Read : public ZCE_Thread_Task
{
public:

    Task_Read()
    {
    }
    virtual int svc()
    {
        int abc = 0;
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            light_rw.lock_read();
            abc = g_daomei_foo;
            light_rw.unlock_read();
        }
        ZCE_UNUSED_ARG(abc);
        return 0;
    }

protected:
    size_t number_prc_ =0;
};


class Task_Write : public ZCE_Thread_Task
{
public:
    Task_Write()
    {
    }

    virtual int svc()
    {
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            light_rw.lock_write();
            g_daomei_foo++;
            light_rw.unlock_write();

        }

        return 0;
    }

};

int test_rw_lock1 (int /*argc*/, char * /*argv*/ [])
{
    //InitializeSRWLock(&srw_lock);

    //计时器，不是定时器呀
    ZCE_Auto_Progress_Timer<ZCE_HR_Progress_Timer> auto_timer;

    Task_Read a1;
    Task_Read a2;

    Task_Write b1;
    Task_Write b2;

    ZCE_THREAD_ID threadid_a1, threadid_a2, threadid_b1, threadid_b2;

    a1.activate(1, &threadid_a1);
    a2.activate(1, &threadid_a2);

    b1.activate(2, &threadid_b1);
    b2.activate(2, &threadid_b2);

    a1.wait_join();
    a2.wait_join();
    b1.wait_join();
    b2.wait_join();

    printf("At last g_daomei_foo = %d\n", g_daomei_foo);

    return 0;
}


ZCE_Thread_RW_Mutex rw_lock;

class Task_Read_1 : public ZCE_Thread_Task
{
public:

    Task_Read_1()
    {
    }
    virtual int svc()
    {
        int abc = 0;
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            rw_lock.lock_read();
            abc = g_daomei_foo;
            rw_lock.unlock_read();
        }
        ZCE_UNUSED_ARG(abc);
        return 0;
    }

protected:
    size_t number_prc_ = 0;
};


class Task_Write_1 : public ZCE_Thread_Task
{
public:
    Task_Write_1()
    {
    }

    virtual int svc()
    {
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            rw_lock.lock_write();
            g_daomei_foo++;
            rw_lock.unlock_write();

        }

        return 0;
    }

};

int test_rw_lock2 (int /*argc*/, char * /*argv*/ [])
{
    //InitializeSRWLock(&srw_lock);

    //计时器，不是定时器呀
    ZCE_Auto_Progress_Timer<ZCE_HR_Progress_Timer> auto_timer;

    Task_Read_1 a1;
    Task_Read_1 a2;

    Task_Write_1 b1;
    Task_Write_1 b2;

    ZCE_THREAD_ID threadid_a1, threadid_a2, threadid_b1, threadid_b2;

    a1.activate(1, &threadid_a1);
    a2.activate(1, &threadid_a2);

    b1.activate(2, &threadid_b1);
    b2.activate(2, &threadid_b2);

    a1.wait_join();
    a2.wait_join();
    b1.wait_join();
    b2.wait_join();

    printf("At last g_daomei_foo = %d\n", g_daomei_foo);

    return 0;
}