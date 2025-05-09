#include "zce/predefine.h"
#include "zce/string/url.h"

namespace
{
// URL编码函数：将保留字符转换为 %XX 格式
std::string url_encode(const std::string& input)
{
    std::ostringstream oss;
    for (unsigned char c : input)
    {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            // RFC 3986 中定义的“unreserved”字符，保留原样
            oss << c;
        }
        else
        {
            // 其他字符进行百分号编码
            oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
    }
    return oss.str();
}

// URL解码函数：将 %XX 转换为原字符
std::string url_decode(const std::string& input)
{
    std::ostringstream oss;
    for (size_t i = 0; i < input.length(); ++i)
    {
        if (input[i] == '%' && i + 2 < input.length())
        {
            int value = 0;
            std::istringstream iss(input.substr(i + 1, 2));
            if (iss >> std::hex >> value)
            {
                oss << static_cast<char>(value);
                i += 2;
            }
            else
            {
                // 非法的编码，保留原样
                oss << '%';
            }
        }
        else if (input[i] == '+')
        {
            // 可选：将 '+' 转为空格（某些表单或旧式编码风格）
            oss << ' ';
        }
        else
        {
            oss << input[i];
        }
    }
    return oss.str();
}

std::map<std::string, std::string> regex_queryparams(const std::string& query)
{
    std::map<std::string, std::string> result;
    std::regex pair_regex(R"(([^=&]+)=?([^&]*)?)");
    auto begin = std::sregex_iterator(query.begin(), query.end(), pair_regex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
    {
        std::string key = url_decode((*it)[1]);
        std::string value = url_decode((*it)[2]);
        result[key] = value;
    }

    return result;
}

int regex_url_str(const char* strurl,
                  zce::url purl)
{
    const std::string URL_SCHEME_REGEX = R"((^(\w+):\/\/)";
    const std::string URL_AUTHORITY_REGEX = R"((?:([^:@\/\[\]]+)(?::([^@\/\[\]]*))?@))";
    const std::string URL_HOST_REGEX = R"((\[[^\]]+\]|[^:/?#]+))";
    const std::string URL_PORT_REGEX = R"((?::(\d+)))";
    const std::string URL_PATH_REGEX = R"((/[^?#]*))";
    const std::string URL_QUERY_REGEX = R"((?:\?([^#]*)))";
    const std::string URL_FRAGMENT_REGEX = R"((?:#(.*)))";

    const std::string URL_PATTERN =
        "^" + URL_SCHEME_REGEX + URL_AUTHORITY_REGEX + "?" + URL_HOST_REGEX + URL_PORT_REGEX + "?" +
        URL_PATH_REGEX + "?" + URL_QUERY_REGEX + "?" + URL_FRAGMENT_REGEX + "?" + "$";

    std::regex url_regex(URL_PATTERN);
    std::cmatch match;
    if (!std::regex_match(strurl, match, url_regex))
    {
        return -1;
    }

    purl.scheme_ = match[1];
    if (match[2].matched)
    {
        purl.user_ = match[2].str();
    }
    if (match[3].matched)
    {
        purl.authority_ = match[3].str();
    }
    purl.host_ = match[4];
    if (match[5].matched)
    {
        purl.port = std::stoi(match[5].str());
    }
    purl.path_ = match[6].matched ? match[6].str() : "/";
    if (match[7].matched)
    {
        purl.query_ = match[7].str();
    }
    if (match[8].matched)
    {
        purl.fragment_ = match[8].str();
    }
    return 0;
}
}
