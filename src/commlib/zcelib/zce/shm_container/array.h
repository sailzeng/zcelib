/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_vector.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月21日
* @brief      2014年，根据新的C++11的规范，重新简化了一个shm_array，
*             shm_array是一开始就分配好空间的，数据尺寸是固定的
*             不能push_back的。
*
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
template <class T> class shm_array
{
    //定义typedef
private:
    typedef shm_array<T> self;
public:
    typedef T* iterator;
    typedef const iterator const_iterator;
    typedef T value_type;
    typedef shmc_size_type size_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;

protected:
    ///共享内存vector头部数据区
    class _array_head
    {
    protected:

        //头部构造函数
        _array_head() = default;
        //析构函数
        ~_array_head() = default;

    public:

        ///内存区的长度
        std::size_t      size_of_mem_ = 0;
        ///结点总数，
        size_type        num_of_node_ = 0;
    };

public:
    ///默认构造函数,就是不给你用
    shm_array() = default;
    //只定义,不实现,
    shm_array(const shm_array&) = delete;
    const self& operator=(const self& others) = delete;
    ///析构函数
    ~shm_array()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }
public:

    ///内存区的构成为 定义区,data区,返回所需要的长度,
    static std::size_t alloc_size(size_type numnode)
    {
        return  sizeof(_array_head) + sizeof(T) * (numnode);
    }

    ///初始化
    bool initialize(size_type num_node,
                    char* mem_addr,
                    bool if_restore = false)
    {
        _array_head* aryhead = reinterpret_cast<_array_head*>(mem_addr);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(num_node) != aryhead->size_of_mem_ ||
                num_node != aryhead->num_of_node_)
            {
                return nullptr;
            }
        }

        //初始化尺寸
        aryhead->size_of_mem_ = alloc_size(num_node);
        aryhead->num_of_node_ = num_node;

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        mem_addr_ = mem_addr;
        array_head_ = aryhead;
        data_base_ = reinterpret_cast<T*>(mem_addr + sizeof(_array_head));

        if (if_restore)
        {
            resotre();
        }
        else
        {
            clear();
        }
        //打完收工
        return true;
    }

    bool initialize(size_type num_node)
    {
        std::size_t sz_alloc = alloc_size(num_node);
        //自己分配一个空间，自己使用
        char *mem_addr = new char[sz_alloc];
        slef_alloc_ = true;
        return initialize(num_node, mem_addr, false);
    }

    //!销毁，析构所有的元素，注意，如果想恢复，不要调用这个函数
    void terminate()
    {
        //生产默认的数据,析构
        for (size_type i = 0; i < array_head_->num_of_node_; ++i)
        {
            (data_base_ + i)->~T();
        }
        array_head_->num_of_node_ = 0;
        if (slef_alloc_)
        {
            delete[] mem_addr_;
            mem_addr_ = nullptr;
        }
    }

    void resotre()
    {
        //重新恢复T，如果T有虚表等指针数据，可以再次恢复，
        for (size_type i = 0; i < array_head_->num_of_node_; ++i)
        {
            T old(std::move(*(data_base_ + i)));
            new(data_base_ + i)T(old);
        }
    }

    ///清理
    void clear()
    {
        //生产默认的数据,这儿是重新构造，不是析构
        for (size_type i = 0; i < array_head_->num_of_node_; ++i)
        {
            new(data_base_ + i)T();
        }
    }

    ///用[]访问数据，越界了自己负责
    T& operator[](size_type n)
    {
        ZCE_ASSERT(n < array_head_->num_of_node_);
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
        return data_base_ + array_head_->num_of_node_;
    }

    //使用了的空间的数量
    size_type size() const
    {
        return array_head_->num_of_node_;
    }
    //返回链表池子的容量
    size_type capacity() const
    {
        return array_head_->num_of_node_;
    }

    ///头部位置
    T& front()
    {
        return data_base_;
    }
    ///尾部位置
    T& back()
    {
        return *(data_base_ + (array_head_->num_of_node_ - 1));
    }

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;

    ///保存一些核心数据的头部指针，目的为了在共享内存使用
    _array_head* array_head_ = nullptr;
    ///数据区起始指针,
    T* data_base_ = nullptr;
};
};
