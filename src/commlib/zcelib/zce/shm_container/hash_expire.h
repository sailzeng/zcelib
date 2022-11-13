/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/hash_expire.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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

#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
template < class T, class Key, class Hash, class Extract, class keyEqual, class Washout > class shm_expire_hashtable;

//LRU HASH 迭代器
template < class T,
    class Key,
    class Hash,
    class Extract,
    class keyEqual,
    class Washout >
class _ht_expire_iterator
{
protected:
    //HASH TABLE的定义
    typedef shm_expire_hashtable <T, Key, Hash, Extract, keyEqual, Washout> expire_hashtable_t;
    //定义迭代器
    typedef _ht_expire_iterator < T, Key, Hash, Extract, keyEqual, Washout> iterator;

public:
    typedef shmc_size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    //迭代器萃取器所有的东东
    typedef std::bidirectional_iterator_tag iterator_category;
public:
    _ht_expire_iterator() = default;
    ~_ht_expire_iterator() = default;

    _ht_expire_iterator(size_type serial, expire_hashtable_t* e_ht_inst) :
        serial_(serial),
        expire_ht_inst_(e_ht_inst)
    {
    }

    T& operator*() const
    {
        return expire_ht_inst_->value_base_[serial_];
    }

    T* operator->() const
    {
        return expire_ht_inst_->value_base_ + serial_;
    }

    //本来只提供前向迭代器，曾经以为使用可以使用LIST保证迭代的高效，发现不行，
    //可能要提供另外的函数
    //前向迭代器
    iterator& operator++()
    {
        size_type oldseq = serial_;
        serial_ = *(expire_ht_inst_->hash_index_base_ + serial_);

        //如果这个节点是末位的节点
        if (serial_ == zce::SHMC_INVALID_POINT)
        {
            //顺着Index查询.
            size_type bucket = expire_ht_inst_->bkt_num_value(*(expire_ht_inst_->value_base_ + oldseq));

            //
            while (serial_ == zce::SHMC_INVALID_POINT && ++bucket < expire_ht_inst_->capacity())
            {
                serial_ = *(expire_ht_inst_->hash_factor_base_ + bucket);
            }
        }

        return *this;
    }

