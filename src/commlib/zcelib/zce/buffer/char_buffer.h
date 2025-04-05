#pragma once

namespace zce
{
//
template<
    class CharT,
    class Traits = std::char_traits<CharT> >
class basic_char_buffer
{
public:

    constexpr basic_char_buffer() noexcept = default;
    constexpr basic_char_buffer(CharT* buf, size_t capacity) noexcept :
        buf_(buf), capacity_(capacity), use_(0)
    {
    }
    constexpr basic_char_buffer(CharT* buf, size_t capacity, size_t use) noexcept :
        buf_(buf), capacity_(capacity), use_(use)
    {
    }
    constexpr basic_char_buffer(const basic_char_buffer& cptr) noexcept = default;
    basic_char_buffer& operator=(const basic_char_buffer& cptr) = default;
    ~basic_char_buffer() = default;
    basic_char_buffer(basic_char_buffer&&) noexcept = delete;
    basic_char_buffer& operator=(basic_char_buffer&&) = delete;

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

    void set(CharT* buf, size_t len)
    {
        ZCE_ASSERT(capacity_ >= len);
        memcpy(buf_, buf, sizeof(CharT) * len);
        use_ = len;
    }

    void push(CharT* buf, size_t len)
    {
        ZCE_ASSERT(capacity_ - use_ >= len);
        memcpy(buf_ + sizeof(CharT) * use_, buf, sizeof(CharT) * len);
        use_ += len;
    }

public:

    //!
    CharT* buf_ = nullptr;
    //!
    size_t capacity_ = 0;
    //!
    size_t use_ = 0;
};

typedef basic_char_buffer<char> char_buf;
typedef basic_char_buffer<wchar_t> wchar_buf;
}
