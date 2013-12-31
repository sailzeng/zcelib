


#include "zealot_predefine.h"


int test_nonr_thread_mutex(int, char * [])
{
    ZCE_Thread_NONR_Mutex  abc;
    abc.lock();
    int last_error = ZCE_OS::last_error();
    abc.lock();
    last_error = ZCE_OS::last_error();
    return 0;
}



//ZCE_Message_Queue_Deque<ZCE_MT_SYNCH,int>   message_queue_(100);
ZCE_Msgqueue_List_Condi <int >  message_queue_(100);
//ZCE_Msgqueue_List_Condi <int >  message_queue_(100);

ZCE_Thread_Light_Mutex  io_mutex;

class Task_Producer : public ZCE_Thread_Task
{
public:
    Task_Producer()
    {
        number_prc_ = 2000;
    }
    virtual int svc()
    {
        for (size_t i = 0; i < number_prc_; ++i)
        {
            message_queue_.enqueue((const int)i);

            {
                ZCE_Lock_Guard<ZCE_Thread_Light_Mutex>  lock(io_mutex);
                std::cout << "queue size:" << message_queue_.size() << " input data " << i << std::endl;
            }
        }

        return 0;
    }

protected:
    size_t number_prc_;
};


class Task_Consumer : public ZCE_Thread_Task
{
public:
    Task_Consumer()
    {
        number_prc_ = 1000;
    }

    virtual int svc()
    {
        int x;
        ZCE_OS::sleep(1);

        for (size_t i = 0; i < number_prc_; ++i)
        {
            message_queue_.dequeue(x);

            {
                ZCE_Lock_Guard<ZCE_Thread_Light_Mutex> lock(io_mutex);
                std::cout << "queue size:" << message_queue_.size() << " output data " << x << std::endl;
            }
        }

        return 0;
    }

protected:
    size_t number_prc_;
};

int test_msgqueue_condi (int /*argc*/, char * /*argv*/ [])
{
    Task_Producer a1;

    Task_Consumer b1;
    Task_Consumer b2;

    ZCE_THREAD_ID threadid_a1, threadid_b1, threadid_b2;

    a1.activate(1, &threadid_a1);

    b1.activate(2, &threadid_b1);
    b2.activate(2, &threadid_b2);

    a1.wait_join();
    b1.wait_join();
    b2.wait_join();

    return 0;
}
























//void producer(size_t number_prc)
//{
//    for(size_t i=0;i<number_prc;++i)
//    {
//        message_queue_.enqueue(i);
//
//        {
//            ZCE_Lock_Guard<ZCE_Thread_Mutex>  lock(io_mutex);
//            std::cout<<"queue size:"<< message_queue_.size() <<" input data "<<i<<std::endl;
//        }
//    }
//
//}
//
//void consumer(size_t number_prc)
//{
//    int x;
//    ::Sleep(1000);
//    for(size_t i=0;i<number_prc;++i)
//    {
//        message_queue_.dequeue(x);
//
//        {
//            ZCE_Lock_Guard<ZCE_Thread_Mutex> lock(io_mutex);
//            std::cout<<"queue size:"<< message_queue_.size() <<" output data "<<x<<std::endl;
//        }
//    }
//
//}
//
