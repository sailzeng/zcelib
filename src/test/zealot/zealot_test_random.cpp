


#include "zealot_predefine.h"
#include "zealot_test_function.h"
#include "zce_boost_random.h"


//�������ڲ��ԣ���ʵ����ȫ��
class random_libc : public zce::t_random_base<0, 0x00007FFF>
{
public:

    random_libc(uint32_t seed)
    {
        ::srand(seed);
    }

    virtual ~random_libc()
    {
    }

    virtual void srand(uint32_t seed)
    {
        ::srand(seed);
    }

    virtual uint32_t rand()
    {
        return ::rand();
    }
};


//�������ڲ��ԣ���ʵ����ȫ��
class random_pet2 : public zce::t_random_base<0, 0xFFFFFFFF>
{
public:

    random_pet2(uint32_t seed):
        seed_(seed)
    {
    }

    virtual ~random_pet2()
    {
    }

    virtual void srand(uint32_t seed)
    {
        seed_ = seed;
    }

    virtual uint32_t rand()
    {
        uint32_t next = seed_;
        uint32_t result = 0;

        next *= 1103515245;
        next += 12345;
        result = (unsigned int) (next >> 16) & 0x07ff;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int) (next >> 16) & 0x03ff;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int) (next >> 16) & 0x03ff;

        seed_ = next;

        return result;
    }

protected:
    uint32_t   seed_;
};


int test_random_example(int /*argc*/, char * /*argv*/[])
{
    const uint32_t TEST_SEED = 1010123;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);
    zce::random_mt19937   mt19937_gen(TEST_SEED);
    zce::random_rand48   rand48_gen(TEST_SEED);
    zce::random_taus88   taus88_gen(TEST_SEED);

    const size_t TEST_NUM = 1024;
    for (size_t i = 0; i < TEST_NUM; ++i)
    {
        std::cout << mt11231b_gen.rand() << " "
                  << mt19937_gen.rand() << " "
                  << rand48_gen.rand() << " "
                  << taus88_gen.rand() << " " << std::endl;
    }

    return 0;
}

//��BOOST�ĺ������Աȵĺ���
//int test_matchboost_random(int /*argc*/ , char * /*argv*/ [])
//{
//
//    const uint32_t TEST_SEED = 1010123;
//
//    zce::random_mt11213b  mt11231b_gen(TEST_SEED);
//    zce::random_mt19937   mt19937_gen(TEST_SEED);
//
//    boost::mt11213b boost_mt11213_rng(TEST_SEED);
//    boost::mt19937  boost_mt19937_rng(TEST_SEED);
//
//    //mt11231b_gen.srand(TEST_SEED);
//    //mt19937_gen.srand(TEST_SEED);
//
//    uint32_t u32_data =  0;
//    u32_data = mt19937_gen.rand();
//    u32_data = boost_mt19937_rng();
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << mt19937_gen.rand() << " ";
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_mt19937_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//
//    std::cout << std::endl;
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << mt11231b_gen.rand() << " ";
//
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_mt11213_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//    zce::random_rand48   rand48_gen(TEST_SEED);
//    zce::random_taus88   taus88_gen(TEST_SEED);
//
//    boost::rand48  boost_rand48_rng(TEST_SEED);
//    boost::taus88  boost_taus88_rng(TEST_SEED);
//
//    u32_data = rand48_gen.rand();
//    u32_data = boost_rand48_rng();
//
//    u32_data = taus88_gen.rand();
//    u32_data = boost_taus88_rng();
//
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << rand48_gen.rand() << " ";
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_rand48_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//
//    std::cout << std::endl;
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << taus88_gen.rand() << " ";
//
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_taus88_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//    zce::random_ranlux223   ranlux223_gen(TEST_SEED);
//    zce::random_ranlux389   ranlux389_gen(TEST_SEED);
//
//    boost::ranlux3  boost_ranlux3_rng(TEST_SEED);
//    boost::ranlux4  boost_ranlux4_rng(TEST_SEED);
//
//    u32_data = ranlux223_gen.rand();
//    u32_data = boost_ranlux3_rng();
//
//    u32_data = ranlux389_gen.rand();
//    u32_data = boost_ranlux4_rng();
//
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << ranlux223_gen.rand() << " ";
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_ranlux3_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//
//    std::cout << std::endl;
//
//    for (size_t i = 0 ; i < 32; ++i)
//    {
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << ranlux389_gen.rand() << " ";
//
//        }
//
//        std::cout << std::endl;
//
//        for (size_t j = 0; j < 8; j++)
//        {
//            std::cout << boost_ranlux4_rng() << " ";
//        }
//
//        std::cout << std::endl;
//    }
//
//    return 0;
//}




