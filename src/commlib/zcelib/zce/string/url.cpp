#include "zce/predefine.h"
#include "zce/string/url.h"

namespace zce
{

// URL编码函数：将保留字符转换为 %XX 格式
std::string url::url_encode(const std::string& input)
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
std::string url::url_decode(const std::string& input)
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

int url::regex_queryparams(std::string_view query)
{

    std::istringstream query_stream(query.data());
    std::string key_value_pair;

    while (std::getline(query_stream, key_value_pair, '&')) {
        auto pos = key_value_pair.find('=');
        if (pos != std::string::npos) {
            std::string key = url_decode(key_value_pair.substr(0, pos));
            std::string value = url_decode(key_value_pair.substr(pos + 1));
            query_params_[key] = value;
        } else {
            query_params_[url_decode(key_value_pair)] = "";
        }
    }

    return 0;
}

int url::regex_urlstr(std::string_view strurl,
                      bool parse_query)
{
    const std::string URL_SCHEME_REGEX = R"((\w+):\/\/)";
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
    if (!std::regex_match(strurl.data(), match, url_regex))
    {
        return -1;
    }

    scheme_ = match[1];
    if (match[2].matched)
    {
        user_ = match[2].str();
    }
    if (match[3].matched)
    {
        authority_ = match[3].str();
    }
    host_ = match[4];
    if (match[5].matched)
    {
        port_ = std::stoi(match[5].str());
    }
    path_ = match[6].matched ? match[6].str() : "/";
    if (match[7].matched)
    {
        query_ = match[7].str();
    }
    if (match[8].matched)
    {
        fragment_ = match[8].str();
    }
    if (parse_query)
    {
        query_params_.parse(query_,'&');
    }
    return 0;
}
}
