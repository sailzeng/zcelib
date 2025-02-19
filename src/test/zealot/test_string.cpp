#include "predefine.h"

int test_string1(int /*argc*/, char* /*argv*/[])
{
    const char FROM_STR1[] = "23";
    char to_1 = 0;
    to_1 = zce::from_str<decltype(to_1)>(FROM_STR1);
    zce::from_str(FROM_STR1, to_1);

    const char FROM_STR2[] = "-23";
    signed char to_2 = 0;
    to_2 = zce::from_str<decltype(to_2)>(FROM_STR2);
    zce::from_str(FROM_STR2, to_2);

    return 0;
}