    //这个函数类似++，但是只在Hash数据的链表上游荡，所以性能更好
    //如果下一个数据是一样的KEY，那么就成为下个数据的迭代器。
    //否则成为end
    //其实也就比你自己做快一点点。
    iterator& goto_next_equal()
    {
        size_type oldseq = serial_;
        serial_ = *(expire_ht_inst_->hash_index_base_ + serial_);

        //如果这个节点不是末位的节点
        if (serial_ != zce::SHMC_INVALID_POINT)
        {
            Extract get_key;
            keyEqual   equal_key;

            if (false == equal_key(get_key(*(expire_ht_inst_->value_base_ + oldseq)),
                get_key(*(expire_ht_inst_->value_base_ + serial_))))
            {
                serial_ = zce::SHMC_INVALID_POINT;
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
        ++* this;
        return tmp;
    }
    //
    bool operator==(const iterator& it) const
    {
        if (expire_ht_inst_ == it.expire_ht_inst_ &&
            serial_ == it.serial_)
        {
            return true;
        }

        return false;
    }
    //
    bool operator!=(const iterator& it) const
    {
        return !(*this == it);
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_type getserial() const
    {
        return serial_;
    }
protected:
    //序列号
    size_type              serial_ = zce::SHMC_INVALID_POINT;
    //
    expire_hashtable_t* expire_ht_inst_ = nullptr;
};

/*!
@brief      代超时处理的在一块内存（共享内存）使用的Hash table，

@tparam     _value_type
@tparam     _key_type
@tparam     Hash
@tparam     Extract
@tparam     keyEqual
@tparam     Washout
@note
*/
template < class T,
    class Key,
    class Hash = shm_hash<Key>,
    class Extract = shm_identity<T>,
    class keyEqual = std::equal_to<Key>,
    class Washout = default_washout<T> >
class shm_expire_hashtable
{
    friend class _ht_expire_iterator < T, Key, Hash, Extract, keyEqual, Washout >;
private:
    //定义自己
    typedef shm_expire_hashtable < T, Key, Hash, Extract, keyEqual, Washout > self;
public:
    //定义迭代器
    typedef _ht_expire_iterator < T, Key, Hash, Extract, keyEqual, Washout > iterator;
    typedef const iterator const_iterator;
    typedef iterator::iterator_category iterator_category;
    typedef T value_type;
    typedef Key key_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef shmc_size_type size_type;

public:
    //protected 构造函数，避免你用
    shm_expire_hashtable() = default;
    //只定义,不实现,避免犯错
    const self& operator=(const self& others) = delete;
    shm_expire_hashtable(const shm_expire_hashtable&) = delete;
    ~shm_expire_hashtable()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }

protected:
    //头部，LRU_HASH的头部结构，放在LRUHASH内存的前面
    class _expire_ht_head
    {
    protected:
        _expire_ht_head() = default;
        ~_expire_ht_head() = default;

    public:
        //内存区的长度
        std::size_t         size_of_mem_ = 0;

        //NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
        size_type           num_of_node_ = 0;

        //FREE的NODE个数
        size_type           sz_freenode_ = 0;
        //USE的NODE个数
        size_type           sz_usenode_ = 0;

        //使用的INDEX个数,可以了解实际开链的负载比率
        size_type           sz_useindex_ = 0;
    };
public:

    /*!
    * @brief      内存区的构成为 define区,index区,data区,返回所需要的长度,
    * @return     size_type   所需的尺寸
    * @param      req_num  请求的NODE数量
    * @param      real_num 实际分配的NODE数量
    * @note       注意返回的是实际INDEX长度,会取一个质数
    */
    static std::size_t alloc_size(size_type req_num, size_type& real_num)
    {
        //取得一个比这个数字做一定放大的质数，
        zce::hash_prime(req_num, real_num);
        std::size_t sz_alloc = 0;
        //
        sz_alloc += sizeof(_expire_ht_head);
        sz_alloc += sizeof(size_type) * real_num;
        sz_alloc += sizeof(size_type) * real_num;
        //
        sz_alloc += sizeof(_shm_list_index) * (real_num + LIST_ADD_NODE_NUMBER);
        sz_alloc += sizeof(uint32_t) * (real_num);
        sz_alloc += sizeof(T) * (real_num);
        return sz_alloc;
    }

    //初始化
    bool initialize(size_type req_num,
                    size_type& real_num,
                    char* mem_addr,
                    bool if_restore = false)
    {
        assert(mem_addr != nullptr && req_num > 0);
        //调整
        std::size_t sz_mmap = alloc_size(req_num, real_num);
        _expire_ht_head* hashhead = reinterpret_cast<_expire_ht_head*>(mem_addr);

        //如果是恢复,数据都在内存中,
        if (true == if_restore)
        {
            //检查一下恢复的内存是否正确,
            //在特殊的恢复，比如取质数的方法被改变了，
            if (sz_mmap != hashhead->size_of_mem_ ||
                real_num != hashhead->num_of_node_)
            {
                //一般情况下不一致返回nullptr，标识恢复失败，
                return false;
            }
        }
        else
        {
            //记录初始化尺寸
            hashhead->size_of_mem_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }

        mem_addr_ = mem_addr;
        char* tmp_base = mem_addr_;
        lru_hash_head_ = reinterpret_cast<_expire_ht_head*>(tmp_base);
        tmp_base = tmp_base + sizeof(_expire_ht_head);
        hash_factor_base_ = reinterpret_cast<size_type*>(tmp_base);
        tmp_base = tmp_base + sizeof(size_type) * real_num;
        hash_index_base_ = reinterpret_cast<size_type*>(tmp_base);

        tmp_base = tmp_base + sizeof(size_type) * real_num;
        lst_index_base_ = reinterpret_cast<_shm_list_index*>(tmp_base);
        tmp_base = tmp_base + sizeof(_shm_list_index) * (real_num + LIST_ADD_NODE_NUMBER);
        lst_use_node_ = lst_index_base_ + real_num;
        lst_free_node_ = lst_index_base_ + real_num + 1;

        priority_base_ = reinterpret_cast<uint32_t*>(tmp_base);
        tmp_base = tmp_base + sizeof(uint32_t) * (real_num);
        value_base_ = reinterpret_cast<T*>(tmp_base);

        if (if_restore)
        {
            //清理初始化所有的内存,所有的节点为FREE
            clear();
        }
        //其实如果是恢复，还应该检查一次所有的队列

        //打完收工
        return true;
    }

    //!初始化，自己内部分配内存，
    bool initialize(size_type req_num,
                    size_type& real_num)
    {
        std::size_t sz_alloc = alloc_size(req_num, real_num);
        //自己分配一个空间，自己使用
        char *mem_addr = new char[sz_alloc];
        slef_alloc_ = true;
        return initialize(req_num, real_num, mem_addr, false);
    }

    //!销毁，析构所有的已有元素，注意，如果想恢复，不要调用这个函数
    void terminate()
    {
        iterator iter_tmp = begin();
        iterator iter_end = end();
        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            size_type pos = iter_tmp.getserial();
            (value_base_ + pos)->~T();
        }
        clear();
        if (slef_alloc_)
        {
            delete[] mem_addr_;
            mem_addr_ = nullptr;
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
            new (value_base_ + pos) T(std::move(val));
        }
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        lru_hash_head_->sz_freenode_ = lru_hash_head_->num_of_node_;
        lru_hash_head_->sz_usenode_ = 0;
        lru_hash_head_->sz_useindex_ = 0;

        //将两个队列都清理为nullptr,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        lst_use_node_->idx_next_ = lru_hash_head_->num_of_node_;
        lst_use_node_->idx_prev_ = lru_hash_head_->num_of_node_;

        lst_free_node_->idx_next_ = lru_hash_head_->num_of_node_ + 1;
        lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ + 1;

        _shm_list_index* pindex = lst_index_base_;

        //初始化free数据区
        for (size_type i = 0; i < lru_hash_head_->num_of_node_; ++i)
        {
            //
            hash_factor_base_[i] = SHMC_INVALID_POINT;
            hash_index_base_[i] = SHMC_INVALID_POINT;
            priority_base_[i] = 0;

            pindex->idx_next_ = (i + 1);
            pindex->idx_prev_ = (i - 1);

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
    template<typename U>
    size_type create_node(U&& v, uint32_t priority)
    {
        //如果没有空间可以分配
        if (lru_hash_head_->sz_freenode_ == 0)
        {
            return SHMC_INVALID_POINT;
        }

        //从链上取1个下来
        size_type newnode = lst_free_node_->idx_next_;

        lst_free_node_->idx_next_ = (lst_index_base_ + newnode)->idx_next_;
        //lst_free_node_->idx_next_已经向后调整一个位置了
        (lst_index_base_ + lst_free_node_->idx_next_)->idx_prev_ = (lst_index_base_ + newnode)->idx_prev_;

        //注意num_of_node_的位置是usenode
        lst_index_base_[newnode].idx_next_ = lst_use_node_->idx_next_;
        lst_index_base_[newnode].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
        lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = newnode;
        lst_use_node_->idx_next_ = newnode;

        //placement new记录数据和优先级
        new (value_base_ + newnode)  T(std::forward<U>(v));
        priority_base_[newnode] = priority;

        lru_hash_head_->sz_usenode_++;
        lru_hash_head_->sz_freenode_--;

        //检查你干错事情么没有
        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
        return newnode;
    }

    //释放一个NODE,将其归还给FREELIST,单向链表就是简单
    void destroy_node(size_type pos)
    {
        size_type freenext = lst_free_node_->idx_next_;

        size_type pos_next = lst_index_base_[pos].idx_next_;
        size_type pos_prev = lst_index_base_[pos].idx_prev_;
        lst_index_base_[pos_next].idx_prev_ = pos_prev;
        lst_index_base_[pos_prev].idx_next_ = pos_next;

        (lst_index_base_ + pos)->idx_next_ = freenext;
        (lst_index_base_ + pos)->idx_prev_ = (lst_index_base_ + freenext)->idx_prev_;

        lst_free_node_->idx_next_ = pos;

        (lst_index_base_ + freenext)->idx_prev_ = pos;
        lru_hash_head_->sz_usenode_--;
        lru_hash_head_->sz_freenode_++;

        hash_index_base_[pos] = SHMC_INVALID_POINT;

        //调用显式的析构函数
        (value_base_ + pos)->~T();
        priority_base_[pos] = 0;

        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }

    template<typename U>
    std::pair<iterator, bool> insert_unique_i(U&& v,
                                              uint32_t priority)
    {
        size_type idx = bkt_num_value(std::forward<U>(v));
        size_type first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key函数
            if (equal_key((get_key(value_base_[first])),
                (get_key(std::forward<U>(v)))) == true)
            {
                return std::pair<iterator, bool>(iterator(first, this), false);
            }

            first = hash_index_base_[first];
        }

        //没有找到,插入新数据
        size_type newnode = create_node(std::forward<U>(v), priority);

        //空间不足,
        if (newnode == SHMC_INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(SHMC_INVALID_POINT, this), false);
        }

        //放入链表中
        hash_index_base_[newnode] = hash_factor_base_[idx];
        hash_factor_base_[idx] = newnode;

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    //! 插入节点,允许相等
    //! 优先级可以，传递入当前时间作为参数，
    template<typename U>
    std::pair<iterator, bool> insert_equal_i(U&& v,
                                             uint32_t priority)
    {
        size_type idx = bkt_num_value(std::forward<U>(v));
        size_type first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key函数
            if (equal_key((get_key(value_base_[first])), (get_key(std::forward<U>(v)))) == true)
            {
                break;
            }

            first = hash_index_base_[first];
        }

        //没有找到,插入新数据
        size_type newnode = create_node(std::forward<U>(v), priority);

        //空间不足,
        if (newnode == SHMC_INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(SHMC_INVALID_POINT, this), false);
        }

        //没有找到相同KEY的数据
        if (first == SHMC_INVALID_POINT)
        {
            //放入链表的首部就可以了
            hash_index_base_[newnode] = hash_factor_base_[idx];
            hash_factor_base_[idx] = newnode;
        }
        //如果找到了相同的KEY节点
        else
        {
            //放到这个节点的后面
            hash_index_base_[newnode] = hash_index_base_[first];
            hash_index_base_[first] = newnode;
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    //从value中取值
    size_type bkt_num_value(const T& obj) const
    {
        Extract get_key;
        return static_cast<size_type>(bkt_num_key(get_key(obj)));
    }

    //为什么不能重载上面的函数,自己考虑一下,
    //重载的话，如果_value_type和_key_type一样，就等着哭吧 ---inmore
    size_type bkt_num_key(const Key& key) const
    {
        Hash hash_fun;
        return static_cast<size_type>(hash_fun(key) % lru_hash_head_->num_of_node_);
    }

public:

    //得到开始的迭代器的位置
    iterator begin()
    {
        for (size_type i = 0; i < lru_hash_head_->num_of_node_; ++i)
        {
            if (*(hash_factor_base_ + i) != SHMC_INVALID_POINT)
            {
                return iterator(*(hash_factor_base_ + i), this);
            }
        }
        return end();
    }

    //得到结束位置
    iterator end()
    {
        return iterator(SHMC_INVALID_POINT, this);
    }
    //当前使用的节点数量
    size_type size() const
    {
        return lru_hash_head_->sz_usenode_;
    }
    //得到容量
    size_type capacity() const
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
    * @param      v        插入的节点
    * @param      priority 优先级可以，传递入当前时间作为参数，淘汰时用小于某个值
    *                      都淘汰的方法处理,所以要保证后面传入数据值更大  我为什
    *                      么不直接用time(nullptr),是给你更大的灵活性,
    * @note       这儿会将插入的数据放在最后淘汰的地方
    */
    std::pair<iterator, bool> insert_unique(const T& v,
                                            uint32_t priority)
    {
        return insert_unique_i(v, priority);
    }
    std::pair<iterator, bool> insert_unique(T&& v,
                                            uint32_t priority)
    {
        return insert_unique_i(v, priority);
    }

    //插入节点,允许相等
    //优先级可以，传递入当前时间作为参数，
    std::pair<iterator, bool> insert_equal(const T& val,
                                           uint32_t priority)
    {
        return insert_equal_i(val, priority);
    }
    std::pair<iterator, bool> insert_equal(T&& val,
                                           uint32_t priority)
    {
        return insert_equal_i(val, priority);
    }
    //查询相应的Key是否有,返回迭代器
    iterator find(const Key& key)
    {
        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        while (first != SHMC_INVALID_POINT && !equal_key(get_key(value_base_[first]), key))
        {
            first = hash_index_base_[first];
        }

        return iterator(first, this);
    }

    //
    iterator find_value(const T& val)
    {
        Extract get_key;
        return find(get_key(val));
    }

    //得到某个KEY的元素个数，有点相当于查询操作
    size_type count(const Key& key)
    {
        size_type equal_count = 0;
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                ++equal_count;
            }

            first = hash_index_base_[first];
        }

        return equal_count;
    }

