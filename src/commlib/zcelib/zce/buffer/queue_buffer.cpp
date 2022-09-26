#include "zce/predefine.h"
#include "zce/buffer/queue_buffer.h"

namespace zce
{
//=========================================================================================
//!class queue_buffer
//!队列形状的buffer，
queue_buffer::queue_buffer(size_t size_of_buffer)
{
    initialize(size_of_buffer);
}

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

//填充数据
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

//从偏移offset开始，填充数据data,长度为szdata,
bool queue_buffer::set(const size_t offset,
                       const char* data,
                       const size_t szdata)
{
    if (szdata + offset > size_of_capacity_)
    {
        return false;
    }
    ::memcpy(buffer_data_ + offset, data, szdata);
    size_of_use_ = szdata + offset;
    return true;
}

//读取数据
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

//继续在尾部增加数据
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