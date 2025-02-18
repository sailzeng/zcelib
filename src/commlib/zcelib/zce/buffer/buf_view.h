#pragma once

namespace zce
{
//<basic_buf_view>
template<typename _CharT >
class basic_buf_view
{
public:

    constexpr basic_buf_view() noexcept
        : _M_len{ 0 }, _M_str{ nullptr }
    { }

    constexpr basic_buf_view(const basic_buf_view&) noexcept = default;

    constexpr basic_buf_view(const _CharT* __str, size_type __len) noexcept
        : len_{ __len }, data_{ __str }
    { }

    //...

private:

    size_t len_;
    const T* data_;
};
}
