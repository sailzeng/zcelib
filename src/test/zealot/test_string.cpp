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

int test_time_string1(int /*argc*/, char* /*argv*/[])
{
    std::ostream_iterator<char> out(std::cout);
    const size_t MAX_LEN = 64;
    char str_tm1[MAX_LEN];
    size_t max_len = MAX_LEN;
    size_t use_len = 0;
    zce::time_value tm1(zce::gettimeofday());
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_DAY);
    std::format_to(out, "Out time string zce::TS_FMT::SHRINK_DAY  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_SEC);
    std::format_to(out, "Out time string zce::TS_FMT::SHRINK_SEC  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_USEC);
    std::format_to(out, "Out time string zce::TS_FMT::SHRINK_USEC is {}\n", str_tm1);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_DAY);
    std::format_to(out, "Out time string zce::TS_FMT::COMPACT_DAY  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_SEC);
    std::format_to(out, "Out time string zce::TS_FMT::COMPACT_SEC  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_USEC);
    std::format_to(out, "Out time string zce::TS_FMT::COMPACT_USEC is {}\n", str_tm1);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_DAY);
    std::format_to(out, "Out time string zce::TS_FMT::ISO_DAY  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_SEC);
    std::format_to(out, "Out time string zce::TS_FMT::ISO_SEC  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_USEC);
    std::format_to(out, "Out time string zce::TS_FMT::ISO_USEC is {}\n", str_tm1);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::HTTP_GMT);
    std::format_to(out, "Out time string zce::TS_FMT::HTTP_GMT  is {}\n", str_tm1);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::EMAIL_DATE);
    std::format_to(out, "Out time string zce::TS_FMT::EMAIL_DATE is {}\n", str_tm1);

    zce::time_value tm2;
    char str_tm2[MAX_LEN];

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_DAY);
    tm2.from_str(str_tm1, false, zce::TS_FMT::SHRINK_DAY);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::SHRINK_DAY);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_SEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::SHRINK_SEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::SHRINK_SEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::SHRINK_USEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::SHRINK_USEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::SHRINK_USEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_DAY);
    tm2.from_str(str_tm1, false, zce::TS_FMT::COMPACT_DAY);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::COMPACT_DAY);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_SEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::COMPACT_SEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::COMPACT_SEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::COMPACT_USEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::COMPACT_USEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::COMPACT_USEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_DAY);
    tm2.from_str(str_tm1, false, zce::TS_FMT::ISO_DAY);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::ISO_DAY);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_SEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::ISO_SEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::ISO_SEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::ISO_USEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::ISO_USEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::ISO_USEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::US_SEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::US_SEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::US_SEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);
    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::US_USEC);
    tm2.from_str(str_tm1, false, zce::TS_FMT::US_USEC);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::US_USEC);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::HTTP_GMT);
    tm2.from_str(str_tm1, false, zce::TS_FMT::HTTP_GMT);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::HTTP_GMT);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    tm1.to_str(str_tm1, max_len, use_len, false, zce::TS_FMT::EMAIL_DATE);
    tm2.from_str(str_tm1, false, zce::TS_FMT::EMAIL_DATE);
    tm2.to_str(str_tm2, max_len, use_len, false, zce::TS_FMT::EMAIL_DATE);
    ZCE_ASSERT(::strcmp(str_tm1, str_tm2) == 0);

    return 0;
}