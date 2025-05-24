#include "zce/predefine.h"
#include "zce/os_adapt/string.h"
#include "zce/string/string_map.h"

namespace zce
{
//! 
bool string_map::has_key(const std::string& key) const
{
    auto iter = map_string_.find(key);
    return iter != map_string_.end();
}

//!
bool string_map::get_value(const std::string& key, std::string& value)  const
{
    
    auto iter = map_string_.find(key);
    if (iter != map_string_.end())
    {
        value = iter->second;
        return true;
    }
    return false;
}

//!
const char* string_map::get_value(const std::string& key)  const
{
    
    auto iter = map_string_.find(key);
    if (iter != map_string_.end())
    {
        return iter->second.c_str();
    }
    return nullptr;
}

const std::string& string_map::operator[](const std::string& key) const
{
    auto it = map_string_.find(key);
    if (it != map_string_.end())
    {
        return it->second;
    }
    else
    {
        static std::string empty_str;
        return empty_str;
    }
}

std::string& string_map::operator[](const std::string& key)
{
    return map_string_[key];
}

int string_map::parse(std::string_view query,
                      char  delimiter)
{

    std::istringstream query_stream(query.data());
    std::string key_value_pair;

    while (std::getline(query_stream, key_value_pair, delimiter))
    {
        // Remove leading and trailing whitespace
        zce::stdstr_trim(key_value_pair);
        // Skip empty pairs
        if (key_value_pair.empty())
        {
            continue;
        }
        // Split the key-value pair
        auto pos = key_value_pair.find('=');
        if (pos != std::string::npos)
        {
            std::string key = key_value_pair.substr(0, pos);
            std::string value = key_value_pair.substr(pos + 1);
            map_string_[key] = value;
        }
        else
        {
            map_string_[key_value_pair] = "";
        }
    }

    return 0;
}

}
