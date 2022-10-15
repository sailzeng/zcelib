/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_rbtree.h
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


#ifndef ZCE_LIB_SHM_RB_TREE_H_
#define ZCE_LIB_SHM_RB_TREE_H_

#include "zce_shm_predefine.h"

namespace zce
{

enum RB_TREE_COLOR
{
    //红节点
    RB_TREE_RED   = 0,
    //黑节点
    RB_TREE_BLACK = 1,
};

//
typedef char  color_type;

template<class _value_type, class _key_type, class _extract_key, class _compare_key> class shm_rb_tree;

//RB TREE的头部数据区
class _shm_rb_tree_head
{
protected:
    _shm_rb_tree_head()
        : size_of_mmap_(0)
        , num_of_node_(0)
        , sz_free_node_(0)
        , sz_use_node_(0)
    {
    }
    ~_shm_rb_tree_head()
    {
    }

public:
    //内存区的长度
    size_t               size_of_mmap_;
    //NODE结点个数
    size_t               num_of_node_;
    //FREE的NODE个数
    size_t               sz_free_node_;
    //USE的NODE个数
    size_t               sz_use_node_;
};

//RBtree的索引的节点
class _shm_rb_tree_index
{

public:
    _shm_rb_tree_index()
        : parent_(_shm_memory_base::_INVALID_POINT)
        , left_(_shm_memory_base::_INVALID_POINT)
        , right_(_shm_memory_base::_INVALID_POINT)
        , color_(RB_TREE_RED)
    {
    }

    _shm_rb_tree_index(const size_t &p, const size_t &l, const size_t &r, char cl)
        : parent_(p)
        , left_(l)
        , right_(r)
        , color_(cl)
    {
    }

    ~_shm_rb_tree_index()
    {
    }

public:
    //父节点
    size_t       parent_;
    //左子树
    size_t       left_;
    //右子树
    size_t       right_;
    //颜色
    color_type   color_;

};

//RBtree的迭代器
template <class _value_type, class _key_type, class _extract_key, class _compare_key> class _shm_rb_tree_iterator
{
    typedef _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    typedef shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key> shm_rb_tree_t;


    //迭代器萃取器所有的东东
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    _shm_rb_tree_iterator(size_t seq, shm_rb_tree_t *instance)
        : serial_(seq)
        , rb_tree_instance_(instance)
    {
    }

    _shm_rb_tree_iterator()
        : serial_(_shm_memory_base::_INVALID_POINT),
          rb_tree_instance_(NULL)
    {
    }

    ~_shm_rb_tree_iterator()
    {
    }

    //初始化
    void initialize(size_t seq, shm_rb_tree_t *instance)
    {
        serial_ = seq;
        rb_tree_instance_ = instance;
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && rb_tree_instance_ == x.rb_tree_instance_ );
    }
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    _value_type &operator*() const
    {
        return *(operator->());
    }

    //在多线程的环境下提供这个运送符号是不安全的,没有加锁,上层自己保证
    _value_type *operator->() const
    {
        return rb_tree_instance_->getdatabase() + serial_;
    }

    iterator &operator++()
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

