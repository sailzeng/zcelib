#pragma once
/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_vector.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月21日
* @brief      在共享内存中使用的vector，其实就是数组（向量）了，
*             但是没有动态扩展特性，这个见谅，因为我毕竟在一块内存里面
*             干活，扩展的事情，亲那是你的！
* @details
*
*/

#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
/*!
* @brief      共享内存中使用的vector，彻底简化版本
* @tparam     _value_type  数组类型
*/
template <class T> class shm_vector
{
    //定义typedef
private:
    typedef shm_vector<T> self;
public:
    typedef T* iterator;
    typedef T value_type;
    typedef std::size_t size_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;

protected:

    ///共享内存vector头部数据区
    class _shm_vector_head
    {
    protected:

        //头部构造函数
        _shm_vector_head() = default;
        //析构函数
        ~_shm_vector_head() = default;

    public:

        ///内存区的长度
        size_type      size_of_mmap_ = 0;
        ///结点总数，
        size_type      num_of_node_ = 0;

        ///表示目前使用的结点个数
        size_type      num_of_use_ = 0;
    };

protected:

    ///默认构造函数,就是不给你用
    shm_vector() = default;

    //只定义,不实现,
    shm_vector(const shm_vector&) = delete;
    const shm_vector& operator=(const shm_vector& others) = delete;

public:

    ///析构函数
    ~shm_vector() = default;
public:

    ///内存区的构成为 定义区,data区,返回所需要的长度,
    static size_type alloc_size(const size_type numnode)
    {
        return  sizeof(_shm_vector_head) + sizeof(T) * (numnode);
    }

    ///初始化
    static self* initialize(const size_type numnode, char* pmmap, bool if_restore = false)
    {
        _shm_vector_head* aryhead = reinterpret_cast<_shm_vector_head*>(pmmap);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(numnode) != aryhead->size_of_mmap_ ||
                numnode != aryhead->num_of_node_)
            {
                return nullptr;
            }
        }

        //初始化尺寸
        aryhead->size_of_mmap_ = alloc_size(numnode);
        aryhead->num_of_node_ = numnode;

        self* instance = new self();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        instance->mem_addr_ = pmmap;
        instance->vector_head_ = aryhead;
        instance->data_base_ = reinterpret_cast<T*>(pmmap + sizeof(_shm_vector_head));

        if (if_restore)
        {
            instance->resotre();
        }
        else
        {
            instance->clear();
        }

        //打完收工
        return instance;
    }

    //!销毁，析构所有的元素，注意，如果想恢复，不要调用这个函数
    void terminate()
    {
        //生产默认的数据,析构
        for (size_type i = 0; i < vector_head_->num_of_use_; ++i)
        {
            (data_base_ + i)->~T();
        }
        vector_head_->num_of_use_ = 0;
    }

    void resotre()
    {
        //重新恢复T，如果T有虚表等指针数据，可以再次恢复，
        for (size_type i = 0; i < vector_head_->num_of_use_; ++i)
        {
            value_type old(std::move(*(data_base_ + i)));
            new(data_base_ + i) value_type(old);
        }
    }

    void resize(size_type num)
    {
        ZCE_ASSERT_DEBUG(num <= vector_head_->num_of_node_);

        //如果是扩大空间，生产数据
        if (num > vector_head_->num_of_use_)
        {
            //生产默认的数据
            for (size_t i = vector_head_->num_of_use_; i < num; ++i)
            {
                new (data_base_ + i) T();
            }
        }
        //如果是缩小空间,销毁数据，调用析构
        else if (num < vector_head_->num_of_use_)
        {
            //生产默认的数据,析构
            for (size_t i = num; i < vector_head_->num_of_use_; ++i)
            {
                (data_base_ + i)->~T();
            }
        }
        //等于，什么都不做
        else
        {
        }
        vector_head_->num_of_use_ = num;
    }

    ///清理
    void clear()
    {
        vector_head_->num_of_use_ = 0;
    }

    ///用[]访问数据，越界了自己负责
    T& operator[](size_type n)
    {
        ZCE_ASSERT_DEBUG(n < vector_head_->num_of_use_);
        return *(data_base_ + n);
    }

    ///开始位置的迭代器
    iterator begin()
    {
        return data_base_;
    }
    ///结束位置的迭代器
    iterator end()
    {
        return data_base_ + vector_head_->num_of_use_;
    }

    ///是否为满和空
    bool empty() const
    {
        return (vector_head_->num_of_use_ == 0);
    }
    ///
    bool full() const
    {
        return (vector_head_->num_of_use_ == vector_head_->num_of_node_);
    }

    //使用了的空间的数量
    size_type size() const
    {
        return vector_head_->num_of_use_;
    }
    //返回链表池子的容量
    size_type capacity() const
    {
        return vector_head_->num_of_node_;
    }
    //剩余空间的容量
    size_type free() const
    {
        return vector_head_->num_of_node_ - vector_head_->num_of_use_;
    }

    ///关键位置
    T& front()
    {
        return data_base_;
    }
    ///
    T& back()
    {
        return *(data_base_ + (vector_head_->num_of_use_ - 1));
    }

    ///向后添加数据
    bool push_back(const T& val)
    {
        return push_back_i(val);
    }

    ///向后添加数据
    bool push_back(T&& val)
    {
        return push_back_i(val);
    }

    ///从后面删除数据
    bool pop_back()
    {
        if (vector_head_->num_of_use_ == 0)
        {
            return false;
        }

        //显式调用析构函数
        (data_base_ + vector_head_->num_of_use_)->~T();

        --(vector_head_->num_of_use_);
        return true;
    }
protected:

    template<typename U>
    bool push_back_i(U&& val)
    {
        if (vector_head_->num_of_use_ == vector_head_->num_of_node_)
        {
            return false;
        }

        //使用placement new 复制对象
        new (data_base_ + vector_head_->num_of_use_) T(std::forward<U>(val));
        ++(vector_head_->num_of_use_);
        return true;
    }
protected:
    //内存基础地址
    char* mem_addr_ = nullptr;
    ///
    _shm_vector_head* vector_head_ = nullptr;
    ///数据区起始指针,
    value_type* data_base_ = nullptr;
};
};
