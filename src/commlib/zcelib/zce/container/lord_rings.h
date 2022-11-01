/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce/container/lord_rings.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年3月11日
* @brief      魔戒，一个循环存放数据的队列,用于各种缓冲区，队列，可以定长，也可以扩展长度
*             和LIST比最大好处是空间是一开始分配好的，不需要多次new，速度飞快，性能利器
*
* @details    先说明这个代码其实和BOOST没有半毛钱关系。
*             BOOST也有类似的实现 circular_buffer，我这个地方没有偷师，殊途同归而已，
*             可以控制在数据满的情况下，是否覆盖,举一个应用的场景的例子，游戏服务器的用户
*             的1s聊天队列，然后广播，你可以设置最大长度200，如果超出，后面的覆盖前面的，
*             这样无论如何不会溢出。呵呵
*
* @note       原来取名cyc_deque，但是一天和Linker聊天他说说他写的类似的类名字是rings，
*             我觉得这个名字比较酷，所以选择这个名字，越来越觉得这个类好用。
*
*             2011年10月7日 做了一次修正，同时增加了迭代器。
*             2021年6月4日
*             最开始的实现是用空间+1,标识前闭后开的空间，中间的修正还是加入了size记录，
*             但最后，我考虑还是，采用了增加1个空间标识后开空间了,因为这样天然的可以SPSC
*             的LOCKFREE。
*
*             喔姐姐，我想回家，牵着我的手，我有些困了
*/

#pragma once

/*!
* @namespace zce 主要用于模版类的名字空间，为了和std统一，模版类一般用了小写
*/

namespace zce
{
/*!
* @tparam    T 数据类型
* @brief     魔戒的的迭代器，在魔戒类里面有typedef成为iterator，
* @note      记住lordrings的迭代器比vector更加危险，因为他的空间是循环利用的，任何一次push,
*            pop操作都会让迭代器失效。所以使用的时候当心，
*
*            个人觉得迭代器的作用不大，因为魔戒的使用主要头尾，几乎不用遍历使用。
*/
template <class T >
class _lordrings_iterator
{
protected:

    //定义迭代器
    typedef _lordrings_iterator<T > iterator;

    //迭代器萃取器所有的东东
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T &reference;
    typedef T value_type;
    typedef std::random_access_iterator_tag iterator_category;

public:
    ///构造函数
    _lordrings_iterator() :
        ring_start_(0),
        ring_capacity_(0),
        rings_value_base_(nullptr),
        serial_(0)
    {
    }
    ///构造函数
    _lordrings_iterator(size_t rings_start,
                        size_t rings_capacity,
                        T *rings_value_base,
                        size_t serial
    ) :
        ring_start_(rings_start),
        ring_capacity_(rings_capacity),
        rings_value_base_(rings_value_base),
        serial_(serial)
    {
    }
    ///析构函数
    ~_lordrings_iterator()
    {
    }

    ///*提领操作，
    T &operator*() const
    {
        return *(rings_value_base_ + (ring_start_ + serial_) % ring_capacity_);
    }

    ///->操作
    T *operator->() const
    {
        return rings_value_base_ + (ring_start_ + serial_) % ring_capacity_;
    }

    ///前向迭代器
    iterator &operator++()
    {
        serial_++;
        return *this;
    }

    ///前向迭代器
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++ *this;
        return tmp;
    }

    ///前向迭代器
    iterator &operator--()
    {
        serial_--;
        return *this;
    }

    ///前向迭代器
    iterator operator--(int)
    {
        iterator tmp = *this;
        -- *this;
        return tmp;
    }

    ///迭代器判定相等
    bool operator==(const iterator &it) const
    {
        if (rings_value_base_ == it.rings_value_base_
            && ring_start_ == it.ring_start_
            && ring_capacity_ == it.ring_capacity_
            && serial_ == it.serial_)
        {
            return true;
        }

        return false;
    }

    ///迭代器判定不相等
    bool operator!=(const iterator &it) const
    {
        return !(*this == it);
    }

    ///保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

protected:
    ///魔戒目前开始的位置，
    size_t ring_start_;
    ///魔戒的容量
    size_t ring_capacity_;
    ///数据的指针，
    T *rings_value_base_;

    //序列号,迭代器前进，增加的就是这个值
    size_t serial_;
};

/*!
* @tparam    T 数据类型
* @brief     魔戒.循环链表，可以自动扩展，可以最后的覆盖第一个，
*/
template<typename T>
class lord_rings
{
public:
    //定义迭代器
    typedef _lordrings_iterator<T > iterator;