    iterator &operator--()
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
        if ( (rb_tree_instance_->index_base_ + serial_)->right_ != _shm_memory_base::_INVALID_POINT )
        {
            //如果有右子节点，就向右走，然后一直沿左子树走到底即可
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;

            while ( (rb_tree_instance_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT )
            {
                serial_ = (rb_tree_instance_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //如果没有右子节点，找到父节点，如果当前节点是某个右子节点，就一直上溯到不为右子节点为止
            size_t y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while ( serial_ == (rb_tree_instance_->index_base_ + y)->right_ )
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            //若此时的右子节点不等于父节点，则父节点即是，否则就是当前节点
            if ( (rb_tree_instance_->index_base_ + serial_)->right_ != y )
            {
                serial_ = y;
            }
        }
    }

    //用于实现operator--，找上一个节点
    void decrement()
    {
        //如果是红节点，且父节点的的父节点等于自己
        if ( (rb_tree_instance_->index_base_ + serial_)->color_ == RB_TREE_RED &&
             (rb_tree_instance_->index_base_ + ((rb_tree_instance_->index_base_ + serial_)->parent_))->parent_ == serial_ )
        {
            //右子节点即是
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;
        }
        //如果有左子节点
        else if ( (rb_tree_instance_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT )
        {
            //令y指向左子节点，找到y的右子节点，向右走到底即是
            size_t y = (rb_tree_instance_->index_base_ + serial_)->left_;

            while ( (rb_tree_instance_->index_base_ + y)->right_ != _shm_memory_base::_INVALID_POINT )
            {
                y = (rb_tree_instance_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //找出父节点，如果当前节点是个左子节点，就一直上溯，直到不再为左子节点，则其的父节点即是
            size_t y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while ( serial_ == (rb_tree_instance_->index_base_ + y)->left_ )
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }

protected:
    //序列号
    size_t          serial_;
    //RBtree的实例指针
    shm_rb_tree_t  *rb_tree_instance_;

};



/*!
* @brief
*
* @tparam     _value_type   节点的实际值型别
* @tparam     _key_type     节点的键的型别
* @tparam     _extract_key  从节点中得到键值的方法,或者函数对象
* @tparam     _compare_key  比较键值大小的方法,或者函数对象
* @note
*/
template < class _value_type,
           class _key_type,
           class _extract_key = smem_identity<_value_type>,
           class _compare_key = std::less<_key_type> >
class shm_rb_tree : public _shm_memory_base
{
public:
    //定义自己
    typedef shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key> self;

    //定义迭代器
    typedef _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    //迭代器友元
    friend class _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key>;

protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore)
        : _shm_memory_base(NULL)
        , index_base_(NULL)
        , data_base_(NULL)
    {
    }

    shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >()
        : _shm_memory_base(NULL)
    {
    }
public:

    ~shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >()
    {
    }

    //只定义,不实现,避免犯错
    const self &operator=(const self &others);

    //得到索引的基础地址
    inline _shm_rb_tree_index *getindexbase()
    {
        return index_base_;
    }

    //得到数据区的基础地质
    inline  _value_type *getdatabase()
    {
        return data_base_;
    }

protected:
    //分配一个NODE,将其从FREELIST中取出
    size_t create_node(const _value_type &val)
    {
        //如果没有空间可以分配
        if (rb_tree_head_->sz_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //从链上取1个下来
        size_t new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        rb_tree_head_->sz_free_node_--;
        rb_tree_head_->sz_use_node_++;

        //初始化
        (index_base_ + new_node)->parent_ = _INVALID_POINT;
        (index_base_ + new_node)->left_ = _INVALID_POINT;
        (index_base_ + new_node)->right_ = _INVALID_POINT;
        (index_base_ + new_node)->color_ = RB_TREE_RED;

        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = free_index_->right_;
        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;
        rb_tree_head_->sz_free_node_++;
        rb_tree_head_->sz_use_node_--;

        //调用显式的析构函数
        (data_base_ + pos)->~_value_type();
    }

public:

    //内存区的构成为 定义区,index区,data区,返回所需要的长度,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_rb_tree_head)  +
                sizeof(_shm_rb_tree_index) * (numnode + ADDED_NUM_OF_INDEX) +
                sizeof(_value_type) * numnode ;
    }

    //初始化
    static self *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_rb_tree_head *rb_tree_head = reinterpret_cast<_shm_rb_tree_head *>(pmmap);

        //如果是恢复,数据都在内存中,
        if ( true == if_restore)
        {
            //检查一下恢复的内存是否正确,
            if (getallocsize(numnode) != rb_tree_head->size_of_mmap_ ||
                numnode != rb_tree_head->num_of_node_ )
            {
                return NULL;
            }
        }

        //初始化尺寸
        rb_tree_head->size_of_mmap_ = getallocsize(numnode);
        rb_tree_head->num_of_node_ = numnode;

        self *instance = new self();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        instance->smem_base_ = pmmap;
        instance->rb_tree_head_ = rb_tree_head;
        instance->index_base_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head));
        instance->data_base_  = reinterpret_cast<_value_type *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode + ADDED_NUM_OF_INDEX) );

        //初始化free_index_,head_index_
        instance->head_index_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode ));
        instance->free_index_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode + 1));

        if ( false == if_restore)
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }

        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        rb_tree_head_->sz_free_node_ = rb_tree_head_->num_of_node_;
        rb_tree_head_->sz_use_node_ = 0;

        //将清理为NULL,让指针都指向自己
        head_index_->parent_ = _INVALID_POINT;
        head_index_->right_  = rb_tree_head_->num_of_node_;
        head_index_->left_   = rb_tree_head_->num_of_node_;
        head_index_->color_  = RB_TREE_RED;

        _shm_rb_tree_index *pindex = index_base_;

        free_index_->right_ = 0;

        //初始化free数据区
        for (size_t i = 0; i < rb_tree_head_->num_of_node_ ; ++i )
        {
            pindex->right_ = (i + 1) ;

            //将所有FREENODE串起来
            if (i == rb_tree_head_->num_of_node_ - 1)
            {
                pindex->right_ = rb_tree_head_->num_of_node_ + 1;
            }

            pindex++;
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
        if (rb_tree_head_->sz_free_node_ == 0 )
        {
            return true;
        }

        return false;
    };

    size_t size() const
    {
        return rb_tree_head_->sz_use_node_;
    }

    size_t capacity() const
    {
        return rb_tree_head_->num_of_node_;
    }

    //空闲的节点个数
    size_t sizefree()
    {
        return rb_tree_head_->sz_free_node_;
    }

