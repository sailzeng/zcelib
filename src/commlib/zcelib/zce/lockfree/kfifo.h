/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_lockfree_deque.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年12月16日
* @brief      一读一写不加锁的消息队列，lock-free的deque，
*
*
* @details    本来在里面放了一把锁，但是考虑既然是实现一个LOCKFREE的队列，锁就显得多余了。放弃了。
*             2019了。应该用atomic改写一下。
*
*
* @note
*
*/
#pragma once

#include "zce/lock/spin_lock.h"

namespace zce::lockfree
{
/*!
@brief

*/
/*!
* @brief      如果1个读,1个写不用加锁,因为用了些技巧,中间有一个空闲区,读只使用pend指针,写只使用pstart指针,
*             判断EMPTY,FULL使用快照,
*             如果非要多个进程读写,要加锁,要自己实现锁,我只提供了一个基类,
*             是一个先进，先出的存放任意大小的数据快的队列
* @tparam     T  Node 最开始的标识长度是用了几个字节
* note        不是容器模版,如果非要容器队列,用shm_list自己解决,很容易
*/

template<typename T>
concept integral_t = std::is_integral<T>::value;

template <typename T> requires integral_t<T>
class kfifo
{
public:

    /*!
    * @brief      可以放如deque的node结构，变长，前面若干个字节表示长度，
    *             长度用模版参数 T 控制
    *             外部使用时，只要你BUFFER最开始标识长度和INTEGRAL_T一致，
    *             就可以强转成指针使用
    *
    */
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
            static_assert(std::is_integral<T>::value, "Not integral!");
            assert(node_len > sizeof(T) &&
                   node_len <= static_cast<size_t>(std::numeric_limits<int>::max()));
            if (node_len <= sizeof(T) ||
                node_len > static_cast<size_t>(std::numeric_limits<int>::max()))

            {
                return nullptr;
            }
            char* ptr = ::new char[node_len];

#ifdef  DEBUG
            //检查帧的哪个地方出现问题，还是这样好一点
            memset(ptr, 0, node_len);
#endif
            //
            ((node*)ptr)->size_of_node_ = (T)node_len;

            return ((node*)ptr);
        }

        ///养成好习惯,写new,就写delete.
        static void delete_node(node * node)
        {
            char* ptr = (char*)node;
            delete[] ptr;
        }
    public:

        ///头部的长度，
        static const size_t NODE_HEAD_LEN = sizeof(T);

        ///最小的CHUNK NODE长度，NODE_HEAD_LEN+1
        static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = NODE_HEAD_LEN + 1;

        /// 整个Node的长度,包括size_of_node_ + chunkdata, 你可以用模版描述这个长度是多少
        /// 这里使用size_t,long在64位下会有问题
        T    size_of_node_;

        /// 数据区的数据，变长的数据,1只是占位符号
        char          chunk_data_[1];
    };

protected:
    /*!
    @brief      队列的头部数据，初始化的放在共享内存的最开始部分，用于记录队列
                的一些状态，关键数据，用于恢复，在恢复的时候会进行比对检查
                内部使用结构，所以前面加了_
    */
    class kfifo_head
    {
        //
        friend  class kfifo;

    protected:

        ///构造函数，不对外提供，
        kfifo_head() = default;
        ///析构函数
        ~kfifo_head() = default;

        //数据也不提供给大众访问，shm_kfifo除外
    protected:

        ///内存区的长度
        size_t               size_of_mem_ = 0;

        ///deque的长度,必须>JUDGE_FULL_INTERVAL
        size_t               size_of_cycle_ = 0;

        ///node的最大长度
        size_t               max_len_node_ = 0;

        ///两个关键内部指针,避免编译器优化,早期用的是volatile，现在改成atomic
        ///环形队列开始的地方，这个地方必现是机器字长
        std::atomic<size_t>  deque_begin_ = 0;
        ///环行队列结束的地方，这个地方必现是机器字长
        std::atomic<size_t>  deque_end_ = 0;
    };