    ///构造函数，后面必须调用,initialize
    lord_rings() = default;

    ///构造函数，同时完成初始化,
    lord_rings(size_t max_len)
    {
        assert(max_len > 0);
        initialize(max_len);
    }

    ///析构函数，释放空间
    ~lord_rings()
    {
        terminate();
    }

    ///初始化数据区，和构造函数干的事情基本一样，只是多了一步原来有数据就清理掉
    bool initialize(size_t max_len)
    {
        assert(max_len > 0 && vptr_ptr_ == nullptr);

        lordring_start_ = 0;
        lordring_end_ = 0;

        auto ret = resize(max_len);
        if (ret == false)
        {
            return false;
        }

        return true;
    }

    ///结束，完成，销毁，求问fini是什么的缩写，
    void terminate()
    {
        clear();
        //清理现场
        if (vptr_ptr_)
        {
            ::free(vptr_ptr_);
            vptr_ptr_ = nullptr;
        }
        lordring_capacity_ = 0;
    }

    ///清理管道，析构素有的数据，
    void clear()
    {
        if (vptr_ptr_ != nullptr)
        {
            auto lordring_size = size();
            for (size_t i = 0; i < lordring_size; ++i)
            {
                vptr_ptr_[(lordring_start_ + i) % lordring_capacity_].~T();
            }
        }
        lordring_start_ = 0;
        lordring_end_ = 0;
    }

    ///尺寸空间
    inline size_t size() const
    {
        if (lordring_end_ >= lordring_start_)
        {
            return lordring_end_ - lordring_start_;
        }
        else
        {
            return lordring_end_ + lordring_capacity_ - lordring_start_;
        }
    }
    ///返回空闲空间的大小
    inline size_t free() const
    {
        return lordring_capacity_ - size() - 1;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return lordring_capacity_ - 1;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        //已经用的空间等于容量
        if ((lordring_end_ + 1) % lordring_capacity_ == lordring_start_)
        {
            return true;
        }
        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //空间等于0
        if (lordring_end_ == lordring_start_)
        {
            return true;
        }
        return false;
    }

    ///重新分配一个空间,
    bool resize(size_t new_size)
    {
        assert(new_size > 0);
        if (new_size <= 0)
        {
            return false;
        }
        size_t deque_size = size();

        size_t new_capacity = new_size + 1;
        T *new_value_ptr = (T *)::malloc(sizeof(T) * new_capacity);
        if (new_value_ptr == nullptr)
        {
            return false;
        }

        //如果原来有数据,
        if (vptr_ptr_ != nullptr)
        {
            //拷贝到新的数据区,下面这个
            size_t i = 0;
            for (; i < deque_size && i < new_size; ++i)
            {
                new_value_ptr[i] = vptr_ptr_[(lordring_start_ + i) % lordring_capacity_];
                vptr_ptr_[(lordring_start_ + i) % lordring_capacity_].~T();
            }
            //如果其实是缩小
            if (new_size < deque_size)
            {
                size_t destruction_size = deque_size - new_size;
                for (; i < destruction_size; ++i)
                {
                    vptr_ptr_[(lordring_start_ + i) % lordring_capacity_].~T();
                }
            }

            //释放原有的数据空间
            ::free(vptr_ptr_);
            vptr_ptr_ = nullptr;
        }

        //调整几个内部参数
        lordring_start_ = 0;
        lordring_capacity_ = new_capacity;
        lordring_end_ = deque_size;

        vptr_ptr_ = new_value_ptr;

        return true;
    }

