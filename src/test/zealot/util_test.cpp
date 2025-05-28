#include "predefine.h"

namespace zeal
{

class RunTest
{
public:
    RunTest() = default;
    int run()
    {
        return 100;
    }
};
int test_singleton()
{
    zce::c_singleton<RunTest>::set(new RunTest());
    zce::c_singleton<RunTest>::instance()->run();
    return 0;
}

int test_static_list(int /*argc*/, char* /*argv*/[])
{
    zce::static_list<int> abc;
    abc.initialize(200);

    return 0;
}

class UtilA001
{
public:
    UtilA001(int a, const char* b)
    {
        a_ = a;
        b_ = b;
    }

    int a_;
    std::string b_;
};

int test_util_001()
{
    //std::cout << std::is_pod<zce::cycle_buffer>::value << std::endl;
    //std::cout << std::is_pod<zce::queue_buffer>::value << std::endl;

    auto a = zce::new_helper<UtilA001, int, const char*>::invoke(1, "help me!");
    if (a == nullptr)
    {
        return -1;
    }

    auto fun = std::bind(zce::new_helper<UtilA001, int, const char*>::invoke, 1, "help me!");
    auto b = fun();
    if (b == nullptr)
    {
        return -1;
    }

    return 0;
}

template<typename T>
int test_container([[maybe_unused]] T& a)
{
    std::cout << zce::is_container<T>::value << std::endl;
    std::cout << zce::has_key_type<T>::value << std::endl;
    std::cout << zce::is_single_type_container<T>::value << std::endl;
    std::cout << zce::is_associative_container<T>::value << std::endl;
    return 0;
}

int test_mpl_001()
{
    std::vector <int> a;
    test_container(a);
    return 0;
}

TEST(UtilTestSuite, TestAPI)
{
    EXPECT_EQ(test_util_001(), 0);
    EXPECT_EQ(test_singleton(), 0);
    EXPECT_EQ(test_mpl_001(), 0);
}

}
