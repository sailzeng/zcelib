#include "zce/predefine.h"
#include "zce/util/buffer.h"

namespace zce
{
//=========================================================================================
//class buffer_cycle

void buffer_cycle::clear()
{
    cycbuf_begin_ = 0;
    cycbuf_end_ = 0;
#if defined DEBUG || defined _DEBUG
    ::memset(cycbuf_data_, 0, size_of_deque_);
#endif
}

bool buffer_cycle::initialize(size_t size_of_deque)
{
    size_of_deque_ = size_of_deque + JUDGE_FULL_INTERVAL;

    cycbuf_data_ = new char[size_of_deque_];

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
        sz_free = size_of_deque_;
    }
    else if (cycbuf_begin_ < cycbuf_end_)
    {
        sz_free = size_of_deque_ - (cycbuf_end_ - cycbuf_begin_);
    }
    else
    {
        sz_free = cycbuf_begin_ - cycbuf_end_;
    }
    sz_free = sz_free - JUDGE_FULL_INTERVAL;
    return sz_free;
}

///容量
size_t buffer_cycle::capacity()
{
    return size_of_deque_ - JUDGE_FULL_INTERVAL;
}

///得到是否为空
bool buffer_cycle::empty()
{
    return cycbuf_begin_ == cycbuf_end_;
}

///得到是否空
bool buffer_cycle::full()
{
    return free() == 0;
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
    if (pend + data_len > cycbuf_data_ + size_of_deque_)
    {
        size_t first = size_of_deque_ - cycbuf_end_;
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
    if (pbegin + data_len > cycbuf_data_ + size_of_deque_)
    {
        size_t first = size_of_deque_ - cycbuf_begin_;
        size_t second = data_len - first;
        memcpy(data, pbegin, first);
        memcpy(data + first, cycbuf_data_, second);
        cycbuf_begin_ = second;
    }
    else
    {
        memcpy(data, pbegin, data_len);
        cycbuf_begin_ += data_len;
    }

    assert(cycbuf_begin_ <= size_of_deque_);
    return true;
}

//=========================================================================================
//class buffer_queue

buffer_queue::buffer_queue() :
    size_of_buffer_(0),
    size_of_use_(0)
{
}

//
buffer_queue::~buffer_queue()
{
}

//
void buffer_queue::fill_write_data(const size_t szdata, const char* data)
{
    memcpy(buffer_data_, data, szdata);
    size_of_use_ += szdata;
    //
}
//
void buffer_queue::get_read_data(size_t& szdata, char* data)
{
    memcpy(data, buffer_data_, szdata);
}

void buffer_queue::clear()
{
    size_of_use_ = 0;
    size_of_buffer_ = 0;
}
}