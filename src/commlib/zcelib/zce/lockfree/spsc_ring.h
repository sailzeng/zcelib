#pragma once

#include "zce/predefine.h"

namespace zce::lockfree
{
/*!
* @brief      SPSC，single-producer/single-consumer 单读单写的ring，
*             如何实现的单独单写呢，比如生产者调用push_back，只改写end位置
*             表示， pop_front只改写front的位置。所以SPSC模式下，读写相互
*             不影响
* @tparam     T 容器对象
* @note       单读单写，您只能成对使用函数push_back，pop_front 一对
*             push_front,pop_back一对，否则一样有风险
*/
template<typename T>
class spsc_ring
{
public:

    ///构造函数，后面必须调用,initialize
    spsc_ring() = default;

    ///构造函数，同时完成初始化,
    spsc_ring(size_t max_len)
    {
        assert(max_len > 0);
        initialize(max_len);
    }

    ///析构函数，释放空间
    ~spsc_ring()
    {
        terminate();
    }

    ///初始化数据区，和构造函数干的事情基本一样，只是多了一步原来有数据就清理掉 Not thread-safe
    bool initialize(size_t max_len)
    {
        assert(max_len > 0 && vptr_ptr_ == nullptr);

        spscring_start_.store(0);
        spscring_end_.store(0);

        auto ret = resize(max_len);
        if (ret == false)
        {
            return false;
        }

        return true;
    }

    ///结束，完成，销毁，Not thread-safe
    void terminate()
    {
        clear();
        //清理现场
        if (vptr_ptr_)
        {
            ::free(vptr_ptr_);
            vptr_ptr_ = nullptr;
        }
        spscring_capacity_ = 0;
    }

    ///清理管道，析构素有的数据，Not thread-safe
    void clear()
    {
        if (vptr_ptr_ != nullptr)
        {
            auto lordring_size = size();
            for (size_t i = 0; i < lordring_size; ++i)
            {
                vptr_ptr_[(spscring_start_ + i) % spscring_capacity_].~T();
            }
        }
        spscring_start_.store(0);
        spscring_end_.store(0);
    }

    ///尺寸空间
    inline size_t size() const
    {
        size_t start = spscring_start_.load();
        size_t end = spscring_end_.load();
        if (end >= start)
        {
            return end - start;
        }
        else
        {
            return end + spscring_capacity_ - start;
        }
    }
    ///返回空闲空间的大小
    inline size_t free() const
    {
        return spscring_capacity_ - size() - 1;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return spscring_capacity_ - 1;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        //已经用的空间等于容量
        if ((spscring_end_ + 1) % spscring_capacity_ == spscring_start_)
        {
            return true;
        }
        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //空间等于0
        if (spscring_end_ == spscring_start_)
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
        if (deque_size > new_max_size || new_max_size <= 0)
        {
            return false;
        }
        size_t new_capacity = new_max_size + 1;
        T *new_value_ptr = (T *)::malloc(sizeof(T) * new_capacity);
        if (new_value_ptr == nullptr)
        {
            return false;
        }

        //如果原来有数据,
        if (vptr_ptr_ != nullptr)
        {
            //拷贝到新的数据区,下面这个
            for (size_t i = 0; i < deque_size && i < new_max_size; ++i)
            {
                new_value_ptr[i] = vptr_ptr_[(spscring_start_ + i) % spscring_capacity_];
                vptr_ptr_[(spscring_start_ + i) % spscring_capacity_].~T();
            }
            //释放原有的数据空间
            ::free(vptr_ptr_);
            vptr_ptr_ = nullptr;
        }

        //调整几个内部参数
        spscring_start_ = 0;
        spscring_capacity_ = new_capacity;
        spscring_end_ = deque_size;

        vptr_ptr_ = new_value_ptr;

        return true;
    }

    //将一个数据放入队列的尾部,如果队列已经满了,返回false
    bool push_back(const T &value_data)
    {
        //如果已经满了
        if (full())
        {
            return false;
        }
        //如果还有空间，直接放在队伍尾部
        vptr_ptr_[spscring_end_] = value_data;
        spscring_end_ = (spscring_end_ + 1) % spscring_capacity_;
        return true;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了
    bool push_front(const T &value_data)
    {
        //如果已经满了
        if (full())
        {
            return false;
        }
        //如果还有空间，直接放在队伍首部
        spscring_start_ = (spscring_start_ > 0) ? spscring_start_ - 1 : spscring_capacity_ - 1;
        vptr_ptr_[spscring_start_] = value_data;
        return true;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(T &value_data)
    {
        //如果是空的返回错误
        if (empty())
        {
            return false;
        }

        value_data = vptr_ptr_[spscring_start_];
        vptr_ptr_[spscring_start_].~T();
        spscring_start_ = (spscring_start_ + 1) % spscring_capacity_;
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
        vptr_ptr_[spscring_start_].~T();
        spscring_start_ = (spscring_start_ + 1) % spscring_capacity_;
        return true;
    }

    ///从队列的尾部pop并且得到一个数据
    bool pop_back(T &value_data)
    {
        //如果是空的返回错误
        if (empty())
        {
            return false;
        }
        spscring_end_ = (spscring_end_ > 0) ? spscring_end_ - 1 : spscring_capacity_ - 1;
        value_data = vptr_ptr_[spscring_end_];
        vptr_ptr_[spscring_end_].~T();

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
        spscring_end_ = (spscring_end_ > 0) ? spscring_end_ - 1 : spscring_capacity_ - 1;
        vptr_ptr_[spscring_end_].~T();
        return true;
    }

protected:

    ///循环队列的起始位置
    std::atomic<size_t>    spscring_start_ = 0;
    ///循环队列的长度，,思路仍然是前开后闭
    ///容器size通过lordring_end_ + spscring_capacity_ - spscring_start_
    /// 或者 spscring_end_ - lordring_start_得到，
    std::atomic<size_t>    spscring_end_ = 0;

    ///队列的长度，
    size_t                 spscring_capacity_ = 0;
    ///存放数据的指针
    T*                     vptr_ptr_ = nullptr;
};
}
