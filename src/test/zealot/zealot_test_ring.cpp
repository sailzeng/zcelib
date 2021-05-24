#include "zealot_predefine.h"
#include "zealot_test_function.h"

int test_dequechunk(int /*argc*/, char* /*argv*/[])
{
    size_t dequesize = 1026;
    size_t szalloc = zce::lockfree::shm_kfifo::getallocsize(dequesize);
    std::cout << "need mem: " << (int)szalloc << std::endl;

    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::lockfree::shm_kfifo* pmmap =
        zce::lockfree::shm_kfifo::initialize(dequesize, 2048, tmproom, false);

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

    zce::lockfree::kfifo_node* pnode = reinterpret_cast<zce::lockfree::kfifo_node*>(&test_abc);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);

    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    zce::lockfree::kfifo_node* pnode1 = NULL;
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    delete pnode1;

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
    pnode = reinterpret_cast<zce::lockfree::kfifo_node*>(&test_def);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    delete pnode1;
    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    delete pnode1;
    //
    pnode1 = zce::lockfree::kfifo_node::new_node(1024);
    pmmap->pop_front(pnode1);
    delete pnode1;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    pnode1 = zce::lockfree::kfifo_node::new_node(1024);
    pmmap->pop_front(pnode1);
    delete pnode1;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free() << std::endl;
    delete pnode1;

    std::cout << "freesize:" << (int)pmmap->free() << std::endl;

    pnode1 = NULL;
    bool bsucc = pmmap->pop_front(pnode1);

    if (bsucc == false)
    {
        std::cout << "pop_frond return false!" << std::endl;
    }

    //pnode1 =NULL;
    //pmmap->pop_frond(pnode1);
    //std::cout<<"pnode1 sz:"<<(int)pnode1->sizeofnode<<std::endl;
    //std::cout<<"free:"<<(int)pmmap->free()<<std::endl;
    //delete pnode1;

    return 0;
}

int test_fifo_cycbuf1(int /*argc*/, char* /*argv*/[])
{
    zce::fifo_cycbuf_u32 a1;

    const size_t NODE_MAX_LEN = 1024;
    const size_t CYCBUF_LEN = 4 * 1024;
    a1.initialize(CYCBUF_LEN, NODE_MAX_LEN);

    zce::mt19937_var_gen random;
    random.srand((uint32_t)time(NULL));

    for (size_t i = 0;; ++i)
    {
        size_t node_len = random.uniform_uint32(zce::fifo_cycbuf_u32::MIN_SIZE_DEQUE_CHUNK_NODE, NODE_MAX_LEN);
        auto ptr1 = zce::fifo_cycbuf_u32::node::new_node(node_len);
        bool ok = a1.push_end(ptr1);
        zce::fifo_cycbuf_u32::node::delete_node(ptr1);
        if (ok)
        {
            ZPP_LOG(RS_DEBUG, "push_end success,no={} node len ={} ring free={}", i, node_len, a1.free());
        }
        else
        {
            ZPP_LOG(RS_DEBUG, "push_end fail,no={} node len ={}", i, node_len);
            break;
        }
    }

    auto ptr2 = zce::fifo_cycbuf_u32::node::new_node(NODE_MAX_LEN);
    for (size_t i = 0;; ++i)
    {
        bool ok = a1.pop_front(ptr2);
        if (ok)
        {
            size_t node_len = ptr2->size_of_node_;
            ZPP_LOG(RS_DEBUG, "pop_front success,no={} node len ={} ring free={}", i, node_len, a1.free());
        }
        else
        {
            ZPP_LOG(RS_DEBUG, "push_end fail,no={} ring free ={}", i, a1.free());
            break;
        }
    }

    auto ptr_2 = zce::fifo_cycbuf_u32::node::new_node(NODE_MAX_LEN);
    for (size_t i = 0; i < 1024 * 128; ++i)
    {
        size_t node_len = random.uniform_uint32(zce::fifo_cycbuf_u32::MIN_SIZE_DEQUE_CHUNK_NODE, NODE_MAX_LEN);
        size_t push_num = random.uniform_uint32(2, 6);
        for (size_t j = 0; j < push_num; ++j)
        {
            auto ptr_1 = zce::fifo_cycbuf_u32::node::new_node(node_len);
            bool ok = a1.push_end(ptr_1);
            zce::fifo_cycbuf_u32::node::delete_node(ptr_1);
            ZPP_LOG(RS_DEBUG, "push_end {},no={}{} ring free ={}", ok, i, j, a1.free());
        }

        size_t pop_num = random.uniform_uint32(2, 6);
        for (size_t k = 0; k < pop_num; ++k)
        {
            bool ok = a1.pop_front(ptr_2);
            node_len = ok ? 0 : ptr_2->size_of_node_;
            ZPP_LOG(RS_DEBUG, "pop_front {},no={}{} ring free ={}", ok, i, k, a1.free());
        }
    }
    return 0;
}

int test_lockfree_ring(int /*argc*/, char* /*argv*/[])
{
    zce::lockfree::rings<int> a1(1024);
    a1.push_back(new int(1));
    a1.push_back(new int(2));

    return 0;
}