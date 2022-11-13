/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/avltree.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年1月16日
* @brief      希望AVLTree主要是完成可以排序的MAP,SET,的MMAP类
*
* @details
*
* @note       这个代码很早想写，但到了2014年3月8日我也没有真正动笔，
*             主要原因是对AVL的删除代码一直没有用心看，所以结果就一直耽搁了。
*             07年的时候，scottxu跳出来，单枪匹马把红黑树搞定了，这个代码
*             就一直没有了用武之地，呵呵。也许那天我还是会实现的。
*
*             一个人坐在办公室，叼根烟装酷。心中暗骂自己，2B。
*
*             开始搞的时候，认真看了Scott的红黑树的实现，发现底质还可以，
*             可以直接在上面改，同时看到这个代码，应该可以参考用，
*             最后发现，写起来最苦逼的还是删除node代码，我至少废了2个方案。
*             因为很多帖子的方案都不太靠谱，我在如何把要删除的节点交换到叶子
*             节点上有犯了傻。
*             而且AVL的删除后，节点的调整也不止那4种变换。
*             http://www.cnblogs.com/fullsail/p/3729015.html
*
*/

#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
template<class T, class Key, class Extract, class Compare> class shm_avltree;

//AVL tree的迭代器
template < class T,
    class Key,
    class Extract,
    class Compare >
class _avl_tree_iterator
{
    typedef _avl_tree_iterator<T, Key, Extract, Compare> iterator;

    typedef shm_avltree<T, Key, Extract, Compare> avl_tree_t;

    //迭代器萃取器所有的东东
    typedef shmc_size_type size_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    //构造函数
    _avl_tree_iterator(size_type seq, avl_tree_t* instance)
        : serial_(seq)
        , avl_tree_inst_(instance)
    {
    }

    _avl_tree_iterator() = default;
    ~_avl_tree_iterator() = default;

    //初始化
    void initialize(size_type seq, avl_tree_t* instance)
    {
        serial_ = seq;
        avl_tree_inst_ = instance;
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_type getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator& x) const
    {
        return (serial_ == x.serial_ && avl_tree_inst_ == x.avl_tree_inst_);
    }
    bool operator!=(const iterator& x) const
    {
        return !(*this == x);
    }

    T& operator*() const
    {
        return *(operator->());
    }

    //在多线程的环境下提供这个运送符号是不安全的,没有加锁,上层自己保证
    T* operator->() const
    {
        return avl_tree_inst_->data_base() + serial_;
    }

    iterator& operator++()
    {
        increment();
        return *this;
    }
    iterator operator++(int)
    {
        iterator tmp = *this;
        increment();
        return tmp;
    }

    iterator& operator--()
    {
        decrement();
        return *this;
    }
    iterator operator--(int)
    {
        iterator tmp = *this;
        decrement();
        return tmp;
    }

