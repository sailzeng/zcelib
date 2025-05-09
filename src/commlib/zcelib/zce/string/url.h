#pragma once

namespace zce
{
struct url
{
    std::string scheme_;
    // std::string delimiter;
    std::string authority_;
    std::string user_;
    std::string host_;
    int port = -1;
    std::string path_;
    std::string query_;
    std::string fragment_;
};

int regex_url_str(const char* strurl,
                  zce::url purl);

std::map<std::string, std::string> regex_queryparams(const std::string& query);

std::string url_decode(const std::string& input);

std::string url_encode(const std::string& input);
}
