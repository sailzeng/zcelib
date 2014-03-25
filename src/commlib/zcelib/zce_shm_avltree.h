/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_avltree.h
* @author     Sailzeng <sailerzeng@gmail.com>
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
*             http://www.cnblogs.com/kiven-code/archive/2013/03/01/2938651.html
*
*/

#ifndef ZCE_LIB_SHM_AVL_TREE_H_
#define ZCE_LIB_SHM_AVL_TREE_H_

#include "zce_shm_predefine.h"

namespace ZCE_LIB
{




template<class _value_type, class _key_type, class _extract_key, class _compare_key> class shm_avl_tree;

///AVL TREE的头部数据区
class _shm_avl_tree_head
{
protected:
    _shm_avl_tree_head()
        : size_of_mmap_(0)
        , num_of_node_(0)
        , sz_free_node_(0)
        , sz_use_node_(0)
    {
    }
    ~_shm_avl_tree_head()
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


//AVL tree的索引的节点
class _shm_avl_tree_index
{

public:
    _shm_avl_tree_index()
        : parent_(_shm_memory_base::_INVALID_POINT)
        , left_(_shm_memory_base::_INVALID_POINT)
        , right_(_shm_memory_base::_INVALID_POINT)
        , balanced_(0)
    {
    }

    _shm_avl_tree_index(const size_t &p, const size_t &l, const size_t &r, int8_t hb)
        : parent_(p)
        , left_(l)
        , right_(r)
        , balanced_(hb)
    {
    }

    ~_shm_avl_tree_index()
    {
    }

public:
    ///父节点
    size_t       parent_;
    ///左子树
    size_t       left_;
    ///右子树
    size_t       right_;
    ///平衡
    int32_t      balanced_;
};


//AVL tree的迭代器
template < class _value_type,
         class _key_type,
         class _extract_key,
         class _compare_key >
class _shm_avl_tree_iterator
{
    typedef _shm_avl_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    typedef shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key> shm_avl_tree_t;


public:
    //构造函数
    _shm_avl_tree_iterator(size_t seq, shm_avl_tree_t *instance)
        : serial_(seq)
        , avl_tree_inst_(instance)
    {
    }

    _shm_avl_tree_iterator()
        : serial_(_shm_memory_base::_INVALID_POINT),
          avl_tree_inst_(NULL)
    {
    }

    ~_shm_avl_tree_iterator()
    {
    }

    //初始化
    void initialize(size_t seq, shm_avl_tree_t *instance)
    {
        serial_ = seq;
        avl_tree_inst_ = instance;
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && avl_tree_inst_ == x.avl_tree_inst_);
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
        return avl_tree_inst_->getdatabase() + serial_;
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