    ///用于实现operator++，找下一个比自己大(比较函数而言)的节点
    void increment()
    {
        if ((avl_tree_inst_->index_base_ + serial_)->right_ != zce::SHMC_INVALID_POINT)
        {
            //如果有右子节点，就向右走，然后一直沿左子树走到底即可
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;

            while ((avl_tree_inst_->index_base_ + serial_)->left_ != zce::SHMC_INVALID_POINT)
            {
                serial_ = (avl_tree_inst_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //如果没有右子节点，找到父节点，如果当前节点是某个右子节点，就一直上溯到不为右子节点为止
            size_type y = (avl_tree_inst_->index_base_ + serial_)->parent_;

            while (serial_ == (avl_tree_inst_->index_base_ + y)->right_)
            {
                serial_ = y;
                y = (avl_tree_inst_->index_base_ + y)->parent_;
            }

            //若此时的右子节点不等于父节点，则父节点即是，否则就是当前节点
            if ((avl_tree_inst_->index_base_ + serial_)->right_ != y)
            {
                serial_ = y;
            }
        }
    }

    //用于实现operator--，找下一个比自己小(比较函数而言)的节点
    void decrement()
    {
        //如果是END，serial_就是head_index_的下标，那么其right_就是end前面的那个
        if (avl_tree_inst_->avl_tree_head_->num_of_node_ == serial_)
        {
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;
        }
        //如果有左子节点
        else if ((avl_tree_inst_->index_base_ + serial_)->left_ != zce::SHMC_INVALID_POINT)
        {
            //令y指向左子节点，找到y的右子节点，向右走到底即是
            size_type y = (avl_tree_inst_->index_base_ + serial_)->left_;

            while ((avl_tree_inst_->index_base_ + y)->right_ != zce::SHMC_INVALID_POINT)
            {
                y = (avl_tree_inst_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //找出父节点，如果当前节点是个左子节点，就一直上溯，直到不再为左子节点，则其的父节点即是
            size_type y = (avl_tree_inst_->index_base_ + serial_)->parent_;

            while (serial_ == (avl_tree_inst_->index_base_ + y)->left_)
            {
                serial_ = y;
                y = (avl_tree_inst_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }

protected:
    //!序列号
    size_type      serial_ = SHMC_INVALID_POINT;
    //!RBtree的实例指针
    avl_tree_t* avl_tree_inst_ = nullptr;
};

/*!
* @brief      AVL Tree的容器，用于排序的处理等，
*             AVL的各方面的处理性能都较为一般，比如插入，删除的耗时都是 O(LOG2N)
*             的级别
* @tparam     T   数据类型
* @tparam     Key   KEY的类型
* @tparam     Extract  如果从T中获取Key的方法
* @tparam     Compare  比较方法
*/
template < class T,
    class Key,
    class Extract = shm_identity<T>,
    class Compare = std::less<Key> >
class shm_avltree
{
    //迭代器友元
    friend class _avl_tree_iterator<T, Key, Extract, Compare>;
private:
    //定义自己
    typedef shm_avltree < T, Key, Extract, Compare > self;
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef Key key_type;
    typedef _avl_tree_iterator < T, Key, Extract, Compare > iterator;
    typedef const iterator const_iterator;
    typedef shmc_size_type size_type;

protected:
    ///AVL TREE的头部数据区
    class _avl_tree_head
    {
    protected:
        _avl_tree_head() = default;
        ~_avl_tree_head() = default;

    public:
        //内存区的长度
        std::size_t             size_of_mem_ = 0;
        //NODE结点个数
        size_type               num_of_node_ = 0;
        //FREE的NODE个数
        size_type               sz_free_node_ = 0;
        //USE的NODE个数
        size_type               sz_use_node_ = 0;
    };

    //AVL tree的索引的节点
    class _avl_tree_index
    {
    public:
        _avl_tree_index() = default;
        ~_avl_tree_index() = default;
        _avl_tree_index(const size_type& p, const size_type& l, const size_type& r, int8_t hb)
            : parent_(p)
            , left_(l)
            , right_(r)
            , balanced_(hb)
        {
        }

    public:
        ///父节点
        size_type    parent_ = SHMC_INVALID_POINT;
        ///左子树
        size_type    left_ = SHMC_INVALID_POINT;
        ///右子树
        size_type    right_ = SHMC_INVALID_POINT;
        ///平衡
        int32_t      balanced_ = 0;
    };

public:

    shm_avltree() = default;
    //只定义,不实现,避免犯错
    shm_avltree(const shm_avltree&) = delete;
    const self& operator=(const self& others) = delete;
    ~shm_avltree()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }

    //得到索引的基础地址
    inline _avl_tree_index* index_base()
    {
        return index_base_;
    }

    //得到数据区的基础地质
    inline  T* data_base()
    {
        return data_base_;
    }

protected:
    //分配一个NODE,将其从FREELIST中取出
    template<typename U>
    size_type create_node(U&& val)
    {
        //如果没有空间可以分配
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return SHMC_INVALID_POINT;
        }

        //从链上取1个下来
        size_type new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        avl_tree_head_->sz_free_node_--;
        avl_tree_head_->sz_use_node_++;

        //初始化
        (index_base_ + new_node)->parent_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->left_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->right_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->balanced_ = 0;

        new (data_base_ + new_node)T(std::forward<U>(val));

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_type pos)
    {
        size_type freenext = free_index_->right_;

        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;

        (index_base_ + pos)->left_ = SHMC_INVALID_POINT;
        (index_base_ + pos)->parent_ = SHMC_INVALID_POINT;
        (index_base_ + pos)->balanced_ = 0;

        avl_tree_head_->sz_free_node_++;
        avl_tree_head_->sz_use_node_--;

        //调用显式的析构函数
        (data_base_ + pos)->~T();
    }

public:

    //内存区的构成为 头部定义区,index区,data区,返回所需要的长度,
    static std::size_t alloc_size(const size_type numnode)
    {
        return  std::size_t(sizeof(_avl_tree_head))
            + sizeof(_avl_tree_index) * (numnode + ADDED_NUM_OF_INDEX)
            + sizeof(T) * numnode;
    }

    //初始化
    bool initialize(const size_type num_node,
                    char* mem_addr,
                    bool if_restore = false)
    {
        ZCE_ASSERT(num_node > 0 && mem_addr_ == nullptr);

        _avl_tree_head* avl_tree_head = reinterpret_cast<_avl_tree_head*>(mem_addr);
        //如果是恢复,数据都在内存中,
        if (true == if_restore)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(num_node) != avl_tree_head->size_of_mem_ ||
                num_node != avl_tree_head->num_of_node_)
            {
                return false;
            }
        }

        //初始化尺寸
        avl_tree_head->size_of_mem_ = alloc_size(num_node);
        avl_tree_head->num_of_node_ = num_node;

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        mem_addr_ = mem_addr;
        //头部
        avl_tree_head_ = avl_tree_head;
        //索引区
        index_base_ = reinterpret_cast<_avl_tree_index*>(
            mem_addr +
            sizeof(_avl_tree_head));
        //数据区
        data_base_ = reinterpret_cast<T*>(
            mem_addr +
            sizeof(_avl_tree_head) +
            sizeof(_avl_tree_index) * (num_node + ADDED_NUM_OF_INDEX));

        //初始化free_index_,head_index_
        head_index_ = reinterpret_cast<_avl_tree_index*>(
            mem_addr +
            sizeof(_avl_tree_head) +
            sizeof(_avl_tree_index) * (num_node));
        free_index_ = reinterpret_cast<_avl_tree_index*>(
            mem_addr_ +
            sizeof(_avl_tree_head) +
            sizeof(_avl_tree_index) * (num_node + 1));

        if (if_restore)
        {
            restore();
        }
        else
        {
            //清理初始化所有的内存,所有的节点为FREE
            clear();
        }
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

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        avl_tree_head_->sz_free_node_ = avl_tree_head_->num_of_node_;
        avl_tree_head_->sz_use_node_ = 0;

        //将清理为nullptr,让指针都指向自己
        head_index_->parent_ = SHMC_INVALID_POINT;
        head_index_->right_ = avl_tree_head_->num_of_node_;
        head_index_->left_ = avl_tree_head_->num_of_node_;
        head_index_->balanced_ = 0;

        //
        free_index_->left_ = SHMC_INVALID_POINT;
        free_index_->parent_ = SHMC_INVALID_POINT;
        free_index_->balanced_ = 0;

        //用right_串起来FREE NODE的列表
        free_index_->right_ = 0;

        //初始化free数据区
        _avl_tree_index* pindex = index_base_;
        for (size_type i = 0; i < avl_tree_head_->num_of_node_; ++i)
        {
            pindex->right_ = (i + 1);

            //将所有FREENODE串起来
            if (i == avl_tree_head_->num_of_node_ - 1)
            {
                pindex->right_ = avl_tree_head_->num_of_node_ + 1;
            }

            pindex++;
        }
    }

    //!恢复函数，用于从(共享)内存中恢复数据，
    void restore()
    {
        iterator iter_tmp = begin();
        iterator iter_end = end();
        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            size_type pos = iter_tmp.getserial();
            T val(std::move(*iter_tmp));
            new (data_base_ + pos) T(std::move(val));
        }
    }

    void terminate()
    {
        iterator iter_tmp = begin();
        iterator iter_end = end();
        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            size_type pos = iter_tmp.getserial();
            (data_base_ + pos)->~T();
        }
        clear();
        if (slef_alloc_)
        {
            delete[] mem_addr_;
            mem_addr_ = nullptr;
        }
    }

    //找到第一个节点
    iterator begin()
    {
        return iterator(head_index_->left_, this);
    };

    //容器应该是前闭后开的,头节点视为最后一个index
    iterator end()
    {
        return iterator(avl_tree_head_->num_of_node_, this);
    }

    //所有节点都在free链上即是空
    bool empty()
    {
        if (avl_tree_head_->sz_free_node_ == avl_tree_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }

    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return true;
        }

        return false;
    };

