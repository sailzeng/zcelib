/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_expire.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       2005年10月26日
* @brief      组合HASH TABLE，LIST的功能的HASH表，用于在LRU淘汰的时候方便的
*             提供HASH的查询功能，同时提供LRU的淘汰机制
*             
* @details    
*             
*             
*             
* @note       
*             
*/



#ifndef ZCE_SHM_LRU_HASH_TABLE_H_
#define ZCE_SHM_LRU_HASH_TABLE_H_

#include "zce_shm_predefine.h"


namespace ZCE_LIB
{


template < class _value_type,
         class _key_type,
         class _hash_fun,
         class _extract_key,
         class _equal_key,
         class _washout_fun > class shm_hashtable_expire;

//LRU HASH 迭代器
template < class _value_type,
         class _key_type,
         class _hashfun,
         class _extract_key,
         class _equal_key ,
         class _washout_fun >
class _hashtable_expire_iterator
{
protected:

    //HASH TABLE的定义
    typedef shm_hashtable_expire < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun  > _lru_hashtable;

    //定义迭代器
    typedef _hashtable_expire_iterator < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

protected:
    //序列号
    size_t                 serial_;
    //
    _lru_hashtable        *lruht_instance_;


public:
    _hashtable_expire_iterator():
        serial_(0),
        lruht_instance_(NULL)
    {
    }

    _hashtable_expire_iterator(size_t serial, _lru_hashtable *lru_ht_inst):
        serial_(serial),
        lruht_instance_(lru_ht_inst)
    {
    }

    ~_hashtable_expire_iterator()
    {
    }

    _value_type &operator*() const
    {
        return lruht_instance_->value_base_ [serial_];
    }

    _value_type *operator->() const
    {
        return lruht_instance_->value_base_ + serial_;
    }

    //本来只提供前向迭代器，曾经以为使用可以使用LIST保证迭代的高效，发现不行，
    //可能要提供另外的函数
    //前向迭代器
    iterator &operator++()
    {
        size_t oldseq = serial_;
        serial_ = *(lruht_instance_->hash_index_base_ + serial_);

        //如果这个节点是末位的节点
        if (serial_ == _shm_memory_base::_INVALID_POINT)
        {
            //顺着Index查询.
            size_t bucket = lruht_instance_->bkt_num_value(*(lruht_instance_->value_base_ + oldseq));

            //
            while (serial_ == _shm_memory_base::_INVALID_POINT && ++bucket < lruht_instance_->capacity() )
            {
                serial_ = *(lruht_instance_->hash_factor_base_ + bucket);
            }
        }

        return *this;
    }

    //这个函数类似++，但是只在Hash数据的链表上游荡，所以性能更好
    //如果下一个数据是一样的KEY，那么就成为下个数据的迭代器。
    //否则成为end
    //其实也就比你自己做快一点点。
    iterator &goto_next_equal()
    {
        size_t oldseq = serial_;
        serial_ = *(lruht_instance_->hash_index_base_ + serial_);

        //如果这个节点不是末位的节点
        if (serial_ != _shm_memory_base::_INVALID_POINT)
        {
            _extract_key get_key;
            _equal_key   equal_key;

            if ( false == equal_key(get_key(*(lruht_instance_->value_base_ + oldseq)),
                                    get_key(*(lruht_instance_->value_base_ + serial_))) )
            {
                serial_ = _shm_memory_base::_INVALID_POINT;
            }
        }
        else
        {
        }

        return *this;
    }

    //前向迭代器
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
    //
    bool operator==(const iterator &it) const
    {
        if (lruht_instance_ == it.lruht_instance_ &&
            serial_ == it.serial_ )
        {
            return true;
        }

        return false;
    }
    //
    bool operator!=(const iterator &it) const
    {
        return !(*this == it);
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }
};


//头部，LRU_HASH的头部结构，放在LRUHASH内存的前面
class _hashtable_expire_head
{
protected:
    _hashtable_expire_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        sz_useindex_(0)
    {
    }
    ~_hashtable_expire_head()
    {
    }

public:
    //内存区的长度
    size_t           size_of_mmap_;

    //NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
    size_t           num_of_node_;

    //FREE的NODE个数
    size_t           sz_freenode_;
    //USE的NODE个数
    size_t           sz_usenode_;