    /*!
    * @brief      将一个数据放入队列的尾部,如果队列已经满了,根据lay_over参数决定是覆盖最后一个数据
    * @return     bool      是否成功push
    * @param      value_data push的数据
    * @param      lay_over  你可以将lay_over参数置位true,覆盖原有的数据
    */
    bool push_back(const T &value_data, bool lay_over = false)
    {
        //如果已经满了
        if (full())
        {
            //如果不要覆盖，返回错误
            if (lay_over == false)
            {
                return false;
            }
            else
            {
                //将最后一个位置覆盖，并且调整起始和结束位置
                vptr_ptr_[lordring_end_] = value_data;
                lordring_end_ = (lordring_end_ + 1) % lordring_capacity_;
                lordring_start_ = (lordring_start_ + 1) % lordring_capacity_;
                return true;
            }
        }
        //如果还有空间，直接放在队伍尾部
        vptr_ptr_[lordring_end_] = value_data;
        lordring_end_ = (lordring_end_ + 1) % lordring_capacity_;
        return true;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了
    bool push_front(const T &value_data, bool lay_over = false)
    {
        //如果已经满了
        if (full())
        {
            //如果不要覆盖，返回错误
            if (lay_over == false)
            {
                return false;
            }
            else
            {
                //将起始位置调整覆盖，并且调整起始和结束位置
                lordring_start_ = (lordring_start_ > 0) ?
                    lordring_start_ - 1 : lordring_capacity_ - 1;
                lordring_end_ = (lordring_end_ > 0) ? lordring_end_ - 1 : lordring_capacity_ - 1;
                vptr_ptr_[lordring_start_] = value_data;
                return true;
            }
        }
        //如果还有空间，直接放在队伍首部
        lordring_start_ = (lordring_start_ > 0) ? lordring_start_ - 1 : lordring_capacity_ - 1;
        vptr_ptr_[lordring_start_] = value_data;
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

        value_data = vptr_ptr_[lordring_start_];
        vptr_ptr_[lordring_start_].~T();
        lordring_start_ = (lordring_start_ + 1) % lordring_capacity_;
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
        vptr_ptr_[lordring_start_].~T();
        lordring_start_ = (lordring_start_ + 1) % lordring_capacity_;
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
        lordring_end_ = (lordring_end_ > 0) ? lordring_end_ - 1 : lordring_capacity_ - 1;
        value_data = vptr_ptr_[lordring_end_];
        vptr_ptr_[lordring_end_].~T();

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
        lordring_end_ = (lordring_end_ > 0) ? lordring_end_ - 1 : lordring_capacity_ - 1;
        vptr_ptr_[lordring_end_].~T();
        return true;
    }

    //在某个位置上插入数据，后面的数据都后移一位
    bool insert(size_t pos, T &value_data)
    {
        if (full())
        {
            return false;
        }
        auto absolute_pos = (lordring_start_ + pos) % lordring_capacity_;
        size_t deque_size = size();
        //都后移一位
        for (size_t i = 0; i < deque_size - pos; ++i)
        {
            vptr_ptr_[(absolute_pos + i + 1) % lordring_capacity_] =
                vptr_ptr_[(absolute_pos + i) % lordring_capacity_];
        }
        vptr_ptr_[absolute_pos] = value_data;
        lordring_end_ = (lordring_end_ + 1) % lordring_capacity_;
        return true;
    }

    //在某个位置上删除数据，后面的数据都前移一位
    bool erase(size_t pos)
    {
        if (empty())
        {
            return false;
        }
        auto absolute_pos = (lordring_start_ + pos) % lordring_capacity_;
        vptr_ptr_[absolute_pos].~T();

        size_t deque_size = size();
        //都前移一位
        for (size_t i = 0; i < deque_size - pos - 1; ++i)
        {
            vptr_ptr_[(absolute_pos + i) % lordring_capacity_] =
                vptr_ptr_[(absolute_pos + i + 1) % lordring_capacity_];
        }
        lordring_end_ = (lordring_end_ > 0) ? lordring_end_ - 1 : lordring_capacity_ - 1;
        return false;
    }

    ///[]数组下标定位,pos不要越界，自己保证，我没兴趣为你干什么
    T &operator[](size_t pos)
    {
        return vptr_ptr_[(lordring_start_ + pos) % lordring_capacity_];
    }

    ///[]数组下标定位,pos不要越界，自己保证，我没兴趣为你干什么
    const T &operator[](size_t pos) const
    {
        return vptr_ptr_[(lordring_start_ + pos) % lordring_capacity_];
    }

    ///返回start的迭代器，开始就是序列号为0的位置
    iterator begin()
    {
        return iterator(lordring_start_, lordring_capacity_, vptr_ptr_, 0);
    }

    ///返回end的迭代器，开始就是序列号为cycdeque_size_的位置
    iterator end()
    {
        return iterator(lordring_start_, lordring_capacity_, vptr_ptr_, size());
    }

protected:

    ///循环队列的起始位置
    size_t                 lordring_start_ = 0;
    ///循环队列的长度，,思路仍然是前开后闭
    ///容器size通过lordring_end_ + lordring_capacity_ - lordring_start_
    /// 或者 lordring_end_ - lordring_start_得到，
    size_t                 lordring_end_ = 0;

    ///队列的长度，
    size_t                 lordring_capacity_ = 0;
    ///存放数据的指针
    T*                     vptr_ptr_ = nullptr;
};
}
