#include "predefine.h"

int test_pool(int /*argc*/, char* /*argv*/[])
{
    zce::queue_buffer_pool pool;

    size_t BUCKET_NUM = 6;
    size_t BUCKET_SIZE_ARY[] = { 128,256,512,768,1024,1536 };
    pool.initialize(BUCKET_NUM,
                    BUCKET_SIZE_ARY,
                    128,
                    512);
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

class CI
{
};

int test_pool6(int /*argc*/, char* /*argv*/[])
{
    zce::queue_buffer_pool pool;

    size_t BUCKET_NUM = 6;
    size_t BUCKET_SIZE_ARY[] = { 128,256,512,768,1024,1536 };
    pool.initialize(BUCKET_NUM,
                    BUCKET_SIZE_ARY,
                    128,
                    512);
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

/*
int test_pool2(int argc, char* argv[])
{
zce::queue_buffer_pool_lock pool;

size_t BUCKET_NUM = 6;
size_t BUCKET_SIZE_ARY[] = { 128,256,512,768,1024,1536 };
pool.initialize(BUCKET_NUM,
                BUCKET_SIZE_ARY,
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
*/
class CA
{
};

class CB
{
};

class CC
{
    int c_1_;
    int c_2_;
    int c_3_;
    int c_4_;
};

int test_multiobj_pool(int /*argc*/, char* /*argv*/[])
{
    zce::multidata_pool<zce::null_lock, CA, CB, CC, int, double> m_o_1;
    m_o_1.initialize<0>(10, 10);
    m_o_1.initialize<1>(10, 10);
    m_o_1.initialize<2>(10, 10);
    m_o_1.initialize<3>(10, 10);
    m_o_1.initialize<4>(10, 10);
    auto sz = m_o_1.size<0>();
    ZCE_LOG(RS_INFO, "m_o_1 %u", sz);

    auto a_1_1 = m_o_1.alloc_object<0>();
    auto a_1_2 = m_o_1.alloc_object<0>();
    m_o_1.free_object<0>(a_1_1);
    m_o_1.free_object<0>(a_1_2);

    m_o_1.terminate<0>();
    m_o_1.terminate<1>();
    m_o_1.terminate<2>();
    m_o_1.terminate<3>();
    m_o_1.terminate<4>();

    zce::multidata_pool<std::mutex, CA, CB, CC, int, double> m_o_2;
    m_o_2.initialize<CA>(10, 10);
    m_o_2.initialize<CB>(10, 10);
    m_o_2.initialize<CC>(10, 10);
    m_o_2.initialize<int>(10, 10);
    m_o_2.initialize<double>(10, 10);

    auto a_2_1 = m_o_2.alloc_object<CA>();
    auto a_2_2 = m_o_2.alloc_object<CA>();
    m_o_2.free_object<CA>(a_2_1);
    m_o_2.free_object<CA>(a_2_2);

    CC* c_ary[20];
    for (size_t i = 0; i < 20; ++i)
    {
        c_ary[i] = m_o_2.alloc_object<CC>();
        ZCE_LOG(RS_INFO, "CC pool size %u", m_o_2.size<CC>());
        ZCE_LOG(RS_INFO, "CC pool capacity %u", m_o_2.capacity<CC>());
    }
    for (size_t i = 0; i < 20; ++i)
    {
        m_o_2.free_object<CC>(c_ary[i]);
        ZCE_LOG(RS_INFO, "CC pool size %u", m_o_2.size<CC>());
        ZCE_LOG(RS_INFO, "CC pool capacity %u", m_o_2.capacity<CC>());
    }
    m_o_2.terminate<CA>();
    m_o_2.terminate<CB>();
    m_o_2.terminate<CC>();
    m_o_2.terminate<int>();
    m_o_2.terminate<double>();

    return 0;
}

class DA
{
public:
};

class DB
{
public:
    DB(int a, const char *b)
    {
        a_ = a;
        b_ = b;
    }
    int a_;
    std::string b_;
};

class DC
{
public:
    DC(int d1, int d2, int d3, double d4)
    {
        d_1_ = d1;
        d_2_ = d2;
        d_3_ = d3;
        d_4_ = d4;
    }
    int d_1_;
    int d_2_;
    int d_3_;
    double d_4_;
};

int test_multiobj_pool_001(int /*argc*/, char* /*argv*/[])
{
    zce::multiobjs_pool<zce::null_lock, DA, DB, DC, int, double> m_o_1;
    m_o_1.initialize<0>(10, 10);
    m_o_1.initialize<1>(10, 10);
    m_o_1.initialize<2>(10, 10);
    m_o_1.initialize<3>(10, 10);
    m_o_1.initialize<4>(10, 10);
    auto sz = m_o_1.size<0>();
    ZCE_LOG(RS_INFO, "m_o_1 %u", sz);

    auto a_1_1 = m_o_1.constructor<0>();
    auto a_1_2 = m_o_1.constructor<0>();
    m_o_1.destroy<0>(a_1_1);
    m_o_1.destroy<0>(a_1_2);

    auto b_1_1 = m_o_1.constructor<1>(1, "Oh.Oh");
    auto b_1_2 = m_o_1.constructor<1>(2, "Oh.Oh");
    m_o_1.destroy<1>(b_1_1);
    m_o_1.destroy<1>(b_1_2);

    auto c_1_1 = m_o_1.constructor<2>(1, 2, 3, 4.001f);
    auto c_1_2 = m_o_1.constructor<2>(5, 6, 7, 8.001f);
    m_o_1.destroy<2>(c_1_1);
    m_o_1.destroy<2>(c_1_2);

    auto d_1_1 = m_o_1.constructor<3>(1);
    auto d_1_2 = m_o_1.constructor<3>(5000);
    m_o_1.destroy<3>(d_1_1);
    m_o_1.destroy<3>(d_1_2);

    auto e_1_1 = m_o_1.constructor<4>(1.185);
    auto e_1_2 = m_o_1.constructor<4>(5000.34);
    m_o_1.destroy<4>(e_1_1);
    m_o_1.destroy<4>(e_1_2);

    m_o_1.terminate<0>();
    m_o_1.terminate<1>();
    m_o_1.terminate<2>();
    m_o_1.terminate<3>();
    m_o_1.terminate<4>();

    zce::multiobjs_pool<zce::null_lock, DA, DB, DC, int, double> m_o_2;
    m_o_2.initialize<DA>(10, 10);
    m_o_2.initialize<DB>(10, 10);
    m_o_2.initialize<DC>(10, 10);
    m_o_2.initialize<int>(10, 10);
    m_o_2.initialize<double>(10, 10);

    auto a_2_1 = m_o_2.constructor<DA>();
    auto a_2_2 = m_o_2.constructor<DA>();
    m_o_2.destroy<DA>(a_2_1);
    m_o_2.destroy<DA>(a_2_2);

    DC* c_ary[20];
    for (size_t i = 0; i < 20; ++i)
    {
        c_ary[i] = m_o_2.constructor<DC>((int)i, 2, 3, 4.001f);
        ZCE_LOG(RS_INFO, "CC pool size %u", m_o_2.size<DC>());
        ZCE_LOG(RS_INFO, "CC pool capacity %u", m_o_2.capacity<DC>());
    }
    for (size_t i = 0; i < 20; ++i)
    {
        m_o_2.destroy<DC>(c_ary[i]);
        ZCE_LOG(RS_INFO, "CC pool size %u", m_o_2.size<DC>());
        ZCE_LOG(RS_INFO, "CC pool capacity %u", m_o_2.capacity<DC>());
    }
    m_o_2.terminate<DA>();
    m_o_2.terminate<DB>();
    m_o_2.terminate<DC>();
    m_o_2.terminate<int>();
    m_o_2.terminate<double>();

    return 0;
}

int test_share_ptr_pool(int /*argc*/, char* /*argv*/[])
{
    zce::shareptr_pool<std::mutex, int> pool;
    pool.initialize(5, 5);
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    auto a = pool.alloc_share();
    auto b = pool.alloc_share();
    auto c = pool.alloc_share();
    auto d = pool.alloc_share();
    auto e = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    *(a.get()) = 1;
    *(a.get()) = 2;
    *(a.get()) = 3;

    auto f = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    auto g = pool.alloc_share();
    auto h = pool.alloc_share();
    auto i = pool.alloc_share();
    auto j = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    a.reset();
    b.reset();
    c.reset();
    d.reset();
    e.reset();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    a = pool.alloc_share();
    b = pool.alloc_share();
    c = pool.alloc_share();
    d = pool.alloc_share();
    e = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    auto k = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;
    auto l = pool.alloc_share();
    auto m = pool.alloc_share();
    auto n = pool.alloc_share();
    auto o = pool.alloc_share();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;

    a.reset();
    b.reset();
    c.reset();
    d.reset();
    e.reset();
    f.reset();
    g.reset();
    h.reset();
    i.reset();
    j.reset();
    k.reset();
    l.reset();
    m.reset();
    n.reset();
    o.reset();
    std::cout << "Pool size:" << pool.size() << " capacity:" << pool.capacity() << std::endl;

    return 0;
}