protected:

    //本来打算把这段代码全部宏定义的，但考虑了一下，觉得还是inline就足够了。
    //宏毕竟会让代码变得丑陋，算了。而且这些函数的长度应该是可以被inline的。

    inline size_t  &header() const
    {
        return rb_tree_head_->num_of_node_;
    }

    inline size_t  &root() const
    {
        return head_index_->parent_;
    }

    inline size_t  &leftmost() const
    {
        return head_index_->left_;
    }

    inline size_t  &rightmost() const
    {
        return head_index_->right_;
    }

    inline size_t  &left(size_t x)
    {
        return (index_base_ + x)->left_;
    }

    inline size_t  &right(size_t x)
    {
        return (index_base_ + x)->right_;
    }

    inline size_t  &parent(size_t x)
    {
        return (index_base_ + x)->parent_;
    }

    inline color_type  &color(size_t x)
    {
        return (index_base_ + x)->color_;
    }

    inline const _value_type  &value(size_t x)
    {
        return *(data_base_ + x);
    }

    inline const _key_type  &key(size_t x)
    {
        return _extract_key()(value(x));
    }

    //取极大值
    size_t minimum(size_t x)
    {
        while ( left(x) != _INVALID_POINT )
        {
            x = left(x);
        }

        return x;
    }

    //取极小值
    size_t maximum(size_t x)
    {
        while ( right(x) != _INVALID_POINT )
        {
            x = right(x);
        }

        return x;
    }