    size_type size() const
    {
        return avl_tree_head_->sz_use_node_;
    }

    size_type capacity() const
    {
        return avl_tree_head_->num_of_node_;
    }

    //空闲的节点个数
    size_type free()
    {
        return avl_tree_head_->sz_free_node_;
    }

public:

    //本来打算把这段代码全部宏定义的，但考虑了一下，觉得还是inline就足够了。
    //宏毕竟会让代码变得丑陋，算了。而且这些函数的长度应该是可以被inline的。

    inline size_type& header() const
    {
        return avl_tree_head_->num_of_node_;
    }

    inline size_type& root() const
    {
        return head_index_->parent_;
    }

    inline size_type& leftmost() const
    {
        return head_index_->left_;
    }

    inline size_type& rightmost() const
    {
        return head_index_->right_;
    }

    inline size_type& left(size_type x)
    {
        return (index_base_ + x)->left_;
    }

    inline size_type& right(size_type x)
    {
        return (index_base_ + x)->right_;
    }

    inline size_type& parent(size_type x)
    {
        return (index_base_ + x)->parent_;
    }

    inline int32_t& balanced(size_type x)
    {
        return (index_base_ + x)->balanced_;
    }

    inline const T& value(size_type x)
    {
        return *(data_base_ + x);
    }