    //得到某个VALUE的元素个数，有点相当于查询操作
    size_type count_value(const T& val)
    {
        Extract get_key;
        return count(get_key(val));
    }

    /*!
    * @brief
    * @return     bool
    * @param      key
    */
    bool erase_unique(const Key& key)
    {
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = hash_factor_base_[idx];
        size_type prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                if (first == hash_factor_base_[idx])
                {
                    hash_factor_base_[idx] = hash_index_base_[first];
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
            first = hash_index_base_[first];
        }
        return false;
    }

    /*!
    * @brief      使用迭代器删除,尽量高效所以不用简化写法
    * @return     bool 返回值
    * @param      it   删除的迭代器
    */
    bool erase(const iterator& it)
    {
        Extract get_key;
        size_type idx = bkt_num_key(get_key(*it));
        size_type first = hash_factor_base_[idx];
        size_type prev = first;
        size_type itseq = it.getserial();

        //
        while (first != SHMC_INVALID_POINT)
        {
            if (first == itseq)
            {
                if (first == hash_factor_base_[idx])
                {
                    hash_factor_base_[idx] = hash_index_base_[first];
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
            first = hash_index_base_[first];
        }
        return false;
    }

    //删除某个值
    bool erase_unique_value(const T& val)
    {
        Extract get_key;
        return erase_unique(get_key(val));
    }

    /*!
    * @brief      删除所有相等的KEY的数据,和insert_equal配对使用，返回删除了几个数据
    * @return     size_type
    * @param      key
    * @note
    */
    size_type erase_equal(const Key& key)
    {
        size_type erase_count = 0;
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = hash_factor_base_[idx];
        size_type prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //循环查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                if (first == hash_factor_base_[idx])
                {
                    hash_factor_base_[idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //删除的情况下prev不用调整，first向后移动
                size_type del_pos = first;
                first = hash_index_base_[first];
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
                first = hash_index_base_[first];
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
    bool active_unique(const Key& key,
                       uint32_t priority /*=static_cast<uint32_t>(time(nullptr))*/)
    {
        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                priority_base_[first] = priority;

                size_type first_prv = lst_index_base_[first].idx_prev_;
                size_type first_nxt = lst_index_base_[first].idx_next_;
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

            first = hash_index_base_[first];
        }

        return false;
    }

    /*!
    * @brief      通过VALUE激活，同时讲值替换成最新的数据VALUE，
    *             优先级参数可以使用当前的时间，MAP使用，
    * @return     bool     是否激活成功
    * @param      v      值
    * @param      priority 优先级
    * @note       LRU中如果，一个值被使用后，可以认为是激活过一次，
    */
    bool active_unique_value(const T& val,
                             uint32_t priority /*=static_cast<uint32_t>(time(nullptr))*/)
    {
        Extract get_key;
        keyEqual   equal_key;

        Key key = get_key(val);
        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                size_type first_prv = lst_index_base_[first].idx_prev_;
                size_type first_nxt = lst_index_base_[first].idx_next_;
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

            first = hash_index_base_[first];
        }
        return false;
    }

    //激活所有相同的KEY,将激活的数据挂到LIST的最开始,淘汰使用expire
    size_type active_equal(const Key& key,
                           uint32_t priority /*=static_cast<uint32_t>(time(nullptr))*/)
    {
        size_type active_count = 0;

        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                priority_base_[first] = priority;

                size_type first_prv = lst_index_base_[first].idx_prev_;
                size_type first_nxt = lst_index_base_[first].idx_next_;
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
            first = hash_index_base_[first];
        }
        return active_count;
    }

    //淘汰过期的数据,假设LIST中间的数据是按照过期实际排序的，这要求你传入的优先级最好是时间
    //小于等于这个优先级的数据将被淘汰
    size_type expire(uint32_t expire_time)
    {
        //从尾部开始检查，
        size_type list_idx = lst_use_node_->idx_prev_;
        size_type expire_num = 0;

        while (list_idx != lru_hash_head_->num_of_node_)
        {
            //小于等于
            if (priority_base_[list_idx] <= expire_time)
            {
                size_type del_iter = list_idx;
                Washout wash_fun;
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
    size_type disuse(size_type disuse_num, bool disuse_eaqul)
    {
        //从尾部开始检查，
        size_type list_idx = lst_use_node_->idx_prev_;
        size_type fact_del_num = 0;
        uint32_t disuse_priority = 0;

        for (size_type i = 0; i < disuse_num && list_idx != lru_hash_head_->num_of_node_; ++i)
        {
            size_type del_iter = list_idx;
            Washout wash_fun;
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

        if (true == disuse_eaqul && disuse_num > 0)
        {
            fact_del_num += expire(disuse_priority);
        }

        return fact_del_num;
    }

    //标注，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark_unique(const Key& key, uint32_t priority)
    {
        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                priority_base_[first] = priority;

                return true;
            }

            first = hash_index_base_[first];
        }

        return false;
    }

    //根据value将优先级跟新，重新给一个数据打一个优先级标签，同时将值替换，
    bool mark_value(const T& val, uint32_t priority)
    {
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        Key key = get_key(val);

        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                return true;
            }
            first = hash_index_base_[first];
        }
        return false;
    }

    //标注所有相等的数据，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark_equal(const Key& key, uint32_t priority)
    {
        size_type mark_count = 0;

        size_type idx = bkt_num_key(key);
        size_type first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //在列表中间查询
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(value_base_[first]), key) == true)
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

            first = hash_index_base_[first];
        }

