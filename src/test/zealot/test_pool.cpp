#include "predefine.h"



int test_pool(int /*argc*/, char* /*argv*/[])
{
    zce::queue_buffer_pool pool;
    size_t BUCKET_NUM = 6;
    size_t BUCKET_SIZE_ARY[] = { 128,256,512,768,1024,1536 };
    pool.initialize(BUCKET_NUM, BUCKET_SIZE_ARY,
                    &zce::queue_buffer::new_self,
                    128, 512);
    zce::queue_buffer* buf = nullptr;
    bool ret = pool.alloc_buffer(64, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer ret ", ret);
        return -1;
    }
    ZPP_LOG(RS_DEBUG, "alloc_buffer capacity {}", buf->capacity());
    pool.free_buffer(buf);

    ret = pool.alloc_buffer(309, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer ret ", ret);
        return -1;
    }
    ZPP_LOG(RS_DEBUG, "alloc_buffer capacity {}", buf->capacity());
    pool.free_buffer(buf);

    ret = pool.alloc_buffer(602, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer 602 ret ", ret);
    }

    pool.free_buffer(buf);

    return 0;
}

class CA
{

};

class CB
{

};

class CC
{

};




int test_multiobj_pool(int /*argc*/, char* /*argv*/[])
{
    zce::multiobjs_pool<CA, CB, CC, int, double> m_o_1;
    m_o_1.initialize<0>(10, 10);
    m_o_1.initialize<1>(10, 10);
    m_o_1.initialize<2>(10, 10);
    m_o_1.initialize<3>(10, 10);
    m_o_1.initialize<4>(10, 10);
    auto sz = m_o_1.size<0>();

    auto a_1_1 = m_o_1.alloc_object<0>();
    auto a_1_2 = m_o_1.alloc_object<0>();
    m_o_1.free_object<0>(a_1_1);
    m_o_1.free_object<0>(a_1_2);

    m_o_1.terminate<0>();
    m_o_1.terminate<1>();
    m_o_1.terminate<2>();
    m_o_1.terminate<3>();
    m_o_1.terminate<4>();

    zce::multiobjs_pool<CA, CB, CC, int, double> m_o_2;
    m_o_2.initialize<CA>(10, 10);
    m_o_2.initialize<CB>(10, 10);
    m_o_2.initialize<CC>(10, 10);
    m_o_2.initialize<int>(10, 10);
    m_o_2.initialize<double>(10, 10);

    auto a_2_1 = m_o_2.alloc_object<CA>();
    auto a_2_2 = m_o_2.alloc_object<CA>();
    m_o_2.free_object<CA>(a_2_1);
    m_o_2.free_object<CA>(a_2_2);

    m_o_2.terminate<CA>();
    m_o_2.terminate<CB>();
    m_o_2.terminate<CC>();
    m_o_2.terminate<int>();
    m_o_2.terminate<double>();

    return 0;
}
