/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2019年8月7日
* @brief
*
*
* @details   两个lockfee的ring队列处理代码。通过循环队列的方式避免ABA问题
*            rings用可写和可读区间来避免重叠冲突，推荐使用这个
*            rings_ptr,存放指针的区间也是，这个带有实验性质
*
* @note
*
*/

#pragma once

#include "zce/predefine.h"

namespace zce::lockfree
{
/*!
* @tparam    T 数据类型
* @brief     LOCKFREE 的魔戒.循环链表LILO，里面存放的是数据的拷贝。
*            如何实现的LOCKFREE以及避免ABA的。
*            整个循环链表又两部分表示，可读（有数据）区域，可写（空闲）区域，
*            这些区域的表示方法都是前开后闭，都由2个ATOMIC的数据表示。
*            数据区数据数量，由可读取数据得到
*            数据区数据数量是否为满，由可写入空间区域判定
*            数据区数据数量是否为空，由可读取空间区域判定
*            push_back 先调整可写入区域，然后写数据，在调整可读区域
*
*/
template<typename T >
class rings
{
public:
    ///构造函数，后面必须调用,initialize
    rings() = default;

    ///构造函数，同时完成初始化,后面完全 没有必要调用,initialize
    explicit rings(size_t max_len)
    {
        assert(max_len > 0);
        initialize(max_len);
    }

    ///析构函数，释放空间
    ~rings()
    {
        terminate();
    }

    ///初始化数据区，和构造函数干的事情基本一样，只是多了一步原来有数据就清理掉
    ///initialize 不是lock-free函数，
    void initialize(size_t max_len)
    {
        assert(max_len > 0);
        assert(value_ptr_ == nullptr);
        rings_capacity_ = max_len + 1;

        clear();

        //清理现场
        if (value_ptr_)
        {
            ::free(value_ptr_);
            value_ptr_ = NULL;
        }
        //不用new避免使用的构造函数
        value_ptr_ = (T*)::malloc(sizeof(T) * rings_capacity_);
    }

    ///结束，完成，销毁，finalize非lock-free函数，
    void terminate()
    {
        //清理现场
        if (value_ptr_)
        {
            size_t rings_size = size();
            for (size_t i = 0; i < rings_size; ++i)
            {
                value_ptr_[(read_start_ + i) % rings_capacity_].~T();
            }

            ::free(value_ptr_);
            value_ptr_ = NULL;
        }
        clear();
        rings_capacity_ = 0;
    }

    ///重新分配一个空间,resize非lock-free函数，
    bool resize(size_t new_max_size)
    {
        assert(new_max_size > 0);
        size_t rings_size = size();

        //如果原来的尺寸大于新的尺寸，无法扩展
        if (rings_size > new_max_size)
        {
            return false;
        }
        new_max_size += 1;
        _value_type* new_value_ptr = (_value_type*)::malloc(sizeof(_value_type) * new_max_size);
        //如果原来有数据,拷贝到新的数据区
        if (value_ptr_ != NULL)
        {
            for (size_t i = 0; i < rings_size && i < new_max_size; ++i)
            {
                new_value_ptr[i] = value_ptr_[(read_start_ + i) % rings_capacity_];
            }

            ::free(value_ptr_);
            value_ptr_ = NULL;
        }

        //调整几个内部参数
        rings_capacity_ = new_max_size;
        read_start_.store(0, std::memory_order_release);
        read_end_.store(rings_size, std::memory_order_release);
        write_start_.store(rings_size, std::memory_order_release);
        write_end_.store(rings_capacity_ - 1, std::memory_order_release);

        value_ptr_ = new_value_ptr;

        return true;
    }

    ///清理管道，
    void clear()
    {
        read_start_.store(0, std::memory_order_release);
        read_end_.store(0, std::memory_order_release);
        write_start_.store(0, std::memory_order_release);
        write_end_.store(rings_capacity_ - 1, std::memory_order_release);
    }

