#pragma once

#include "zce/string/string_map.h"

namespace zce
{

class url
{

public:
    url() = default;
    ~url() = default;

    int regex_urlstr(std::string_view strurl);

    int regex_queryparams(std::string_view query);

    static std::string url_encode(const std::string& input);
    
    static std::string url_decode(const std::string& input);

public:

    //!
    std::string scheme_;
    //!
    std::string authority_;
    //!
    std::string user_;
    //!
    std::string host_;
    //!
    int port = -1;
    //!
    std::string path_;
    //!
    std::string query_;
    //!
    zce::string_map query_params_;
    //!
    std::string fragment_;

    
};

}
