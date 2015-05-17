/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_lord_rings.h
* @author     Sailzeng <sailerzeng@gmail.com>
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
*             2011年10月7日 做了一次修正，原来的实现是用空间+1标识前闭后开的空间，后来还
*             是加入了size记录，不需要再增加1个空间标识后开空间了。同时增加了迭代器。
*
*             喔姐姐，我想回家，牵着我的手，我有些困了
*/

#ifndef ZCE_LIB_BOOST_LORD_RINGS_H_
#define ZCE_LIB_BOOST_LORD_RINGS_H_

/*!
* @namespace ZCE_LIB 主要用于模版类的名字空间，为了和std统一，模版类一般用了小写
*/

namespace ZCE_LIB
{

/*!
* @tparam    _value_type 数据类型
* @brief     魔戒的的迭代器，在魔戒类里面有typedef成为iterator，
* @note      记住lordrings的迭代器比vector更加危险，因为他的空间是循环利用的，任何一次push,pop操作都会让迭代器失效。
*            所以使用的时候当心，
*            个人觉得迭代器的作用不大，因为魔戒的使用主要头尾，几乎不用遍历使用。
*/
template <class _value_type >
class _lordrings_iterator
{
protected:

    //定义迭代器
    typedef _lordrings_iterator<_value_type > iterator;


    //迭代器萃取器所有的东东
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::random_access_iterator_tag iterator_category;

public:
    ///构造函数
    _lordrings_iterator():
        rings_start_(0),
        rings_capacity_(0),
        rings_value_base_(NULL),
        serial_(0)
    {
    }
    ///构造函数
    _lordrings_iterator(size_t rings_start,
                        size_t rings_capacity,
                        _value_type *rings_value_base,
                        size_t serial
                       ):
        rings_start_(rings_start),
        rings_capacity_(rings_capacity),
        rings_value_base_(rings_value_base),
        serial_(serial)
    {
    }
    ///析构函数
    ~_lordrings_iterator()
    {
    }

    ///*提领操作，
    _value_type &operator*() const
    {
        return *(rings_value_base_ + (rings_start_ +  serial_) % rings_capacity_);
    }

    ///->操作
    _value_type *operator->() const
    {
        return rings_value_base_ + (rings_start_ +  serial_) % rings_capacity_;
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
        ++*this;
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
        --*this;
        return tmp;
    }

    ///迭代器判定相等
    bool operator==(const iterator &it) const
    {
        if (rings_value_base_ == it.rings_value_base_
            && rings_start_ == it.rings_start_
            && rings_capacity_ == it.rings_capacity_
            && serial_ == it.serial_ )
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
    size_t                 rings_start_;
    ///魔戒的容量
    size_t                 rings_capacity_;
    ///数据的指针，
    _value_type           *rings_value_base_;

    //序列号,迭代器前进，增加的就是这个值
    size_t                 serial_;
};

/*!
* @tparam    _value_type 数据类型
* @brief     魔戒.循环链表，可以自动扩展，可以最后的覆盖第一个，
*/
template<class _value_type >
class lordrings
{
public:
    //定义迭代器
    typedef _lordrings_iterator<_value_type > iterator;

public:
    ///构造函数，后面必须调用,initialize
    lordrings():
        cycdeque_start_(0),
        cycdeque_size_(0),
        cycdeque_capacity_(0),
        value_ptr_(NULL)
    {
    }

    ///构造函数，同时完成初始化,后面完全 没有必要调用,initialize
    lordrings(size_t max_len):
        cycdeque_start_(0),
        cycdeque_size_(0),
        cycdeque_capacity_(max_len),
        value_ptr_(NULL)
    {
        assert(max_len > 0);
        value_ptr_ = new _value_type[cycdeque_capacity_];
    }

    ///析构函数，释放空间
    ~lordrings()
    {
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }
    }

    ///初始化数据区，和构造函数干的事情基本一样，只是多了一步原来有数据就清理掉
    void initialize(size_t max_len)
    {
        assert(max_len > 0);

        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
        cycdeque_capacity_ = max_len;

        //清理现场
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }

        value_ptr_ = new _value_type[cycdeque_capacity_];
    }

