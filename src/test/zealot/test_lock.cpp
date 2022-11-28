#include "predefine.h"

zce::thread_rw_mutex light_rw;

int g_daomei_foo = 0;

//SRWLOCK srw_lock;
//pthread_rwlock_t prw_lock;

size_t TEST_NUMBER = 10000000;

class Task_Read
{
public:

    Task_Read()
    {
    }
    int svc()
    {
        int abc = 0;
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            light_rw.try_lock_shared();
            abc = g_daomei_foo;
            light_rw.unlock_shared();
        }
        ZCE_UNUSED_ARG(abc);
        return 0;
    }

protected:
    size_t number_prc_ = 0;
};

class Task_Write
{
public:
    Task_Write()
    {
    }

    int svc()
    {
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            light_rw.lock();
            g_daomei_foo++;
            light_rw.unlock();
        }

        return 0;
    }
};

int test_rw_lock1(int /*argc*/, char* /*argv*/[])
{
    //InitializeSRWLock(&srw_lock);

    //计时器，不是定时器呀
    zce::auto_progress_timer<zce::hr_progress_timer> auto_timer;

    Task_Read a1;
    Task_Read a2;

    Task_Write b1;
    Task_Write b2;

    zce::thread_task t_a1, t_a2, t_b1, t_b2;
    t_a1.activate(&Task_Read::svc, &a1);
    t_a2.activate(&Task_Read::svc, &a2);

    t_b1.activate(&Task_Write::svc, &b1);
    t_b2.activate(&Task_Write::svc, &b2);

    t_a1.wait_join();
    t_a2.wait_join();
    t_b1.wait_join();
    t_b2.wait_join();

    printf("At last g_daomei_foo = %d\n", g_daomei_foo);

    return 0;
}

zce::thread_rw_mutex rw_lock;

class Task_Read_1
{
public:

    Task_Read_1()
    {
    }
    int svc()
    {
        int abc = 0;
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            rw_lock.lock_shared();
            abc = g_daomei_foo;
            rw_lock.unlock_shared();
        }
        ZCE_UNUSED_ARG(abc);
        return 0;
    }

protected:
    size_t number_prc_ = 0;
};

class Task_Write_1
{
public:
    Task_Write_1()
    {
    }

    int svc()
    {
        for (size_t i = 0; i < TEST_NUMBER; ++i)
        {
            rw_lock.lock();
            g_daomei_foo++;
            rw_lock.unlock();
        }

        return 0;
    }
};

int test_rw_lock2(int /*argc*/, char* /*argv*/[])
{
    //InitializeSRWLock(&srw_lock);

    //计时器，不是定时器呀
    zce::auto_progress_timer<zce::hr_progress_timer> auto_timer;

    Task_Read_1 a1;
    Task_Read_1 a2;

    Task_Write_1 b1;
    Task_Write_1 b2;

    zce::thread_task t_a1, t_a2, t_b1, t_b2;

    t_a1.activate(&Task_Read_1::svc, &a1);
    t_a2.activate(&Task_Read_1::svc, &a2);

    //t_b1.activate(&Task_Write_1::svc, &b1);
    //t_b2.activate(&Task_Write_1::svc, &b2);

    t_a1.wait_join();
    t_a2.wait_join();
    t_b1.wait_join();
    t_b2.wait_join();

    printf("At last g_daomei_foo = %d\n", g_daomei_foo);

    return 0;
}

int test_bit_lock(int /*argc*/, char* /*argv*/[])
{
    zce::bit_lock bl;
    int ret = bl.open(10240);
    if (ret != 0)
    {
        return ret;
    }
    bool bret = bl.try_lock(1024);
    std::cout << std::boolalpha;
    std::cout << "bret:" << bret << std::endl;
    bret = bl.try_lock(1024);
    std::cout << "bret:" << bret << std::endl;
    bret = bl.unlock(1024);
    std::cout << "bret:" << bret << std::endl;
    return 0;
}