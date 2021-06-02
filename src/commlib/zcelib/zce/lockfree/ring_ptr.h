/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2019年8月7日
* @brief      rings_ptr,内部存放指针的循环队列，
*
*
* @details   内部采用一个循环BUFFER存放atomic 指针，
*            内部其实采用指针存放，指针为空表示空状态，用这个检查某个node是否
*            可写。
*            （你无法知道数据的读写是不是lockfree的。指针的读写是LOCKFREE）
*            实验性质很大，因为内部保持的是指针，你在外部必须new和delete，
*            所以性能肯定不会特别良好。
*            也没有像很多接口设计那样，内部new，delete，因为感觉可能会给你错觉。
*
* @note      保留其的主要目的是因为这个方法表示一种lockfree的实现方式。
*
*/

#pragma once

#include "zce/predefine.h"

namespace zce::lockfree
{
//==============================================================================================

/*!
* @tparam    T 数据类型
* @brief     环戒.lookfree循环链表，内部存放的指针，所以你必须new了放入，
*            同时因为反复会让你用new这类操作，速度不快很正常
* @note      这个有实验性质，我个人推荐使用rings
*
*/
template<class T >
class rings_ptr
{
public:
    ///构造函数，后面必须调用,initialize
    rings_ptr() = default;

    ///构造函数，同时完成初始化,后面无需调用再次调用initialize
    explicit rings_ptr(size_t max_len)
    {
        assert(max_len > 0);
        initialize(max_len);
    }

    ///析构函数，释放空间
    ~rings_ptr()
    {
        terminate();
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
        vptr_ptr_ = new std::atomic<T*>[max_len];
        for (size_t i = 0; i < max_len; ++i)
        {
            vptr_ptr_[i] = nullptr;
        }
    }

    ///结束，完成，销毁
    void terminate()
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

    /*!
    * @brief      将一个数据放入队列的尾部,如果队列已经满了,
    *             覆盖原有的数据
    * @return     bool 是否成功放入
    * @param      value_ptr 放入的数据的指针
    * @note       容器里面存放的都是指针，所以你必须new之后放入
    */
    bool push_back(T * const value_ptr)
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

            //注意，这儿写入的是end，因为队列是前开后闭的
            if (vptr_ptr_[end] != nullptr)
            {
                continue;
            }
            //先写入数据
            T * write_null = nullptr;
            bool succ = vptr_ptr_[end].compare_exchange_weak(write_null,
                                                             value_ptr,
                                                             std::memory_order_acq_rel);
            if (succ)
            {
                auto new_end = (end + 1) % ring_capacity_;
                //最后在移动end标识
                ring_end_.compare_exchange_strong(end, new_end);
                return true;
            }
        }
        return false;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,
    bool push_front(T * const value_ptr)
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

            //直接放在队首部
            auto new_start = (start > 0) ? start - 1 : ring_capacity_ - 1;
            if (vptr_ptr_[new_start] != nullptr)
            {
                continue;
            }
            //如果仍然是null，而且交互成功，
            T * write_null = nullptr;
            bool succ = vptr_ptr_[new_start].compare_exchange_weak(write_null,
                                                                   value_ptr,
                                                                   std::memory_order_acq_rel);
            if (succ)
            {
                lordring_start_.compare_exchange_strong(start, new_start);
                return true;
            }
        }
        return false;
    }

    /*!
    * @brief      从队列的前面pop并且得到一个数据
    * @return     bool
    * @param      value_ptr
    * @note
    */
    bool pop_front(T *&value_ptr)
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

            value_ptr = vptr_ptr_[start];
            if (value_ptr == nullptr)
            {
                continue;
            }
            bool succ = vptr_ptr_[start].compare_exchange_weak(value_ptr,
                                                               nullptr,
                                                               std::memory_order_acq_rel);
            if (succ)
            {
                auto new_start = (start + 1) % ring_capacity_;
                ring_start_.compare_exchange_strong(start, new_start);
                return true;
            }
        }
        return false;
    }

    ///从队列的尾部pop并且得到一个数据
    bool pop_back(T *&value_ptr)
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
            auto new_end = (end > 0) ? end - 1 : ring_capacity_ - 1;
            value_ptr = vptr_ptr_[new_end];

            if (value_ptr == nullptr)
            {
                continue;
            }
            bool succ = vptr_ptr_[new_end].compare_exchange_weak(value_ptr,
                                                                 nullptr,
                                                                 std::memory_order_acq_rel);
            if (succ)
            {
                ring_end_.compare_exchange_strong(end, new_end);
                return true;
            }
        }
        return false;
    }

protected:
    ///队列的长度，
    size_t ring_capacity_ = 0;

    ///循环队列的起始位置
    std::atomic<size_t> ring_start_ = 0;

    ///循环队列的尾部位置，
    std::atomic<size_t> ring_end_ = 0;

    ///存放数据的指针，
    std::atomic<T*> *vptr_ptr_ = nullptr;
};
};
