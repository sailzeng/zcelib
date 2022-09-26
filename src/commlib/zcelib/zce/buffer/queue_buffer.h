/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief
*             queue_buffer :
*             一个只能向尾部添加数据的BUFFER，
*             数据能多次放入，但取出数据要全部取出。
*             读取数据可以直接使用point指针。
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
//================================================================================
//!一个只能向尾部添加数据的BUFFER
class queue_buffer
{
public:
    //!构造，析构，赋值函数，为了加速，写了右值处理的函数
    queue_buffer() = default;
    queue_buffer(size_t buf_size);
    ~queue_buffer();
    queue_buffer(const queue_buffer& others);
    queue_buffer(queue_buffer&& others) noexcept;
    queue_buffer& operator=(const queue_buffer& others);
    queue_buffer& operator=(queue_buffer&& others) noexcept;

    //!初始化
    bool initialize(size_t buf_size);

    //!清理
    void clear();

    static queue_buffer* new_self(size_t buf_size)
    {
        return new queue_buffer(buf_size);
    }

    //!容量
    inline size_t capacity()
    {
        return size_of_capacity_;
    }

    //!已经使用的空间
    inline size_t size()
    {
        return size_of_use_;
    }

    //!剩余的空间
    inline size_t free()
    {
        return size_of_capacity_ - size_of_use_;
    }

    //!是否已经满了
    inline bool full()
    {
        if (size_of_use_ >= size_of_capacity_)
        {
            return true;
        }
        return false;
    }

    //!是否为空
    inline bool empty()
    {
        if (size_of_use_ == 0)
        {
            return true;
        }
        return false;
    };

    //!填充数据data,长度为szdata
    bool set(const char* data, size_t szdata);
    //!从偏移offset开始，填充数据data,长度为szdata,
    bool set(size_t offset, const char* data, size_t szdata);
    //!读取数据
    bool get(char* data, size_t& szdata);
    //!继续在尾部增加数据
    bool add(const char* data, size_t szdata);

    //
    inline char* point(size_t offset = 0)
    {
        return buffer_data_ + offset;
    }
protected:

    //当前要使用的缓冲长度，当前处理的帧的长度,没有得到长度前填写0
    size_t      size_of_capacity_ = 0;

    //使用的尺寸
    size_t      size_of_use_ = 0;

    //数据缓冲区
    char* buffer_data_ = nullptr;
};
}