    //使用的INDEX个数,可以了解实际开链的负载比率
    size_t           sz_useindex_;
};







/*!
@brief      代超时处理的在一块内存（共享内存）使用的Hash table，

@tparam     _value_type
@tparam     _key_type
@tparam     _hash_fun
@tparam     _extract_key
@tparam     _equal_key
@tparam     _washout_fun
@note
*/
template < class _value_type,
         class _key_type,
         class _hash_fun = smem_hash<_key_type>,
         class _extract_key = smem_identity<_value_type>,
         class _equal_key = std::equal_to<_key_type> ,
         class _washout_fun = _default_washout_fun<_value_type> >
class shm_hashtable_expire : public  _shm_memory_base
{
public:
    //定义迭代器
    typedef _hashtable_expire_iterator < _value_type,
            _key_type,
            _hash_fun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

    friend class _hashtable_expire_iterator < _value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key,
        _washout_fun >;

protected:
    //
    static const size_t  LIST_ADD_NODE_NUMBER = 2;

protected:
    //
    _hashtable_expire_head *lru_hash_head_;

    //Hash因子的BASE
    size_t                 *hash_factor_base_;
    //Hash的索引,hash链的索引,注释写得不清楚，自己都记不得了.
    size_t                 *hash_index_base_;


    //LIST的索引
    _shm_list_index       *lst_index_base_;
    //FREE节点链表的开始
    _shm_list_index       *lst_free_node_;
    //USE节点链表的开始
    _shm_list_index       *lst_use_node_;

    //优先级的数据指针,用32位的数据保存优先级
    unsigned int           *priority_base_;
    //数据区指针
    _value_type            *value_base_;


protected:

    shm_hashtable_expire():
        lru_hash_head_(NULL),
        hash_factor_base_(NULL),
        hash_index_base_(NULL),
        lst_index_base_(NULL),
        lst_free_node_(NULL),
        lst_use_node_(NULL),
        priority_base_(NULL),
        value_base_(NULL)
    {
    }

public:

    ~shm_hashtable_expire()
    {
    }

protected:



public:


    /*!
    * @brief      内存区的构成为 define区,index区,data区,返回所需要的长度,
    * @return     size_t   所需的尺寸
    * @param      req_num  请求的NODE数量
    * @param      real_num 实际分配的NODE数量
    * @note       注意返回的是实际INDEX长度,会取一个质数
    */
    static size_t getallocsize(size_t req_num,size_t &real_num)
    {
        //取得一个比这个数字做一定放大的质数，
        ZCE_LIB::hash_prime(req_num, real_num);
        size_t sz_alloc =  0;
        //
        sz_alloc += sizeof(_hashtable_expire_head);
        sz_alloc += sizeof(size_t)* real_num;
        sz_alloc += sizeof(size_t)* real_num;
        //
        sz_alloc += sizeof(_shm_list_index)* (real_num + LIST_ADD_NODE_NUMBER);
        sz_alloc += sizeof(unsigned int)* (real_num);
        sz_alloc += sizeof(_value_type)* (real_num);
        return sz_alloc;
    }

    //初始化
    static shm_hashtable_expire < _value_type,
           _key_type,
           _hash_fun,
           _extract_key,
           _equal_key,
           _washout_fun > *
           initialize(size_t req_num, size_t &real_num, char *pmmap, bool if_restore = false)
    {
        assert(pmmap != NULL && req_num > 0);
        //调整
        size_t sz_mmap = getallocsize(req_num, real_num);
        _hashtable_expire_head *hashhead =  reinterpret_cast< _hashtable_expire_head * >(pmmap);

        //如果是恢复,数据都在内存中,
        if ( true == if_restore )
        {


            //检查一下恢复的内存是否正确,
            //在特殊的恢复，比如取质数的方法被改变了，
            if (sz_mmap != hashhead->size_of_mmap_ ||
                real_num != hashhead->num_of_node_ )
            {

//一般情况下不一致返回NULL，标识恢复失败，
#if ALLOW_RESTORE_INCONFORMITY != 1
                return NULL;
#else
                ZCE_LOGMSG(RS_ALERT, "Expire hash node initialize number[%lu|%lu] and restore number [%lu|%lu] "
                    "is different,but user defind ALLOW_RESTORE_INCONFORMITY == 1.Please notice!!! ",
                    sz_mmap,
                    real_num,
                    hashhead->size_of_mmap_,
                    hashhead->num_of_node_);
#endif
            }
        }
        else
        {
            //记录初始化尺寸
            hashhead->size_of_mmap_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }
        

        shm_hashtable_expire< _value_type, _key_type , _hash_fun, _extract_key, _equal_key, _washout_fun >* instance
            = new shm_hashtable_expire< _value_type, _key_type , _hash_fun, _extract_key, _equal_key, _washout_fun>();

        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;
        instance->lru_hash_head_ = reinterpret_cast<_hashtable_expire_head *>(tmp_base);
        tmp_base = tmp_base + sizeof(_hashtable_expire_head);
        instance->hash_factor_base_ = reinterpret_cast<size_t *>(tmp_base);
        tmp_base = tmp_base + sizeof(size_t) * real_num;
        instance->hash_index_base_ = reinterpret_cast<size_t *>(tmp_base);

        tmp_base = tmp_base + sizeof(size_t) * real_num;
        instance->lst_index_base_ = reinterpret_cast<_shm_list_index *>(tmp_base);
        tmp_base = tmp_base + sizeof(_shm_list_index) * (real_num + LIST_ADD_NODE_NUMBER);
        instance->lst_use_node_ = instance->lst_index_base_ + real_num;
        instance->lst_free_node_ = instance->lst_index_base_ + real_num + 1;

        instance->priority_base_ = reinterpret_cast<unsigned int *>(tmp_base);
        tmp_base = tmp_base + sizeof(unsigned int) * (real_num );
        instance->value_base_ = reinterpret_cast<_value_type *>(tmp_base);

        if ( false == if_restore )
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }
        //其实如果是恢复，还应该检查一次所有的队列