        return mark_count;
    }

    //标注所有相等的数据。这个函数有点别扭。我不喜欢，，因为VALUE没有（没法）跟新
    //size_type mark_value_equal(const _value_type &v, uint32_t priority )
    //{
    //    return mark(Extract(v),priority);
    //}

    //淘汰优先级过低的数据,LIST中间的数据是是乱序的也可以.只淘汰num_wash数量的数据
    //但限制淘汰是从头部开始，感觉不是太好。
    void washout(uint32_t wash_priority, size_type num_wash)
    {
        size_type list_idx = lst_use_node_->idx_next_;
        size_type num_del = 0;

        //不为nullptr，而且删除的个数没有达到，从头部开始是否好呢?我不确定，打算在提供一个函数
        while (list_idx != lru_hash_head_->num_of_node_ && num_del < num_wash)
        {
            //如果优先级小于淘汰系数
            if (priority_base_[list_idx] < wash_priority)
            {
                ++num_del;
                size_type del_iter = list_idx;
                list_idx = lst_index_base_[list_idx].idx_next_;
                //
                Washout wash_fun;
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

protected:
    //
    static const size_type  LIST_ADD_NODE_NUMBER = 2;

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;
    //头部指针
    _expire_ht_head* lru_hash_head_ = nullptr;

    //Hash因子的BASE
    size_type* hash_factor_base_ = nullptr;
    //Hash的索引,hash链的索引,注释写得不清楚，自己都记不得了.
    size_type* hash_index_base_;

    //LIST的索引
    _shm_list_index* lst_index_base_ = nullptr;
    //FREE节点链表的开始
    _shm_list_index* lst_free_node_ = nullptr;
    //USE节点链表的开始
    _shm_list_index* lst_use_node_ = nullptr;

    //优先级的数据指针,用32位的数据保存优先级
    uint32_t* priority_base_ = nullptr;
    //数据区指针
    T* value_base_ = nullptr;
};

/************************************************************************************************************
template           : shm_expire_hashset
************************************************************************************************************/
template < class T,
    class Hash = std::hash<T>,
    class keyEqual = std::equal_to<T>,
    class Washout = default_washout<T> >
class shm_expire_hashset :
    public shm_expire_hashtable < T,
    T,
    Hash,
    shm_identity<T>,
    keyEqual,
    Washout >
{
private:
    typedef shm_expire_hashset<T, Hash, keyEqual, Washout> self;
    typedef shm_expire_hashtable<T, T, Hash, shm_identity<T>, keyEqual, Washout> shm_expire_hashtable_t;
    typedef shmc_size_type size_type;
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_expire_hashset() = default;
    shm_expire_hashset(const shm_expire_hashset& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_expire_hashset() = default;

public:
    static self*
        initialize(size_type num_node,
                   size_type& real_num,
                   char* mem_addr,
                   bool if_restore = false)
    {
        return reinterpret_cast<self*>(shm_expire_hashtable_t::initialize(
            num_node,
            real_num,
            mem_addr,
            if_restore));
    }
};

/************************************************************************************************************
template           : shm_expire_hashmap
************************************************************************************************************/
template < class Key,
    class T,
    class Hash = std::hash<Key>,
    class Extract = shm_select1st <std::pair <Key, T> >,
    class keyEqual = std::equal_to<Key>,
    class Washout = default_washout<T> >
class shm_expire_hashmap :
    public shm_expire_hashtable < std::pair <Key, T>,
    Key,
    Hash,
    Extract,
    keyEqual,
    Washout >
{
private:
    typedef shm_expire_hashmap<Key, T, Hash, Extract, keyEqual, Washout > self;
    typedef shm_expire_hashtable< std::pair <Key, T>, Key, Hash, Extract, keyEqual, Washout > shm_expire_hashtable_t;
    typedef shmc_size_type size_type;
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化

    //这个函数,不给你用,就是不给你用
    shm_expire_hashmap() = default;

    shm_expire_hashmap(const shm_expire_hashmap& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_expire_hashmap() = default;

public:
    static self* initialize(size_type num_node,
                            size_type& real_num,
                            char* mem_addr,
                            bool if_restore = false)
    {
        return reinterpret_cast<self*>(
            shm_expire_hashtable_t::initialize(
            num_node,
            real_num,
            mem_addr,
            if_restore));
    }
    //[]操作符号有优点和缺点，
    T& operator[](const Key& key)
    {
        return (find(key)).second;
    }
};

template < class T, class Key, class Hash, class keyEqual, class Washout >
using static_expire_hashtable = zce::shm_expire_hashtable<T, Key, Hash, keyEqual, Washout>;

template<class T, class Hash, class keyEqual, class Washout >
using static_expire_hashset = zce::shm_expire_hashset<T, Hash, keyEqual, Washout>;

template<class Key, class T, class Hash, class keyEqual, class Washout>
using static_expire_hashmap = zce::shm_expire_hashmap<Key, T, Hash, keyEqual, Washout>;
};
