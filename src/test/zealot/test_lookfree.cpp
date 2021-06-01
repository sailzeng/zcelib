#include "predefine.h"

int test_dequechunk(int /*argc*/, char* /*argv*/[])
{
    typedef zce::lockfree::shm_kfifo<uint32_t> kfifo_32_t;
    size_t dequesize = 1026;
    size_t szalloc = kfifo_32_t::getallocsize(dequesize);
    std::cout << "need mem: " << (int)szalloc << std::endl;

    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    kfifo_32_t* pmmap =
        kfifo_32_t::initialize(dequesize, 2048, tmproom, false);

    if (pmmap->empty())
    {
        std::cout << "empty" << std::endl;
    }

    struct abc
    {
        unsigned int sz;
        char   data[252];
    } test_abc;

    test_abc.sz = 256;
    memset(test_abc.data, '1', 251);
    test_abc.data[250] = 'A';
    test_abc.data[251] = 0;

    auto* pnode = reinterpret_cast<kfifo_32_t::node*>(&test_abc);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);

    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    kfifo_32_t::node* pnode1 = kfifo_32_t::node::new_node(1024);
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    struct def
    {
        unsigned int sz;
        char   data[4];
    } test_def;
    test_def.sz = 8;
    memset(test_def.data, '2', 2);
    test_def.data[2] = 'B';
    test_def.data[3] = 0;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pnode = reinterpret_cast<kfifo_32_t::node*>(&test_def);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    pmmap->pop_front(pnode1);
    pmmap->pop_front(pnode1);
    pmmap->pop_front(pnode1);

    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pnode1 = kfifo_32_t::node::new_node(1024);
    pmmap->pop_front(pnode1);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    bool bsucc = pmmap->pop_front(pnode1);
    if (bsucc == false)
    {
        std::cout << "pop_frond return false!" << std::endl;
    }

    kfifo_32_t::node::delete_node(pnode1);
    //pnode1 =NULL;
    //pmmap->pop_frond(pnode1);
    //std::cout<<"pnode1 sz:"<<(int)pnode1->sizeofnode<<std::endl;
    //std::cout<<"free:"<<(int)pmmap->free()<<std::endl;
    //delete pnode1;

    return 0;
}

zce::lockfree::rings_ptr<int> g_ring_ptr(10240);
std::mutex mtx;

int thread_do_push(int push_start, size_t push_num)
{
    for (size_t i = 0; i < push_num;)
    {
        int *data = new int(push_start + push_num + i);
        auto ret = g_ring_ptr.push_back(data);
        if (!ret)
        {
            mtx.lock();
            ZPP_LOG(RS_INFO, "push_back data = {} fail.", *data);
            mtx.unlock();
            continue;
        }
        else
        {
            i++;
        }
    }
    return 0;
}

int thread_do_pop(size_t pop_num)
{
    for (size_t i = 0; i < pop_num;)
    {
        int *data = nullptr;
        auto ret = g_ring_ptr.pop_front(data);
        if (ret)
        {
            mtx.lock();
            ZPP_LOG(RS_INFO, "pop_front data = {}", *data);
            mtx.unlock();
            delete data;
            i++;
        }
        else
        {
            continue;
        }
    }
    return 0;
}

int test_lockfree_ring_ptr1(int /*argc*/, char* /*argv*/[])
{
    int *data1 = new int(1024);
    auto ret = g_ring_ptr.push_back(data1);
    if (ret)
    {
        ZPP_LOG(RS_INFO, "push_back data = {} succ.", *data1);
    }
    else
    {
        ZPP_LOG(RS_INFO, "push_back data = {} fail.", *data1);
    }
    int *data2 = nullptr;
    ret = g_ring_ptr.pop_front(data2);
    if (ret)
    {
        ZPP_LOG(RS_INFO, "pop_front data = {}", *data2);
    }
    else
    {
        ZPP_LOG(RS_INFO, "pop_front data fail ");
    }
    return 0;
}

