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

zce::msgring_condi<std::any> requst_queue_1(1024);
zce::msgring_condi<std::shared_ptr<void>> requst_queue_2(1024);

//! 对象池子，用于分配对象
zce::multishare_pool<std::mutex,
    zce::aio::FS_ATOM,
    zce::aio::DIR_ATOM,
    zce::aio::MYSQL_ATOM,
    zce::aio::HOST_ATOM,
    zce::aio::SOCKET_TIMEOUT_ATOM,
    zce::aio::EVENT_ATOM,
    zce::aio::TIMER_ATOM > aio_share_pool;

int test_msgring_condi(int /*argc*/, char* /*argv*/[])
{
    aio_share_pool.initialize<0>(10, 10);
    aio_share_pool.initialize<1>(10, 10);
    aio_share_pool.initialize<2>(10, 10);
    aio_share_pool.initialize<3>(10, 10);
    aio_share_pool.initialize<4>(10, 10);
    aio_share_pool.initialize<5>(10, 10);

    auto fs_1_1 = aio_share_pool.alloc_share<0>();
    auto fs_1_2 = aio_share_pool.alloc_share<0>();
    std::cout << "Type info " << typeid(fs_1_1).name() << std::endl;
    std::shared_ptr<void> ptr_1_1 = fs_1_1;
    requst_queue_2.enqueue(std::move(ptr_1_1));

    std::cout << "FS:" << ptr_1_1.use_count() << " VOID:" << ptr_1_1.use_count() << std::endl;

    //void * ptr = ptr_1_1.get();
    //不能转化回来的。
    //std::shared_ptr<zce::aio::FS_ATOM> fs_1_3 = ptr_1_1;

    return 0;
}