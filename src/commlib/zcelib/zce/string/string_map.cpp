#include "zce/predefine.h"
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
}
