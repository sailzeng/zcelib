#include "predefine.h"

class RunTest
{
public:
    RunTest() = default;
    int run()
    {
        return 100;
    }
};
int test_singleton(int /*argc*/, char* /*argv*/[])
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
    UtilA001(int a, const char *b)
    {
        a_ = a;
        b_ = b;
    }

    int a_;
    std::string b_;
};

int test_util_001(int /*argc*/, char* /*argv*/[])
{
    //std::cout << std::is_pod<zce::cycle_buffer>::value << std::endl;
    //std::cout << std::is_pod<zce::queue_buffer>::value << std::endl;

    auto a = zce::new_helper<UtilA001, int, const char *>::invoke(1, "help me!");
    if (a == nullptr)
    {
        return -1;
    }

    auto fun = std::bind(zce::new_helper<UtilA001, int, const char *>::invoke, 1, "help me!");
    auto b = fun();
    if (b == nullptr)
    {
        return -1;
    }

    return 0;
}