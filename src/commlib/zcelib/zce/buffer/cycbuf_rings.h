/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief      循环的一个buffer，里面每个存放node，每个node都是的头部都有一个
*             长度标识，长度标识可以是字节，双字节，四字节的整数，通过模板参数
*             定义。node可以是变长的，你可以在buffer中放入node，取出node。
*             操作方式是fifo的。
*
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
template <typename INTEGRAL_T>
class cycbuf_rings
{
public:

    class node
    {
    protected:
        node() = delete;
        node& operator=(const node & others) = delete;
        ~node() = delete;
    public:
        ///*!
        //@brief      重载了new操作，用于得到一个变长得到架构
        //@return     void* operator
        //@param      size_t    new的默认参数
        //@param      node_len   node节点的长度
        //*
        static node* new_node(size_t node_len)
        {
            static_assert(std::is_integral<INTEGRAL_T>::value, "Not integral!");
            assert(node_len > sizeof(INTEGRAL_T) &&
                   node_len <= static_cast<size_t>(std::numeric_limits<int>::max()));
            if (node_len <= sizeof(INTEGRAL_T) ||
                node_len > static_cast<size_t>(std::numeric_limits<int>::max()))
            {
                return nullptr;
            }
            char* ptr = ::new char[node_len];

#ifdef  DEBUG
            memset(ptr, 0, node_len);
#endif
            //
            ((node*)ptr)->size_of_node_ = (INTEGRAL_T)node_len;
            return ((node*)ptr);
        }

        ///养成好习惯,写new,就写delete.
        static void delete_node(node* node)
        {
            char* ptr = (char*)node;
            delete[] ptr;
        }

        /// 整个Node的长度,包括size_of_node_ + node_data_数据,
        /// 这里使用size_t,long在64位下会有问题
        INTEGRAL_T    size_of_node_;

        /// 数据区的数据，变长的数据
        char            node_data_[1];
    };
public:

    ///头部的长度，
    static const size_t NODE_HEAD_LEN = sizeof(INTEGRAL_T);

    ///最小的CHUNK NODE长度，4+1
    static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = NODE_HEAD_LEN + 1;

public:

    cycbuf_rings()
    {
    }

    ~cycbuf_rings()
    {
        if (cycbuf_data_)
        {
            delete[] cycbuf_data_;
            cycbuf_data_ = nullptr;
        }
    }

    ///得到FREE空间的快照
    size_t free()
    {
        //计算尺寸
        size_t sz_free;
        if (cycbuf_begin_ == cycbuf_end_)
        {
            sz_free = size_of_cycle_;
        }
        else if (cycbuf_begin_ < cycbuf_end_)
        {
            sz_free = size_of_cycle_ - (cycbuf_end_ - cycbuf_begin_);
        }
        else
        {
            sz_free = cycbuf_begin_ - cycbuf_end_;
        }
        sz_free = sz_free - JUDGE_FULL_INTERVAL;
        return sz_free;
    }

    ///容量
    size_t capacity()
    {
        return size_of_cycle_ - JUDGE_FULL_INTERVAL;
    }

    ///得到是否为空
    bool empty()
    {
        return cycbuf_begin_ == cycbuf_end_;
    }

    ///得到是否空
    bool full()
    {
        return free() == 0;
    }

    void clear()
    {
        cycbuf_begin_ = 0;
        cycbuf_end_ = 0;
        memset(cycbuf_data_, 0, size_of_cycle_);
    }

    bool initialize(size_t size_of_deque,
                    size_t max_len_node)
    {
        //必须大于间隔长度
        if (size_of_deque <= sizeof(INTEGRAL_T) + JUDGE_FULL_INTERVAL)
        {
            return false;
        }
        if (max_len_node > static_cast<size_t>(
            std::numeric_limits<int>::max()))
        {
            return false;
        }
        size_of_cycle_ = size_of_deque + JUDGE_FULL_INTERVAL;
        max_len_node_ = max_len_node;

        cycbuf_data_ = new char[size_of_cycle_];

        clear();
        return true;
    }