    inline const Key& key(size_type x)
    {
        return Extract()(value(x));
    }

    //取极大值
    size_type minimum(size_type x)
    {
        while (left(x) != SHMC_INVALID_POINT)
        {
            x = left(x);
        }

        return x;
    }

    //取极小值
    size_type maximum(size_type x)
    {
        while (right(x) != SHMC_INVALID_POINT)
        {
            x = right(x);
        }
        return x;
    }

protected:

    /*!
    * @brief      真正的插入是由这个函数完成的
    * @return     std::pair<iterator, bool> 返回的插入结构，包括迭代器和结果
    * @param      x   插入点,大部分时候为SHMC_INVALID_POINT
    * @param      y   插入点的父节点
    * @param      val 插入的数据
    */
    template<typename U>
    std::pair<iterator, bool> _insert(size_type x, size_type y, U&& val)
    {
        //分配一个空间
        size_type z = create_node(std::forward<U>(val));
        //日过空间不足，无法插入，返回end,false的pair
        if (SHMC_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //把此二货插入进去，而且调整各种东东

        //如果1.插入的是root节点，2.如果插入节点不是空节点，3.如果比较为TRUE
        if (y == header() || x != SHMC_INVALID_POINT || Compare()(Extract()(val), key(y)))
        {
            left(y) = z;

            if (y == header())
            {
                root() = z;
                rightmost() = z;
            }
            //如果Y是最小值，则吧最小值改为Y
            else if (y == leftmost())
            {
                leftmost() = z;
            }
        }
        else
        {
            right(y) = z;

            if (y == rightmost())
            {
                rightmost() = z;
            }
        }

        parent(z) = y;
        left(z) = SHMC_INVALID_POINT;
        right(z) = SHMC_INVALID_POINT;

        //如果不是根节点，我们进行平衡调整
        if (y != header())
        {
            _balance_adjust(z, true);
        }

        return   std::pair<iterator, bool>(iterator(z, this), true);
    }

    /*!
    * @brief      进行平衡调整，内部函数，
    * @param[in]  z 插入的节点位置
    * @param[in]  if_inssert 是否是插入操作进行调整，如果是删除操作，填写false
    */
    void _balance_adjust(size_type z, bool if_inssert)
    {
        //其实这个地方直接使用常量还更加清晰一点,所以我没有用枚举或者宏

        //找到最小的不平衡的点,

        size_type s = parent(z);
        size_type t = z, u = 0;
        int32_t mod_balance = 0;
        for (; s != header(); t = s, s = parent(s))
        {
            if (if_inssert)
            {
                mod_balance = (t == left(s)) ? 1 : -1;
            }
            else
            {
                mod_balance = (t == left(s)) ? -1 : 1;
            }

            //如果是平衡的，修改平衡参数，继续向上干活
            if (0 == balanced(s))
            {
                balanced(s) += mod_balance;

                //如果是插入，原来节点是平衡的，继续向上，如果是删除，原来节点是平衡的，到此为止
                if (if_inssert)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            //这个点上原来就不平衡，找到最小的不平衡树，进行旋转，让其平衡
            else
            {
                balanced(s) += mod_balance;
                //根据不平衡的情况，决定进行什么样的旋转
                if (2 == balanced(s))
                {
                    //考虑到删除的特殊情况，这儿不能直接用t
                    u = left(s);
                    int32_t u_b = balanced(u);
                    if (-1 == u_b)
                    {
                        size_type u_r = right(u);
                        _lr_rotate(s, u, right(u));
                        s = u_r;
                    }
                    else if (1 == u_b)
                    {
                        _ll_rotate(s, u);
                        s = u;
                    }
                    else
                    {
                        _ll_rotate(s, u);
                        //u_b == 0 只可能在删除的情况发生,而且这样操作后，高度不变化，
                        break;
                    }
                }
                else if (-2 == balanced(s))
                {
                    u = right(s);
                    int32_t u_b = balanced(u);
                    if (1 == u_b)
                    {
                        size_type u_l = left(u);
                        _rl_rotate(s, u, left(u));
                        s = u_l;
                    }
                    else if (-1 == u_b)
                    {
                        _rr_rotate(s, u);
                        s = u;
                    }
                    else
                    {
                        _rr_rotate(s, u);
                        //u_b == 0 只可能在删除的情况发生
                        break;
                    }
                }

                //对于删除来说，旋转后，理论上这个子树的高度会有变化，所以要继续
                if (if_inssert)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
        return;
    }

    /*!
    * @brief      LL旋转，
    * @param      a   父节点，最小的不平衡树的根节点
    * @param      b  左边的子节点
    */
    void _ll_rotate(size_type a, size_type b)
    {
        size_type gf = parent(a);
        size_type b_r = right(b);
        parent(a) = b;
        left(a) = b_r;
        if (SHMC_INVALID_POINT != b_r)
        {
            parent(b_r) = a;
        }
        right(b) = a;
        parent(b) = gf;

        //调整平衡因子
        if (1 == balanced(b))
        {
            balanced(a) = 0;
            balanced(b) = 0;
        }
        //对于插入LL，balanced(b)只可能等于1，但对于删除balanced(b) 还可能等于0
        else
        {
            balanced(a) = 1;
            balanced(b) = -1;
        }

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (gf == header())
        {
            root() = b;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = b;
            }
            else
            {
                right(gf) = b;
            }
        }
    }

    /*!
    * @brief      LR旋转
    * @param      a   父节点，最小的不平衡树的根节点
    * @param      b   a的左子节点
    * @param      c   b的右子节点
    */
    void _lr_rotate(size_type a, size_type b, size_type c)
    {
        size_type gf = parent(a);
        size_type c_l = left(c), c_r = right(c);
        parent(a) = c;
        left(a) = c_r;
        if (SHMC_INVALID_POINT != c_r)
        {
            parent(c_r) = a;
        }
        parent(b) = c;
        right(b) = c_l;
        if (SHMC_INVALID_POINT != c_l)
        {
            parent(c_l) = b;
        }
        left(c) = b;
        right(c) = a;
        parent(c) = gf;

        //根据C的状态调整平衡因子
        if (1 == balanced(c))
        {
            balanced(a) = -1;
            balanced(b) = 0;
            balanced(c) = 0;
        }
        else if (-1 == balanced(c))
        {
            balanced(a) = 0;
            balanced(b) = 1;
            balanced(c) = 0;
        }
        else
        {
            balanced(a) = 0;
            balanced(b) = 0;
            balanced(c) = 0;
        }

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (gf == header())
        {
            root() = c;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = c;
            }
            else
            {
                right(gf) = c;
            }
        }
    }

    /*!
    * @brief      RR旋转，
    * @param      a  父节点，最小的不平衡树的根节点
    * @param      b  右边的子节点
    */
    void _rr_rotate(size_type a, size_type b)
    {
        size_type gf = parent(a);
        size_type b_l = left(b);
        parent(a) = b;
        right(a) = b_l;
        if (SHMC_INVALID_POINT != b_l)
        {
            parent(b_l) = a;
        }
        left(b) = a;
        parent(b) = gf;

        //调整平衡因子
        if (-1 == balanced(b))
        {
            balanced(a) = 0;
            balanced(b) = 0;
        }
        //对于插入RR旋转，balanced(b)只可能等于-1，但对于删除balanced(b) 还可能等于0
        else
        {
            balanced(a) = -1;
            balanced(b) = 1;
        }

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (gf == header())
        {
            root() = b;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = b;
            }
            else
            {
                right(gf) = b;
            }
        }
    }

    void _rl_rotate(size_type a, size_type b, size_type c)
    {
        size_type gf = parent(a);
        size_type c_l = left(c), c_r = right(c);
        parent(a) = c;
        right(a) = c_l;
        if (SHMC_INVALID_POINT != c_l)
        {
            parent(c_l) = a;
        }
        parent(b) = c;
        left(b) = c_r;
        if (SHMC_INVALID_POINT != c_r)
        {
            parent(c_r) = b;
        }
        left(c) = a;
        right(c) = b;
        parent(c) = gf;

        //根据C的状态调整平衡因子
        if (balanced(c) == 1)
        {
            balanced(a) = 0;
            balanced(b) = -1;
            balanced(c) = 0;
        }
        else if (balanced(c) == -1)
        {
            balanced(a) = 1;
            balanced(b) = 0;
            balanced(c) = 0;
        }
        else
        {
            balanced(a) = 0;
            balanced(b) = 0;
            balanced(c) = 0;
        }

        //调整C的父节点的左右子树，让其指向新的子树新根
        if (gf == header())
        {
            root() = c;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = c;
            }
            else
            {
                right(gf) = c;
            }
        }
    }

