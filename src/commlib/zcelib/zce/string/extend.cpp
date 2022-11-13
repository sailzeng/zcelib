#include "zce/predefine.h"
#include "zce/string/extend.h"

namespace zce
{
void _string_format_outlist(std::string& foo_string,
                            const char*& foo_fmt_spec)
{
    foo_string.append(foo_fmt_spec);
}

//如果没有参数的特殊处理
void _sformat_outlist(char*& foo_buffer,
                      size_t& foo_max_len,
                      size_t& foo_use_len,
                      const char*& foo_fmt_spec)
{
    size_t len_of_str = ::strlen(foo_fmt_spec);
    if (len_of_str > foo_max_len)
    {
        foo_max_len = 0;
        foo_use_len += foo_max_len;
        ::memcpy(foo_buffer, foo_fmt_spec, foo_max_len);
    }
    else
    {
        foo_max_len -= len_of_str;
        foo_use_len += len_of_str;
        ::memcpy(foo_buffer, foo_fmt_spec, len_of_str);
    }
}

void _splice_outlist(char*& /*foo_buffer*/,
                     size_t /*foo_max_len*/,
                     size_t& /*foo_use_len*/,
                     char /*separator_char*/)
{
}

void _string_splice_list(std::string& /*foo_string*/,
                         char /*separator_char*/)
{
}
}