#include "zce/predefine.h"
#include "zce/buffer/queue_buffer.h"

namespace zce
{
//=========================================================================================
//!class queue_buffer
//!队列形状的buffer，
queue_buffer::queue_buffer(size_t size_of_capacity)
{
    initialize(size_of_capacity);
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
    start_point_(others.start_point_),
    end_point_(others.end_point_),
    buffer_data_(nullptr)
{
    if (size_of_capacity_)
    {
        buffer_data_ = new char[size_of_capacity_];
        if (end_point_)
        {
            ::memcpy(buffer_data_, others.buffer_data_, end_point_);
        }
    }
}

queue_buffer::queue_buffer(queue_buffer&& others) noexcept :
    size_of_capacity_(others.size_of_capacity_),
    start_point_(others.start_point_),
    end_point_(others.end_point_),
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
    start_point_ = others.start_point_;
    end_point_ = others.end_point_;
    buffer_data_ = new char[size_of_capacity_];
    if (end_point_)
    {
        ::memcpy(buffer_data_, others.buffer_data_, end_point_);
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
    start_point_ = others.start_point_;
    end_point_ = others.end_point_;
    buffer_data_ = others.buffer_data_;

    others.size_of_capacity_ = 0;
    others.start_point_ = 0;
    others.end_point_ = 0;
    others.buffer_data_ = nullptr;
    return *this;
}

bool queue_buffer::initialize(size_t size_of_capacity)
{
    assert(buffer_data_ == nullptr);
    size_of_capacity_ = size_of_capacity;
    buffer_data_ = new char[size_of_capacity];

    clear();
    return true;
}

void queue_buffer::clear()
{
    start_point_ = 0;
    end_point_ = 0;
#if defined DEBUG || defined _DEBUG
    //::memset(buffer_data_, 0, size_of_capacity_);
#endif
}

//继续在尾部增加数据
bool queue_buffer::push_back(const char* data, const size_t szdata)
{
    if (szdata > size_of_capacity_ - end_point_)
    {
        return false;
    }
    ::memcpy(buffer_data_ + end_point_, data, szdata);
    end_point_ += szdata;
    return true;
}

bool queue_buffer::push_back(size_t szdata)
{
    if (szdata > size_of_capacity_ - end_point_)
    {
        return false;
    }
    end_point_ += szdata;
    return true;
}

//读取数据
void queue_buffer::pop_front(char* data,
                             size_t& szdata,
                             bool squeeze_front)
{
    auto data_size = end_point_ - start_point_;
    if (szdata < data_size)
    {
        //不用完整取出，取出一部分
        ::memcpy(data, buffer_data_ + start_point_, szdata);
        //如果需要腾出空间，
        if (squeeze_front)
        {
            ::memmove(buffer_data_,
                      buffer_data_ + start_point_ + szdata,
                      data_size - szdata);
            end_point_ = end_point_ - start_point_ - szdata;
            start_point_ = 0;
        }
        else
        {
            start_point_ += szdata;
        }
    }
    else
    {
        //完全取走了数据
        ::memcpy(data, buffer_data_ + start_point_, data_size);
        szdata = data_size;
        if (squeeze_front)
        {
            clear();
        }
        else
        {
            start_point_ += szdata;
        }
    }
    return;
}

void queue_buffer::pop_front(size_t& szdata,
                             bool squeeze_front)
{
    auto data_size = end_point_ - start_point_;
    if (szdata < data_size)
    {
        //如果需要腾出空间，
        if (squeeze_front)
        {
            ::memmove(buffer_data_,
                      buffer_data_ + start_point_ + szdata,
                      data_size - szdata);
            end_point_ = end_point_ - start_point_ - szdata;
            start_point_ = 0;
        }
        else
        {
            start_point_ += szdata;
        }
    }
    else
    {
        szdata = data_size;
        if (squeeze_front)
        {
            clear();
        }
        else
        {
            start_point_ += szdata;
        }
    }
    return;
}

//挤出前面的空间
void queue_buffer::squeeze_front()
{
    if (start_point_)
    {
        if (start_point_ == end_point_)
        {
            clear();
        }
        else
        {
            ::memmove(buffer_data_,
                      buffer_data_ + start_point_,
                      end_point_ - start_point_);
            end_point_ = end_point_ - start_point_;
            start_point_ = 0;
        }
    }
}

//从start_point_+ offset偏移开始，填充数据data,长度为szdata,
bool queue_buffer::set(
    const char* data,
    const size_t szdata,
    const size_t offset)
{
    //超出现有的数据空间
    if (szdata + offset + start_point_ > end_point_)
    {
        return false;
    }
    ::memcpy(buffer_data_ + start_point_ + offset, data, szdata);
    return true;
}

//读取数据
bool queue_buffer::get(char* data,
                       size_t& szdata,
                       size_t offset,
                       bool whole) const
{
    auto data_size = end_point_ - start_point_;
    if (szdata < data_size - offset)
    {
        //如果空间不够，而且要求完整
        if (whole)
        {
            return false;
        }
        else
        {
            //不用完整取出，取出一部分
            ::memcpy(data, buffer_data_ + start_point_ + offset, szdata);
        }
    }
    else
    {
        ::memcpy(data,
                 buffer_data_ + start_point_ + offset,
                 data_size - offset);
    }
    return true;
}
}