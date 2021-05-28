/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief      循环的一个buffer，里面存放数据，操作方式是fifo的。
*             存放是使用buffer和长度
*             取出的时候也可以根据长度参数取出对应长度的数据，
*
*
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
//================================================================================
class buffer_cycle
{
public:
    //构造，析构，赋值函数，为了加速，谢了右值处理的函数
    buffer_cycle() = default;
    ~buffer_cycle();
    buffer_cycle(const buffer_cycle & others);
    buffer_cycle(buffer_cycle && others) noexcept;
    buffer_cycle& operator=(const buffer_cycle & others);
    buffer_cycle& operator=(buffer_cycle && others) noexcept;

    void clear();

    bool initialize(size_t size_of_buffer);

    ///得到FREE空间的快照
    size_t free();

    ///容量
    inline size_t capacity()
    {
        return size_of_cycle_ - JUDGE_FULL_INTERVAL;
    }

    ///得到是否为空
    inline bool empty()
    {
        return cycbuf_begin_ == cycbuf_end_;
    }

    ///得到是否空
    inline bool full()
    {
        return free() == 0;
    }

    //将一个NODE放入尾部
    bool push_end(const char * data, size_t data_len);

    //
    bool pop_front(char * const data, size_t data_len);

protected:

    ///判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    ///cycbuf_begin_ = cycbuf_end_ 表示队列为NULL
    ///cycbuf_begin_ = cycbuf_end_ + JUDGE_FULL_INTERVAL 表示队列满
    static const size_t   JUDGE_FULL_INTERVAL = 8;

protected:

    ///deque的长度,必须>JUDGE_FULL_INTERVAL
    size_t size_of_cycle_ = 0;

    ///两个关键内部指针,避免编译器优化
    ///环形队列开始的地方，这个地方必现是机器字长
    size_t cycbuf_begin_ = 0;
    ///环行队列结束的地方，这个地方必现是机器字长
    size_t cycbuf_end_ = 0;

    char* cycbuf_data_ = nullptr;
};

//================================================================================
class buffer_queue
{
public:
    //构造，析构，赋值函数，为了加速，谢了右值处理的函数
    buffer_queue() = default;
    ~buffer_queue();
    buffer_queue(const buffer_queue& others);
    buffer_queue(buffer_queue&& others) noexcept;
    buffer_queue& operator=(const buffer_queue& others);
    buffer_queue& operator=(buffer_queue&& others) noexcept;

    //
    bool initialize(size_t size_of_buffer);

    //
    void clear();

    //填充数据
    void fill_write_data(const size_t szdata, const char* data);
    //读取数据
    void get_read_data(size_t& szdata, char* data);

    //
    inline char* get_use_point()
    {
        return buffer_data_ + size_of_use_;
    }
    //
    inline size_t free()
    {
        return size_of_buffer_ - size_of_use_;
    }

    //是否已经满了
    inline bool full()
    {
        if (size_of_use_ >= size_of_buffer_)
        {
            return true;
        }
        return false;
    }

    //是否为空
    inline bool empty()
    {
        if (size_of_use_ == 0)
        {
            return true;
        }
        return false;
    };

public:

    //当前要使用的缓冲长度，当前处理的帧的长度,没有得到长度前填写0
    size_t      size_of_buffer_ = 0;

    //使用的尺寸
    size_t      size_of_use_ = 0;

    //数据缓冲区
    char       *buffer_data_ = nullptr;
};
}