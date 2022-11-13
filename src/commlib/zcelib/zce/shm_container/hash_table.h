/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/hash_table.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月26日
* @brief      在共享内存中使用的hashtable
*             为了简化偷懒，我们设计的Hashtable，可以存放相同的key，只要你调用
*             insert_equal 这类函数就可以了，而不是STL那样，用另外一个类去封装
*
* @details    STL的HASHTABLE太TMD的经典!,从算法到封装,特别是封装，简直让我流口水!.
*
*             2010年4月，
*             重新整理了这个类，和后来一些封装思路保持一致，去掉了几个外部包装的封装
*             去掉了_shm_hashtable_node ,_shm_hashtable_index，可以保持代码
*             一致，另外，从性能的角度讲，少一次拷贝转换？
*
*/

#pragma once

#include "zce/shm_container/common.h"

//原来有这两个结构，后来删除，原因有2，
//其他后写的代码没有用类似的方法，
//写恢复的时候如果外面有一层结构要麻烦一点，

//hashtable的节点,去掉的原因是避免多次拷贝构造，
//template <class _value_type> class _shm_hashtable_node
//{
//public:
//    //NODE链表的指针,
//    size_type        next_;
//    //ValueType数据
//    _value_type   hash_data_;
//

namespace zce
{
template < class T,
    class Key,
    class Hash,
    class Extract,
    class keyEqual > class shm_hashtable;

/*!
* @brief      hash_table的迭代器.
*             只提供了单向迭代器，++，
*             模版参数我就不解释，反正和hash_table一致，你看下面的说明把。
*/
template <class T, class Key, class Hash, class Extract, class keyEqual>
class _shm_ht_iterator
{
public:
    typedef shmc_size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    //迭代器萃取器所有的东东
    typedef std::bidirectional_iterator_tag iterator_category;
private:
    ///迭代器定义，方便下面的使用
    typedef _shm_ht_iterator<T, Key, Hash, Extract, keyEqual> iterator;
    ///hash_type定义，方便使用
    typedef shm_hashtable<T, Key, Hash, Extract, keyEqual> shm_hashtable_t;

public:
    ///默认构造函数
    _shm_ht_iterator() = default;
    ~_shm_ht_iterator() = default;
    ///构造函数
    _shm_ht_iterator(size_type seq, shm_hashtable_t* instance) :
        serial_(seq),
        ht_instance_(instance)
    {
    }

    ///保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_type getserial() const
    {
        return serial_;
    }

    ///提供这个方法事为了加快很多函数的速度,
    size_type getnext()
    {
        return *(ht_instance_->next_index_ + serial_);
    }

    ///
    bool operator==(const iterator& x) const
    {
        return (serial_ == x.serial_ && ht_instance_ == x.ht_instance_);
    }
    ///
    bool operator!=(const iterator& x) const
    {
        return !(*this == x);
    }

    ///提领操作
    T& operator*() const
    {
        return *(ht_instance_->data_base_ + serial_);
    }
    ///在多线程的环境下提供这个运送符号是不安全的,我没有加锁,原因如说明
    T* operator->() const
    {
        //
        return &(operator*());
    }

    ///单向迭代器
    iterator& operator++()
    {
        size_type oldseq = serial_;
        serial_ = *(ht_instance_->next_index_ + serial_);

        //如果这个节点是末位的节点
        if (serial_ == zce::SHMC_INVALID_POINT)
        {
            //顺着Index查询.
            size_type bucket = ht_instance_->bkt_num_value(*(ht_instance_->data_base_ + oldseq));

            while (serial_ == zce::SHMC_INVALID_POINT && ++bucket < ht_instance_->capacity())
            {
                serial_ = *(ht_instance_->index_base_ + bucket);
            }
        }

        return *this;
    }

