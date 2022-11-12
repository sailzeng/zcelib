/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/rbtree.h
* @author     scottxu(徐昊)
* @version
* @date       2007年7月24日
* @brief      RBTree主要是完成可以排序的MAP,SET,MultiMap,MultiSet的MMAP类
*             实现了RBTree以及MAP,SET,MultiMap,MultiSet
*
* @details
*
*
*
* @note       不保证线程安全，上层自己来保证
*
*/
#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
enum RB_TREE_COLOR
{
    //红节点
    RB_TREE_RED = 0,
    //黑节点
    RB_TREE_BLACK = 1,
};

//
typedef char  color_type;

template<class T, class Key, class Extract, class Compare> class rb_tree;

//RBtree的迭代器
template <class T, class Key, class Extract, class Compare> class _rb_tree_iterator
{
public:
    typedef shmc_size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    //双向访问的迭代器萃取
    typedef std::bidirectional_iterator_tag iterator_category;
protected:
    typedef _rb_tree_iterator<T, Key, Extract, Compare> iterator;
    typedef rb_tree<T, Key, Extract, Compare> shm_rb_tree_t;

public:
    _rb_tree_iterator(size_type seq, shm_rb_tree_t* instance)
        : serial_(seq)
        , rb_tree_instance_(instance)
    {
    }

    _rb_tree_iterator()
        : serial_(),
        rb_tree_instance_(nullptr)
    {
    }

    ~_rb_tree_iterator()
    {
    }

    //初始化
    void initialize(size_type seq, shm_rb_tree_t* instance)
    {
        serial_ = seq;
        rb_tree_instance_ = instance;
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_type getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator& x) const
    {
        return (serial_ == x.serial_ && rb_tree_instance_ == x.rb_tree_instance_);
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
        return rb_tree_instance_->data_base() + serial_;
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

    //用于实现operator++，找下一个节点
    void increment()
    {
        if ((rb_tree_instance_->index_base_ + serial_)->right_ != zce::SHMC_INVALID_POINT)
        {
            //如果有右子节点，就向右走，然后一直沿左子树走到底即可
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;

            while ((rb_tree_instance_->index_base_ + serial_)->left_ != zce::SHMC_INVALID_POINT)
            {
                serial_ = (rb_tree_instance_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //如果没有右子节点，找到父节点，如果当前节点是某个右子节点，就一直上溯到不为右子节点为止
            size_type y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while (serial_ == (rb_tree_instance_->index_base_ + y)->right_)
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            //若此时的右子节点不等于父节点，则父节点即是，否则就是当前节点
            if ((rb_tree_instance_->index_base_ + serial_)->right_ != y)
            {
                serial_ = y;
            }
        }
    }

    //用于实现operator--，找上一个节点
    void decrement()
    {
        //如果是红节点，且父节点的的父节点等于自己
        if ((rb_tree_instance_->index_base_ + serial_)->color_ == RB_TREE_RED &&
            (rb_tree_instance_->index_base_ +
            ((rb_tree_instance_->index_base_ + serial_)->parent_))->parent_ == serial_)
        {
            //右子节点即是
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;
        }
        //如果有左子节点
        else if ((rb_tree_instance_->index_base_ + serial_)->left_ != zce::SHMC_INVALID_POINT)
        {
            //令y指向左子节点，找到y的右子节点，向右走到底即是
            size_type y = (rb_tree_instance_->index_base_ + serial_)->left_;

            while ((rb_tree_instance_->index_base_ + y)->right_ != zce::SHMC_INVALID_POINT)
            {
                y = (rb_tree_instance_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //找出父节点，如果当前节点是个左子节点，就一直上溯，直到不再为左子节点，则其的父节点即是
            size_type y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while (serial_ == (rb_tree_instance_->index_base_ + y)->left_)
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }

protected:
    //序列号
    size_type          serial_ = SHMC_INVALID_POINT;
    //RBtree的实例指针
    shm_rb_tree_t * rb_tree_instance_ = nullptr;
};

/*!
* @brief
*
* @tparam     _value_type   节点的实际值型别
* @tparam     _key_type     节点的键的型别
* @tparam     Extract  从节点中得到键值的方法,或者函数对象
* @tparam     Compare  比较键值大小的方法,或者函数对象
* @note
*/
template < class T,
    class Key,
    class Extract = shm_identity<T>,
    class Compare = std::less<Key> >
class rb_tree
{
    //迭代器友元
    friend class _rb_tree_iterator<T, Key, Extract, Compare>;
    //定义typedef
private:
    typedef rb_tree<T, Key, Extract, Compare> self;
public:
    typedef _rb_tree_iterator<T, Key, Extract, Compare> iterator;
    typedef const iterator const_iterator;
    typedef iterator::iterator_category iterator_category;
    typedef T value_type;
    typedef Key key_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef shmc_size_type size_type;

    //RB TREE的头部数据区
    class _shm_rb_tree_head
    {
    protected:
        _shm_rb_tree_head() = default;

        ~_shm_rb_tree_head() = default;

    public:
        //内存区的长度
        std::size_t             size_of_mem_ = 0;
        //NODE结点个数;
        size_type               num_of_node_ = 0;
        //FREE的NODE个数
        size_type               sz_free_node_ = 0;
        //USE的NODE个数
        size_type               sz_use_node_ = 0;
    };

    //RBtree的索引的节点
    class _shm_rb_tree_index
    {
    public:
        _shm_rb_tree_index() = default;
        ~_shm_rb_tree_index() = default;

        _shm_rb_tree_index(size_type p,
                           size_type l,
                           size_type r,
                           char cl)
            : parent_(p)
            , left_(l)
            , right_(r)
            , color_(cl)
        {
        }

    public:
        //父节点
        size_type    parent_ = SHMC_INVALID_POINT;
        //左子树
        size_type    left_ = SHMC_INVALID_POINT;
        //右子树
        size_type    right_ = SHMC_INVALID_POINT;
        //颜色
        color_type   color_ = RB_TREE_RED;
    };

public:
    //构造函数，析构函数
    rb_tree() = default;
    //只定义,不实现,避免犯错
    rb_tree(const rb_tree&) = delete;
    const self& operator=(const self& others) = delete;
    ~rb_tree()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }
    //得到索引的基础地址
    inline _shm_rb_tree_index* index_base()
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
    size_type create_node(U&& v)
    {
        //如果没有空间可以分配
        if (rb_tree_head_->sz_free_node_ == 0)
        {
            return SHMC_INVALID_POINT;
        }

        //从链上取1个下来
        size_type new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        rb_tree_head_->sz_free_node_--;
        rb_tree_head_->sz_use_node_++;

        //初始化
        (index_base_ + new_node)->parent_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->left_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->right_ = SHMC_INVALID_POINT;
        (index_base_ + new_node)->color_ = RB_TREE_RED;

        new (data_base_ + new_node)T(std::forward<U>(v));

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_type pos)
    {
        size_type freenext = free_index_->right_;
        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;
        rb_tree_head_->sz_free_node_++;
        rb_tree_head_->sz_use_node_--;

        //调用显式的析构函数
        (data_base_ + pos)->~T();
    }
protected:

    //本来打算把这段代码全部宏定义的，但考虑了一下，觉得还是inline就足够了。
    //宏毕竟会让代码变得丑陋，算了。而且这些函数的长度应该是可以被inline的。
    inline size_type& header() const
    {
        return rb_tree_head_->num_of_node_;
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

    inline color_type& color(size_type x)
    {
        return (index_base_ + x)->color_;
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

    //真正的插入是由这个函数完成的
    template<typename U>
    std::pair<iterator, bool>  _insert(size_type x, size_type y, U&& v)
    {
        size_type z = create_node(v);
        //如果空间不足，无法插入，返回end,false的pair
        if (SHMC_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        if (y == header() || x != SHMC_INVALID_POINT ||
            Compare()(Extract()(std::forward<U>(v)), key(y)))
        {
            left(y) = z;

            if (y == header())
            {
                root() = z;
                rightmost() = z;
            }
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
        *(data_base_ + z) = std::forward<U>(v);

        _rb_tree_rebalance(z, parent(header()));
        return  std::pair<iterator, bool>(iterator(z, this), true);
    }

    //通过旋转和变色，调整整个树，让其符合RBTree要求
    //参数1：新增节点
    //参数2：根节点
    void _rb_tree_rebalance(size_type x, size_type& root)
    {
        color(x) = RB_TREE_RED;

        while (x != root && color(parent(x)) == RB_TREE_RED)
        {
            if (parent(x) == left(parent(parent(x))))
            {
                size_type y = right(parent(parent(x)));

                if (y != SHMC_INVALID_POINT && color(y) == RB_TREE_RED)
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if (x == right(parent(x)))
                    {
                        x = parent(x);
                        _rb_tree_rotate_left(x, root);
                    }

                    color(parent(x)) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    _rb_tree_rotate_right(parent(parent(x)), root);
                }
            }
            else
            {
                size_type y = left(parent(parent(x)));

                if (y != SHMC_INVALID_POINT && color(y) == RB_TREE_RED)
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if (x == left(parent(x)))
                    {
                        x = parent(x);
                        _rb_tree_rotate_right(x, root);
                    }

                    color(parent(x)) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    _rb_tree_rotate_left(parent(parent(x)), root);
                }
            }
        }

        color(root) = RB_TREE_BLACK;
    }

    //左旋函数
    //参数1：左旋节点
    //参数2：根节点
    void _rb_tree_rotate_left(size_type x, size_type& root)
    {
        size_type y = right(x);
        right(x) = left(y);

        if (left(y) != SHMC_INVALID_POINT)
        {
            parent(left(y)) = x;
        }

        parent(y) = parent(x);

        if (x == root)
        {
            root = y;
        }
        else if (x == left(parent(x)))
        {
            left(parent(x)) = y;
        }
        else
        {
            right(parent(x)) = y;
        }

        left(y) = x;
        parent(x) = y;
    }

    //右旋函数
    //参数1：右旋节点
    //参数2：根节点
    void _rb_tree_rotate_right(size_type x, size_type& root)
    {
        size_type y = left(x);
        left(x) = right(y);

        if (right(y) != SHMC_INVALID_POINT)
        {
            parent(right(y)) = x;
        }

        parent(y) = parent(x);

        if (x == root)
        {
            root = y;
        }
        else if (x == right(parent(x)))
        {
            right(parent(x)) = y;
        }
        else
        {
            left(parent(x)) = y;
        }

        right(y) = x;
        parent(x) = y;
    }

    //删除时的树形调整，让其符合RBTree要求
    size_type _erase(size_type z)
    {
        size_type y = z;
        size_type x = SHMC_INVALID_POINT;
        size_type x_parent = SHMC_INVALID_POINT;

        //如果左子树为nullptr，选择右子树
        if (left(y) == SHMC_INVALID_POINT)
        {
            x = right(y);
        }
        else
        {
            //如果左子树不为nullptr，而右子树为nullptr
            if (right(x) == SHMC_INVALID_POINT)
            {
                x = left(y);
            }
            //如果左右子树都不为nullptr
            else
            {
                y = minimum(right(y));
                x = right(y);
            }
        }

        //y不是z,表示y不是自己这颗子树的极小值
        if (y != z)
        {
            parent(left(z)) = y;
            left(y) = left(z);

            if (y != right(z))
            {
                x_parent = parent(y);

                if (x != SHMC_INVALID_POINT)
                {
                    parent(x) = parent(y);
                }

                left(parent(y)) = x;
                right(y) = right(z);
                parent(right(z)) = y;
            }
            else
            {
                x_parent = y;
            }

            if (root() == z)
            {
                root() = y;
            }
            else if (left(parent(z)) == z)
            {
                left(parent(z)) = y;
            }
            else
            {
                right(parent(z)) = y;
            }

            parent(y) = parent(z);
            color_type  c = color(y);
            color(y) = color(z);
            color(z) = c;
            y = z;
        }
        else
        {
            x_parent = parent(y);

            if (x != SHMC_INVALID_POINT)
            {
                parent(x) = parent(y);
            }

            if (root() == z)
            {
                root() = x;
            }
            else
            {
                if (left(parent(z)) == z)
                {
                    left(parent(z)) = x;
                }
                else
                {
                    right(parent(z)) = x;
                }
            }

            if (leftmost() == z)
            {
                if (right(z) == SHMC_INVALID_POINT)
                {
                    leftmost() = parent(z);
                }
                else
                {
                    leftmost() = minimum(x);
                }
            }

            if (rightmost() == z)
            {
                if (left(z) == SHMC_INVALID_POINT)
                {
                    rightmost() = parent(z);
                }
                else
                {
                    rightmost() = maximum(x);
                }
            }
        }

        if (color(y) != RB_TREE_RED)
        {
            while (x != root() && (x == SHMC_INVALID_POINT || color(x) == RB_TREE_BLACK))
            {
                if (x == left(x_parent))
                {
                    size_type w = right(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_left(x_parent, root());
                        w = right(x_parent);
                    }

                    if ((left(w) == SHMC_INVALID_POINT || color(left(w)) == RB_TREE_BLACK) &&
                        (right(w) == SHMC_INVALID_POINT || color(right(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (right(w) == SHMC_INVALID_POINT || color(right(w)) == RB_TREE_BLACK)
                        {
                            if (left(w) != SHMC_INVALID_POINT)
                            {
                                color(left(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_right(w, root());
                            w = right(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (right(w) != SHMC_INVALID_POINT)
                        {
                            color(right(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_left(x_parent, root());
                        break;
                    }
                }
                else
                {
                    size_type w = left(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_right(x_parent, root());
                        w = left(x_parent);
                    }

                    if ((right(w) == SHMC_INVALID_POINT || color(right(w)) == RB_TREE_BLACK) &&
                        (left(w) == SHMC_INVALID_POINT || color(left(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (left(w) == SHMC_INVALID_POINT || color(left(w)) == RB_TREE_BLACK)
                        {
                            if (right(w) != SHMC_INVALID_POINT)
                            {
                                color(right(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_left(w, root());
                            w = left(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (left(w) != SHMC_INVALID_POINT)
                        {
                            color(left(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_right(x_parent, root());
                        break;
                    }
                }
            }

            if (x != SHMC_INVALID_POINT)
            {
                color(x) = RB_TREE_BLACK;
            }
        }

        return y;
    }

    //为了万能引用定义了个模板函数
    template<typename U>
    std::pair<iterator, bool>  _insert_equal(U&& v)
    {
        size_type y = header();
        size_type x = root();

        while (x != SHMC_INVALID_POINT)
        {
            y = x;
            x = Compare()(Extract()(std::forward<U>(v)),
                          key(x)) ? left(x) : right(x);
        }

        return _insert(x, y, std::forward<U>(v));
    }

    template<typename U>
    std::pair<iterator, bool> _insert_unique(U&& v)
    {
        size_type y = header();
        size_type x = root();
        bool comp = true;

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
            return _insert(x, y, std::forward<U>(v));
        }

        return std::pair<iterator, bool>(j, false);
    }

public:

    //内存区的构成为 定义区,index区,data区,返回所需要的长度,
    static std::size_t alloc_size(size_type numnode)
    {
        return  sizeof(_shm_rb_tree_head) +
            sizeof(_shm_rb_tree_index) * (numnode + ADDED_NUM_OF_INDEX) +
            sizeof(T) * numnode;
    }

    /*!
        * @brief 初始化，得到一个self的对象指针，
        * @param num_node    对象数量
        * @param mem_addr   内心地址指针，可以是共享内存指针
        * @param if_restore 如果是共享内存，可以尝试恢复
        * @return self，自己的对象指针
    */
    bool initialize(const size_type num_node,
                    char* mem_addr,
                    bool if_restore = false)
    {
        ZCE_ASSERT(mem_addr != nullptr && num_node > 0);
        mem_addr_ = mem_addr;
        _shm_rb_tree_head* rb_tree_head = reinterpret_cast<_shm_rb_tree_head*>(mem_addr);

        //如果是恢复,数据都在内存中,
        if (true == if_restore)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(num_node) != rb_tree_head->size_of_mem_ ||
                num_node != rb_tree_head->num_of_node_)
            {
                return false;
            }
        }

        //初始化尺寸
        rb_tree_head->size_of_mem_ = alloc_size(num_node);
        rb_tree_head->num_of_node_ = num_node;

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        rb_tree_head_ = rb_tree_head;
        index_base_ = reinterpret_cast<_shm_rb_tree_index*>(
            mem_addr + sizeof(_shm_rb_tree_head));
        data_base_ = reinterpret_cast<T*>(
            mem_addr + sizeof(_shm_rb_tree_head) +
            sizeof(_shm_rb_tree_index) * (num_node + ADDED_NUM_OF_INDEX));

        //初始化free_index_,head_index_
        head_index_ = reinterpret_cast<_shm_rb_tree_index*>(
            mem_addr + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (num_node));
        free_index_ = reinterpret_cast<_shm_rb_tree_index*>(
            mem_addr + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (num_node + 1));

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

    //!自己内部分配内存的初始化
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
        rb_tree_head_->sz_free_node_ = rb_tree_head_->num_of_node_;
        rb_tree_head_->sz_use_node_ = 0;

        //将清理为nullptr,让指针都指向自己
        head_index_->parent_ = SHMC_INVALID_POINT;
        head_index_->right_ = rb_tree_head_->num_of_node_;
        head_index_->left_ = rb_tree_head_->num_of_node_;
        head_index_->color_ = RB_TREE_RED;

        _shm_rb_tree_index* pindex = index_base_;

        free_index_->right_ = 0;

        //初始化free数据区
        for (size_type i = 0; i < rb_tree_head_->num_of_node_; ++i)
        {
            pindex->right_ = (i + 1);

            //将所有FREENODE串起来
            if (i == rb_tree_head_->num_of_node_ - 1)
            {
                pindex->right_ = rb_tree_head_->num_of_node_ + 1;
            }

            pindex++;
        }
    }

    //!销毁，析构所有的已有元素，注意，如果想恢复，不要调用这个函数
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

    //找到第一个节点
    iterator begin()
    {
        return iterator(head_index_->left_, this);
    };

    //容器应该是前闭后开的,头节点视为最后一个index
    iterator end()
    {
        return iterator(rb_tree_head_->num_of_node_, this);
    }

    //所有节点都在free链上即是空
    bool empty()
    {
        if (rb_tree_head_->sz_free_node_ == rb_tree_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }

    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (rb_tree_head_->sz_free_node_ == 0)
        {
            return true;
        }

        return false;
    };

    size_type size() const
    {
        return rb_tree_head_->sz_use_node_;
    }

    size_type capacity() const
    {
        return rb_tree_head_->num_of_node_;
    }

    //空闲的节点个数
    size_type free()
    {
        return rb_tree_head_->sz_free_node_;
    }
    //重复key插入则失败的插入函数，Map、Sap用这个
    std::pair<iterator, bool> insert_unique(const T& v)
    {
        return _insert_unique(v);
    }
    std::pair<iterator, bool> insert_unique(T&& v)
    {
        return _insert_unique(v);
    }

    //允许重复key插入的插入函数，Multimap、Multimap用这个
    std::pair<iterator, bool>  insert_equal(const T& v)
    {
        return _insert_equal(v);
    }
    std::pair<iterator, bool>  insert_equal(T&& v)
    {
        return _insert_equal(v);
    }

    //通过迭代器删除一个节点
    iterator erase(const iterator& pos)
    {
        size_type tmp = _erase(pos.getserial());
        destroy_node(pos.getserial());
        return iterator(tmp, this);
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

    //通过key删除节点，Map和Set用
    size_type erase_unique(const Key& k)
    {
        iterator it = find(k);

        if (it != end())
        {
            erase(it);
            return 1;
        }

        return 0;
    }

    //通过value删除节点，Map和Set用
    size_type erase_unique_value(const T& v)
    {
        Extract get_key;
        return erase_unique(get_key(v));
    }

    //通过key删除节点，Multimap和Multiset用
    size_type erase_equal(const Key& k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //通过值删除节点，Multimap和Multiset用
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

protected:
    //index区要增加两个数据,一个是头指针，一个是空节点的头指针
    static const size_type ADDED_NUM_OF_INDEX = 2;

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;
    //RBTree头部
    _shm_rb_tree_head* rb_tree_head_ = nullptr;

    //所有的指针都是根据基地址计算得到的,用于方便计算,每次初始化会重新计算
    //索引数据区,
    _shm_rb_tree_index* index_base_ = nullptr;

    //数据区起始指针,
    T* data_base_ = nullptr;

    //头节点的头指针,N+1个索引位表示
    _shm_rb_tree_index* head_index_ = nullptr;

    //空节点的头指针,N+2个索引位表示（这里利用right节点做链接，把空节点串起来）
    _shm_rb_tree_index* free_index_ = nullptr;
};

//用RBTree实现SET，不区分multiset和set，通过不通的insert自己区分
template<class T, class Compare = std::less<T> >
class shm_set :
    public rb_tree< T, T, shm_identity<T>, Compare >
{
private:
    typedef shm_set<T, Compare > self;
    typedef rb_tree< T, T, shm_identity<T>, Compare> rb_tree_t;
    typedef shmc_size_type size_type;
protected:
    shm_set() = default;
    shm_set(const shm_set& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_set() = default;
public:
    static shm_set*
        initialize(size_type& numnode, char* pmmap, bool if_restore = false)
    {
        return reinterpret_cast<self *>(
            rb_tree_t::initialize(numnode, pmmap, if_restore));
    }
};

//用RBTree实现MAP，不区分multiset和set，通过不通的insert自己区分
template<class Key, class T, class Extract = shm_select1st <std::pair <Key, T> >, class Compare = std::less<T>  >
class shm_map :
    public rb_tree< std::pair <Key, T>, Key, Extract, Compare  >
{
private:
    typedef shm_map<Key, T, Extract, Compare > self;
    typedef rb_tree< std::pair <Key, T>, Key, Extract, Compare> rb_tree_t;
    typedef shmc_size_type size_type;
protected:
    shm_map() = default;
    shm_map(const shm_map& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_map() = default;

    static shm_map*
        initialize(size_type& numnode, char* pmmap, bool if_restore = false)
    {
        return reinterpret_cast<self *>(
            rb_tree_t::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，谨慎使用
    T& operator[](const Key& key)
    {
        return (find_or_insert(std::pair<Key, T >(key, T()))).second;
    }
};

template < class T, class Key, class Extract, class Compare >
using static_rbtree = zce::rb_tree<T, Key, Extract, Compare>;

template<class T, class Compare >
using static_set = zce::shm_set<T, Compare>;

template<class Key, class T, class Extract, class Compare>
using static_map = zce::shm_map<Key, T, Extract, Compare>;
};
