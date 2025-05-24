#pragma once

#include "zce/string/string_map.h"

namespace zce
{

class url
{

public:
    url() = default;
    ~url() = default;

    int regex_urlstr(std::string_view strurl,
                     bool parse_query = false);

    int regex_queryparams(std::string_view query);

    static std::string url_encode(const std::string& input);
    
    static std::string url_decode(const std::string& input);

    auto scheme(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.scheme_);
    }
    
    auto authority(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.authority_);
    }
    
    auto user(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.user_);
    }

    auto host(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.host_);
    }

    auto port(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.port_);
    }

    auto path(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.path_);
    }

    auto query(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.query_);
    }

    auto fragment(this auto&& self)
    {
        return std::forward_like<decltype(self)>(self.fragment_);
    }
    

protected:

    //!
    std::string scheme_;
    //!
    std::string authority_;
    //!
    std::string user_;
    //!
    std::string host_;
    //!
    int port_ = -1;
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