    ///iter++,
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

protected:
    ///序列号
    size_type           serial_ = zce::SHMC_INVALID_POINT;
    ///HashTable的指针,
    shm_hashtable_t* ht_instance_ = nullptr;
};

/*!
* @brief      HashTABLE的定义,看完HashTable,才发现封装太太经典,爱死STL了,向那些聪明的脑瓜致敬
*             另外,最大负载系数我设计为1,本来想搞大一些,但是考虑STL的hasttable在复杂达到1后会调整,
*             这儿的参数顺序我不喜欢,但遵照STL的顺序把
* @tparam     T        节点的实际值型别
* @tparam     Key      节点的键的型别
* @tparam     Hash     Key的hash函数,得到Hash因子，std::hash满足大部分情况，
* @tparam     Extract  从节点中得到Key键值的方法,或者函数对象
* @tparam     keyEqual 比较Key键值是否相等的方法,或者函数对象
* @note
*/
template < class T,
    class Key,
    class Hash = std::hash<Key>,
    class Extract = shm_identity<T>,
    class keyEqual = std::equal_to<Key> >
class shm_hashtable
{
    //迭代器作为自己友元
    friend class _shm_ht_iterator<T, Key, Hash, Extract, keyEqual>;
private:
    //定义自己
    typedef shm_hashtable<T, Key, Hash, Extract, keyEqual> self;
public:
    //定义迭代器
    typedef _shm_ht_iterator<T, Key, Hash, Extract, keyEqual> iterator;
    typedef const iterator const_iterator;
    typedef iterator::iterator_category iterator_category;
    typedef T value_type;
    typedef Key key_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef shmc_size_type size_type;

public:
    shm_hashtable() = default;
    //只定义,不实现,
    shm_hashtable(const shm_hashtable&) = delete;
    const self& operator=(const self& others) = delete;
    //析构
    ~shm_hashtable()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }

protected:

    /*!
    * @brief _shm_ht_head的头部数据区的结构
    *        记录大小，使用情况等
    */
    class _shm_ht_head
    {
    protected:
        //默认的构造函数
        _shm_ht_head() = default;
        ~_shm_ht_head() = default;

    public:
        ///内存区的长度,这个长度可能超过32bit
        std::size_t          size_of_mem_ = 0;

        ///NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
        size_type            num_of_node_ = 0;

        ///FREE的NODE个数
        size_type            sz_freenode_ = 0;
        ///USE的NODE个数
        size_type            sz_usenode_ = 0;

        ///使用的INDEX个数,可以了解实际开链的负载比率
        size_type            sz_useindex_ = 0;

        ///FREE NODE的头指针
        size_type             free_headnode_ = 0;
    };