//����ֲ���������
int test_random_distribution(zce::random_base *rand_gen)
{
    const size_t TEST_SIZE = 0xFFFFF; //10*1000*1000;
    const size_t TEST_NUMBER = TEST_SIZE;

    char *coverage_chart = NULL;
    coverage_chart = new char [TEST_SIZE];

    uint32_t u32_data =  0;
    size_t conflict_count = 0;

    memset(coverage_chart, 0, TEST_SIZE);
    conflict_count = 0;

    for (size_t i = 0 ; i < TEST_NUMBER; ++i)
    {
        u32_data = rand_gen->rand();

        if ( coverage_chart [ u32_data % TEST_SIZE] != 0 )
        {
            conflict_count++;
        }

        coverage_chart [ u32_data % TEST_SIZE] ++;
    }

    std::cout << typeid(rand_gen).name() << " non repeat :" << TEST_SIZE - conflict_count << " conflict count :" << conflict_count << std::endl;

    return 0;
}

//����ֲ���������
template<typename rand_gen_t>
int test_random_distribution2()
{
    const uint32_t TEST_SEED = 1010123;
    rand_gen_t rand_gen(TEST_SEED);

    const size_t TEST_SIZE = 100 * 100;
    const size_t TEST_NUMBER = TEST_SIZE * 10;


    char *coverage_chart = NULL;
    coverage_chart = new char [TEST_SIZE];

    uint32_t u32_data =  0;
    size_t conflict_count = 0;
    size_t non_repeat_size = 0;
    size_t i = 0;

    memset(coverage_chart, 0, TEST_SIZE);
    conflict_count = 0;
    non_repeat_size = 0;

    for ( i = 0 ; i < TEST_NUMBER ; ++i)
    {
        u32_data = rand_gen.rand();

        if ( coverage_chart [ u32_data % TEST_SIZE] == 0 )
        {
            coverage_chart [ u32_data % TEST_SIZE] ++;
            ++non_repeat_size;

            if (non_repeat_size >= TEST_SIZE )
            {
                break;
            }
        }
        else
        {
            conflict_count++;
        }
    }

    std::cout << typeid(rand_gen).name() << "  non repeat :" << TEST_SIZE - conflict_count << " conflict count :" << conflict_count << std::endl;

    delete coverage_chart;
    coverage_chart = NULL;

    return 0;
}

