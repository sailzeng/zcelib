#include "predefine.h"

int test_nonr_thread_mutex(int, char*[])
{
    zce::thread_nonr_mutex  abc;
    abc.lock();
    int last_error = zce::last_error();
    abc.lock();
    last_error = zce::last_error();

    ZCE_UNUSED_ARG(last_error);
    return 0;
}

//zce::MsgQueue_Deque<ZCE_MT_SYNCH,int>   message_queue_(100);
//zce::MsgList_Condi <int >  message_queue_(100);
zce::msglist_sema <int >  message_queue_(100);
//ZCE_Msgqueue_List_Condi <int >  message_queue_(100);

zce::thread_light_mutex  io_mutex;

class Task_Producer : public zce::thread_task
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
            int data = static_cast<int>(i);
            message_queue_.enqueue(data);

            {
                zce::lock_guard<zce::thread_light_mutex>  lock(io_mutex);
                std::cout << "queue size:" << message_queue_.size() << " input data " << i << std::endl;
            }
        }

        return 0;
    }

protected:
    size_t number_prc_;
};

class Task_Consumer : public zce::thread_task
{
public:
    Task_Consumer()
    {
        number_prc_ = 1000;
    }

    virtual int svc()
    {
        int x = 0;
        zce::sleep(1);

        for (size_t i = 0; i < number_prc_; ++i)
        {
            message_queue_.dequeue(x);

            {
                zce::lock_guard<zce::thread_light_mutex> lock(io_mutex);
                std::cout << "queue size:" << message_queue_.size() << " output data " << x << std::endl;
            }
        }

        return 0;
    }

protected:
    size_t number_prc_;
};

int test_msgqueue_condi(int /*argc*/, char* /*argv*/[])
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
//            lock_guard<ZCE_Thread_Mutex>  lock(io_mutex);
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
//            lock_guard<ZCE_Thread_Mutex> lock(io_mutex);
//            std::cout<<"queue size:"<< message_queue_.size() <<" output data "<<x<<std::endl;
//        }
//    }
//
//}
//