    /*!
    * @brief      真正的删除函数实现，感觉AVL的删除绝对不快
    * @param      x 为删除的位置
    * @param      y 为X的父节点
    */
    void _erase(size_type x, size_type y)
    {
        //对leftmost，rightmost进行处理，
        if (x == leftmost())
        {
            iterator iter(x, this);
            ++iter;
            leftmost() = iter.getserial();
        }
        if (x == rightmost())
        {
            iterator iter(x, this);
            if (iter != begin())
            {
                --iter;
                rightmost() = iter.getserial();
            }
            else
            {
                rightmost() = avl_tree_head_->num_of_node_;
            }
        }

        size_type a = x, a_p = y, a_l = left(a), a_r = right(a), b = 0, b_p = 0;
        //要把A向下交换，选择和他最接近的节点B替换他，比如左子树的一直向右边的节点，
        //比如右子树的一直向左边的节点，直到A是叶子节点
        while (a_l != SHMC_INVALID_POINT || a_r != SHMC_INVALID_POINT)
        {
            if (a_l != SHMC_INVALID_POINT)
            {
                b = a_l;
                while (SHMC_INVALID_POINT != right(b))
                {
                    b = right(b);
                }
            }
            else
            {
                b = a_r;
                while (SHMC_INVALID_POINT != left(b))
                {
                    b = left(b);
                }
            }
            b_p = parent(b);
            //把A，B进行交换
            _exchange(a, a_p, b, b_p);

            a_l = left(a);
            a_r = right(a);
            a_p = parent(a);
        }

        //做平衡调整
        _balance_adjust(a, false);

        size_type last_p = parent(x);
        if (last_p != header())
        {
            if (left(last_p) == a)
            {
                left(last_p) = SHMC_INVALID_POINT;
            }
            else
            {
                right(last_p) = SHMC_INVALID_POINT;
            }
        }
        else
        {
            root() = SHMC_INVALID_POINT;
        }
        //
        destroy_node(x);

        return;
    }