        //打完收工
        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        lru_hash_head_->sz_freenode_ = lru_hash_head_->num_of_node_;
        lru_hash_head_->sz_usenode_ = 0;
        lru_hash_head_->sz_useindex_ = 0;

        //将两个队列都清理为NULL,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        lst_use_node_->idx_next_ = lru_hash_head_->num_of_node_ ;
        lst_use_node_->idx_prev_ = lru_hash_head_->num_of_node_ ;

        lst_free_node_->idx_next_ = lru_hash_head_->num_of_node_ + 1;
        lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ + 1;

        _shm_list_index *pindex = lst_index_base_;

        //初始化free数据区
        for (size_t i = 0; i < lru_hash_head_->num_of_node_ ; ++i )
        {

            //
            hash_factor_base_[i] = _INVALID_POINT;
            hash_index_base_[i] = _INVALID_POINT;
            priority_base_[i] = 0;

            pindex->idx_next_ = (i + 1) ;
            pindex->idx_prev_ = (i - 1) ;

            //将所有的数据用FREENODE串起来
            if (0 == i)
            {
                pindex->idx_prev_ = lst_free_node_->idx_next_;
                lst_free_node_->idx_next_ = 0;
            }

            if (i == lru_hash_head_->num_of_node_ - 1)
            {
                pindex->idx_next_ = lst_free_node_->idx_prev_;
                lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ - 1;
            }

            pindex++;
        }
    }

protected:

    //分配一个NODE,将其从FREELIST中取出
    size_t create_node(const _value_type &val, unsigned int priority )
    {
        //如果没有空间可以分配
        if (lru_hash_head_->sz_freenode_ == 0)
        {
            return _INVALID_POINT;
        }

        //从链上取1个下来
        size_t newnode = lst_free_node_->idx_next_;

        lst_free_node_->idx_next_ = (lst_index_base_ + newnode)->idx_next_;
        //lst_free_node_->idx_next_已经向后调整一个位置了
        (lst_index_base_ + lst_free_node_->idx_next_)->idx_prev_ = (lst_index_base_ + newnode)->idx_prev_;

        //注意num_of_node_的位置是usenode
        lst_index_base_[newnode].idx_next_ = lst_use_node_->idx_next_;
        lst_index_base_[newnode].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
        lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = newnode;
        lst_use_node_->idx_next_ = newnode;

        //placement new记录数据和优先级
        new (value_base_ + newnode)  _value_type( val);
        priority_base_[newnode] = priority;

        lru_hash_head_->sz_usenode_  ++;
        lru_hash_head_->sz_freenode_ --;

        //检查你干错事情么没有
        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
        return newnode;
    }

