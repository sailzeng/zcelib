#include "zealot_predefine.h"


static const size_t TEST_TIMER_NUMBER = 10;

int TEST_TIMER_ACT [10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

class Test_Timer_Handler : public ZCE_Timer_Handler
{
public:
    virtual int timer_timeout(const ZCE_Time_Value &now_timenow_time,
                              const void *act)
    {
        char time_str[128];
        int timer_action = *(int *)act;
        std::cout << now_timenow_time.timestamp(time_str, 128) << " " << "Timer action =" << timer_action << std::endl;
        ZCE_Timer_Queue_Base::instance()->cancel_timer(this);

        ZCE_Time_Value delay_time(1, 0);
        ZCE_Time_Value interval_time(0, 0);

        int time_id = ZCE_Timer_Queue_Base::instance()->schedule_timer(this,
                                                                       &TEST_TIMER_ACT[timer_action - 1],
                                                                       delay_time,
                                                                       interval_time);
        std::cout << now_timenow_time.timestamp(time_str, 128) << " " << "Timer id =" << time_id << std::endl;

        return 0;
    }

};

int test_timer_expire(int /*argc*/, char * /*argv*/ [])
{
    ZCE_Timer_Queue_Base::instance(new ZCE_Timer_Wheel(1024));

    Test_Timer_Handler test_timer[10];
    int timer_id[10];
    ZCE_Time_Value delay_time(1, 0);
    ZCE_Time_Value interval_time(12, 0);
    for (size_t i = 0; i < TEST_TIMER_NUMBER; ++i)
    {
        delay_time.sec(i);
        timer_id[i] = ZCE_Timer_Queue_Base::instance()->schedule_timer(&test_timer[i],
                                                                       &TEST_TIMER_ACT[i],
                                                                       delay_time,
                                                                       interval_time);
    }

    for (size_t j = 0; j < 100000; j++)
    {
        ZCE_Timer_Queue_Base::instance()->expire();
        zce::usleep(100000);
    }
    ZCE_UNUSED_ARG(timer_id);
    return 0;
}

//���ڲ���ĳЩ��������Ĵ��롣
int test_timer_expire2(int /*argc*/, char * /*argv*/ [])
{
    ZCE_Timer_Queue_Base::instance(new ZCE_Timer_Wheel(1024));

    Test_Timer_Handler test_timer[10];
    int timer_id[10];
    ZCE_Time_Value delay_time(1, 0);
    ZCE_Time_Value interval_time(1, 0);
    for (size_t i = 0; i < TEST_TIMER_NUMBER; ++i)
    {
        delay_time.sec(i);
        timer_id[i] = ZCE_Timer_Queue_Base::instance()->schedule_timer(&test_timer[i],
                                                                       &TEST_TIMER_ACT[i],
                                                                       delay_time,
                                                                       interval_time);
    }

    //һЩ��������£���ʱ���ܳ�ʱ���޷����������µ�����
    for (size_t j = 0; j < 100000; j++)
    {
        zce::sleep(60);
        ZCE_Timer_Queue_Base::instance()->expire();

    }
    ZCE_UNUSED_ARG(timer_id);
    return 0;
}

int test_os_time(int /*argc*/, char * /*argv*/[])
{
    int tz = zce::gettimezone();
    std::cout << "Time zone :" << tz << std::endl;
    tz = zce::gettimezone();
    std::cout << "Time zone :" << tz << std::endl;
    return 0;
}