int test_lockfree_ring_ptr2(int /*argc*/, char* /*argv*/[])
{
    std::thread push_t1(thread_do_push, 1, 1024);
    std::thread pop_t1(thread_do_pop, 1024);
    std::thread push_t2(thread_do_push, 1024, 1024);
    std::thread pop_t2(thread_do_pop, 1024);
    std::thread push_t3(thread_do_push, 2048, 1024);
    std::thread pop_t3(thread_do_pop, 1024);

    push_t1.join();
    pop_t1.join();
    push_t2.join();
    push_t3.join();
    pop_t2.join();
    pop_t3.join();

    ZPP_LOG(RS_INFO, "g_ring capacity{} size{} ",
            g_ring_ptr.capacity(),
            g_ring_ptr.size()
    );

    return 0;
}

size_t MAX_RINGS_SIZE = 1024;
zce::lockfree::rings<int> g_ring(MAX_RINGS_SIZE);

int test_lockfree_ring1(int /*argc*/, char* /*argv*/[])
{
    bool ret = false;
    int data = 0;
    for (size_t i = 0; i < MAX_RINGS_SIZE; i++)
    {
        data = 0 + i;
        ret = g_ring.push_back(data);
        if (ret)
        {
            ZPP_LOG(RS_INFO, "push_back data = {} succ.", data);
        }
        else
        {
            ZPP_LOG(RS_INFO, "push_back data = {} fail.", data);
        }
    }
    ZPP_LOG(RS_INFO, "g_ring capacity{} size{} ",
            g_ring.capacity(),
            g_ring.size());

    data = 2048;
    ret = g_ring.push_back(data);
    if (ret)
    {
        ZPP_LOG(RS_INFO, "push_back data = {} succ.", data);
    }
    else
    {
        ZPP_LOG(RS_INFO, "push_back data = {} fail.", data);
    }

    for (size_t i = 0; i < MAX_RINGS_SIZE; i++)
    {
        data = 0 + i;
        ret = g_ring.pop_front(data);
        if (ret)
        {
            ZPP_LOG(RS_INFO, "pop_front data = {} succ.", data);
        }
        else
        {
            ZPP_LOG(RS_INFO, "pop_front data = {} fail.");
        }
    }
    ret = g_ring.pop_front(data);
    if (ret)
    {
        ZPP_LOG(RS_INFO, "push_back data = {} succ.", data);
    }
    else
    {
        ZPP_LOG(RS_INFO, "push_back data fail.");
    }
    return 0;
}

int thread_rings_push(int push_start, size_t push_num)
{
    for (size_t i = 0; i < push_num;)
    {
        int data = (push_start + push_num + i);
        auto ret = g_ring.push_back(data);
        if (!ret)
        {
            mtx.lock();
            ZPP_LOG(RS_INFO, "push_back data = {} fail.", data);
            mtx.unlock();
            continue;
        }
        else
        {
            i++;
        }
    }
    return 0;
}

int thread_rings_pop(size_t pop_num)
{
    for (size_t i = 0; i < pop_num; )
    {
        int data = 0;
        auto ret = g_ring.pop_front(data);
        if (ret)
        {
            mtx.lock();
            ZPP_LOG(RS_INFO, "pop_front data = {}", data);
            mtx.unlock();
            i++;
        }
        else
        {
            continue;
        }
    }
    return 0;
}

int test_lockfree_ring2(int /*argc*/, char* /*argv*/[])
{
    bool ret = false;
    int data = 0;
    for (size_t i = 0; i < MAX_RINGS_SIZE; i++)
    {
        data = 0 + i;
        ret = g_ring.push_back(data);
        if (!ret)
        {
            ZPP_LOG(RS_INFO, "push_back data = {} fail.", data);
        }
    }
    ZPP_LOG(RS_INFO, "g_ring capacity{} size{} ",
            g_ring.capacity(),
            g_ring.size());

    std::thread push_t1(thread_rings_push, 1, 1024);
    std::thread pop_t1(thread_rings_pop, 1024);
    std::thread push_t2(thread_rings_push, 1024, 1024);
    std::thread pop_t2(thread_rings_pop, 1024);
    std::thread push_t3(thread_rings_push, 2048, 1024);
    std::thread pop_t3(thread_rings_pop, 2048);

    push_t1.join();
    pop_t1.join();
    push_t2.join();
    push_t3.join();
    pop_t2.join();
    pop_t3.join();

    ZPP_LOG(RS_INFO, "g_ring capacity{} size{} ",
            g_ring.capacity(),
            g_ring.size()
    );

    return 0;
}