    //释放一个NODE,将其归还给FREELIST,单向链表就是简单
    void destroy_node(size_t pos)
    {
        size_t freenext = lst_free_node_->idx_next_;

        size_t pos_next = lst_index_base_[pos].idx_next_ ;
        size_t pos_prev = lst_index_base_[pos].idx_prev_ ;
        lst_index_base_[pos_next].idx_prev_ = pos_prev;
        lst_index_base_[pos_prev].idx_next_ = pos_next;

        (lst_index_base_ + pos)->idx_next_ = freenext;
        (lst_index_base_ + pos)->idx_prev_ = (lst_index_base_ + freenext)->idx_prev_;

        lst_free_node_->idx_next_ = pos;

        (lst_index_base_ + freenext)->idx_prev_ = pos;
        lru_hash_head_->sz_usenode_  --;
        lru_hash_head_->sz_freenode_ ++;

        hash_index_base_[pos] = _INVALID_POINT;


        //调用显式的析构函数
        (value_base_ + pos)->~_value_type();
        priority_base_[pos] = 0;

        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }


    //从value中取值
    size_t bkt_num_value(const _value_type &obj) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj)));
    }

    //为什么不能重载上面的函数,自己考虑一下,
    //重载的话，如果_value_type和_key_type一样，就等着哭吧 ---inmore
    size_t bkt_num_key(const _key_type &key) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % lru_hash_head_->num_of_node_);
    }

