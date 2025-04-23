#include "predefine.h"
#include "zealot_main.h"

int co_coroutine1(int* count,
                  int num)
{
    *count += num;
    return 0;
}

int co_coroutine2(int* count,
                  int num1,
                  int num2)
{
    *count += (num1 + num2);
    return 0;
}

int co_coroutine3(int* count,
                  int num1,
                  int num2,
                  double num3)
{
    *count += (num1 + num2 + (int)num3);
    return 0;
}
const size_t COROUTINE_LOOP_NUMBER = 7;

void co_7in7out(coroutine_t* loop_context,
                int* count,
                int num)
{
    for (size_t i = 0; i < COROUTINE_LOOP_NUMBER; ++i)
    {
        *count += num;
        zce::yeild_coroutine(loop_context);
    }
}

int test_coroutine1()
{
    int num = 1, num1 = 0, num2 = 0;
    double num3 = 0.0;
    int count = 0;
    int ret = 0;
    coroutine_t co1;
    ret = zce::make_coroutine(&co1,
                              8192 * 100,
                              true,
                              co_coroutine1,
                              &count,
                              num);
    EXPECT_EQ(ret, 0);
    zce::resume_coroutine(&co1);
    zce::delete_coroutine(&co1);
    EXPECT_EQ(count, 1);

    coroutine_t co2;
    count = 0, num1 = 10, num2 = 20;
    ret = zce::make_coroutine(&co2,
                              8192 * 100,
                              true,
                              co_coroutine2,
                              &count,
                              num1,
                              num2);
    EXPECT_EQ(ret, 0);
    zce::resume_coroutine(&co2);
    zce::delete_coroutine(&co2);
    EXPECT_EQ(count, 30);

    coroutine_t co3;
    count = 0, num1 = 10, num2 = 20, num3 = 30.0;
    ret = zce::make_coroutine(&co3,
                              8192 * 100,
                              true,
                              co_coroutine3,
                              &count,
                              num1,
                              num2,
                              num3);
    EXPECT_EQ(ret, 0);
    zce::resume_coroutine(&co3);
    zce::delete_coroutine(&co3);
    EXPECT_EQ(count, 60);

    coroutine_t co4;
    count = 0, num = 3;
    ret = zce::make_coroutine(&co4,
                              8192 * 100,
                              true,
                              co_7in7out,
                              &co4,
                              &count,
                              num);
    EXPECT_EQ(ret, 0);

    for (size_t i = 0; i < COROUTINE_LOOP_NUMBER; ++i)
    {
        zce::resume_coroutine(&co4);
        std::cout << "co_7in7out count:" << count << std::endl;
    }
    zce::delete_coroutine(&co4);
    EXPECT_EQ(count, 21);

    return 0;
}

TEST(CoroutineTestSuite, TestAPI)
{
    EXPECT_EQ(test_coroutine1(), 0);
}
