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

#include "zce/predefine.h"

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
    rings() = default;

    ///构造函数，同时完成初始化,后面完全 没有必要调用,initialize
    rings(size_t max_len)
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
        assert(vptr_ptr_ == nullptr);

        ring_start_ = 0;
        ring_end_ = 0;
        ring_capacity_ = max_len + 1;
        //
        vptr_ptr_ = new std::atomic<_value_type*>[max_len];
        for (size_t i = 0; i < max_len; ++i)
        {
            vptr_ptr_[i] = nullptr;
        }
    }

    ///结束，完成，销毁
    void finalize()
    {
        clear();
        //清理现场
        if (vptr_ptr_)
        {
            delete[] vptr_ptr_;
            vptr_ptr_ = nullptr;
        }
        ring_capacity_ = 0;
    }

    ///清理管道，
    void clear()
    {
        ring_start_ = 0;
        ring_end_ = 0;
    }

    ///尺寸空间
    inline size_t size() const
    {
        if (ring_end_ >= ring_start_)
        {
            return ring_end_ - ring_start_;
        }
        else
        {
            return ring_end_ + ring_capacity_ - ring_start_;
        }
    }
    ///返回空闲空间的大小
    inline size_t free() const
    {
        return ring_capacity_ - size() - 1;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return ring_capacity_ - 1;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        //已经用的空间等于容量
        if ((ring_end_ + 1) % ring_capacity_ == ring_start_)
        {
            return true;
        }
        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //如果发现开始==结束
        if (ring_start_ == ring_end_)
        {
            return true;
        }
        return false;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_back(_value_type * const value_ptr)
    {
        //
        while (true)
        {
            size_t start = ring_start_.load();
            size_t end = ring_end_.load();
            //用快照进行判定，如果已经满了
            if ((end + 1) % ring_capacity_ == start)
            {
                return false;
            }
            auto new_end = (end + 1) % ring_capacity_;
            if (vptr_ptr_[new_end] != nullptr)
            {
                continue;
            }
            //先写入数据
            _value_type * write_null = nullptr;
            bool succ = vptr_ptr_[new_end].compare_exchange_strong(write_null,
                                                                   value_ptr);
            if (succ)
            {
                //最后在移动end标识
                ring_end_.compare_exchange_strong(end, new_end);
                return true;
            }
        }
        return false;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_front(_value_type * const value_ptr)
    {
        //
        while (true)
        {
            size_t start = ring_start_.load();
            size_t end = ring_end_.load();
            //用快照进行判定
            if ((end + 1) % ring_capacity_ == start)
            {
                return false;
            }

            //直接放在队尾
            auto new_start = (start > 0) ? start - 1 : ring_capacity_ - 1;
            if (vptr_ptr_[new_start] != nullptr)
            {
                continue;
            }
            //直接放在队尾
            _value_type * write_null = nullptr;
            bool succ = vptr_ptr_[new_start].compare_exchange_strong(write_null,
                                                                     value_ptr);
            if (succ)
            {
                lordring_start_.compare_exchange_strong(start, new_start);
                return true;
            }
        }
        return false;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(_value_type *&value_ptr)
    {
        while (true)
        {
            size_t start = ring_start_.load();
            size_t end = ring_end_.load();
            //通过快照判定，为空，先返回
            if (start == end)
            {
                return false;
            }

            //因为存在push_front 放入了数据，但没有改写ring_start_可能
            value_ptr = vptr_ptr_[start];
            if (value_ptr == nullptr)
            {
                continue;
            }
            bool succ = vptr_ptr_[start].compare_exchange_strong(value_ptr,
                                                                 nullptr);
            if (succ)
            {
                auto new_start = (start + 1) % ring_capacity_;
                ring_start_.compare_exchange_strong(start, new_start);
            }
        }
        return false;
    }

    ///从队列的尾部pop并且得到一个数据
    bool pop_back(_value_type *&value_ptr)
    {
        while (true)
        {
            size_t start = ring_start_.load();
            size_t end = ring_end_.load();
            //通过快照判定，为空，先返回
            if (start == end)
            {
                return false;
            }
            //因为存在push_front 放入了数据，但没有改写ring_end_可能
            value_ptr = vptr_ptr_[end];
            if (value_ptr == nullptr)
            {
                continue;
            }
            bool succ = vptr_ptr_[end].compare_exchange_strong(value_ptr,
                                                               nullptr);
            if (succ)
            {
                auto new_end = (end > 0) ? end - 1 : ring_capacity_ - 1;
                ring_end_.compare_exchange_strong(end, new_end);
            }
        }
        return false;
    }

protected:

    ///循环队列的起始位置
    std::atomic<size_t> ring_start_ = 0;

    ///循环队列的尾部位置，
    std::atomic<size_t> ring_end_ = 0;

    ///队列的长度，
    size_t ring_capacity_ = 0;

    ///存放数据的指针，
    std::atomic<_value_type*> *vptr_ptr_ = nullptr;
};
};
