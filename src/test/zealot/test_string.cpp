#include "predefine.h"

namespace ztest
{
int test_from_string()
{
    const char FROM_STR1[] = "23";
    char to_1 = 0;
    to_1 = zce::from_str_to<decltype(to_1)>(FROM_STR1);
    zce::from_str(FROM_STR1, to_1);

    const char FROM_STR2[] = "-23";
    signed char to_2 = 0;
    to_2 = zce::from_str_to<decltype(to_2)>(FROM_STR2);
    zce::from_str(FROM_STR2, to_2);

    return 0;
}

int test_time_string()
{
    std::ostream_iterator<char> out(std::cout);
    const size_t MAX_LEN = 64;
    char str_tm1[MAX_LEN];
    size_t max_len = MAX_LEN;
    size_t use_len = 0;
    zce::time_value tm1(zce::gettimeofday());
    zce::TMS_FMT fmt1 = zce::TMS_FMT::TMS_FMT_INVALID, fmt2 = zce::TMS_FMT::TMS_FMT_INVALID;

    fmt1 = zce::TMS_FMT::SHRINK_DAY;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::SHRINK_SEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::SHRINK_USEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);

    fmt1 = zce::TMS_FMT::COMPACT_DAY;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::COMPACT_SEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::COMPACT_USEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);

    fmt1 = zce::TMS_FMT::ISO_TIME_SEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::ISO_TIME_USEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::ISO_DATE_DAY;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::ISO_DATE_SEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::ISO_DATE_USEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);

    fmt1 = zce::TMS_FMT::US_SEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::US_USEC;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);

    fmt1 = zce::TMS_FMT::HTTP_GMT;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);
    fmt1 = zce::TMS_FMT::EMAIL_DATE;
    tm1.to_str(str_tm1, max_len, use_len, fmt1);
    std::format_to(out, "Out time string formt {}  is {}\n", magic_enum::enum_name(fmt1), str_tm1);
    fmt2 = zce::fuzzy_str_fmt(str_tm1);
    EXPECT_EQ(fmt1 == fmt2, true);

    zce::time_value tm2;
    char str_tm2[MAX_LEN];

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::SHRINK_DAY);
    tm2.from_str(str_tm1, zce::TMS_FMT::SHRINK_DAY);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::SHRINK_DAY);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::SHRINK_SEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::SHRINK_SEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::SHRINK_SEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::SHRINK_USEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::SHRINK_USEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::SHRINK_USEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::COMPACT_DAY);
    tm2.from_str(str_tm1, zce::TMS_FMT::COMPACT_DAY);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::COMPACT_DAY);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::COMPACT_SEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::COMPACT_SEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::COMPACT_SEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::COMPACT_USEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::COMPACT_USEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::COMPACT_USEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_TIME_SEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_TIME_SEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_TIME_SEC, true);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_TIME_USEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_TIME_USEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_TIME_USEC, true);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_DATE_DAY);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_DATE_DAY);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_DATE_DAY);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_DATE_DAY);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_DATE_DAY);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_DATE_DAY);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_DATE_SEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_DATE_SEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_DATE_SEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::ISO_DATE_USEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::ISO_DATE_USEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::ISO_DATE_USEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::US_SEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::US_SEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::US_SEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);
    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::US_USEC);
    tm2.from_str(str_tm1, zce::TMS_FMT::US_USEC);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::US_USEC);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::HTTP_GMT);
    tm2.from_str(str_tm1, zce::TMS_FMT::HTTP_GMT);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::HTTP_GMT);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    tm1.to_str(str_tm1, max_len, use_len, zce::TMS_FMT::EMAIL_DATE);
    tm2.from_str(str_tm1, zce::TMS_FMT::EMAIL_DATE);
    tm2.to_str(str_tm2, max_len, use_len, zce::TMS_FMT::EMAIL_DATE);
    EXPECT_EQ(::strcmp(str_tm1, str_tm2), 0);

    return 0;
}

int test_time_string2()
{
    char MILLENNIUM_STR[] = "2000-01-01 00:00:00.000000+0000";
    zce::time_value tm1, tm2;
    tm1.from_fuzzy_str(MILLENNIUM_STR, false);
    tm2.from_str(MILLENNIUM_STR, zce::ISO_DATE_USEC, false);
    EXPECT_EQ(tm1.sec(), 946684800);
    EXPECT_EQ(tm2.sec(), 946684800);
    EXPECT_EQ(tm1.usec(), 0);
    EXPECT_EQ(tm2.usec(), 0);
    char BJ_OLYMPIC_STR[] = "2008-08-08 20:08:08.000000+0800";
    tm1.from_fuzzy_str(BJ_OLYMPIC_STR, false);
    tm2.from_str(BJ_OLYMPIC_STR, zce::ISO_DATE_USEC, false);
    EXPECT_EQ(tm1.sec(), 1218197288);
    EXPECT_EQ(tm1.usec(), 0);
    EXPECT_EQ(tm2.sec(), 1218197288);
    EXPECT_EQ(tm2.usec(), 0);
    zce::ztm my_ztm;
    zce::fuzzy_str_to_ztm(BJ_OLYMPIC_STR, &my_ztm);
    EXPECT_EQ(my_ztm.tz_, 8 * (-1) * 3600);
    return 0;
}

TEST(StringTestSuite, TestAPI)
{
    EXPECT_EQ(test_from_string(), 0);
    EXPECT_EQ(test_time_string(), 0);
    EXPECT_EQ(test_time_string2(), 0);
}
}
