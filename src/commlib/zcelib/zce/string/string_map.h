#pragma once

namespace zce
{

class string_map  
{  

public:

    string_map() = default;  
    ~string_map() = default;  

    //!  
    bool has_key(const std::string& key) const;  

    //!  
    bool get_value(const std::string& key, std::string& value) const;

    //!  
    const char* get_value(const std::string& key) const;

    template<typename T>  
    T get_value(const std::string& key) const;


    template<typename T>  
    bool get_value(const std::string& key, T& value) const;

    template<typename T>
    bool set_value(const std::string& key, T& value);

    const std::string& operator[](const std::string& key) const;

    std::string& operator[](const std::string& key);

    int parse(std::string_view query,
              char delimiter);

protected:  
    // Ignore case  
    struct lessofcasestr
    {  
    public:  
        bool operator()(const std::string& src, const std::string& dst) const  
        {  
            return (strcasecmp(src.c_str(), dst.c_str()) < 0);  
        }  
    };  

    /// Command parameters  
    using MAP_STRING = std::map<std::string, std::string, lessofcasestr>;  

    //!  
    MAP_STRING map_string_;  
};
 
}