protected:
    //真正的插入是由这个函数完成的
    std::pair<iterator, bool>  _insert(size_t x, size_t y, const _value_type &v)
    {
        size_t z = create_node(v);
        //如果空间不足，无法插入，返回end,false的pair
        if (_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        if ( y == header() || x != _INVALID_POINT || _compare_key()(_extract_key()(v), key(y)) )
        {
            left(y) = z;

            if ( y == header() )
            {
                root() = z;
                rightmost() = z;
            }
            else if ( y == leftmost() )
            {
                leftmost() = z;
            }
        }
        else
        {
            right(y) = z;

            if ( y == rightmost())
            {
                rightmost() = z;
            }
        }

        parent(z) = y;
        left(z) = _INVALID_POINT;
        right(z) = _INVALID_POINT;
        *(data_base_ + z) = v;

        _rb_tree_rebalance(z, parent(header()));
        return  std::pair<iterator, bool>(iterator(z, this), true);
    }

    //通过旋转和变色，调整整个树，让其符合RBTree要求
    //参数1：新增节点
    //参数2：根节点
    void _rb_tree_rebalance(size_t x, size_t &root)
    {
        color(x) = RB_TREE_RED;

        while ( x != root && color(parent(x)) == RB_TREE_RED )
        {
            if ( parent(x) == left(parent(parent(x))) )
            {
                size_t y = right(parent(parent(x)));

                if ( y != _INVALID_POINT && color(y) == RB_TREE_RED )
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if ( x == right(parent(x)) )
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
                size_t y = left(parent(parent(x)));

                if ( y != _INVALID_POINT && color(y) == RB_TREE_RED )
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if ( x == left(parent(x)) )
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
    void _rb_tree_rotate_left(size_t x, size_t &root)
    {
        size_t y = right(x);
        right(x) = left(y);

        if ( left(y) != _INVALID_POINT)
        {
            parent(left(y)) = x;
        }

        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == left(parent(x)) )
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
    void _rb_tree_rotate_right(size_t x, size_t &root)
    {
        size_t y = left(x);
        left(x) = right(y);

        if ( right(y) != _INVALID_POINT )
        {
            parent(right(y)) = x;
        }

        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == right(parent(x)) )
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
    size_t _erase(size_t z)
    {
        size_t y = z;
        size_t x = _INVALID_POINT;
        size_t x_parent = _INVALID_POINT;

        //如果左子树为NULL，选择右子树
        if (left(y) == _INVALID_POINT)
        {
            x = right(y);
        }
        else
        {
            //如果左子树不为NULL，而右子树为NULL
            if (right(x) == _INVALID_POINT)
            {
                x = left(y);
            }
            //如果左右子树都不为NULL
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

                if (x != _INVALID_POINT)
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

            if (x != _INVALID_POINT)
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
                if (right(z) == _INVALID_POINT)
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
                if (left(z) == _INVALID_POINT)
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
            while (x != root() && (x == _INVALID_POINT || color(x) == RB_TREE_BLACK))
            {
                if (x == left(x_parent))
                {
                    size_t w = right(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_left(x_parent, root());
                        w = right(x_parent);
                    }

                    if ((left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK) &&
                        (right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK)
                        {
                            if (left(w) != _INVALID_POINT)
                            {
                                color(left(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_right(w, root());
                            w = right(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (right(w) != _INVALID_POINT)
                        {
                            color(right(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_left(x_parent, root());
                        break;
                    }
                }
                else
                {
                    size_t w = left(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_right(x_parent, root());
                        w = left(x_parent);
                    }

                    if ((right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK) &&
                        (left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK)
                        {
                            if (right(w) != _INVALID_POINT)
                            {
                                color(right(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_left(w, root());
                            w = left(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (left(w) != _INVALID_POINT)
                        {
                            color(left(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_right(x_parent, root());
                        break;
                    }
                }
            }

            if (x != _INVALID_POINT)
            {
                color(x) = RB_TREE_BLACK;
            }

        }

        return y;

    }

public:

    //允许重复key插入的插入函数，Multimap、Multimap用这个
    std::pair<iterator, bool>  insert_equal(const _value_type &v)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            y = x;
            x = _compare_key()( _extract_key()(v), key(x) ) ? left(x) : right(x);
        }

        return _insert(x, y, v);
    }

    //重复key插入则失败的插入函数，Map、Sap用这个
    std::pair<iterator, bool> insert_unique(const _value_type &v)
    {
        size_t y = header();
        size_t x = root();
        bool comp = true;

        while (x != _INVALID_POINT)
        {
            y = x;
            comp = _compare_key()( _extract_key()(v), key(x) );
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);

        if (comp)
        {
            if ( j == begin() )
            {
                return _insert(x, y, v);
            }
            else
            {
                --j;
            }
        }

        if ( _compare_key()(key(j.getserial()), _extract_key()(v)) )
        {
            return _insert(x, y, v);
        }

        return std::pair<iterator, bool>(j, false);
    }

    //通过迭代器删除一个节点
    iterator erase(const iterator &pos)
    {
        size_t tmp = _erase(pos.getserial());
        destroy_node(pos.getserial());
        return iterator(tmp, this);
    }

    //通过起始迭代器删除一段节点
    size_t erase(iterator first, iterator last)
    {
        size_t erase_count = 0;

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
    size_t erase_unique(const _key_type &k)
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
    size_t erase_unique_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_unique(get_key(v));
    }

    //通过key删除节点，Multimap和Multiset用
    size_t erase_equal(const _key_type &k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //通过值删除节点，Multimap和Multiset用
    size_t erase_equal_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_equal(get_key(v));
    }

    //找到第一个key值相同的节点
    iterator lower_bound(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            //上下两个函数就这行代码不一样，注意先后比较
            if (!_compare_key()(key(x), k) )
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
    iterator upper_bound(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            //上下两个函数就这行代码不一样，注意先后比较关系
            if (_compare_key()(k, key(x)))
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
    iterator find(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while ( x != _INVALID_POINT )
        {
            if ( !_compare_key()(key(x), k) )
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
        return (j == end() || _compare_key()(k, key(j.getserial()))) ? end() : j;
    }

    //找value相同的节点
    iterator find_value(const _value_type &v)
    {
        _extract_key get_key;
        return find(get_key(v));
    }

    //找value相同的节点，如未找到则插入
    _value_type &find_or_insert(const _value_type &v)
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
    static const size_t ADDED_NUM_OF_INDEX = 2;

protected:
    //RBTree头部
    _shm_rb_tree_head                  *rb_tree_head_;

    //所有的指针都是根据基地址计算得到的,用于方便计算,每次初始化会重新计算
    //索引数据区,
    _shm_rb_tree_index                 *index_base_;

    //数据区起始指针,
    _value_type                         *data_base_;

    //头节点的头指针,N+1个索引位表示
    _shm_rb_tree_index                 *head_index_;

    //空节点的头指针,N+2个索引位表示（这里利用right节点做链接，把空节点串起来）
    _shm_rb_tree_index                 *free_index_;

};

//用RBTree实现SET，不区分multiset和set，通过不通的insert自己区分
template<class _value_type, class _compare_key = std::less<_value_type> >
class mmap_set :
    public shm_rb_tree< _value_type, _value_type, smem_identity<_value_type>, _compare_key >
{
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    mmap_set<_value_type, _compare_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_rb_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_set<_value_type, _compare_key >()
    {
    }

public:
    static mmap_set< _value_type, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_set< _value_type, _compare_key  >*>(
                   shm_rb_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>::initialize(numnode, pmmap, if_restore));
    }
};

//用RBTree实现MAP，不区分multiset和set，通过不通的insert自己区分
template<class _key_type, class _value_type, class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >, class _compare_key = std::less<_value_type>  >
class mmap_map :
    public shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >
{
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    mmap_map<_key_type, _value_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_map<_key_type, _value_type, _extract_key, _compare_key >()
    {
    }
public:
    static mmap_map< _key_type, _value_type, _extract_key, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_map< _key_type, _value_type, _extract_key, _compare_key  >*>(
                   shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key>::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，谨慎使用
    _value_type &operator[](const _key_type &key)
    {
        return (find_or_insert(std::pair<_key_type, _value_type >(key, _value_type()))).second;
    }
};

};

#endif //ZCE_LIB_SHM_RB_TREE_H_
