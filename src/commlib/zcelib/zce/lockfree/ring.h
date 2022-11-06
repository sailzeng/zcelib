/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2019年8月7日
* @brief
*
*
* @details   rings用可写和可读区间来避免重叠冲突，推荐使用这个
*            通过循环队列的方式避免ABA问题
*
* @note
*
*/

#pragma once

namespace zce::lockfree
{
/*!
* @tparam    T 数据类型
* @brief     LOCKFREE 的魔戒.循环链表LILO，里面存放的是数据的拷贝，可以多读多写。
*            如何实现的LOCKFREE以及避免ABA的？
*            整个循环链表又两部分表示，可读（有数据）区域，可写（空闲）区域，
*            这些区域的表示方法都是前开后闭，都由2个ATOMIC的数据表示。
*            数据区数据数量，由可读取数据得到
*            数据区数据数量是否为满，由可写入空间区域判定
*            数据区数据数量是否为空，由可读取空间区域判定
*            push_back 先调整可写入区域，然后写数据，再调整可读区域
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

    rings(const rings& other) = delete;
    rings& operator=(const rings& other) = delete;

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
            value_ptr_ = nullptr;
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
            value_ptr_ = nullptr;
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
        T* new_value_ptr = (T*)::malloc(sizeof(T) * new_max_size);
        //如果原来有数据,拷贝到新的数据区
        if (value_ptr_ != nullptr)
        {
            for (size_t i = 0; i < rings_size && i < new_max_size; ++i)
            {
                new_value_ptr[i] = value_ptr_[(read_start_ + i) % rings_capacity_];
            }

            ::free(value_ptr_);
            value_ptr_ = nullptr;
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

    bool push_back(const T& value)
    {
        return push_back_i(value);
    }
    bool push_back(T&& value)
    {
        return push_back_i(std::move(value));
    }
    bool push_front(const T& value)
    {
        return push_front_i(value);
    }
    bool push_front(T&& value)
    {
        return push_front_i(std::move(value));
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(T& value)
    {
        size_t r_start = 0, r_end = 0, w_end = 0;
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
        value = value_ptr_[r_start];
        value_ptr_[r_start].~T();

        do
        {
            w_end = write_end_;
        } while (write_end_.compare_exchange_weak(w_end,
                 (w_end + 1) % rings_capacity_,
                 std::memory_order_acq_rel));
        return true;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_back(T& value)
    {
        size_t r_start = 0, r_end = 0, w_start = 0;
        do
        {
            r_start = read_start_.load(std::memory_order_acquire);
            r_end = read_end_.load(std::memory_order_acquire);
            if (r_end == r_start)
            {
                return false;
            }
        } while (!read_end_.compare_exchange_weak(r_end,
                 (r_end > 0) ? r_end - 1 : rings_capacity_ - 1,
                 std::memory_order_acq_rel));
        //读取数据，
        value = value_ptr_[r_end];
        value_ptr_[r_end].~T();

        do
        {
            w_start = write_start_;
        } while (write_end_.compare_exchange_weak(w_start,
                 (w_start > 0) ? w_start - 1 : rings_capacity_ - 1,
                 std::memory_order_acq_rel));
        return true;
    }
protected:

    ///将一个数据放入队列的尾部
    bool push_back_i(T value)
    {
        //先调整可写入区域，然后写数据，在调整可读区域
        size_t w_start = 0, w_end = 0, r_end;
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
        new (value_ptr_ + w_start) T(value);

        //调整可读范围.只步进+1，所以稍微的不同步不影响任何使用
        do
        {
            r_end = read_end_;
        } while (read_end_.compare_exchange_weak(r_end,
                 (r_end + 1) % rings_capacity_,
                 std::memory_order_acq_rel));
        return true;
    }

    ///将一个数据放入队列的头部
    bool push_front_i(T value)
    {
        //先调整可写入区域，然后写数据，在调整可读区域
        size_t w_start = 0, w_end = 0, r_start = 0;
        do
        {
            w_start = write_start_.load(std::memory_order_acquire);
            w_end = write_end_.load(std::memory_order_acquire);

            //如果可写入空间没了，已经满了
            if (w_start == w_end)
            {
                return false;
            }
            //写结束步进-1
        } while (!write_end_.compare_exchange_weak(w_end,
                 (w_end > 0) ? w_end - 1 : rings_capacity_ - 1,
                 std::memory_order_acq_rel));

        //写入数据，直接放在队尾
        new (value_ptr_ + w_end) T(value);

        //调整可读范围,步进-1
        do
        {
            r_start = read_start_;
        } while (read_start_.compare_exchange_weak(r_start,
                 (r_start > 0) ? r_start - 1 : rings_capacity_ - 1,
                 std::memory_order_acq_rel));
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
}