    /*!
    * @brief      将a,b两个节点左交换，注意这个函数只会进行交换，不会考虑平衡等
    * @param      a   a 节点，a是b的父节点或者更祖先的的几点，
    * @param      a_p a他妈
    * @param      b   b节点，
    * @param      b_p b他妈
    */
    void _exchange(size_type a, size_type a_p, size_type b, size_type b_p)
    {
        size_type a_l = left(a);
        size_type a_r = right(a);
        uint32_t a_b = balanced(a);

        size_type b_l = left(b);
        size_type b_r = right(b);
        uint32_t b_b = balanced(b);

        if (a_p != header())
        {
            if (left(a_p) == a)
            {
                left(a_p) = b;
            }
            else
            {
                right(a_p) = b;
            }
        }
        else
        {
            root() = b;
        }

        parent(b) = a_p;

        if (a_l == b)
        {
            left(b) = a;
        }
        else
        {
            left(b) = a_l;
        }
        if (a_r == b)
        {
            right(b) = a;
        }
        else
        {
            right(b) = a_r;
        }

        balanced(b) = a_b;

        parent(a) = b_p;
        left(a) = b_l;
        right(a) = b_r;
        balanced(a) = b_b;

        if (a_l != SHMC_INVALID_POINT)
        {
            if (a_l != b)
            {
                parent(a_l) = b;
            }
            else
            {
                parent(a) = b;
            }
        }
        if (a_r != SHMC_INVALID_POINT)
        {
            if (a_r != b)
            {
                parent(a_r) = b;
            }
            else
            {
                parent(a) = b;
            }
        }
        if (b_l != SHMC_INVALID_POINT)
        {
            parent(b_l) = a;
        }
        if (b_r != SHMC_INVALID_POINT)
        {
            parent(b_r) = a;
        }
    }

public:

    /*!
    * @brief      允许重复key插入的插入函数，Multimap、Multimap用这个
    *             因为空间可能满，所以返回的数据中还是有bool的pair
    * @return     std::pair<iterator, bool>  返回的iterator为迭代器，bool为是否插入成功
    * @param      v        插入的_value_type的数据
    */
    std::pair<iterator, bool> insert_equal(const T& v)
    {
        return _insert_equal(v);
    }
    std::pair<iterator, bool> insert_equal(T&& v)
    {
        return _insert_equal(v);
    }
protected:

