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
* @note       这个代码很早想写，但到了2013年3月8日我也没有真正动笔，
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

enum
{
    AVLTREE_BALANCED = 0,
    AVLTREE_LEFT_BALANCED = 1,
    AVLTREE_RIGHT_BALANCED = -1,
    AVLTREE_LEFT_HIGH = 2,
    AVLTREE_RIGHT_HIGH = -2,
};


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
    //父节点
    size_t       parent_;
    //左子树
    size_t       left_;
    //右子树
    size_t       right_;
    //平衡高度
    int32_t      balanced_;

    _shm_avl_tree_index()
        : parent_(_shm_memory_base::_INVALID_POINT)
        , left_(_shm_memory_base::_INVALID_POINT)
        , right_(_shm_memory_base::_INVALID_POINT)
        , balanced_(AVLTREE_BALANCED)
    {
    }

    _shm_avl_tree_index(const size_t &p, const size_t &l, const size_t &r, int32_t hb)
        : parent_(p)
        , left_(l)
        , right_(r)
        , balanced_(hb)
    {
    }

    ~_shm_avl_tree_index()
    {
    }
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

protected:
    //序列号
    size_t          serial_;
    //RBtree的实例指针
    shm_avl_tree_t  *avl_tree_inst_;

public:
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

    //用于实现operator--，找上一个节点
    void decrement()
    {
        //如果是红节点，且父节点的的父节点等于自己
        if ((avl_tree_inst_->index_base_ + serial_)->color == RB_TREE_RED &&
            (avl_tree_inst_->index_base_ + ((avl_tree_inst_->index_base_ + serial_)->parent_))->parent_ == serial_)
        {
            //右子节点即是
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
};


/*!
* @brief      
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
        (index_base_ + new_node)->balanced_ = AVLTREE_BALANCED;

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
        head_index_->balanced_ = AVLTREE_BALANCED;

        //
        free_index_->left_ = _INVALID_POINT;
        free_index_->parent_ = _INVALID_POINT;
        free_index_->balanced_ = AVLTREE_BALANCED;

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

    inline int32_t  &balanced(size_t x)
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

    //真正的插入是由这个函数完成的
    std::pair<iterator, bool> _insert(size_t x, size_t y, const _value_type &v)
    {
        size_t z = create_node();
        if (_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        return  iterator(z, this);
    }


    void _balance_adjust(AVLNode<T>* pNode)
    {
        //     
        while (pNode != NULL)//删除节点的子节点进行平衡
        {
            pPNode = pNode->pParent;
            
            bool bIsLeft = false;
            if (pPNode != NULL && pNode == pPNode->pLeft)
            {
                bIsLeft = true;
            }
                
            
            pNode->nHeight = Max(Height(pNode->pLeft), Height(pNode->pRight)) + 1;
            // AVL树不平衡  执行LL型或者LR型旋转
            if (Height(pNode->pLeft) - Height(pNode->pRight) == 2)    
            {
                if (Height(pNode->pLeft->pLeft) - Height(pNode->pLeft->pRight) == -1)
                    pNode = RotateLeftRight(pNode);
                else
                    pNode = RotateLeft(pNode);
                261
                    262             if (pPNode != NULL && bIsLeft)
                    263                 pPNode->pLeft = pNode;
                    else if (pPNode != NULL)
                    {
                        pPNode->pRight = pNode;
                    }
                
            }
            // AVL树不平衡  执行RR型或者RL型旋转
            else if (Height(pNode->pLeft) - Height(pNode->pRight) == -2)    
                268         {
                269             if (Height(pNode->pRight->pLeft) - Height(pNode->pRight->pRight) == 1)
                    270                 pNode = RotateRightLeft(pNode);
                271             else
                    272                 pNode = RotateRight(pNode);
                273
                    274             if (pPNode != NULL && bIsLeft)
                    275                 pPNode->pLeft = pNode;
                276             else if (pPNode != NULL)
                    277                 pPNode->pRight = pNode;
                278         }
            
            pRoot = pNode;
            pNode = pPNode;
         }
        
        return pRoot;
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

        //插入到一个叶子上
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
        size_t tmp = __rb_tree_rebalance_for_erase(pos.getserial(), /*head_index_->parent*/root(), leftmost(), rightmost());
        destroy_node(pos.getserial());
        return iterator(tmp, this);
    }

    //通过起始迭代器删除一段节点
    size_t erase(iterator __first, iterator __last)
    {
        size_t ret = 0;

        if (__first == begin() && __last == end())
        {
            ret = size();
            clear();
        }
        else
        {
            while (__first != __last)
            {
                ++ret;
                erase(__first++);
            }
        }

        return ret;
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

        return iterator(y, this);
    }

    //找到最后一个key值相同的节点
    iterator upper_bound(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
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
                   shm_avl_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key>::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，谨慎使用
    _value_type &operator[](const _key_type &key)
    {
        return (find_or_insert(std::pair<_key_type, _value_type >(key, _value_type()))).second;
    }
};


};





#endif //ZCE_LIB_SHM_AVL_TREE_H_

