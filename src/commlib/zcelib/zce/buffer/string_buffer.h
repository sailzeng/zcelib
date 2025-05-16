#pragma once

namespace zce
{
//
template<
    class CharT,
    class Traits = std::char_traits<CharT> >
class basic_string_buffer
{
public:

    constexpr basic_string_buffer() noexcept = default;
    constexpr basic_string_buffer(CharT* buf, size_t capacity) noexcept :
        buf_(buf), capacity_(capacity), use_(0)
    {
    }
    constexpr basic_string_buffer(CharT* buf, size_t capacity, size_t use) noexcept :
        buf_(buf), capacity_(capacity), use_(use)
    {
    }
    constexpr basic_string_buffer(const basic_string_buffer& cptr) noexcept = default;
    basic_string_buffer& operator=(const basic_string_buffer& cptr) = default;
    ~basic_string_buffer() = default;
    basic_string_buffer(basic_string_buffer&&) noexcept = delete;
    basic_string_buffer& operator=(basic_string_buffer&&) = delete;

    const CharT operator[](size_t id) const
    {
        return buf_[id];
    }
    bool empty() const
    {
        if (buf_)
        {
            return true;
        }
        return false;
    }

    CharT* data() const
    {
        return  buf_;
    }

    size_t capacity() const
    {
        return capacity_;
    }
    size_t size() const
    {
        return use_;
    }

    void assign(const CharT* buf, size_t len)
    {
        assert(capacity_ >= len);
        memcpy(buf_, buf, sizeof(CharT) * len);
        use_ = len;
    }

    void push(const CharT* buf, size_t len)
    {
        assert(capacity_ - use_ >= len);
        memcpy(buf_ + sizeof(CharT) * use_, buf, sizeof(CharT) * len);
        use_ += len;
    }

protected:

    //!
    CharT* buf_ = nullptr;
    //!
    size_t capacity_ = 0;
    //!
    size_t use_ = 0;
};

using string_buf = basic_string_buffer<char>;
using wstring_buf = basic_string_buffer<wchar_t>;
}