    ///用于实现operator++，找下一个比自己大(比较函数而言)的节点
    void increment()
    {
        if ((avl_tree_inst_->index_base_ + serial_)->right_ != _shm_memory_base::_INVALID_POINT)
        {
            //如果有右子节点，就向右走，然后一直沿左子树走到底即可
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;

            while ((avl_tree_inst_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT)
            {
                serial_ = (avl_tree_inst_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //如果没有右子节点，找到父节点，如果当前节点是某个右子节点，就一直上溯到不为右子节点为止
            size_t y = (avl_tree_inst_->index_base_ + serial_)->parent_;

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
        if (avl_tree_inst_->avl_tree_head_.num_of_node_ == serial_)
        {
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;
        }
        //如果有左子节点
        else if ((avl_tree_inst_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT)
        {
            //令y指向左子节点，找到y的右子节点，向右走到底即是
            size_t y = (avl_tree_inst_->index_base_ + serial_)->left_;

            while ((avl_tree_inst_->index_base_ + y)->right_ != _shm_memory_base::_INVALID_POINT)
            {
                y = (avl_tree_inst_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //找出父节点，如果当前节点是个左子节点，就一直上溯，直到不再为左子节点，则其的父节点即是
            size_t y = (avl_tree_inst_->index_base_ + serial_)->parent_;

            while (serial_ == (avl_tree_inst_->index_base_ + y)->left_)
            {
                serial_ = y;
                y = (avl_tree_inst_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }


protected:
    //序列号
    size_t          serial_;
    //RBtree的实例指针
    shm_avl_tree_t  *avl_tree_inst_;

};


/*!
* @brief      AVL Tree的容器，用于排序的处理等，
*
* @tparam     _value_type   数据类型
* @tparam     _key_type     KEY的类型
* @tparam     _extract_key  如果从_value_type中获取_key_type的方法
* @tparam     _compare_key  比较方法
*/
template < class _value_type,
         class _key_type,
         class _extract_key = smem_identity<_value_type>,
         class _compare_key = std::less<_key_type> >
class shm_avl_tree : public _shm_memory_base
{
public:
    //定义自己
    typedef shm_avl_tree < _value_type,
            _key_type,
            _extract_key,
            _compare_key > self;

    //定义迭代器
    typedef _shm_avl_tree_iterator < _value_type,
            _key_type,
            _extract_key,
            _compare_key > iterator;

    //迭代器友元
    friend class _shm_avl_tree_iterator<_value_type, _key_type, _extract_key, _compare_key>;



public:

    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore)
        : _shm_memory_base(NULL)
        , index_base_(NULL)
        , data_base_(NULL)
    {
    }

    shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >()
        : _shm_memory_base(NULL)
    {
    }

    ~shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >()
    {
    }

    //只定义,不实现,避免犯错
    const self &operator=(const self &others);

    //得到索引的基础地址
    inline _shm_avl_tree_index *getindexbase()
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
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //从链上取1个下来
        size_t new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        avl_tree_head_->sz_free_node_--;
        avl_tree_head_->sz_use_node_++;

        //初始化
        (index_base_ + new_node)->parent_ = _INVALID_POINT;
        (index_base_ + new_node)->left_ = _INVALID_POINT;
        (index_base_ + new_node)->right_ = _INVALID_POINT;
        (index_base_ + new_node)->balanced_ = 0;

        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = free_index_->right_;
        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;
        avl_tree_head_->sz_free_node_++;
        avl_tree_head_->sz_use_node_--;
    }

public:

    //内存区的构成为 头部定义区,index区,data区,返回所需要的长度,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_avl_tree_head)
                + sizeof(_shm_avl_tree_index) * (numnode + ADDED_NUM_OF_INDEX)
                + sizeof(_value_type) * numnode;
    }

    //初始化
    static self *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_avl_tree_head *avl_tree_head = reinterpret_cast<_shm_avl_tree_head *>(pmmap);

        //如果是恢复,数据都在内存中,
        if (true == if_restore)
        {
            //检查一下恢复的内存是否正确,
            if (getallocsize(numnode) != avl_tree_head->size_of_mmap_ ||
                numnode != avl_tree_head->num_of_node_)
            {
                return NULL;
            }
        }

        //初始化尺寸
        avl_tree_head->size_of_mmap_ = getallocsize(numnode);
        avl_tree_head->num_of_node_ = numnode;

        self *instance = new self();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        instance->smem_base_ = pmmap;
        //头部
        instance->avl_tree_head_ = avl_tree_head;
        //索引区
        instance->index_base_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head));
        //数据区
        instance->data_base_ = reinterpret_cast<_value_type *>(
                                   pmmap +
                                   sizeof(_shm_rb_tree_head) +
                                   sizeof(_shm_avl_tree_index) * (numnode + ADDED_NUM_OF_INDEX));

        //初始化free_index_,head_index_
        instance->head_index_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head) +
                                    sizeof(_shm_avl_tree_index) * (numnode));
        instance->free_index_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head) +
                                    sizeof(_shm_avl_tree_index) * (numnode + 1));

        if (false == if_restore)
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
        avl_tree_head_->sz_free_node_ = avl_tree_head_->num_of_node_;
        avl_tree_head_->sz_use_node_ = 0;

        //将清理为NULL,让指针都指向自己
        head_index_->parent_ = _INVALID_POINT;
        head_index_->right_ = avl_tree_head_->num_of_node_;
        head_index_->left_ = avl_tree_head_->num_of_node_;
        head_index_->balanced_ = 0;

        //
        free_index_->left_ = _INVALID_POINT;
        free_index_->parent_ = _INVALID_POINT;
        free_index_->balanced_ = 0;

        //用right_串起来FREE NODE的列表
        free_index_->right_ = 0;

        //初始化free数据区
        _shm_avl_tree_index *pindex = index_base_;
        for (size_t i = 0; i < avl_tree_head_->num_of_node_; ++i)
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

    size_t size() const
    {
        return avl_tree_head_->sz_use_node_;
    }

    size_t capacity() const
    {
        return avl_tree_head_->num_of_node_;
    }

    //空闲的节点个数
    size_t sizefree()
    {
        return avl_tree_head_->sz_free_node_;
    }

protected:
    inline size_t  &header() const
    {
        return avl_tree_head_->num_of_node_;
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

    inline int32_t &balanced(size_t x)
    {
        return (index_base_ + x)->balanced_;
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
        while (left(x) != _INVALID_POINT)
        {
            x = left(x);
        }

        return x;
    }

    //取极小值
    size_t maximum(size_t x)
    {
        while (right(x) != _INVALID_POINT)
        {
            x = right(x);
        }

        return x;
    }

protected:


    /*!
    * @brief      真正的插入是由这个函数完成的
    * @return     std::pair<iterator, bool> 返回的插入结构，包括迭代器和结果
    * @param      x   插入点,大部分时候为_INVALID_POINT
    * @param      y   插入点的父节点
    * @param      val 插入的数据
    */
    std::pair<iterator, bool> _insert(size_t x, size_t y, const _value_type &val)
    {
        //分配一个空间
        size_t z = create_node(val);
        //日过空间不足，无法插入，返回end,false的pair
        if (_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //把此二货插入进去，而且调整各种东东

        //如果1.插入的是root节点，2.如果插入节点不是空节点，3.如果比较为TRUE
        if (y == header() || x != _INVALID_POINT || _compare_key()(_extract_key()(val), key(y)))
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
        left(z) = _INVALID_POINT;
        right(z) = _INVALID_POINT;

        //如果不是根节点，我们进行平衡调整
        if (y != header())
        {
            _balance_adjust(z,true);
        }

        return   std::pair<iterator, bool>(iterator(z, this), true);
    }



    /*!
    * @brief      进行平衡调整，内部函数，
    * @param[in]  z 插入的节点位置
    * @param[in]  if_inssert 是否是插入操作进行调整，如果是删除操作，填写false
    */
    void _balance_adjust(size_t z,bool if_inssert)
    {
        //其实这个地方直接使用常量还更加清晰一点,所以我没有用枚举或者宏

        //找到最小的不平衡的点,
        size_t s = z;
        size_t t = _INVALID_POINT;
        int32_t mod_balance = 0;
        while (s != header())
        {
            t = s;
            s = parent(s);
            if (if_inssert)
            {
                mod_balance = (t == left(s)) ? 1 : -1;
            }
            else
            {
                mod_balance = (t == left(s)) ? -1 : 1;
            }

            //如果是平衡的，修改平衡参数，继续向上干活
            if (0  == balanced(s) )
            {
                balanced(s) += mod_balance;
                continue;
            }
            //这个点上原来就不平衡，找到最小的不平衡树，进行旋转，让其平衡
            else
            {
                balanced(s) += mod_balance;
                //根据不平衡的情况，决定进行什么样的旋转
                if (2 == balanced(s) )
                {
                    if (1 == balanced(t))
                    {
                        _ll_rotate(s, t);
                    }
                    else
                    {
                        _lr_rotate(s, t, right(t));
                    }
                }
                else if (-2 == balanced(s))
                {
                    if (1 == balanced(t))
                    {
                        _rl_rotate(s, t, left(t));
                    }
                    else
                    {
                        _rr_rotate(s, t);
                    }
                }
                break;
            }
        }
        return ;
    }

    /*!
    * @brief      LL旋转，
    * @param      p   父节点，最小的不平衡树的根节点
    * @param      lc  左边的子节点
    */
    void _ll_rotate(size_t p, size_t lc)
    {
        size_t gf = parent(p);
        parent(p) = lc;
        left(p) = right(lc);
        right(lc) = p;
        parent(lc) = gf;

        //调整平衡因子
        balanced(p) = 0;
        balanced(lc) = 0;

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (left(gf) == p)
        {
            left(gf) = lc;
        }
        else
        {
            right(gf) = lc;
        }
    }

    /*!
    * @brief      LR旋转
    * @param      p   父节点，最小的不平衡树的根节点
    * @param      lc  p的左子节点
    * @param      rgs lc的右子节点
    */
    void _lr_rotate(size_t p, size_t lc, size_t rgs)
    {
        size_t gf = parent(p);
        parent(p) = rgs;
        left(p) = right(rgs);
        parent(lc) = rgs;
        right(lc) = left(rgs);
        left(rgs) = lc;
        right(rgs) = p;

        //调整平衡因子
        balanced(p) = -1;
        balanced(lc) = 0;
        balanced(rgs) = 0;

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (left(gf) == p)
        {
            left(gf) = rgs;
        }
        else
        {
            right(gf) = rgs;
        }
    }

    /*!
    * @brief      RR旋转，
    * @param      p   父节点，最小的不平衡树的根节点
    * @param      rc  右边的子节点
    */
    void _rr_rotate(size_t p, size_t rc)
    {
        size_t gf = parent(p);
        parent(p) = rc;
        right(p) = left(rc);
        left(rc) = p;
        parent(rc) = gf;

        //调整平衡因子
        balanced(p) = 0;
        balanced(rc) = 0;

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (left(gf) == p)
        {
            left(gf) = rc;
        }
        else
        {
            right(gf) = rc;
        }
    }

    void _rl_rotate(size_t p, size_t rc, size_t lgs)
    {
        size_t gf = parent(p);
        parent(p) = lgs;
        right(p) = left(lgs);
        parent(rc) = lgs;
        left(rc) = right(lgs);
        left(lgs) = p;
        right(lgs) = rc;


        //调整平衡因子
        balanced(p) = 0;
        balanced(rc) = -1;
        balanced(lgs) = 0;

        //调整p的父节点的左右子树，让其指向新的子树新根
        if (left(gf) == p)
        {
            left(gf) = lgs;
        }
        else
        {
            right(gf) = lgs;
        }
    }

public:

    /*!
    * @brief      允许重复key插入的插入函数，Multimap、Multimap用这个
    *             因为空间可能满，所以返回的数据中还是有bool的pair
    * @return     std::pair<iterator, bool>  返回的iterator为迭代器，bool为是否插入成功
    * @param      v        插入的_value_type的数据
    */
    std::pair<iterator, bool> insert_equal(const _value_type &v)
    {
        //如果依据满了，也返回失败
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        size_t y = header();
        size_t x = root();

        //插入到一个空节点上
        while (x != _INVALID_POINT)
        {
            y = x;
            x = _compare_key()(_extract_key()(v), key(x)) ? left(x) : right(x);
        }

        return _insert(x, y, v);
    }

    /*!
    * @brief      重复key插入则失败的插入函数，Map、Sap用这个
    * @return     std::pair<iterator, bool> 返回的iterator为迭代器，bool为是否插入成功
    * @param      v 插入的_value_type的数据
    */
    std::pair<iterator, bool> insert_unique(const _value_type &v)
    {
        //如果依据满了，也返回失败
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        size_t y = header();
        size_t x = root();
        bool comp = true;

        //如果比较(比如是<)返回true,就向左，否则(>=)就向右，
        while (x != _INVALID_POINT)
        {
            y = x;
            comp = _compare_key()(_extract_key()(v), key(x));
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);

        if (comp)
        {
            if (j == begin())
            {
                return _insert(x, y, v);
            }
            else
            {
                --j;
            }
        }

        if (_compare_key()(key(j.getserial()), _extract_key()(v)))
        {
            return _insert(x, y, v);
        }

        //如果既不是>,又不是<，那么就是==,那么返回错误
        return std::pair<iterator, bool>(j, false);
    }

    //通过迭代器删除一个节点
    iterator erase(const iterator &pos)
    {
        //x,为删除的位置，y为X的父节点，z用于为替换x的节点
        size_t x = pos.getserial();
        size_t y = parent(x);
        size_t z = _INVALID_POINT;

        //默认右子树的值
        if (right(x) != _INVALID_POINT)
        {
            z = maximum(right(x));
        }
        else if (left(x) != _INVALID_POINT)
        {
            z = minimum(left(x));

        }
        else
        {
            z = _INVALID_POINT;
        }

        //
        if (left(y) == x)
        {
            left(y) = z;
            left(z) = left(x);
            right(z) = right(x);
            if (y == header())
            {
                root() = z;
                rightmost() = z;
            }
            //如果Y是最小值，则吧最小值改为Y
            else if (x == leftmost())
            {
                leftmost() = z;
            }
        }
        else
        {
            right(y) = z;
            left(z) = left(x);
            right(z) = right(x);
            if (x == rightmost())
            {
                rightmost() = z;
            }
        }

        //如果不是根节点，我们进行平衡调整
        if (y != header())
        {
            _balance_adjust(z, false);
        }
        destroy_node(x);

        return iterator(z,this);
    }

    //通过key删除节点，Map和Set用
    size_t erase_unique(const _key_type &k)
    {

        iterator find_iter = find(k);
        //没有找到相应的节点，删除失败
        if (find_iter == end())
        {
            return 0;
        }

        erase(find_iter);

        return 1;
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

        iterator find_iter = find(k);
        //没有找到相应的节点，删除失败
        if (find_iter == end())
        {
            return 0;
        }
        
        iterator j = erase(find_iter);
        if (j == end())
        {
            return 1;
        }
        
        size_t erase_count = 1;
        while (_compare_key()(k, key(j.getserial())) &&
            _compare_key()(key(j.getserial()), k))
        {
            j = erase(j);
            ++erase_count;
        }
        return erase_count;
    }

    //通过值删除节点，Multimap和Multiset用
    size_t erase_equal_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_equal(get_key(v));
    }

    //找key相同的节点
    iterator find(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            if (!_compare_key()(key(x), k))
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
        //注意两次调用_compare_key的比较参数顺序喔
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
    ///RBTree头部
    _shm_avl_tree_head            *avl_tree_head_;

    ///所有的指针都是根据基地址计算得到的,用于方便计算,每次初始化会重新计算
    ///索引数据区,
    _shm_avl_tree_index            *index_base_;

    ///数据区起始指针,
    _value_type                   *data_base_;

    ///头节点的头指针,N+1个索引位表示
    _shm_avl_tree_index            *head_index_;

    ///空节点的头指针,N+2个索引位表示（这里利用right节点做链接，把空节点串起来）
    _shm_avl_tree_index            *free_index_;

};

//用AVL Tree实现SET，不区分multiset和set，通过不通的insert自己区分
template < class _value_type,
         class _compare_key = std::less<_value_type> >
class mmap_avl_set :
    public shm_avl_tree< _value_type, _value_type, smem_identity<_value_type>, _compare_key >
{
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    mmap_avl_set<_value_type, _compare_key >(size_t numnode, void *pmmap, bool if_restore) :
        shm_avl_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_avl_set<_value_type, _compare_key >()
    {
    }

public:

    static mmap_avl_set< _value_type, _compare_key  >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_set< _value_type, _compare_key  >*>(
                   shm_avl_tree < _value_type,
                   _value_type,
                   smem_identity<_value_type>,
                   _compare_key >::initialize(numnode, pmmap, if_restore));
    }
};

//用AVL Tree实现MAP，不区分multiset和set，通过不通的insert自己区分
template < class _key_type,
         class _value_type,
         class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >,
         class _compare_key = std::less<_value_type>  >
class mmap_avl_map :
    public shm_avl_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >
{
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    mmap_avl_map<_key_type, _value_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore) :
        shm_avl_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_avl_map<_key_type, _value_type, _extract_key, _compare_key >()
    {
    }
public:
    static mmap_avl_map< _key_type, _value_type, _extract_key, _compare_key  >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast < mmap_avl_map < _key_type,
               _value_type,
               _extract_key,
               _compare_key  > * > (
                   shm_avl_tree < std::pair < _key_type,
                   _value_type > ,
                   _key_type,
                   _extract_key,
                   _compare_key >::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，谨慎使用
    _value_type &operator[](const _key_type &key)
    {
        return (find_or_insert(std::pair<_key_type, _value_type >(key, _value_type()))).second;
    }
};


};





#endif //ZCE_LIB_SHM_AVL_TREE_H_

