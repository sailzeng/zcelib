#include "zce/predefine.h"
#include "zce/comm/common.h"
#include "zce/string/regex_parse.h"

int zce::parse_str_to_ztm(const char* strtm,
                          zce::ztm* pztm)
{
    const std::string POSIX_DATE_REGEX = R"(([ \t]*(\d{4}|\d{2})[-\/]?(\d{2})[-\/]?(\d{2})))";
    const std::string TIME_REGEX = R"(([ \t]*(\d{2})\:?(\d{2})\:?(\d{2})([.](\d{6}))?))";
    const std::string WEEK_REGEX = R"(([ \t]*(sun|mon|tue|wed|thu|fri|sat),?))";
    const std::string TIMEZONE_REGEX = R"(([ \t]*(GMT|[\+-]\d{4})))";
    const std::string USEURO_DATE_REGEX = R"(([ \t]*((jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)[ \t]*(\d{2})|(\d{2})[ \t]*(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec))[ \t]*(\d{4})))";
    const std::string TIME_PATTERN[] =
    {
        POSIX_DATE_REGEX + TIME_REGEX + "?" + TIMEZONE_REGEX + "?",
        WEEK_REGEX + "?" + USEURO_DATE_REGEX + TIME_REGEX + "?" + TIMEZONE_REGEX + "?",
    };
    std::regex tm_regex_0(TIME_PATTERN[0]);
    std::regex tm_regex_1(TIME_PATTERN[1], std::regex::icase);

    enum class RP_MFT
    {
        NONE,
        POSIX,
        US_EURO
    } tm_fmt = RP_MFT::NONE;

    std::cmatch tm_cmatch0, tm_cmatch1;
    if (std::regex_search(strtm, tm_cmatch0, tm_regex_0))
    {
        tm_fmt = RP_MFT::POSIX;
    }
    else if (std::regex_search(strtm, tm_cmatch1, tm_regex_1))
    {
        tm_fmt = RP_MFT::US_EURO;
    }
    else
    {
        return -1;
    }

    if (tm_fmt == RP_MFT::POSIX)
    {
        pztm->year_ = std::stoi(tm_cmatch0[2]);
        pztm->mon_ = std::stoi(tm_cmatch0[3]);
        pztm->day_ = std::stoi(tm_cmatch0[4]);
        pztm->hour_ = std::stoi(tm_cmatch0[6]);
        pztm->min_ = std::stoi(tm_cmatch0[7]);
        pztm->sec_ = std::stoi(tm_cmatch0[8]);
        pztm->usec_ = std::stoi(tm_cmatch0[10]);
        pztm->tz_ = std::stoi(tm_cmatch0[12]);
    }
    else if (tm_fmt == RP_MFT::US_EURO)
    {
        std::string mon_str;
        if (tm_cmatch1[5].matched)
        {
            pztm->mon_ = std::stoi(tm_cmatch0[5]);
            mon_str = tm_cmatch1[6].str();
        }
        else if (tm_cmatch1[7].matched)
        {
            pztm->mon_ = std::stoi(tm_cmatch0[7]);
            mon_str = tm_cmatch1[8].str();
        }
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str.c_str(), MONTH_NAME[i], 3) == 0)
            {
                pztm->mon_ = i;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        pztm->year_ = std::stoi(tm_cmatch1[9]);
        pztm->hour_ = std::stoi(tm_cmatch1[11]);
        pztm->min_ = std::stoi(tm_cmatch1[12]);
        pztm->sec_ = std::stoi(tm_cmatch1[13]);
        if (tm_cmatch1[15].matched)
        {
            pztm->usec_ = std::stoi(tm_cmatch1[15]);
        }
        if (tm_cmatch1[17].matched)
        {
            pztm->tz_ = std::stoi(tm_cmatch1[17]);
        }
    }

    if (tm_cmatch1[17].compare("GMT"))
    {
        pztm->tz_ = 0;
    }
    else
    {
        pztm->tz_ = std::stoi(tm_cmatch1[17]);
    }
    return 0;
}