    template<typename U>
    std::pair<iterator, bool> _insert_equal(U&& v)
    {
        //如果依据满了，也返回失败
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        size_type y = header();
        size_type x = root();

        //插入到一个空节点上
        while (x != SHMC_INVALID_POINT)
        {
            y = x;
            x = Compare()(Extract()(std::forward<U>(v)), key(x)) ? left(x) : right(x);
        }

        return _insert(x, y, std::forward<U>(v));
    }

public:
    /*!
    * @brief      重复key插入则失败的插入函数，Map、Sap用这个
    * @return     std::pair<iterator, bool> 返回的iterator为迭代器，bool为是否插入成功
    * @param      v 插入的_value_type的数据
    */
    std::pair<iterator, bool> insert_unique(const T& v)
    {
        return _insert_unique(v);
    }
    std::pair<iterator, bool> insert_unique(T&& v)
    {
        return _insert_unique(v);
    }
protected:
    template<typename U>
    std::pair<iterator, bool> _insert_unique(U&& v)
    {
        //如果依据满了，也返回失败
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        size_type y = header();
        size_type x = root();
        bool comp = true;

        //如果比较(比如是<)返回true,就向左，否则(>=)就向右，
        while (x != SHMC_INVALID_POINT)
        {
            y = x;
            comp = Compare()(Extract()(std::forward<U>(v)), key(x));
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);
        if (comp)
        {
            if (j == begin())
            {
                return _insert(x, y, std::forward<U>(v));
            }
            else
            {
                --j;
            }
        }

        if (Compare()(key(j.getserial()), Extract()(v)))
        {
            return _insert(x, y, v);
        }

        //如果既不是>,又不是<，那么就是==,那么返回错误
        return std::pair<iterator, bool>(j, false);
    }
public:
    /*!
    * @brief      通过迭代器删除一个节点
    * @return     void 注意，微软的这个函数好像返回一个迭代器，
    * @param      pos 删除的迭代器
    */
    void erase(const iterator& pos)
    {
        //x,为删除的位置，y为X的父节点，z用于为替换x的节点
        size_type x = pos.getserial();
        size_type y = parent(x);
        return _erase(x, y);
    }

    //通过起始迭代器删除一段节点
    size_type erase(iterator first, iterator last)
    {
        size_type erase_count = 0;

        //特殊情况的加速
        if (first == begin() && last == end())
        {
            erase_count = size();
            clear();
        }
        else
        {
            //
            while (first != last)
            {
                ++erase_count;
                //注意这个地方用的是erase(first++)，是first
                erase(first++);
            }
        }
        return erase_count;
    }

    //通过key删除节点，MAP使用
    size_type erase_unique(const Key& k)
    {
        //先尝试找到节点
        iterator find_iter = find(k);
        if (find_iter == end())
        {
            return 0;
        }
        erase(find_iter);
        return 1;
    }

    //通过value删除节点，SET使用
    size_type erase_unique_value(const T& v)
    {
        Extract get_key;
        return erase_unique(get_key(v));
    }

