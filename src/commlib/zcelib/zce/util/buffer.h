/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief      cycle_buffer :
*             循环的一个buffer，里面存放数据，操作方式是fifo的。
*             存放是使用buffer和长度
*             取出的时候也可以根据长度参数取出对应长度的数据，
*             queue_buffer :
*             一个只能向尾部添加数据的BUFFER，取出数据要全部取出
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
class cycle_buffer
{
public:
    //构造，析构，赋值函数，为了加速，写了右值处理的函数
    cycle_buffer() = default;
    ~cycle_buffer();
    cycle_buffer(const cycle_buffer & others);
    cycle_buffer(cycle_buffer && others) noexcept;
    cycle_buffer& operator=(const cycle_buffer & others);
    cycle_buffer& operator=(cycle_buffer && others) noexcept;

    void clear();

    bool initialize(size_t size_of_buffer);

    ///得到已经使用空间的尺寸
    size_t size();

    ///得到FREE空间的尺寸
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

    ///将一个data_len长度数据data放入cycle_buffer尾部
    bool push_end(const char * data, size_t data_len);

    ///从cycle_buffer头部，取出一个data_len长度的数据放入data
    bool pop_front(char * const data, size_t data_len);

protected:

    ///判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    ///cycbuf_begin_ = cycbuf_end_ 表示队列为NULL
    ///cycbuf_begin_ = cycbuf_end_ + JUDGE_FULL_INTERVAL 表示队列满
    static const size_t   JUDGE_FULL_INTERVAL = 4;

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
class queue_buffer
{
public:
    //构造，析构，赋值函数，为了加速，写了右值处理的函数
    queue_buffer() = default;
    ~queue_buffer();
    queue_buffer(const queue_buffer& others);
    queue_buffer(queue_buffer&& others) noexcept;
    queue_buffer& operator=(const queue_buffer& others);
    queue_buffer& operator=(queue_buffer&& others) noexcept;

    //根据size_of_buffer的长度初始化buffer
    bool initialize(size_t size_of_buffer);

    //
    void clear();

    //填充数据
    bool set(const char* data, const size_t szdata);
    //读取数据
    bool get(char* data, size_t& szdata);
    //继续在尾部增加数据
    bool add(const char* data, const size_t szdata);

    //容量
    inline size_t capacity()
    {
        return size_of_capacity_;
    }

    //已经使用的空间
    inline size_t size()
    {
        return size_of_use_;
    }

    //剩余的空间
    inline size_t free()
    {
        return size_of_capacity_ - size_of_use_;
    }

    //是否已经满了
    inline bool full()
    {
        if (size_of_use_ >= size_of_capacity_)
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

protected:

    //当前要使用的缓冲长度，当前处理的帧的长度,没有得到长度前填写0
    size_t      size_of_capacity_ = 0;

    //使用的尺寸
    size_t      size_of_use_ = 0;

    //数据缓冲区
    char       *buffer_data_ = nullptr;
};
}