    //将一个NODE放入尾部
    bool push_end(const node* node)
    {
        //粗略的检查,如果长度不合格,返回不成功
        if (node->size_of_node_ < sizeof(INTEGRAL_T) ||
            node->size_of_node_ > max_len_node_)
        {
            return false;
        }

        //检查队列的空间是否够用
        if (free() < node->size_of_node_)
        {
            return false;
        }

        //如果空间足够
        char* pend = cycbuf_data_ + cycbuf_end_;

        //如果绕圈
        if (pend + node->size_of_node_ > cycbuf_data_ + size_of_cycle_)
        {
            size_t first = size_of_cycle_ - cycbuf_end_;
            size_t second = node->size_of_node_ - first;
            memcpy(pend, reinterpret_cast<const char*>(node), first);
            memcpy(cycbuf_data_, reinterpret_cast<const char*>(node) + first, second);
            cycbuf_end_ = second;
        }
        //如果可以一次拷贝完成
        else
        {
            memcpy(pend, reinterpret_cast<const char*>(node), node->size_of_node_);
            cycbuf_end_ += node->size_of_node_;
        }

        return true;
    }

    bool pop_front(node* const node)
    {
        assert(node != NULL);

        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        char* pbegin = cycbuf_data_ + cycbuf_begin_;
        size_t node_len = get_front_len();

        assert(node_len > 0);
        assert(cycbuf_begin_ <= size_of_cycle_);

        //如果被分为2截
        if (pbegin + node_len > cycbuf_data_ + size_of_cycle_)
        {
            size_t first = size_of_cycle_ - cycbuf_begin_;
            size_t second = node_len - first;
            memcpy(reinterpret_cast<char*>(node), pbegin, first);
            memcpy(reinterpret_cast<char*>(node) + first, cycbuf_data_, second);
            cycbuf_begin_ = second;
        }
        else
        {
            memcpy(reinterpret_cast<char*>(node), pbegin, node_len);
            cycbuf_begin_ += node->size_of_node_;
            assert(cycbuf_begin_ <= size_of_cycle_);
        }

        assert(cycbuf_begin_ <= size_of_cycle_);

        return true;
    }

    //因为这个函数的使用语境大部分是empty之后，
    size_t get_front_len()
    {
        //还是要担心长度截断2节,头大,头大,多写好多代码
        char* tmp1 = cycbuf_data_ + cycbuf_begin_;
        INTEGRAL_T node_len = 0;
        char* tmp2 = reinterpret_cast<char*>(&node_len);

        //如果管道的长度也绕圈，采用野蛮的法子得到长度
        if (tmp1 + NODE_HEAD_LEN > cycbuf_data_ + size_of_cycle_)
        {
            //一个个字节读取长度
            for (size_t i = 0; i < NODE_HEAD_LEN; ++i)
            {
                if (tmp1 >= cycbuf_data_ + size_of_cycle_)
                {
                    tmp1 = cycbuf_data_;
                }

                *tmp2 = *tmp1;
                ++tmp1;
                ++tmp2;
            }
        }
        //
        else
        {
            node_len = *(reinterpret_cast<INTEGRAL_T*>(tmp1));
        }

        return node_len;
    }

protected:

    ///判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    ///cycbuf_begin_ = cycbuf_end_ 表示队列为NULL
    ///cycbuf_begin_ = cycbuf_end_ + JUDGE_FULL_INTERVAL 表示队列满
    static const size_t   JUDGE_FULL_INTERVAL = 8;

protected:

    ///deque的长度,必须>JUDGE_FULL_INTERVAL
    size_t size_of_cycle_ = 0;

    ///node的最大长度
    size_t max_len_node_ = 0;

    ///两个关键内部指针,避免编译器优化
    ///环形队列开始的地方，这个地方必现是机器字长
    size_t cycbuf_begin_ = 0;
    ///环行队列结束的地方，这个地方必现是机器字长
    size_t cycbuf_end_ = 0;

    char* cycbuf_data_ = nullptr;
};

//node的头部标识长度的字节长度是uint16_t还是uint32_t
typedef cycbuf_rings<uint16_t> cycbuf_rings_u16;
typedef cycbuf_rings<uint32_t> cycbuf_rings_u32;
typedef cycbuf_rings<uint64_t> cycbuf_rings_u64;
}