    //通过key删除节点，MULTIMAP用
    size_type erase_equal(const Key& k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //通过值删除节点，MULTISET用
    size_type erase_equal_value(const T& v)
    {
        Extract get_key;
        return erase_equal(get_key(v));
    }

    //找到第一个key值相同的节点
    iterator lower_bound(const Key& k)
    {
        size_type y = header();
        size_type x = root();

        while (x != SHMC_INVALID_POINT)
        {
            //上下两个函数就这行代码不一样，注意先后比较
            if (!Compare()(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        return iterator(y, this);
    }

    //找到最后一个key值相同的节点
    iterator upper_bound(const Key& k)
    {
        size_type y = header();
        size_type x = root();

        while (x != SHMC_INVALID_POINT)
        {
            //上下两个函数就这行代码不一样，注意先后比较关系
            if (Compare()(k, key(x)))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        return iterator(y, this);
    }

    //找key相同的节点
    iterator find(const Key& k)
    {
        size_type y = header();
        size_type x = root();

        while (x != SHMC_INVALID_POINT)
        {
            if (!Compare()(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        iterator j = iterator(y, this);
        //注意两次调用Compare的比较参数顺序喔
        return (j == end() || Compare()(k, key(j.getserial()))) ? end() : j;
    }

    //找value相同的节点
    iterator find_value(const T& v)
    {
        Extract get_key;
        return find(get_key(v));
    }

    //找value相同的节点，如未找到则插入
    T& find_or_insert(const T& v)
    {
        iterator iter = find_value(v);

        if (iter == end())
        {
            std::pair<iterator, bool> pair_iter = insert(v);
            return (*(pair_iter.first));
        }

        return *iter;
    }

    //调试代码，如果_value_type是整数 的时候生效，否则无效
    void debug_note(size_type x,
                    typename std::enable_if<std::is_integral<T>::value >::type* /*ptr*/ = 0)
    {
        std::cout << "Note :" << std::setw(6) << x
            << " Data:" << std::setw(8) << data_base_[x]
            << " parent:" << std::setw(6) << (int)parent(x)
            << " left:" << std::setw(6) << (int)left(x)
            << " right:" << std::setw(6) << (int)right(x)
            << " balanced:" << balanced(x)
            << std::endl;
    }

    //检查树形结构是否平衡
    bool check_balanced(size_type x)
    {
        int32_t x_b = balanced(x);
        size_type x_lh = height(left(x));
        size_type x_rh = height(right(x));
        if (x_b != (int32_t)(x_lh - x_rh))
        {
            return false;
        }
        return true;
    }

    //得到某个节点的高度
    size_type height(size_type x)
    {
        if (x == SHMC_INVALID_POINT)
        {
            return 0;
        }
        size_type l_h = 0, r_h = 0;
        l_h = height(left(x)) + 1;
        r_h = height(right(x)) + 1;

        return l_h > r_h ? l_h : r_h;
    }

protected:
    //index区要增加两个数据,一个是头指针，一个是空节点的头指针
    static const size_type ADDED_NUM_OF_INDEX = 2;

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;

    ///RBTree头部
    _avl_tree_head* avl_tree_head_ = nullptr;

    ///所有的指针都是根据基地址计算得到的,用于方便计算,每次初始化会重新计算
    ///索引数据区,
    _avl_tree_index* index_base_ = nullptr;

    ///数据区起始指针,
    T* data_base_ = nullptr;

    ///头节点的头指针,N+1个索引位表示
    _avl_tree_index* head_index_ = nullptr;

    ///空节点的头指针,N+2个索引位表示（这里利用right节点做链接，把空节点串起来）
    _avl_tree_index* free_index_ = nullptr;
};

//用AVL Tree实现SET，不区分multiset和set，通过不通的insert自己区分
template < class T,
    class Compare = std::less<T> >
class shm_avlset :
    public shm_avltree< T, T, shm_identity<T>, Compare >
{
private:
    typedef shm_avlset<T, Compare > self;
    typedef shm_avltree<T, T, shm_identity<T>, Compare> avl_tree_t;
    typedef shmc_size_type size_type;
protected:
    shm_avlset() = default;
    shm_avlset(const shm_avlset& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_avlset() = default;

public:

    static self*
        initialize(size_type& numnode, char* pmmap, bool if_restore = false)
    {
        return reinterpret_cast<self *>(
            avl_tree_t::initialize(numnode, pmmap, if_restore));
    }
};

//用AVL Tree实现MAP，不区分multiset和set，通过不通的insert自己区分
template < class Key,
    class T,
    class Extract = shm_select1st <std::pair <Key, T> >,
    class Compare = std::less<T>  >
class shm_avlmap :
    public shm_avltree< std::pair <Key, T>, Key, Extract, Compare  >
{
private:
    typedef shm_avlmap<Key, T, Extract, Compare > self;
    typedef shm_avltree<std::pair <Key, T>, Key, Extract, Compare> avl_tree_t;
    typedef shmc_size_type size_type;
protected:
    shm_avlmap() = default;
    shm_avlmap(const shm_avlmap& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_avlmap() = default;

public:
    static self*
        initialize(size_type& numnode, char* pmmap, bool if_restore = false)
    {
        return reinterpret_cast <self *> (
            avl_tree_t::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，谨慎使用
    T& operator[](const Key& key)
    {
        return (find_or_insert(std::pair<Key, T >(key, T()))).second;
    }
};

template < class T, class Key, class Extract, class Compare >
using static_avltree = zce::shm_avltree<T, Key, Extract, Compare>;

template<class T, class Compare >
using static_avlset = zce::shm_avlset<T, Compare>;

template<class Key, class T, class Extract, class Compare>
using static_avlmap = zce::shm_avlmap<Key, T, Extract, Compare>;
} //namespace zce
