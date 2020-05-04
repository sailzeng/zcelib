
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_math.h"

// Euclid's greatest common divisor algorithm.
uint32_t zce::gcd (uint32_t x, uint32_t y)
{
    while (y != 0)
    {
        u_long r = x % y;
        x = y;
        y = r;
    }

    return x;
}

//找到输入参数n最近(大于)的一个质数，
size_t zce::nearest_prime(const size_t n)
{
    size_t test_num = n;
    for (;; ++test_num)
    {
        if (zce::is_prime(test_num))
        {
            break;
        }
    }
    return test_num;
}