    ///结束，完成，销毁，求问fini是什么的缩写，
    void finish()
    {
        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
        cycdeque_capacity_ = 0;

        //清理现场
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }
    }

    ///清理管道，
    void clear()
    {
        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
    }

    ///尺寸空间
    inline size_t size() const
    {
        return cycdeque_size_;
    }
    ///返回空闲空间的大小
    inline size_t freesize() const
    {
        return cycdeque_capacity_ - cycdeque_size_ ;
    }

    ///返回队列的容量
    inline size_t capacity() const
    {
        return cycdeque_capacity_;
    }

    ////检查是否已经满了
    inline bool full() const
    {
        //已经用的空间等于容量
        if (cycdeque_size_  == cycdeque_capacity_ )
        {
            return true;
        }

        return false;
    }

    ///判断队列是否为空
    inline bool empty() const
    {
        //空间等于0
        if (cycdeque_size_ == 0)
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
        if ( deque_size > new_max_size )
        {
            return false;
        }

        _value_type *new_value_ptr = new _value_type[new_max_size];

        //如果原来有数据
        if (value_ptr_ != NULL)
        {
            for (size_t i = 0; i < deque_size; ++i)
            {
                new_value_ptr[i] = value_ptr_[(cycdeque_start_ + i) % cycdeque_capacity_];
            }

            delete[] value_ptr_;
            value_ptr_ = NULL;
        }

        //调整几个内部参数
        cycdeque_start_ = 0;
        cycdeque_capacity_ = new_max_size ;
        //cycdeque_size_ 不变

        value_ptr_ = new_value_ptr;

        return true;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_back(const _value_type &value_data, bool lay_over = false)
    {
        //如果已经满了
        if ( full() )
        {
            //如果不要覆盖，返回错误
            if (lay_over == false)
            {
                return false;
            }
            //如果要覆盖
            else
            {
                //将最后一个位置覆盖，并且调整起始和结束位置
                value_ptr_[ (cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_] = value_data;
                cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;

                return true;
            }
        }

        //直接放在队尾

        value_ptr_[(cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_] = value_data;
        ++cycdeque_size_;

        return true;
    }

    ///将一个数据放入队列的尾部,如果队列已经满了,你可以将lay_over参数置位true,覆盖原有的数据
    bool push_front(const _value_type &value_data, bool lay_over = false)
    {
        //如果已经满了
        if ( full() )
        {
            //如果不要覆盖，返回错误
            if (lay_over == false)
            {
                return false;
            }
            //如果要覆盖
            else
            {
                //将第一个位置调整覆盖，并且调整起始和结束位置
                cycdeque_start_ = (cycdeque_start_ > 0) ? cycdeque_start_ - 1 : cycdeque_capacity_ - 1;
                value_ptr_[cycdeque_start_] = value_data;

                //覆盖，尺寸也不用调整

                return true;
            }
        }

        //直接放在队尾
        cycdeque_start_ = (cycdeque_start_ > 0) ? cycdeque_start_ - 1 : cycdeque_capacity_ - 1;
        value_ptr_[cycdeque_start_] = value_data;

        ++cycdeque_size_;

        return true;
    }

    ///从队列的前面pop并且得到一个数据
    bool pop_front(_value_type &value_data)
    {
        //如果是空的返回错误
        if ( empty() )
        {
            return false;
        }

        value_data = value_ptr_[cycdeque_start_];
        cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;

        --cycdeque_size_;

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

        cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;
        --cycdeque_size_;

        return true;
    }

    ///从队列的尾部pop并且得到一个数据
    bool pop_back(_value_type &value_data)
    {
        //如果是空的返回错误
        if ( empty())
        {
            return false;
        }

        value_data = value_ptr_[(cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_];
        --cycdeque_size_;
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

        --cycdeque_size_;

        return true;
    }

    ///[]数组下标定位,ID不要越界，自己保证，我没兴趣为你干什么
    _value_type &operator[](size_t id)
    {
        return value_ptr_[(cycdeque_start_ + id) % cycdeque_capacity_];
    }

    ///[]数组下标定位,ID不要越界，自己保证，我没兴趣为你干什么
    const _value_type &operator[](size_t id) const
    {
        return value_ptr_[(cycdeque_start_ + id) % cycdeque_capacity_];
    }

    ///返回start的迭代器，开始就是序列号为0的位置
    iterator begin()
    {
        return iterator(cycdeque_start_, cycdeque_capacity_, value_ptr_, 0);
    }

    ///返回end的迭代器，开始就是序列号为cycdeque_size_的位置
    iterator end()
    {
        return iterator(cycdeque_start_, cycdeque_capacity_, value_ptr_, cycdeque_size_);
    }

protected:

    ///循环队列的起始位置
    size_t                 cycdeque_start_;

    ///循环队列的长度，结束位置通过cycdeque_start_+cycdeque_size_%cycdeque_capacity_得到，思路仍然是前开后闭
    size_t                 cycdeque_size_;
    ///队列的长度，
    size_t                 cycdeque_capacity_;
    ///存放数据的指针
    _value_type           *value_ptr_;

};

};

#endif //#ifndef ZCE_LIB_BOOST_LORD_RINGS_H_

