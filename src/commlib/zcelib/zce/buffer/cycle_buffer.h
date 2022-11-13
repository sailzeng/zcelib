/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief      cycle_buffer :
*             一个循环buffer，里面存放数据，可以多次存放，多次取出
*             操作方式是fifo的，可以一点点放入，一点点取出，也可以
*             一次全部出局。节省存储空间。
*             所以内部数据有起始位置，结束位置，存放可能绕2截，数据
*             长度根据起始位置，结束位置计算。
*             优点是节省空间，可以一点点取出，缺点是数据只能取出后数
*             据后使用，
*
* @details    有点像滑动窗口
*
* @note
*
*/

#pragma once

namespace zce
{
//================================================================================
//!环形的一个buffer，里面存放数据，操作方式是fifo的，可以多次操作存放和取出
class cycle_buffer
{
public:
    //!构造，析构，赋值函数，为了加速，写了右值处理的函数
    cycle_buffer() = default;
    cycle_buffer(size_t buf_size);
    ~cycle_buffer();
    cycle_buffer(const cycle_buffer& others);
    cycle_buffer(cycle_buffer&& others) noexcept;
    cycle_buffer& operator=(const cycle_buffer& others);
    cycle_buffer& operator=(cycle_buffer&& others) noexcept;

    //!初始化
    bool initialize(size_t buf_size);

    void clear();

    static cycle_buffer* new_slef(size_t buf_size)
    {
        return new cycle_buffer(buf_size);
    }

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
    bool push_end(const char* data,
                  size_t data_len,
                  char*& write_ptr);

    ///在尾部填充长度为fill_len，的fill_data的字符
    bool push_end(char fill_ch,
                  size_t fill_len,
                  char*& write_ptr);

    ///从cycle_buffer头部，取出一个data_len长度的数据放入data
    bool pop_front(char* const data,
                   size_t data_len);

    ///从cycle_buffer头部，抛弃一个data_len长度的数据
    bool pop_front(size_t data_len);

    ///从偏移量pos的位置开始，填充
    bool set_data(size_t pos,
                  const char* data,
                  size_t fill_len,
                  char*& write_ptr);

    ///从pos（相对于cycbuf_begin_）读取数据，
    bool get_data(size_t pos,
                  char* data,
                  size_t read_len);

    ///从绝对位置read_ptr开始读取数据
    bool acquire_data(const char* read_ptr,
                      char* data,
                      size_t read_len);

    ///重新调整整个buffer的尺寸，缩小，或者增大尾部。
    bool reduce(size_t buf_len);

    //得到某个位置的数据指针
    inline char* point(size_t offset = 0)
    {
        return cycbuf_data_ +
            (cycbuf_begin_ + offset) % size_of_cycle_;
    }

protected:

    //!判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    //!cycbuf_begin_ = cycbuf_end_ 表示队列为nullptr
    //!cycbuf_begin_ = cycbuf_end_ + JUDGE_FULL_INTERVAL 表示队列满
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
}