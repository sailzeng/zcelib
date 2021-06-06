#include "zce/predefine.h"
#include "zce/util/buffer.h"

namespace zce
{
//=========================================================================================
//class cycle_buffer

cycle_buffer::~cycle_buffer()
{
    if (cycbuf_data_)
    {
        delete[] cycbuf_data_;
        cycbuf_data_ = nullptr;
    }
}

cycle_buffer::cycle_buffer(const cycle_buffer & others)
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

cycle_buffer::cycle_buffer(cycle_buffer && others) noexcept
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

cycle_buffer& cycle_buffer::operator=(const cycle_buffer & others)
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

cycle_buffer& cycle_buffer::operator=(cycle_buffer && others) noexcept
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

void cycle_buffer::clear()
{
    cycbuf_begin_ = 0;
    cycbuf_end_ = 0;
#if defined DEBUG || defined _DEBUG
    ::memset(cycbuf_data_, 0, size_of_cycle_);
#endif
}

bool cycle_buffer::initialize(size_t size_of_buffer)
{
    assert(cycbuf_data_ == nullptr);
    size_of_cycle_ = size_of_buffer + JUDGE_FULL_INTERVAL;

    cycbuf_data_ = new char[size_of_cycle_];

    clear();
    return true;
}

//得到已经使用空间的尺寸
size_t cycle_buffer::size()
{
    if (cycbuf_begin_ == cycbuf_end_)
    {
        return 0;
    }
    else if (cycbuf_begin_ < cycbuf_end_)
    {
        return (cycbuf_end_ - cycbuf_begin_);
    }
    else
    {
        return size_of_cycle_ + cycbuf_end_ - cycbuf_begin_ - JUDGE_FULL_INTERVAL;
    }
}

//得到FREE空间
size_t cycle_buffer::free()
{
    return size_of_cycle_ - size() - JUDGE_FULL_INTERVAL;
}

//将一个data_len长度数据data放入cycle_buffer尾部
bool cycle_buffer::push_end(const char * data, size_t data_len)
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

//从cycle_buffer头部，取出一个data_len长度的数据放入data
bool cycle_buffer::pop_front(char * const data, size_t data_len)
{
    assert(data != NULL && data_len > 0);

    //检查是否有数据给他取走
    if (size() < data_len)
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
//class queue_buffer

queue_buffer::~queue_buffer()
{
    if (buffer_data_)
    {
        delete[] buffer_data_;
        buffer_data_ = nullptr;
    }
}
queue_buffer::queue_buffer(const queue_buffer& others) :
    size_of_capacity_(others.size_of_capacity_),
    size_of_use_(others.size_of_use_),
    buffer_data_(nullptr)
{
    if (size_of_capacity_)
    {
        buffer_data_ = new char[size_of_capacity_];
        if (size_of_use_)
        {
            ::memcpy(buffer_data_, others.buffer_data_, size_of_use_);
        }
    }
}

queue_buffer::queue_buffer(queue_buffer&& others) noexcept :
    size_of_capacity_(others.size_of_capacity_),
    size_of_use_(others.size_of_use_),
    buffer_data_(others.buffer_data_)
{
    others.buffer_data_ = nullptr;
}

//赋值函数
queue_buffer& queue_buffer::operator=(const queue_buffer& others)
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
    size_of_capacity_ = others.size_of_capacity_;
    size_of_use_ = others.size_of_use_;
    buffer_data_ = new char[size_of_capacity_];
    if (size_of_use_)
    {
        ::memcpy(buffer_data_, others.buffer_data_, size_of_use_);
    }
    return *this;
}
//右值赋值函数，
queue_buffer& queue_buffer::operator=(queue_buffer&& others) noexcept
{
    if (this == &others)
    {
        return *this;
    }
    size_of_capacity_ = others.size_of_capacity_;
    size_of_use_ = others.size_of_use_;
    buffer_data_ = others.buffer_data_;

    others.size_of_capacity_ = 0;
    others.size_of_use_ = 0;
    others.buffer_data_ = nullptr;
    return *this;
}

bool queue_buffer::initialize(size_t size_of_buffer)
{
    assert(buffer_data_ == nullptr);
    size_of_capacity_ = size_of_buffer;
    buffer_data_ = new char[size_of_buffer];

    clear();
    return true;
}

void queue_buffer::clear()
{
    size_of_use_ = 0;
#if defined DEBUG || defined _DEBUG
    ::memset(buffer_data_, 0, size_of_capacity_);
#endif
}

//
bool queue_buffer::set(const char* data, const size_t szdata)
{
    if (szdata > size_of_capacity_)
    {
        return false;
    }
    ::memcpy(buffer_data_, data, szdata);
    size_of_use_ = szdata;
    return true;
}

//
bool queue_buffer::get(char* data, size_t& szdata)
{
    if (szdata < size_of_use_)
    {
        return false;
    }
    ::memcpy(data, buffer_data_, szdata);
    szdata = size_of_use_;
    return true;
}

bool queue_buffer::add(const char* data, const size_t szdata)
{
    if (szdata > size_of_capacity_ - size_of_use_)
    {
        return false;
    }
    ::memcpy(buffer_data_ + size_of_use_, data, szdata);
    size_of_use_ = szdata;
    return true;
}
}