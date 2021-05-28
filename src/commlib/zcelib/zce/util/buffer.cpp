#include "zce/predefine.h"
#include "zce/util/buffer.h"

namespace zce
{
//=========================================================================================
//class buffer_cycle

buffer_cycle::~buffer_cycle()
{
    if (cycbuf_data_)
    {
        delete[] cycbuf_data_;
        cycbuf_data_ = nullptr;
    }
}

buffer_cycle::buffer_cycle(const buffer_cycle & others)
    :size_of_cycle_(others.size_of_cycle_)
    , cycbuf_begin_(others.cycbuf_begin_)
    , cycbuf_end_(others.cycbuf_end_)
{
    if (size_of_cycle_)
    {
        cycbuf_data_ = new char[size_of_cycle_];
        ::memcpy(cycbuf_data_, others.cycbuf_data_, size_of_cycle_);
    }
}

buffer_cycle::buffer_cycle(buffer_cycle && others) noexcept
    :size_of_cycle_(others.size_of_cycle_)
    , cycbuf_begin_(others.cycbuf_begin_)
    , cycbuf_end_(others.cycbuf_end_)
    , cycbuf_data_(others.cycbuf_data_)
{
    others.size_of_cycle_ = 0;
    others.cycbuf_begin_ = 0;
    others.cycbuf_end_ = 0;
    others.cycbuf_data_ = nullptr;
}

buffer_cycle& buffer_cycle::operator=(const buffer_cycle & others)
{
    if (this == &others)
    {
        return *this;
    }
    if (cycbuf_data_)
    {
        delete cycbuf_data_;
        cycbuf_data_ = nullptr;
    }
    size_of_cycle_ = others.size_of_cycle_;
    cycbuf_begin_ = others.cycbuf_begin_;
    cycbuf_end_ = others.cycbuf_end_;
    if (size_of_cycle_)
    {
        cycbuf_data_ = new char[size_of_cycle_];
        ::memcpy(cycbuf_data_, others.cycbuf_data_, size_of_cycle_);
    }
    return *this;
}

buffer_cycle& buffer_cycle::operator=(buffer_cycle && others) noexcept
{
    if (this == &others)
    {
        return *this;
    }
    size_of_cycle_ = others.size_of_cycle_;
    cycbuf_begin_ = others.cycbuf_begin_;
    cycbuf_end_ = others.cycbuf_end_;
    cycbuf_data_ = others.cycbuf_data_;
    others.size_of_cycle_ = 0;
    others.cycbuf_begin_ = 0;
    others.cycbuf_end_ = 0;
    others.cycbuf_data_ = nullptr;
    return *this;
}

void buffer_cycle::clear()
{
    cycbuf_begin_ = 0;
    cycbuf_end_ = 0;
#if defined DEBUG || defined _DEBUG
    ::memset(cycbuf_data_, 0, size_of_cycle_);
#endif
}

bool buffer_cycle::initialize(size_t size_of_buffer)
{
    assert(cycbuf_data_ == nullptr);
    size_of_cycle_ = size_of_buffer + JUDGE_FULL_INTERVAL;

    cycbuf_data_ = new char[size_of_cycle_];

    clear();
    return true;
}

///得到FREE空间的快照
size_t buffer_cycle::free()
{
    //计算尺寸
    size_t sz_free;
    if (cycbuf_begin_ == cycbuf_end_)
    {
        sz_free = size_of_cycle_;
    }
    else if (cycbuf_begin_ < cycbuf_end_)
    {
        sz_free = size_of_cycle_ - (cycbuf_end_ - cycbuf_begin_);
    }
    else
    {
        sz_free = cycbuf_begin_ - cycbuf_end_;
    }
    sz_free = sz_free - JUDGE_FULL_INTERVAL;
    return sz_free;
}

//将一个data放入尾部
bool buffer_cycle::push_end(const char * data, size_t data_len)
{
    assert(data != NULL);
    //检查队列的空间是否够用
    if (free() < data_len)
    {
        return false;
    }

    //如果空间足够
    char* pend = cycbuf_data_ + cycbuf_end_;

    //如果绕圈
    if (pend + data_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - cycbuf_end_;
        size_t second = data_len - first;
        ::memcpy(pend, data, first);
        ::memcpy(cycbuf_data_, data + first, second);
        cycbuf_end_ = second;
    }
    //如果可以一次拷贝完成
    else
    {
        memcpy(pend, data, data_len);
        cycbuf_end_ += data_len;
    }
    return true;
}

//
bool buffer_cycle::pop_front(char * const data, size_t data_len)
{
    assert(data != NULL && data_len > 0);

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    char* pbegin = cycbuf_data_ + cycbuf_begin_;

    //如果被分为2截
    if (pbegin + data_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - cycbuf_begin_;
        size_t second = data_len - first;
        ::memcpy(data, pbegin, first);
        ::memcpy(data + first, cycbuf_data_, second);
        cycbuf_begin_ = second;
    }
    else
    {
        ::memcpy(data, pbegin, data_len);
        cycbuf_begin_ += data_len;
    }

    assert(cycbuf_begin_ <= size_of_cycle_);
    return true;
}

//=========================================================================================
//class buffer_queue

buffer_queue::~buffer_queue()
{
    if (buffer_data_)
    {
        delete[] buffer_data_;
        buffer_data_ = nullptr;
    }
}
buffer_queue::buffer_queue(const buffer_queue& others) :
    size_of_buffer_(others.size_of_buffer_),
    size_of_use_(others.size_of_use_),
    buffer_data_(nullptr)
{
    if (size_of_buffer_)
    {
        buffer_data_ = new char[size_of_buffer_];
        if (size_of_use_)
        {
            ::memcpy(buffer_data_, others.buffer_data_, size_of_use_);
        }
    }
}

buffer_queue::buffer_queue(buffer_queue&& others) noexcept :
    size_of_buffer_(others.size_of_buffer_),
    size_of_use_(others.size_of_use_),
    buffer_data_(others.buffer_data_)
{
    others.buffer_data_ = nullptr;
}

//赋值函数
buffer_queue& buffer_queue::operator=(const buffer_queue& others)
{
    if (buffer_data_)
    {
        delete[] buffer_data_;
        buffer_data_ = nullptr;
    }
    if (this == &others)
    {
        return *this;
    }
    size_of_buffer_ = others.size_of_buffer_;
    size_of_use_ = others.size_of_use_;
    buffer_data_ = new char[size_of_buffer_];
    if (size_of_use_)
    {
        ::memcpy(buffer_data_, others.buffer_data_, size_of_use_);
    }
    return *this;
}
//右值赋值函数，
buffer_queue& buffer_queue::operator=(buffer_queue&& others) noexcept
{
    if (this == &others)
    {
        return *this;
    }
    size_of_buffer_ = others.size_of_buffer_;
    size_of_use_ = others.size_of_use_;
    buffer_data_ = others.buffer_data_;

    others.size_of_buffer_ = 0;
    others.size_of_use_ = 0;
    others.buffer_data_ = nullptr;
    return *this;
}

bool buffer_queue::initialize(size_t size_of_buffer)
{
    assert(buffer_data_ == nullptr);
    size_of_buffer_ = size_of_buffer;
    buffer_data_ = new char[size_of_buffer];

    clear();
    return true;
}

void buffer_queue::clear()
{
    size_of_use_ = 0;
#if defined DEBUG || defined _DEBUG
    ::memset(buffer_data_, 0, size_of_buffer_);
#endif
}

//
void buffer_queue::fill_write_data(const size_t szdata, const char* data)
{
    ::memcpy(buffer_data_, data, szdata);
    size_of_use_ += szdata;
    //
}
//
void buffer_queue::get_read_data(size_t& szdata, char* data)
{
    ::memcpy(data, buffer_data_, szdata);
}
}