    ///尺寸空间
    inline size_t size() const
    {
        size_t end = read_end_.load(std::memory_order_acquire);
        size_t start = read_start_.load(std::memory_order_acquire);
        size_t rings_size = (rings_capacity_ - start + end) % rings_capacity_;
        return rings_size;
    }
    ///返回空闲空间的大小
    inline size_t free() const
    {
        size_t end = write_end_.load(std::memory_order_acquire);
        size_t start = write_start_.load(std::memory_order_acquire);
        size_t free_size = (rings_capacity_ - start + end) % rings_capacity_;
        return free_size;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return rings_capacity_ - 1;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        size_t end = write_end_.load(std::memory_order_acquire);
        size_t start = write_start_.load(std::memory_order_acquire);
        //可写空间为0
        if (start == end)
        {
            return true;
        }
        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //可读空间等于0
        size_t end = read_end_.load(std::memory_order_acquire);
        size_t start = read_start_.load(std::memory_order_acquire);
        if (end == start)
        {
            return true;
        }
        return false;
    }

    ///将一个数据放入队列的尾部
    bool push_back(const T& value_data)
    {
        //先调整可写入区域，然后写数据，在调整可读区域
        size_t w_start = 0, w_end = 0;
        do
        {
            w_end = write_end_.load(std::memory_order_acquire);
            w_start = write_start_.load(std::memory_order_acquire);

            //如果可写入空间没了，已经满了
            if (w_start == w_end)
            {
                return false;
            }
            //写开始步进1
        } while (!write_start_.compare_exchange_weak(w_start,
                 (w_start + 1) % rings_capacity_,
                 std::memory_order_acq_rel));

        //写入数据，直接放在队尾
        new (value_ptr_ + w_start) T(value_data);

        //调整可读范围
        bool ret = false;
        do
        {
            size_t expected = w_start;
            ret = read_end_.compare_exchange_weak(expected,
                                                  (w_start + 1) % rings_capacity_,
                                                  std::memory_order_acq_rel);
        } while (!ret);
        return true;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(T& value_data)
    {
        size_t r_start = 0, r_end = 0;
        do
        {
            r_start = read_start_.load(std::memory_order_acquire);
            r_end = read_end_.load(std::memory_order_acquire);
            if (r_end == r_start)
            {
                return false;
            }
        } while (!read_start_.compare_exchange_weak(r_start,
                 (r_start + 1) % rings_capacity_,
                 std::memory_order_acq_rel));
        //读取数据，
        value_data = value_ptr_[r_start];
        value_ptr_[r_start].~T();

        bool ret = false;
        do
        {
            size_t expected = (r_start - 1 + rings_capacity_) % rings_capacity_;
            ret = write_end_.compare_exchange_weak(expected,
                                                   r_start,
                                                   std::memory_order_acq_rel);
        } while (!ret);
        return true;
    }

protected:

    ///队列的长度，
    size_t rings_capacity_ = 0;

    ///循环队列能读取的起始位置[read_start_,read_end_)为可读取的区间范围
    ///read_end_ = read_start_ 为空
    ///rings size(已经使用的空间) 为  rings_capacity_ - read_start_ + read_end_
    std::atomic<size_t> read_start_ = 0;
    ///循环队列能读取的结束位置，
    std::atomic<size_t> read_end_ = 0;

    ///循环队列能可写的起始位置 [write_start_,write_end_)为可以写入的空间
    ///(write_start_ +1 % rings_capacity_)= write_end_ 为满(注意循环)
    std::atomic<size_t> write_start_ = 0;
    ///循环队列可写的结束位置，
    std::atomic<size_t> write_end_ = 0;

    ///存放数据的指针
    T* value_ptr_ = nullptr;
};

//==============================================================================================

/*!
* @tparam    T 数据类型
* @brief     环戒.lookfree循环链表，内部存放的指针，所以你必须new了放入，
*            同时因为反复会让你用new这类操作，速度不快很正常
* @note      实验性质很大，因为内部保持的是指针，你在外部必须new和delete，
*            所以性能肯定不会特别良好。
*            也没有像很多借口设计那样，内部new，delete，因为可能会给你错觉。
*            这个实现没有多层保护，所以不能直接存放数据拷贝，只能存放指针，
*           （你无法知道数据的读写是不是lockfree的。指针的读写是LOCKFREE）
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