template<typename rand_gen_t>
int test_random_vargen()
{
    const uint32_t TEST_SEED = 1010123;
    rand_gen_t rand_gen(TEST_SEED);

    std::cout << typeid(rand_gen).name() << std::endl;

    for (size_t i = 0 ; i < 32; ++i)
    {
        for (size_t j = 0; j < 8; j++)
        {
            std::cout << rand_gen.rand() << " ";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    return 0;
}


//���ܲ���
template<typename rand_gen_t>
int test_random_elapsed_t()
{
    const uint32_t TEST_SEED = 1010123;
    uint32_t u32_data = 0;

    rand_gen_t rand_gen(TEST_SEED);
    ZCE_Progress_Timer rand_timer;
    const size_t TEST_SIZE = 10000 * 10000;

    rand_timer.restart();

    for (size_t i = 0 ; i < TEST_SIZE; ++i)
    {
        u32_data += rand_gen.rand();
    }

    rand_timer.end();
    std::cout << typeid(rand_gen_t).name() << " elapsed: " << std::setprecision(6) <<
              rand_timer.elapsed_sec() << " one second generate :"
              << std::setiosflags( std::ios::fixed ) << std::setprecision(6)
              << double(TEST_SIZE / rand_timer.elapsed_sec() ) << std::endl;
    std::cout << "last data = " << u32_data << std::endl;
    return 0;
}

int test_random_variance(const char *rand_fun_name,
                         zce::random_base *rand_gen)
{

    const size_t TEST_SIZE = 1000 * 1000; //10*1000*1000;
    const size_t TEST_LUN = 100;
    const size_t TEST_NUMBER = TEST_LUN * TEST_SIZE;

    int *coverage_chart = NULL;
    coverage_chart = new int [TEST_SIZE];

    memset(coverage_chart, 0, TEST_SIZE * sizeof(int));

    double fangcai = 0.0;
    uint32_t max_caizhi = 0, u32_data = 0;


    for (size_t i = 0 ; i < TEST_NUMBER; ++i)
    {
        u32_data = rand_gen->rand();
        coverage_chart [ u32_data % TEST_SIZE] ++;
    }

    for (size_t j = 0; j < TEST_SIZE; ++j)
    {
        if (max_caizhi < (uint32_t)(::abs( double(coverage_chart [j]) - TEST_LUN)))
        {
            max_caizhi = (uint32_t) ::abs(double(coverage_chart [j]) - TEST_LUN);
        }

        fangcai += pow(double(coverage_chart [j]) - TEST_LUN, 2);
    }

    std::cout << std::endl;
    fangcai = fangcai / TEST_SIZE;
    std::cout << rand_fun_name << " variance " << fangcai << " max caizhi :" << max_caizhi << std::endl;

    delete coverage_chart;
    coverage_chart = NULL;

    return 0;
}

//���㼸������������ķ���
template<typename rand_gen_t>
int test_random_variance_t()
{
    const uint32_t TEST_SEED = 1010123;
    rand_gen_t rand_gen(TEST_SEED);

    const size_t TEST_SIZE = 1000 * 1000; //10*1000*1000;
    const size_t TEST_LUN = 100;
    const size_t TEST_NUMBER = TEST_LUN * TEST_SIZE;

    int *coverage_chart = NULL;
    coverage_chart = new int [TEST_SIZE];

    memset(coverage_chart, 0, TEST_SIZE * sizeof(int));

    double fangcai = 0.0;
    uint32_t max_caizhi = 0, u32_data = 0;


    for (size_t i = 0 ; i < TEST_NUMBER; ++i)
    {
        u32_data = rand_gen.rand();
        coverage_chart [ u32_data % TEST_SIZE] ++;
    }

    for (size_t j = 0; j < TEST_SIZE; ++j)
    {
        if (max_caizhi < (uint32_t)(::abs( double(coverage_chart [j]) - TEST_LUN)))
        {
            max_caizhi = (uint32_t) ::abs(double(coverage_chart [j]) - TEST_LUN);
        }

        fangcai += pow(double(coverage_chart [j]) - TEST_LUN, 2);
    }

    std::cout << std::endl;
    fangcai = fangcai / TEST_SIZE;
    std::cout << typeid(rand_gen_t).name() << " variance " << fangcai << " max caizhi :" << max_caizhi << std::endl;

    delete coverage_chart;
    coverage_chart = NULL;

    return 0;
}


//���㼸��ȥ���ӵ�����µķ������
template<typename rand_gen_t>
int test_die_variance_t()
{
    const uint32_t TEST_SEED = 1010123;
    rand_gen_t rand_gen(TEST_SEED);

    //�ǳ�С������
    const size_t TEST_SIZE = 10;
    const size_t TEST_LUN = 1000000;
    const size_t TEST_NUMBER = TEST_LUN * TEST_SIZE;

    int *coverage_chart = NULL;
    coverage_chart = new int [TEST_SIZE];

    memset(coverage_chart, 0, TEST_SIZE * sizeof(int));

    double fangcai = 0.0;
    uint32_t max_caizhi = 0, u32_data = 0;


    for (size_t i = 0 ; i < TEST_NUMBER; ++i)
    {
        u32_data = rand_gen.rand();
        coverage_chart [ u32_data % TEST_SIZE] ++;
    }

    for (size_t j = 0; j < TEST_SIZE; ++j)
    {
        if (max_caizhi < (uint32_t)(::abs( double(coverage_chart [j]) - TEST_LUN)))
        {
            max_caizhi = (uint32_t) ::abs(double(coverage_chart [j]) - TEST_LUN);
        }

        fangcai += pow(double(coverage_chart [j]) - TEST_LUN, 2);
    }

    std::cout << std::endl;
    fangcai = fangcai / TEST_SIZE;
    std::cout << "normal die " << typeid(rand_gen_t).name() << " variance " << (uint32_t) fangcai << " max caizhi :" << max_caizhi << std::endl;

    //�Ŵ����ӣ�ȡ�����Ӻ���ȥ���еĸ�λ
    memset(coverage_chart, 0, TEST_SIZE * sizeof(int));
    fangcai = 0.0;
    max_caizhi = 0, u32_data = 0;

    for (size_t i = 0 ; i < TEST_NUMBER; ++i)
    {
        u32_data = (rand_gen.rand() % 10000) / 1000;
        coverage_chart [ u32_data ] ++;
    }

    for (size_t j = 0; j < TEST_SIZE; ++j)
    {
        if (max_caizhi < (uint32_t)(::abs( double(coverage_chart [j]) - TEST_LUN)))
        {
            max_caizhi = (uint32_t) ::abs(double(coverage_chart [j]) - TEST_LUN);
        }

        fangcai += pow(double(coverage_chart [j]) - TEST_LUN, 2);
    }

    std::cout << std::endl;
    fangcai = fangcai / TEST_SIZE;
    std::cout << "fangda die " << typeid(rand_gen_t).name() << " variance " << (uint32_t) fangcai << " max caizhi :" << max_caizhi << std::endl;


    delete coverage_chart;
    coverage_chart = NULL;

    return 0;

}

//���㼸������������ĺ�ʱ���
int test_random_elapsed_time(int /*argc*/, char * /*argv*/ [])
{

    test_random_elapsed_t<random_libc>();
    test_random_elapsed_t<random_pet2>();

    test_random_elapsed_t<zce::random_bsdrand>();
    test_random_elapsed_t<zce::random_rand48>();
    test_random_elapsed_t<zce::random_taus88>();

    test_random_elapsed_t<zce::random_mt11213a>();
    test_random_elapsed_t<zce::random_mt11213b>();
    test_random_elapsed_t<zce::random_mt19937>();

    test_random_elapsed_t<zce::random_ranlux223>();
    test_random_elapsed_t<zce::random_ranlux389>();

    ::srand(1010123);
    ZCE_Progress_Timer rand_timer;
    const size_t TEST_SIZE = 10000 * 10000;

    rand_timer.restart();

    uint32_t u32_data = 0;
    for (size_t i = 0 ; i < TEST_SIZE; ++i)
    {
        u32_data += ::rand();
    }

    rand_timer.end();
    std::cout << " Test rand:" << rand_timer.elapsed_sec() << std::endl;

    return 0;
}

int test_random_variance(int /*argc*/, char * /*argv*/ [])
{
    test_random_vargen<zce::random_bsdrand>();
    test_random_vargen<zce::random_rand48>();
    test_random_vargen<zce::random_taus88>();

    test_random_vargen<zce::random_mt11213a>();
    test_random_vargen<zce::random_mt11213b>();
    test_random_vargen<zce::random_mt19937>();

    test_random_vargen<zce::random_ranlux223>();
    test_random_vargen<zce::random_ranlux389>();

    //
    test_random_variance_t<random_libc>();
    test_random_variance_t<random_pet2>();
    test_random_variance_t<zce::random_bsdrand>();
    test_random_variance_t<zce::random_rand48>();
    test_random_variance_t<zce::random_taus88>();

    test_random_variance_t<zce::random_mt11213a>();
    test_random_variance_t<zce::random_mt11213b>();
    test_random_variance_t<zce::random_mt19937>();

    test_random_variance_t<zce::random_ranlux223>();
    test_random_variance_t<zce::random_ranlux389>();

    return 0;
}


//ʹ��ģ�溯���Ĳ���
int test_template_random(int /*argc*/, char * /*argv*/ [])
{

    test_die_variance_t<random_libc>();
    test_die_variance_t<random_pet2>();
    test_die_variance_t<zce::random_bsdrand>();
    test_die_variance_t<zce::random_rand48>();
    test_die_variance_t<zce::random_taus88>();

    test_die_variance_t<zce::random_mt11213a>();
    test_die_variance_t<zce::random_mt11213b>();
    test_die_variance_t<zce::random_mt19937>();

    test_die_variance_t<zce::random_ranlux223>();
    test_die_variance_t<zce::random_ranlux389>();




    return 0;
}



//ʹ��OO�����Ĳ���
int test_oo_random(int /*argc*/, char * /*argv*/ [])
{
    const uint32_t TEST_SEED = (uint32_t) time(NULL);

    zce::random_bsdrand  randbsd_gen(TEST_SEED);
    zce::random_rand48   rand48_gen(TEST_SEED);
    zce::random_taus88   taus88_gen(TEST_SEED);

    zce::random_mt11213a  mt11213a_gen(TEST_SEED);
    zce::random_mt11213b  mt11213b_gen(TEST_SEED);
    zce::random_mt19937   mt19937_gen(TEST_SEED);

    zce::random_ranlux223   ranlux223_gen(TEST_SEED);
    zce::random_ranlux389   ranlux389_gen(TEST_SEED);


    test_random_variance(typeid(&randbsd_gen).name(), &randbsd_gen);
    test_random_variance(typeid(&randbsd_gen).name(), &rand48_gen);
    test_random_variance(typeid(&randbsd_gen).name(), &taus88_gen);

    test_random_variance(typeid(&mt11213a_gen).name(), &mt11213a_gen);
    test_random_variance(typeid(&mt11213b_gen).name(), &mt11213b_gen);
    test_random_variance(typeid(&mt19937_gen).name(), &mt19937_gen);

    test_random_variance(typeid(&ranlux223_gen).name(), &ranlux223_gen);
    test_random_variance(typeid(&ranlux389_gen).name(), &ranlux389_gen);


    return 0;
}



//���㼸������������ķ���
template<typename rand_gen_t>
int test_random_repeat_t()
{
    const uint32_t TEST_SEED = 1010123;
    rand_gen_t rand_gen(TEST_SEED);



    uint32_t  u32_data1 = 0;
    uint32_t  u32_data2 = 0;

    u32_data1 = rand_gen.rand();
    u32_data2 = rand_gen.rand();

    uint64_t i = 0;
    for (; i < 4 * 0xFFFFFFFFULL; ++i)
    {
        if (u32_data1 == rand_gen.rand() && u32_data2 == rand_gen.rand() )
        {
            break;
        }
    }


    std::cout << std::endl;
    std::cout << "Repeat" << typeid(rand_gen_t).name() << " use times " << std::hex << i << std::endl;

    return 0;
}

//���㼸��������������ظ����
int test_random_repeat_number(int /*argc*/, char * /*argv*/ [])
{

    test_random_repeat_t<random_libc>();
    test_random_repeat_t<random_pet2>();

    test_random_repeat_t<zce::random_bsdrand>();
    test_random_repeat_t<zce::random_rand48>();
    test_random_repeat_t<zce::random_taus88>();

    test_random_repeat_t<zce::random_mt11213a>();
    test_random_repeat_t<zce::random_mt11213b>();
    test_random_repeat_t<zce::random_mt19937>();

    return 0;
}


//���㼸��������������ظ����
int test_random_var_obj_speed(int /*argc*/, char * /*argv*/ [])
{
    ZCE_Progress_Timer rand_timer;
    const size_t TEST_SIZE = 10000 * 10000;
    uint32_t u32_data = 0;

    zce::random_mt19937 test1;
    rand_timer.restart();
    for (size_t i = 0 ; i < TEST_SIZE; ++i)
    {
        u32_data += test1.get_uint32();
    }
    rand_timer.end();
    std::cout << " Test rand:" << rand_timer.elapsed_sec() << std::endl;



    rand_timer.restart();
    for (size_t i = 0 ; i < TEST_SIZE; ++i)
    {
        u32_data += zce::mt19937_instance::instance()->get_uint32();
    }
    rand_timer.end();
    std::cout << " Test rand:" << rand_timer.elapsed_sec() << std::endl;

    return 0;
}