public:

    ///构造函数，避免你用了
    kfifo() = default;
    ///析构函数
    ~kfifo() = default;

    ///只定义不实现
    kfifo(const kfifo &) = delete;
    const kfifo& operator=(const kfifo&) = delete;

protected:

    /*!
    @brief      得到两个关键指针的快照，用于判定队列里面的数据还有多少，是否为满或者空
                这些操作都是一些快照操作,不加锁进行，这个操作基于一点,32位操作系统中的
                32位整数操作是原子操作
                这儿费一点口舌解释为什么一个人放入,一个人读取是安全的,因为一个人同时只
                使用1个,即使错误也是一个快照错误
    @param      pstart  返回的循环队列起始位置
    @param      pend    返回的循环队列结束位置
    */
    void snap_getpoint(size_t& pstart, size_t& pend)
    {
        pstart = kfifo_head_->deque_begin_;
        pend = kfifo_head_->deque_end_;
        return;
    }
public:

    /*!
    @brief      得到要分配的空间的大小，
    @return     size_t         实际所需的空间长度，会大于你的长度，
    @param      size_of_deque  你所必须的deque长度,你按这个大小申请内存
    */
    static size_t alloc_size(const size_t size_of_deque)
    {
        return  sizeof(kfifo_head) + size_of_deque + JUDGE_FULL_INTERVAL;
    }

    /*!
    * @brief      根据参数初始化
    * @return     bool
    * @param      size_of_deque    deque的长度，(就是 alloc_size 的参数，不是返回值呀)
    * @param      max_len_node     放入的note最大长度，我会帮你检查一下
    * @param      mmap_ptr         内存的指针，共享内存也可以，普通内存也可以
    * @param      if_restore       是否是进行恢复操作，如果是，会保留原来的数据，如果不是，会调用clear清理
    */
    bool initialize(size_t size_of_deque,
                    size_t max_len_node,
                    char* mmap_ptr,
                    bool if_restore = false,
                    bool multi_thread = false)
    {
        //必须大于间隔长度
        if (size_of_deque <= sizeof(T) + JUDGE_FULL_INTERVAL)
        {
            return false;
        }

        //
        kfifo_head* dequechunk_head = reinterpret_cast<kfifo_head*>(mmap_ptr);

        //如果是恢复，检查几个值是否相等
        if (if_restore == true)
        {
            //坚持关键数据是否一致
            if (dequechunk_head->size_of_mem_ != alloc_size(size_of_deque)
                || dequechunk_head->size_of_cycle_ != size_of_deque + JUDGE_FULL_INTERVAL
                || dequechunk_head->max_len_node_ != max_len_node)
            {
                return false;
            }
        }

        dequechunk_head->size_of_mem_ = alloc_size(size_of_deque);
        dequechunk_head->size_of_cycle_ = size_of_deque + JUDGE_FULL_INTERVAL;
        dequechunk_head->max_len_node_ = max_len_node;

        //得到空间大小
        shm_base_ = mmap_ptr;
        kfifo_head_ = dequechunk_head;
        kfifo_data_ = mmap_ptr + sizeof(kfifo_head);
        if (if_restore == false)
        {
            clear();
        }
        //restore没事情可做
        if (multi_thread == false)
        {
            spin_lock_ = std::move(std::make_unique<zce::spin_lock>());
        }
        return true;
    }

    ///清理成没有使用过的状态
    void clear()
    {
        //
        kfifo_head_->deque_begin_ = 0;
        kfifo_head_->deque_end_ = 0;
        ::memset(kfifo_data_, 0, kfifo_head_->size_of_cycle_);
    }
    /*!
    @brief      将一个NODE放入尾部
    @return     bool
    @param      node
    */
    bool push_end(const node* i)
    {
        //粗略的检查,如果长度不合格,返回不成功
        if (i->size_of_node_ < node::MIN_SIZE_DEQUE_CHUNK_NODE ||
            i->size_of_node_ > kfifo_head_->max_len_node_)
        {
            return false;
        }

        //检查队列的空间是否够用
        if (free() < i->size_of_node_)
        {
            return false;
        }

        //如果空间足够
        char* pend = kfifo_data_ + kfifo_head_->deque_end_;

        //如果绕圈
        if (pend + i->size_of_node_ > kfifo_data_ + kfifo_head_->size_of_cycle_)
        {
            size_t first = kfifo_head_->size_of_cycle_ - kfifo_head_->deque_end_;
            size_t second = i->size_of_node_ - first;
            memcpy(pend, reinterpret_cast<const char*>(i), first);
            memcpy(kfifo_data_, reinterpret_cast<const char*>(i) + first, second);
            kfifo_head_->deque_end_ = second;
        }
        //如果可以一次拷贝完成
        else
        {
            memcpy(pend, reinterpret_cast<const char*>(i), i->size_of_node_);
            kfifo_head_->deque_end_ += i->size_of_node_;
        }

        return true;
    }

    /*!//将队列一个NODE拷贝取出,
    @brief      将队列一个NODE从队首部取出,要求node!=nullptr,已经分配好了数据区
    @return     bool  true表示成功取出，否则表示没有取出
    @param      node  保存pop 数据的的buffer，
    */
    bool pop_front(node* const i)
    {
        assert(i != nullptr);

        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        char* pbegin = kfifo_data_ + kfifo_head_->deque_begin_;
        size_t tmplen = get_front_len();

        assert(tmplen > 0);
        assert(tmplen < 64 * 1024);
        assert(kfifo_head_->deque_begin_ <= kfifo_head_->size_of_cycle_);

        //如果被分为2截
        if (pbegin + tmplen > kfifo_data_ + kfifo_head_->size_of_cycle_)
        {
            size_t first = kfifo_head_->size_of_cycle_ - kfifo_head_->deque_begin_;
            size_t second = tmplen - first;
            memcpy(reinterpret_cast<char*>(i), pbegin, first);
            memcpy(reinterpret_cast<char*>(i) + first, kfifo_data_, second);
            kfifo_head_->deque_begin_ = second;
        }
        else
        {
            memcpy(reinterpret_cast<char*>(i), pbegin, tmplen);
            kfifo_head_->deque_begin_ += i->size_of_node_;
            assert(kfifo_head_->deque_begin_ <= kfifo_head_->size_of_cycle_);
        }

        assert(kfifo_head_->deque_begin_ <= kfifo_head_->size_of_cycle_);

        return true;
    }

    /*!
    @brief      读取队列的第一个NODE,但是不取出,要求node!=nullptr,我认为你已经分配好了数据区
    @return     bool  true表示成功读取
    @param      node  保存read 数据的的buffer，
    */
    bool read_front(node* const i)
    {
        assert(i != nullptr);

        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        //如果空间足够
        char* pbegin = kfifo_data_ + kfifo_head_->deque_begin_;
        size_t tmplen = get_front_len();

        //如果被分为2截
        if (pbegin + tmplen > kfifo_data_ + kfifo_head_->size_of_cycle_)
        {
            size_t first = kfifo_head_->size_of_cycle_ - kfifo_head_->deque_begin_;
            size_t second = tmplen - first;
            memcpy(reinterpret_cast<char*>(i), pbegin, first);
            memcpy(reinterpret_cast<char*>(i) + first, kfifo_data_, second);
        }
        else
        {
            memcpy(reinterpret_cast<char*>(i), pbegin, tmplen);
        }

        return true;
    }

    /*!
    @brief      将队列一个NODE从队首部取出,我根据node的长度帮你分配空间,
                要求new_node=nullptr,表示你要函数帮你分配缓冲,
    @return     bool      true表示成功读取
    @param      new_node  获得数据的指针，这个数据你要自己释放，我概不负责了
    */
    bool pop_front_new(node*& n)
    {
        assert(n == nullptr);

        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        size_t node_len = get_front_len();
        n = node::new_node(node_len);

        //这样写会有一些重复调用，但是我觉得这个地方性能不会是问题。
        return pop_front(n);
    }

    /*!
    @brief      读取队列的第一个NODE，我根据node的长度帮你分配空间,要求new_node=nullptr,表示你要函数帮你分配缓冲,
    @return     bool      true表示成功读取
    @param      new_node
    */
    bool read_front_new(node*& n)
    {
        assert(n == nullptr);

        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        size_t node_len = get_front_len();
        n = node::new_node(node_len);

        return read_front(n);
    }

    /*!
    @brief      丢弃队列前面的第一个NODE
    @return     bool 是否丢弃成功
    */
    bool discard_frond()
    {
        //检查是否为空
        if (empty() == true)
        {
            return false;
        }

        //如果空间足够
        char* pbegin = kfifo_data_ + kfifo_head_->deque_begin_;
        size_t tmplen = get_front_len();

        //如果要求帮使用者分配,切记释放,

        //如果被分为2截
        if (pbegin + tmplen > kfifo_data_ + kfifo_head_->size_of_cycle_)
        {
            size_t first = kfifo_head_->size_of_cycle_ - kfifo_head_->deque_begin_;
            size_t second = tmplen - first;
            kfifo_head_->deque_begin_ = second;
        }
        else
        {
            kfifo_head_->deque_begin_ += tmplen;
        }

        return true;
    }

    ///取队列头的buffer长度,你必须在确认pipe里面有数据才能调用这个函数，否则后果自负。
    ///因为这个函数的使用语境大部分是empty之后，
    inline size_t get_front_len()
    {
        //还是要担心长度截断2节,头大,头大,多写好多代码
        char* tmp1 = kfifo_data_ + kfifo_head_->deque_begin_;
        T node_len = 0;
        char* tmp2 = reinterpret_cast<char*>(&node_len);

        //如果管道的长度也绕圈，采用野蛮的法子得到长度
        if (tmp1 + node::NODE_HEAD_LEN >
            kfifo_data_ + kfifo_head_->size_of_cycle_)
        {
            //一个个字节读取长度
            for (size_t i = 0; i < sizeof(T); ++i)
            {
                if (tmp1 >= kfifo_data_ + kfifo_head_->size_of_cycle_)
                {
                    tmp1 = kfifo_data_;
                }

                *tmp2 = *tmp1;
                ++tmp1;
                ++tmp2;
            }
        }
        //
        else
        {
            node_len = *(reinterpret_cast<T*>(tmp1));
        }

        return node_len;
    }

    ///得到FREE空间的快照
    size_t free()
    {
        //取快照
        size_t start, end, sz_free;
        snap_getpoint(start, end);

        //计算尺寸
        if (start == end)
        {
            sz_free = kfifo_head_->size_of_cycle_;
        }
        else if (start < end)
        {
            sz_free = kfifo_head_->size_of_cycle_ - (end - start);
        }
        else
        {
            sz_free = start - end;
        }

        //重要：FREE长度应该减去预留部分长度，保证首尾不会相接
        sz_free -= JUDGE_FULL_INTERVAL;
        return sz_free;
    }

    ///容量
    size_t capacity()
    {
        return kfifo_head_->size_of_mem_;
    }

    ///得到是否满的快照
    bool empty()
    {
        return free() == kfifo_head_->size_of_cycle_ - JUDGE_FULL_INTERVAL;
    }

    ///得到是否空的快照
    bool full()
    {
        return free() == 0;
    }

protected:

    ///判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    ///deque_begin_ = deque_end_ 表示队列为nullptr
    ///deque_begin_ = deque_end_ + JUDGE_FULL_INTERVAL 表示队列满
    static const size_t   JUDGE_FULL_INTERVAL = 8;

protected:

    //内存基础地址
    char* shm_base_ = nullptr;

    ///内存的头部
    kfifo_head* kfifo_head_ = nullptr;

    ///数据区的头指针,方便计算
    char* kfifo_data_ = nullptr;

    //!spin_lock
    std::unique_ptr<zce::spin_lock>  spin_lock_;
};
};