    //分配一个NODE,将其从FREELIST中取出
    template<typename U>
    size_type create_node(U&& v)
    {
        //如果没有空间可以分配
        if (hash_head_->sz_freenode_ == 0)
        {
            return SHMC_INVALID_POINT;
        }

        //从连上取下一个空闲节点
        size_type new_node = hash_head_->free_headnode_;
        hash_head_->free_headnode_ = *(next_index_ + new_node);
        --hash_head_->sz_freenode_;
        ++hash_head_->sz_usenode_;

        //用placement new进行赋值
        new (data_base_ + new_node)T(std::forward<U>(v));

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST,单向链表就是简单
    void destroy_node(size_type pos)
    {
        size_type freenext = hash_head_->free_headnode_;
        *(next_index_ + pos) = freenext;
        hash_head_->free_headnode_ = pos;

        ++hash_head_->sz_freenode_;
        --hash_head_->sz_usenode_;

        //调用显式的析构函数
        (data_base_ + pos)->~T();
    }

    template<typename U>
    std::pair<iterator, bool> insert_unique_i(U&& v)
    {
        size_type idx = bkt_num_value(std::forward<U>(v));
        size_type first_idx = *(index_base_ + idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        size_type nxt_idx = first_idx;

        while (nxt_idx != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key函数
            if (equal_key((get_key(*(data_base_ + nxt_idx))), (get_key(std::forward<U>(v)))) == true)
            {
                return std::pair<iterator, bool>(iterator(nxt_idx, this), false);
            }

            nxt_idx = *(next_index_ + nxt_idx);
        }

        //没有找到,插入新数据
        size_type newnode = create_node(std::forward<U>(v));
        //空间不足,
        if (newnode == SHMC_INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(SHMC_INVALID_POINT, this), false);
        }

        //放入链表中
        *(next_index_ + newnode) = *(index_base_ + idx);
        *(index_base_ + idx) = newnode;

        //如果第一个位置就不是无效的INDEX
        if (first_idx == SHMC_INVALID_POINT)
        {
            //记录使用量一个索引
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    template<typename U>
    std::pair<iterator, bool> insert_equal_i(U&& v)
    {
        size_type idx = bkt_num_value(std::forward<U>(v));
        size_type first_idx = *(index_base_ + idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        size_type nxt_idx = first_idx;

        while (nxt_idx != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key函数,会将相同的数据放在一起，便于处理
            if (equal_key((get_key(*(data_base_ + nxt_idx))), (get_key(std::forward<U>(v)))) == true)
            {
                break;
            }

            nxt_idx = *(next_index_ + nxt_idx);
        }

        //没有找到,插入新数据
        size_type newnode = create_node(std::forward<U>(v));

        //空间不足,
        if (newnode == SHMC_INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(SHMC_INVALID_POINT, this), false);
        }

        //没有找到相同KEY的数据
        if (nxt_idx == SHMC_INVALID_POINT)
        {
            //放入链表的首部就可以了
            (next_index_ + newnode) = *(index_base_ + idx);
            *(index_base_ + idx) = newnode;
        }
        //如果找到了相同的KEY节点
        else
        {
            //放到这个节点的后面
            *(next_index_ + newnode) = *(next_index_ + nxt_idx);
            *(next_index_ + nxt_idx) = newnode;
        }

        //如果第一个位置就不是无效的INDEX,记录使用了INDEX
        if (first_idx == SHMC_INVALID_POINT)
        {
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

public:

    /*!
    * @brief      内存区的构成为 define区,index区,data区,返回所需要的长度,
    * @return     size_type   所需的尺寸
    * @param      req_num  请求的NODE数量
    * @param      real_num 实际分配的NODE数量
    * @note       注意返回的是实际INDEX长度,会取一个质数
    */
    static std::size_t alloc_size(size_type req_num,
                                  size_type& real_num)
    {
        zce::hash_prime(req_num, real_num);
        return  sizeof(_shm_ht_head) +
            sizeof(size_type) * (real_num)+
            sizeof(T) * real_num +
            sizeof(size_type) * (real_num);
    }

    /*!
    * @brief      初始化
    * @return     self*
    * @param      req_num   请求的NODE数量
    * @param      real_num
    * @param      pmmap
    * @param      if_restore
    */
    bool initialize(size_type req_num,
                    size_type& real_num,
                    char* mem_addr,
                    bool if_restore = false)
    {
        ZCE_ASSERT(mem_addr != nullptr && req_num > 0);
        //调整
        std::size_t sz_mmap = alloc_size(req_num, real_num);
        _shm_ht_head* hashhead = reinterpret_cast<_shm_ht_head*>(mem_addr);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (sz_mmap != hashhead->size_of_mem_ ||
                real_num != hashhead->num_of_node_)
            {
                //一般情况下不一致返回nullptr，标识恢复失败，

                return false;
            }
        }
        else
        {
            //初始化尺寸
            hashhead->size_of_mem_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算

        //计算这些指针的地址，主要是为了方便后面的处理
        mem_addr_ = mem_addr;
        char* tmp_base = mem_addr_;

        hash_head_ = hashhead;
        tmp_base = tmp_base + sizeof(_shm_ht_head);

        index_base_ = reinterpret_cast<size_type*>(tmp_base);
        tmp_base = tmp_base + sizeof(size_type) * (real_num);

        data_base_ = reinterpret_cast<T*>(tmp_base);
        tmp_base = tmp_base + sizeof(T) * (real_num);

        next_index_ = reinterpret_cast<size_type*>(tmp_base);

        //根据是否恢复抉择处理方式
        if (if_restore)
        {
            restore();
        }
        else
        {
            //清理初始化所有的内存,所有的节点为FREE
            clear();
        }
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

    //!清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        hash_head_->sz_freenode_ = hash_head_->num_of_node_;
        hash_head_->sz_usenode_ = 0;
        hash_head_->sz_useindex_ = 0;

        //初始化free数据区
        for (size_type i = 0; i < hash_head_->num_of_node_; ++i)
        {
            index_base_[i] = SHMC_INVALID_POINT;
        }

        //清理FREELIST的单向NODE,
        hash_head_->free_headnode_ = 0;

        //
        for (size_type i = 0; i < hash_head_->num_of_node_; ++i)
        {
            next_index_[i] = i + 1;
            if (i == hash_head_->num_of_node_ - 1)
            {
                next_index_[i] = SHMC_INVALID_POINT;
            }
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
            mem_addr_ = nullptr;
        }
    }

    //从value中取值
    size_type bkt_num_value(const T& obj) const
    {
        Extract get_key;
        return static_cast<size_type>(bkt_num_key(get_key(obj)));
    }
    //为什么不能重载上面的函数,自己考虑一下,
    size_type bkt_num_key(const Key& key) const
    {
        Hash hash_fun;
        return static_cast<size_type>(hash_fun(key) % hash_head_->num_of_node_);
    }

    //HashTable的Begin 不是高效函数,不建议太多使用
    iterator begin()
    {
        for (size_type i = 0; i < hash_head_->num_of_node_; ++i)
        {
            if (*(index_base_ + i) != SHMC_INVALID_POINT)
            {
                return iterator(*(index_base_ + i), this);
            }
        }

        return end();
    }
    //用无效指针
    iterator end()
    {
        return iterator(SHMC_INVALID_POINT, this);
    }

    //
    bool empty()
    {
        if (hash_head_->sz_freenode_ == hash_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }
    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (hash_head_->sz_freenode_ == 0)
        {
            return true;
        }

        return false;
    }

    //查询相应的Key是否有
    iterator find(const Key& key)
    {
        size_type idx = bkt_num_key(key);
        size_type first = *(index_base_ + idx);
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //
        while (first != SHMC_INVALID_POINT && !equal_key(get_key(*(data_base_ + first)), key))
        {
            first = *(next_index_ + first);
        }

        return iterator(first, this);
    }
    //
    iterator find_value(const T& val)
    {
        Extract get_key;
        return find(get_key(val));
    }

    //
    T& find_or_insert(const T& val)
    {
        iterator iter = find_value(val);
        if (iter == end())
        {
            std::pair<iterator, bool> pair_iter = insert_unique(val);
            return (*(pair_iter.first));
        }

        return *iter;
    }

    /*!
    * @brief      插入节点,不允许出现相同节点的插入
    * @return     std::pair<iterator, bool> iterator为返回的迭代器，bool为是否插入成功，
    * @param      val 插入的数据
    */
    std::pair<iterator, bool> insert_unique(const T& val)
    {
        return insert_unique_i(val);
    }
    std::pair<iterator, bool> insert_unique(T&& val)
    {
        return insert_unique_i(val);
    }
    /*!
    * @brief      插入节点,允许相等（KEY）的节点插入,
    * @return     std::pair<iterator, bool> iterator为返回的迭代器，bool为是否插入成功，
    * @param      val 插入的数据
    */
    std::pair<iterator, bool> insert_equal(const T& val)
    {
        return insert_equal_i(val);
    }
    std::pair<iterator, bool> insert_equal(T&& val)
    {
        return insert_equal_i(val);
    }

    /*!
    * @brief      得到KEY相同的元素个数，有点相当于查询操作
    * @return     size_type 数量
    * @param      key    查询的key，
    */
    size_type count(const Key& key)
    {
        size_type equal_count = 0;
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = *(index_base_ + idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first)), key) == true)
            {
                ++equal_count;
            }

            first = (next_index_ + first);
        }

        return equal_count;
    }

    /*!
    * @brief      根据key删除,
    * @return     bool 是否删除成功
    * @param      key 删除依据的key
    */
    bool erase_unique(const Key& key)
    {
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = *(index_base_ + idx);
        size_type prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first)), key) == true)
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first);
                }
                else
                {
                    *(next_index_ + prev) = *(next_index_ + first);
                }

                destroy_node(first);

                //如果INDEX已经被删除了，取消记录
                if (*(index_base_ + idx) == zce::SHMC_INVALID_POINT)
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first);
        }

        return false;
    }

    /*!
    * @brief      使用迭代器删除,尽量高效所以不用简化写法
    * @return     bool 是否删除成功
    * @param      it 删除依据的迭代器，
    */
    bool erase(const iterator& it)
    {
        Extract get_key;
        size_type idx = bkt_num_key(get_key(*it));
        size_type first = *(index_base_ + idx);
        size_type prev = first;
        size_type itseq = it.getserial();

        //
        while (first != SHMC_INVALID_POINT)
        {
            if (first == itseq)
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first);
                }
                else
                {
                    *(next_index_ + prev) = *(next_index_ + first);
                }

                destroy_node(first);

                //如果INDEX已经被删除了，取消记录
                if (*(index_base_ + idx) == zce::SHMC_INVALID_POINT)
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first);
        }

        return false;
    }

    //删除所有相等的KEY的数据,和insert_equal配对使用，返回删除了几个数据
    size_type erase_equal(const Key& key)
    {
        size_type erase_count = 0;
        size_type idx = bkt_num_key(key);
        //从索引中找到第一个
        size_type first = *(index_base_ + idx);
        size_type prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        keyEqual   equal_key;

        //
        while (first != SHMC_INVALID_POINT)
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first)), key) == true)
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first);
                }
                else
                {
                    *(next_index_ + prev) = *(next_index_ + first);
                }

                //删除的情况下prev不用调整，first向后移动
                size_type del_pos = first;
                first = *(next_index_ + first);

                destroy_node(del_pos);

                //如果INDEX已经被删除了，取消记录
                if (*(index_base_ + idx) == zce::SHMC_INVALID_POINT)
                {
                    --(hash_head_->sz_useindex_);
                }

                ++erase_count;
            }
            else
            {
                //如果已经删除过，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (erase_count > 0)
                {
                    break;
                }

                //没有找到，继续向后移动
                prev = first;
                first = *(next_index_ + first);
            }
        }

        return erase_count;
    }

    //返回链表中已经有的元素个数
    size_type size()
    {
        return hash_head_->sz_usenode_;
    }

    //返回NODE池子的容量
    size_type capacity()
    {
        return hash_head_->num_of_node_;
    }

    //空闲的节点个数
    size_type free()
    {
        return hash_head_->sz_freenode_;
    }

    //使用的索引的个数
    size_type index_size()
    {
        return hash_head_->sz_useindex_;
    }

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;

    //头部数据
    _shm_ht_head* hash_head_ = nullptr;

    ///所有的指针都是根据基地址计算得到的,保留他们主要用于方便计算,
    ///每次初始化会重新计算,所以不会有地址错误的问题，而且这些值是各自拥有，不共享的，
    //索引数据区,
    size_type* index_base_ = nullptr;

    //数据区起始指针,
    T* data_base_ = nullptr;

    //这个元素后面跟的开链的索引
    size_type* next_index_ = nullptr;
};

