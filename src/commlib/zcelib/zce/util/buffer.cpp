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
        return size_of_cycle_ + cycbuf_end_ - cycbuf_begin_;
    }
}

//得到FREE空间
size_t cycle_buffer::free()
{
    return size_of_cycle_ - size() - JUDGE_FULL_INTERVAL;
}

//将一个data_len长度数据data放入cycle_buffer尾部
bool cycle_buffer::push_end(const char * data,
                            size_t data_len,
                            char *&write_ptr)
{
    assert(data != NULL);
    write_ptr = nullptr;
    //检查队列的空间是否够用
    if (free() < data_len)
    {
        return false;
    }

    write_ptr = cycbuf_data_ + cycbuf_end_;
    //如果绕圈
    if (write_ptr + data_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - cycbuf_end_;
        size_t second = data_len - first;
        ::memcpy(write_ptr, data, first);
        ::memcpy(cycbuf_data_, data + first, second);
        cycbuf_end_ = second;
    }
    //如果可以一次拷贝完成
    else
    {
        ::memcpy(write_ptr, data, data_len);
        cycbuf_end_ += data_len;
    }
    return true;
}

//在尾部填充长度为fill_len，的fill_data的字符
bool cycle_buffer::push_end(char fill_ch,
                            size_t fill_len,
                            char *&write_ptr)
{
    //检查队列的空间是否够用
    if (free() < fill_len)
    {
        return false;
    }

    write_ptr = cycbuf_data_ + cycbuf_end_;
    //如果绕圈
    if (write_ptr + fill_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - cycbuf_end_;
        size_t second = fill_len - first;
        ::memset(write_ptr, fill_ch, first);
        ::memset(cycbuf_data_, fill_ch, second);
        cycbuf_end_ = second;
    }
    //如果可以一次拷贝完成
    else
    {
        ::memset(write_ptr, fill_ch, fill_len);
        cycbuf_end_ += fill_len;
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

bool cycle_buffer::pop_front(size_t data_len)
{
    assert(data_len > 0);
    if (size() < data_len)
    {
        return false;
    }
    cycbuf_begin_ = (cycbuf_begin_ + data_len) % size_of_cycle_;
    return true;
}

///从偏移量的位置开始，填充
bool cycle_buffer::set_data(size_t pos,
                            const char * data,
                            size_t data_len,
                            char *&write_ptr)
{
    //检查队列的空间是否够用
    if (capacity() < pos + data_len)
    {
        return false;
    }

    size_t w_pos = ((cycbuf_begin_ + pos) % size_of_cycle_);
    write_ptr = cycbuf_data_ + w_pos;
    //cycbuf_end_ += ((cycbuf_begin_ + pos + data_len) % size_of_cycle_);
    if (write_ptr + data_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - w_pos;
        size_t second = data_len - first;
        ::memcpy(write_ptr, data, first);
        ::memcpy(cycbuf_data_, data, second);
        cycbuf_end_ = second;
    }
    else
    {
        ::memcpy(write_ptr, data, data_len);
        cycbuf_end_ = w_pos + data_len;
    }
    return true;
}

///重新缩小调整整个buffer的尺寸，（增加大不是提供一堆无意义的数据）
bool cycle_buffer::reduce(size_t buf_len)
{
    if (size() < buf_len)
    {
        return false;
    }
    cycbuf_end_ += (cycbuf_begin_ + buf_len) % size_of_cycle_;
    return true;
}

///从pos（相对于cycbuf_begin_）读取数据，
bool cycle_buffer::get_data(size_t pos,
                            char *data,
                            size_t read_len)
{
    if (size() < pos + read_len)
    {
        return false;
    }
    size_t r_pos = ((cycbuf_begin_ + pos) % size_of_cycle_);
    char *read_ptr = cycbuf_data_ + r_pos;
    if (read_ptr + read_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - r_pos;
        size_t second = read_len - first;
        ::memcpy(data, read_ptr, first);
        ::memcpy(data, cycbuf_data_, second);
    }
    else
    {
        ::memcpy(data, read_ptr, read_len);
    }
    return true;
}
///从绝对位置read_ptr开始读取数据
bool cycle_buffer::get_data(const char *read_ptr,
                            char *data,
                            size_t read_len)
{
    assert(read_ptr >= cycbuf_data_ && read_ptr < cycbuf_data_ + size_of_cycle_);
    if (size() < read_ptr - cycbuf_data_ + read_len)
    {
        return false;
    }
    size_t r_pos = read_ptr - cycbuf_data_;
    if (read_ptr + read_len > cycbuf_data_ + size_of_cycle_)
    {
        size_t first = size_of_cycle_ - r_pos;
        size_t second = read_len - first;
        ::memcpy(data, read_ptr, first);
        ::memcpy(data, cycbuf_data_, second);
    }
    else
    {
        ::memcpy(data, read_ptr, read_len);
    }
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
queue_buffer::queue_buffer(const queue_buffer & others) :
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

queue_buffer::queue_buffer(queue_buffer && others) noexcept :
    size_of_capacity_(others.size_of_capacity_),
    size_of_use_(others.size_of_use_),
    buffer_data_(others.buffer_data_)
{
    others.buffer_data_ = nullptr;
}

//赋值函数
queue_buffer& queue_buffer::operator=(const queue_buffer & others)
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
queue_buffer& queue_buffer::operator=(queue_buffer && others) noexcept
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
bool queue_buffer::get(char* data, size_t & szdata)
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