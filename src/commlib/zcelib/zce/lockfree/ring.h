/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2019年8月7日
* @brief
*
*
* @details  这个代码还没有改造完成。
*
*
* @note
*
*/

#pragma once

#include "zce/shm_container/common.h"

namespace zce::lockfree
{
/*!
* @tparam    _value_type 数据类型
* @brief     魔戒.循环链表，可以自动扩展，可以最后的覆盖第一个，
*/
template<class _value_type >
class rings
{
public:
    ///构造函数，后面必须调用,initialize
    rings() :
        rings_start_(0),
        rings_end_(0),
        rings_capacity_(0),
        value_ptr_(NULL)
    {
    }

    ///构造函数，同时完成初始化,后面完全 没有必要调用,initialize
    rings(size_t max_len) :
        rings_start_(0),
        rings_end_(0),
        rings_capacity_(max_len),
        value_ptr_(NULL)
    {
        assert(max_len > 0);
        initialize(max_len);
    }

    ///析构函数，释放空间
    ~rings()
    {
        finalize();
    }

    ///初始化数据区，和构造函数干的事情基本一样，只是多了一步原来有数据就清理掉
    ///initialize 不加锁，
    void initialize(size_t max_len)
    {
        assert(max_len > 0);

        rings_start_ = 0;
        rings_end_ = 0;
        rings_capacity_ = max_len + 1;

        //清理现场
        if (value_ptr_)
        {
            free(value_ptr_);
            value_ptr_ = NULL;
        }
        //不用new避免过多的构造函数
        value_ptr_ = (_value_type*)malloc(sizeof(_value_type) * capacity_);
    }

    ///结束，完成，销毁
    void finalize()
    {
        rings_start_ = 0;
        rings_end_ = 0;
        rings_capacity_ = 0;

        //清理现场
        if (value_ptr_)
        {
            size_t sz = rings_end_.load();
            for (; read < sz; ++read)
            {
                value_ptr_[read].~_value_type();
            }

            free(value_ptr_);
            value_ptr_ = NULL;
        }
    }

    ///清理管道，
    void clear()
    {
        rings_start_ = 0;
        rings_end_ = 0;
    }

    ///尺寸空间
    inline size_t size() const
    {
        if (cycdeque_end_ >= cycdeque_start_)
        {
            return rings_end_ - rings_start_;
        }
        else
        {
            return rings_end_ + rings_capacity_ - rings_start_;
        }
    }
    ///返回空闲空间的大小
    inline size_t free() const
    {
        return rings_capacity_ - size() - 1;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return rings_capacity_ - 1;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        //已经用的空间等于容量
        if ((rings_end_ + 1) % rings_capacity_ == rings_start_)
        {
            return true;
        }
        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //如果发现开始==结束
        if (rings_start_ == rings_end_)
        {
            return true;
        }
        return false;
    }

    ///重新分配一个空间,
    bool resize(size_t new_max_size)
    {
        assert(new_max_size > 0);
        size_t deque_size = size();

        //如果原来的尺寸大于新的尺寸，无法扩展
        if (deque_size > new_max_size)
        {
            return false;
        }

        _value_type* new_value_ptr = new _value_type[new_max_size + 1];

        //如果原来有数据,拷贝到新的数据区
        if (value_ptr_ != NULL)
        {
            for (size_t i = 0; i < deque_size && i < new_max_size; ++i)
            {
                new_value_ptr[i] = value_ptr_[(rings_start_ + i) % rings_capacity_];
            }

            delete[] value_ptr_;
            value_ptr_ = NULL;
        }

        //调整几个内部参数
        rings_start_ = 0;
        rings_end_ = deque_size;
        rings_capacity_ = new_max_size + 1;

        value_ptr_ = new_value_ptr;

        return true;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_back(const _value_type& value_data)
    {
        //如果已经满了
        while (true)
        {
            size_t end = 0, start = 0;
            ring_start_.load(start);
            rings_end_.load(end);

            if ((end + 1) % rings_capacity_ == start)
            {
                return false;
            }
            new_end = (end + 1) % rings_capacity_;
            //直接放在队尾
            bool succ = rings_end_.compare_exchange_strong(end, new_end);
            if (succ)
            {
                value_ptr_[new_end] = value_data;
                return true;
            }
        }
        return false;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_front(const _value_type& value_data)
    {
        //如果已经满了
        if (full())
        {
            return false;
        }

        //直接放在队尾
        rings_start_ = (rings_start_ > 0) ? rings_start_ - 1 : rings_capacity_ - 1;
        value_ptr_[rings_start_] = value_data;

        return true;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(_value_type& value_data)
    {
        //如果是空的返回错误
        if (empty())
        {
            return false;
        }
        auto start = rings_start_;
        rings_start_ = (rings_start_ + 1) % rings_capacity_;
        value_data = value_ptr_[start];
        return true;
    }

    ///从队列前面pop一个数据
    bool pop_front()
    {
        //如果是空的返回错误
        if (empty())
        {
            return false;
        }

        rings_start_ = (rings_start_ + 1) % rings_capacity_;
        return true;
    }

    ///从队列的尾部pop并且得到一个数据
    bool pop_back(_value_type& value_data)
    {
        //如果是空的返回错误
        if (empty())
        {
            return false;
        }

        ring_end_ = (ring_end_ > 0) ? ring_end_ - 1 : rings_capacity_ - 1;
        value_data = value_ptr_[ring_end_];
        return true;
    }

    ///从队列的尾部pop一个数据
    bool pop_back()
    {
        //
        if (size() == 0)
        {
            return false;
        }
        cycdeque_end_ = (cycdeque_end_ > 0) ? cycdeque_end_ - 1 : cycdeque_len_ - 1;
        return true;
    }

protected:

    ///循环队列的起始位置
    std::atomic<size_t> rings_start_;

    ///循环队列的长度，
    ///没有用结束为止是方便计算， 结束位置通过(rings_start_+cycdeque_size_)%rings_capacity_得到，思路仍然是前开后闭
    std::atomic<size_t> rings_end_;

    ///队列的长度，
    size_t rings_capacity_;
    ///存放数据的指针
    _value_type* value_ptr_;
};
};