template<class T, class Hash = shm_hash<T>, class keyEqual = std::equal_to<T> >
class shm_hashset :
    public shm_hashtable< T, T, Hash, shm_identity<T>, keyEqual>
{
private:
    typedef shm_hashset< T, Hash, keyEqual> self;
    typedef shm_hashtable< T, T, Hash, shm_identity<T>, keyEqual> shm_hashtable_t;
    typedef shmc_size_type size_type;
protected:
    shm_hashset() = default;
    shm_hashset(const shm_hashset& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_hashset() = default;

public:
    static self*
        initialize(size_type req_num,
                   size_type& real_num,
                   char* mem_addr,
                   bool if_restore = false)
    {
        return reinterpret_cast<self *>(
            shm_hashtable_t::initialize(req_num,
            real_num,
            mem_addr,
            if_restore));
    }
};

//HASH MAP
template<class Key, class T, class Hash = shm_hash<Key>,
    class keyEqual = std::equal_to<Key> >
class shm_hashmap :
    public shm_hashtable< std::pair <Key, T>, Key, Hash,
    shm_select1st <std::pair <Key, T> >, keyEqual  >
{
private:
    typedef shm_hashmap< Key, T, Hash, keyEqual> self;
    typedef shm_hashtable<T, Key, Hash, shm_select1st<std::pair <Key, T> >, keyEqual> shm_hashtable_t;
    typedef shmc_size_type size_type;
protected:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_hashmap() = default;
    shm_hashmap(const shm_hashmap& others) = delete;
    const self& operator=(const self& others) = delete;
public:
    ~shm_hashmap() = default;

public:
    static self*
        initialize(size_type req_num,
                   size_type& real_num,
                   char* mem_addr,
                   bool if_restore = false)
    {
        return reinterpret_cast<self *>(
            shm_hashtable_t::initialize(req_num,
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

template < class T, class Key, class Hash, class keyEqual >
using static_hashtable = zce::shm_hashtable<T, Key, Hash, keyEqual>;

template<class T, class Hash, class keyEqual >
using static_hashset = zce::shm_hashset<T, Hash, keyEqual>;

template<class Key, class T, class Hash, class keyEqual>
using static_hashmap = zce::shm_hashmap<Key, T, Hash, keyEqual>;
}