public:


    //得到开始的迭代器的位置
    iterator begin()
    {
        for (size_t i = 0; i < lru_hash_head_->num_of_node_; ++i)
        {
            if (*(hash_factor_base_ + i) != _INVALID_POINT)
            {
                return iterator(*(hash_factor_base_ + i), this);
            }
        }

        return end();
    }

    //得到结束位置
    iterator end()
    {
        return iterator(_INVALID_POINT, this);
    }
    //当前使用的节点数量
    size_t size() const
    {
        return lru_hash_head_->sz_usenode_;
    }
    //得到容量
    size_t capacity() const
    {
        return lru_hash_head_->num_of_node_;
    }
    //
    bool empty() const
    {
        return (lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }
    //是否空间已经满了
    bool full() const
    {
        return (lru_hash_head_->sz_freenode_ == 0);
    }


    /*!
    * @brief      插入节点
    * @return     std::pair<iterator, bool>  返回的迭代器和bool,
    * @param      val      插入的节点
    * @param      priority 优先级可以，传递入当前时间作为参数，淘汰时用小于某个值
    *                      都淘汰的方法处理,所以要保证后面传入数据值更大  我为什
    *                      么不直接用time(NULL),是给你更大的灵活性,
    * @note       这儿会将插入的数据放在最后淘汰的地方
    */
    std::pair<iterator, bool> insert_unique(const _value_type &val,
                                            unsigned int priority  /*=reinterpret_cast<unsigned int>(time(NULL))*/  )
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {

            //如果找到相同的Key函数
            if (equal_key((get_key(value_base_[first])), (get_key(val))) == true )
            {
                return std::pair<iterator, bool>(iterator(first, this), false);
            }

            first = hash_index_base_[ first ];
        }

        //没有找到,插入新数据
        size_t newnode = create_node(val, priority);

        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //放入链表中
        hash_index_base_[newnode]  = hash_factor_base_[idx];
        hash_factor_base_[idx] = newnode;


        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }


    //插入节点,允许相等
    //优先级可以，传递入当前时间作为参数，
    std::pair<iterator, bool> insert_equal(const _value_type &val,
                                           unsigned int priority /*=reinterpret_cast<unsigned int>(time(NULL))*/ )
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key函数
            if (equal_key((get_key(value_base_[first])), (get_key(val))) == true )
            {
                break;
            }

            first = hash_index_base_[ first ];
        }

        //没有找到,插入新数据
        size_t newnode = create_node(val, priority);

        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //没有找到相同KEY的数据
        if (first == _INVALID_POINT)
        {
            //放入链表的首部就可以了
            hash_index_base_[newnode]  = hash_factor_base_[idx];
            hash_factor_base_[idx] = newnode;
        }
        //如果找到了相同的KEY节点
        else
        {
            //放到这个节点的后面
            hash_index_base_ [newnode] = hash_index_base_[first];
            hash_index_base_[first] = newnode;
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    //查询相应的Key是否有,返回迭代器
    iterator find(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT && !equal_key(get_key(value_base_[first]), key))
        {
            first = hash_index_base_[ first ];
        }

        return iterator(first, this);
    }

    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    //得到某个KEY的元素个数，有点相当于查询操作
    size_t count(const _key_type &key)
    {
        size_t equal_count = 0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                ++equal_count;
            }

            first = hash_index_base_[ first ];
        }

        return equal_count;
    }

    //得到某个VALUE的元素个数，有点相当于查询操作
    size_t count_value(const _value_type &val)
    {
        _extract_key get_key;
        return count( get_key(val));
    }

    /*!
    * @brief      
    * @return     bool
    * @param      key
    */
    bool erase_unique(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //回收空间
                destroy_node(first);

                return true;
            }

            prev = first;
            first = hash_index_base_[ first ];
        }

        return false;
    }


    /*!
    * @brief      使用迭代器删除,尽量高效所以不用简化写法
    * @return     bool 返回值
    * @param      it   删除的迭代器
    */
    bool erase(const iterator &it)
    {
        _extract_key get_key;
        size_t idx = bkt_num_key(get_key(*it));
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;
        size_t itseq = it.getserial();

        //
        while (first != _INVALID_POINT )
        {
            if (first == itseq )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //回收空间
                destroy_node(first);

                return true;
            }

            prev = first;
            first = hash_index_base_[ first ];
        }

        return false;
    }

    //删除某个值
    bool erase_unique_value(const _value_type &val )
    {
        _extract_key get_key;
        return erase_unique( get_key(val));
    }


    /*!
    * @brief      删除所有相等的KEY的数据,和insert_equal配对使用，返回删除了几个数据
    * @return     size_t
    * @param      key
    * @note       
    */
    size_t erase_equal(const _key_type &key)
    {
        size_t erase_count = 0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //循环查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //删除的情况下prev不用调整，first向后移动
                size_t del_pos = first;
                first = hash_index_base_[ first ];
                //回收空间
                destroy_node(del_pos);
                ++erase_count;
            }
            else
            {
                //如果已经删除过，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (erase_count > 0)
                {
                    break;
                }

                prev = first;
                first = hash_index_base_[ first ];
            }
        }

        return erase_count;
    }



    /*!
    * @brief      激活,将激活的数据挂到LIST的最开始,淘汰使用expire,disuse
    * @return     bool
    * @param      key
    * @param      priority 优先级参数可以使用当前的时间
    */
    bool active_unique(const _key_type &key,
                       unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }

            first = hash_index_base_[ first ];
        }

        return false;
    }



    /*!
    * @brief      通过VALUE激活，同时讲值替换成最新的数据VALUE，
    *             优先级参数可以使用当前的时间，MAP使用，
    * @return     bool     是否激活成功
    * @param      val      值
    * @param      priority 优先级
    * @note       LRU中如果，一个值被使用后，可以认为是激活过一次，
    */
    bool active_unique_value(const _value_type &val,
                             unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key = get_key(val);
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?



        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //激活所有相同的KEY,将激活的数据挂到LIST的最开始,淘汰使用expire
    size_t active_equal(const _key_type &key,
                        unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        size_t active_count = 0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[first].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_use_node_->idx_next_ = first;
            }
            else
            {
                //如果已经有数据，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (active_count > 0)
                {
                    break;
                }
            }

            first = hash_index_base_[ first ];

        }

        return active_count;
    }



    //淘汰过期的数据,假设LIST中间的数据是按照过期实际排序的，这要求你传入的优先级最好是时间
    //小于等于这个优先级的数据将被淘汰
    size_t expire(unsigned int expire_time)
    {
        //从尾部开始检查，
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t expire_num = 0;

        while (list_idx != lru_hash_head_->num_of_node_)
        {
            //小于等于
            if (priority_base_[list_idx] <= expire_time)
            {
                size_t del_iter = list_idx;
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
            else
            {
                break;
            }

            //如果删除了，还是检查第一个
            list_idx = lst_use_node_->idx_prev_;
        }

        return expire_num;
    }

    //希望淘汰掉disuse_num个数据，
    //如果disuse_eaqul == ture，则删除和最后删除的那个优先级相等的所有元素
    //disuse_eaqul可以保证数据的整体淘汰，避免一个KEY的部分数据在内存，一部分不在
    size_t disuse(size_t disuse_num, bool disuse_eaqul)
    {
        //从尾部开始检查，
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t fact_del_num = 0;
        unsigned int disuse_priority = 0;

        for (size_t i = 0; i < disuse_num && list_idx != lru_hash_head_->num_of_node_; ++i)
        {
            size_t del_iter = list_idx;
            _washout_fun wash_fun;
            wash_fun(value_base_[del_iter]);
            ++fact_del_num;
            //
            iterator iter_tmp(del_iter, this);
            erase(iter_tmp);
            disuse_priority = priority_base_[list_idx];
            //
            //如果删除了，还是检查第一个
            list_idx = lst_use_node_->idx_prev_;
        }

        if ( true == disuse_eaqul && disuse_num > 0)
        {
            fact_del_num += expire(disuse_priority);
        }

        return fact_del_num;
    }

    //标注，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark_unique(const _key_type &key, unsigned int priority)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                return true;
            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //根据value将优先级跟新，重新给一个数据打一个优先级标签，同时将值替换，
    bool mark_value(const _value_type &val, unsigned int priority )
    {
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key = get_key(val);


        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                return true;
            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //标注所有相等的数据，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark_equal(const _key_type &key, unsigned int priority)
    {
        size_t mark_count = 0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
            }
            else
            {
                //如果已经有数据，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (mark_count > 0)
                {
                    break;
                }
            }

            first = hash_index_base_[ first ];

        }

        return mark_count;
    }

    //标注所有相等的数据。这个函数有点别扭。我不喜欢，，因为VALUE没有（没法）跟新
    //size_t mark_value_equal(const _value_type &val, unsigned int priority )
    //{
    //    return mark(_extract_key(val),priority);
    //}


    //淘汰优先级过低的数据,LIST中间的数据是是乱序的也可以.只淘汰num_wash数量的数据
    //但限制淘汰是从头部开始，感觉不是太好。
    void washout(unsigned int wash_priority, size_t num_wash)
    {
        size_t list_idx = lst_use_node_->idx_next_;
        size_t num_del = 0;

        //不为NULL，而且删除的个数没有达到，从头部开始是否好呢?我不确定，打算在提供一个函数
        while ( list_idx != lru_hash_head_->num_of_node_ && num_del < num_wash)
        {
            //如果优先级小于淘汰系数
            if (priority_base_[list_idx] < wash_priority)
            {
                ++num_del;
                size_t del_iter = list_idx;
                list_idx = lst_index_base_[list_idx].idx_next_;
                //
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
            else
            {
                continue;
            }
        }
    }
};



/************************************************************************************************************
template           : shm_hashset_expire
************************************************************************************************************/
template < class _value_type,
         class _hash_fun = smem_hash<_value_type>,
         class _equal_key = std::equal_to<_value_type>,
         class _washout_fun = _default_washout_fun<_value_type> >
class shm_hashset_expire :
    public shm_hashtable_expire < _value_type,
    _value_type ,
    _hash_fun,
    smem_identity<_value_type>,
    _equal_key,
    _washout_fun >
{

protected:

    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_hashset_expire<_value_type, _hash_fun, _equal_key, _washout_fun >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable_expire<_value_type, _value_type , _hash_fun, smem_identity<_value_type>, _equal_key, _washout_fun>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashset_expire<_value_type, _hash_fun, _equal_key, _washout_fun>()
    {
    }

public:
    static shm_hashset_expire< _value_type, _hash_fun, _equal_key , _washout_fun >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashset_expire< _value_type, _hash_fun, _equal_key , _washout_fun >*>(
                   shm_hashtable_expire<_value_type, _value_type , _hash_fun, smem_identity<_value_type>, _equal_key , _washout_fun>::initialize(numnode, pmmap, if_restore));
    }
};



/************************************************************************************************************
template           : smem_hashmap_expire
************************************************************************************************************/
template < class _key_type,
         class _value_type,
         class _hash_fun = smem_hash<_key_type>,
         class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >,
         class _equal_key = std::equal_to<_key_type>,
         class _washout_fun = _default_washout_fun<_value_type> >
class smem_hashmap_expire :
    public shm_hashtable_expire < std::pair <_key_type, _value_type> ,
    _key_type,
    _hash_fun ,
    _extract_key,
    _equal_key ,
    _washout_fun >
{


protected:

    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    smem_hashmap_expire<_key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable_expire< std::pair <_key_type, _value_type> , _key_type, _extract_key, _equal_key, _washout_fun >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~smem_hashmap_expire<_key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >()
    {
    }

public:
    static smem_hashmap_expire< _key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun  >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<smem_hashmap_expire< _key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >*>(
                   shm_hashtable_expire< std::pair <_key_type, _value_type>, _key_type , _hash_fun, _extract_key, _equal_key, _washout_fun >::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，
    _value_type &operator[](const _key_type &key)
    {
        return (find(key)).second;
    }
};

};




#endif //#ifndef ZCE_SHM_LRU_HASH_TABLE_H_ 


