/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_table.h
* @author     Sailzeng <sailerzeng@gmail.com>
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

#ifndef ZCE_LIB_SMEM_HASH_TABLE_H_
#define ZCE_LIB_SMEM_HASH_TABLE_H_

#include "zce_shm_predefine.h"

//原来有这两个结构，后来删除，原因有2，
//其他后写的代码没有用类似的方法，
//写恢复的时候如果外面有一层结构要麻烦一点，

//hashtable的节点,去掉的原因是避免多次拷贝构造，
//template <class _value_type> class _shm_hashtable_node
//{
//public:
//    //NODE链表的指针,
//    size_t        next_;
//    //ValueType数据
//    _value_type   hash_data_;
//

namespace ZCE_LIB
{

template < class _value_type,
         class _key_type,
         class _hash_fun,
         class _extract_key,
         class _equal_key > class shm_hashtable;

///

/*!
* @brief      hash_table的迭代器.
*             只提供了单向迭代器，++，
*             模版参数我就不解释，反正和hash_table一致，你看下面的说明把。
*/
template <class _value_type, class _key_type, class _hash_fun, class _extract_key, class _equal_key>
class _shm_hashtable_iterator
{
protected:

    ///hash_type定义，方便使用
    typedef shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> shm_hashtable_t;
    ///迭代器定义，方便下面的使用
    typedef _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> iterator;

public:

    ///构造函数
    _shm_hashtable_iterator(size_t seq, shm_hashtable_t *instance):
        serial_(seq),
        ht_instance_(instance)
    {
    }

    ///默认构造函数
    _shm_hashtable_iterator()
    {
    }

    ///保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

    ///提供这个方法事为了加快很多函数的速度,
    size_t getnext()
    {
        return *(ht_instance_->next_index_ + serial_);
    }

    ///
    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && ht_instance_ == x.ht_instance_);
    }
    ///
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    ///提领操作
    _value_type &operator*() const
    {
        return *(ht_instance_->data_base_ + serial_);
    }
    ///在多线程的环境下提供这个运送符号是不安全的,我没有加锁,原因如说明
    _value_type *operator->() const
    {
        //
        return &(operator*());
    }

    ///单向迭代器
    iterator &operator++()
    {
        size_t oldseq = serial_;
        serial_ = *(ht_instance_->next_index_ + serial_);

        //如果这个节点是末位的节点
        if (serial_ == _shm_memory_base::_INVALID_POINT)
        {
            //顺着Index查询.
            size_t bucket = ht_instance_->bkt_num_value(*(ht_instance_->data_base_ + oldseq));

            while (serial_ == _shm_memory_base::_INVALID_POINT && ++bucket < ht_instance_->capacity() )
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
    size_t           serial_;
    ///HashTable的指针,
    shm_hashtable_t *ht_instance_;
};

/*!
* @brief shm_hash_table的头部数据区的结构
*        记录大小，使用情况等
*/
class _shm_hash_table_head
{
protected:
    _shm_hash_table_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        sz_useindex_(0),
        free_headnode_(0)
    {
    }
    ~_shm_hash_table_head()
    {
    }

public:
    ///内存区的长度
    size_t                             size_of_mmap_;

    ///NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
    size_t                             num_of_node_;

    ///FREE的NODE个数
    size_t                             sz_freenode_;
    ///USE的NODE个数
    size_t                             sz_usenode_;

    ///使用的INDEX个数,可以了解实际开链的负载比率
    size_t                             sz_useindex_;

    ///FREE NODE的头指针
    size_t                             free_headnode_;

};

/*!
* @brief      HashTABLE的定义,看完HashTable,才发现封装太太经典,爱死STL了,向那些聪明的脑瓜致敬
*             另外,最大负载系数我设计为1,本来想搞大一些,但是考虑STL的hasttable在复杂达到1后会调整,
*             这儿的参数顺序我不喜欢,但遵照STL的顺序把
* @tparam     _value_type  节点的实际值型别
* @tparam     _key_type    节点的键的型别
* @tparam     _hash_fun    _key_type的hash函数,或者函数对象
* @tparam     _extract_key 从节点中得到键值的方法,或者函数对象
* @tparam     _equal_key    比较键值是否相等的方法,或者函数对象
* @note
*/
template < class _value_type,
         class _key_type,
         class _hash_fun = smem_hash<_key_type>,
         class _extract_key = smem_identity<_value_type>,
         class _equal_key = std::equal_to<_key_type> >
class shm_hashtable : public _shm_memory_base
{
public:
    //定义自己
    typedef shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> self;

    //定义迭代器
    typedef _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> iterator;

    //友元
    friend class _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key>;

public:
    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(NULL),
        hash_head_(NULL),
        index_base_(NULL),
        data_base_(NULL)
    {
    }

    shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >():
        _shm_memory_base(NULL)
    {
    }

    ~shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >()
    {
    }

    //只定义,不实现,
    const self &operator=(const self &others);

protected:

    //分配一个NODE,将其从FREELIST中取出
    size_t create_node(const _value_type &val)
    {
        //如果没有空间可以分配
        if (hash_head_->sz_freenode_ == 0)
        {
            return _INVALID_POINT;
        }

        //从连上取下一个空闲节点
        size_t new_node = hash_head_->free_headnode_;
        hash_head_->free_headnode_ = *(next_index_ + new_node);
        -- hash_head_->sz_freenode_ ;
        ++ hash_head_->sz_usenode_ ;

        //用placement new进行赋值
        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //释放一个NODE,将其归还给FREELIST,单向链表就是简单
    void destroy_node(size_t pos)
    {
        size_t freenext = hash_head_->free_headnode_;
        *(next_index_ + pos) = freenext;
        hash_head_->free_headnode_ = pos;

        ++ hash_head_->sz_freenode_ ;
        -- hash_head_->sz_usenode_ ;

        //调用显式的析构函数
        (data_base_ + pos)->~_value_type();
    }

public:


    /*!
    * @brief      内存区的构成为 define区,index区,data区,返回所需要的长度,
    * @return     size_t   所需的尺寸
    * @param      req_num  请求的NODE数量
    * @param      real_num 实际分配的NODE数量
    * @note       注意返回的是实际INDEX长度,会取一个质数
    */
    static size_t getallocsize(size_t req_num, size_t &real_num)
    {
        ZCE_LIB::hash_prime(req_num, real_num);
        return  sizeof(_shm_hash_table_head)  +
                sizeof(size_t) * (real_num) +
                sizeof(_value_type) * real_num +
                sizeof(size_t) * (real_num);
    }


    /*!
    * @brief      初始化
    * @return     self*
    * @param      req_num   请求的NODE数量
    * @param      real_num
    * @param      pmmap
    * @param      if_restore
    */
    static self *initialize(size_t req_num, size_t &real_num, char *pmmap, bool if_restore = false)
    {
        assert(pmmap != NULL && req_num > 0);
        //调整
        size_t sz_mmap = getallocsize(req_num, real_num);

        _shm_hash_table_head *hashhead =  reinterpret_cast< _shm_hash_table_head * >(pmmap);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (sz_mmap != hashhead->size_of_mmap_ ||
                real_num != hashhead->num_of_node_ )
            {
                //一般情况下不一致返回NULL，标识恢复失败，
#if ALLOW_RESTORE_INCONFORMITY != 1
                return NULL;
#else
                ZCE_LOGMSG(RS_ALERT, "Hash Table node initialize number[%lu|%lu] and restore number [%lu|%lu] "
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
            //初始化尺寸
            hashhead->size_of_mmap_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }


        shm_hashtable< _value_type, _key_type , _hash_fun, _extract_key, _equal_key  >* instance
            = new shm_hashtable< _value_type, _key_type , _hash_fun, _extract_key, _equal_key  >();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算

        //计算这些指针的地址，主要是为了方便后面的处理
        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;

        instance->hash_head_ = hashhead;
        tmp_base = tmp_base + sizeof(_shm_hash_table_head);

        instance->index_base_ = reinterpret_cast<size_t *>(tmp_base );
        tmp_base = tmp_base +  sizeof(size_t) * (real_num );

        instance->data_base_ = reinterpret_cast< _value_type *>(tmp_base );
        tmp_base = tmp_base +  sizeof(_value_type) * (real_num );

        instance->next_index_ = reinterpret_cast< size_t *>(tmp_base );

        if (if_restore == false)
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }

        //打完收工
        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        hash_head_->sz_freenode_ = hash_head_->num_of_node_;
        hash_head_->sz_usenode_ = 0;
        hash_head_->sz_useindex_ = 0;

        //初始化free数据区
        for (size_t i = 0; i < hash_head_->num_of_node_ ; ++i )
        {
            index_base_[i] = _INVALID_POINT;
        }

        //清理FREELIST的单向NODE,
        hash_head_->free_headnode_ = 0;

        //
        for (size_t i = 0; i < hash_head_->num_of_node_ ; ++i )
        {
            next_index_[i] = i + 1;

            if (i == hash_head_->num_of_node_ - 1)
            {
                next_index_[i] = _INVALID_POINT;
            }
        }
    }

    //从value中取值
    size_t bkt_num_value(const _value_type &obj) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj)));
    }
    //为什么不能重载上面的函数,自己考虑一下,
    size_t bkt_num_key(const _key_type &key) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % hash_head_->num_of_node_);
    }

    //HashTable的Begin 不是高效函数,不建议太多使用
    iterator begin()
    {
        for (size_t i = 0; i < hash_head_->num_of_node_; ++i)
        {
            if (*(index_base_ + i) != _INVALID_POINT)
            {
                return iterator(*(index_base_ + i), this);
            }
        }

        return end();
    }
    //用无效指针
    iterator end()
    {
        return iterator(_INVALID_POINT, this);
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
        if (hash_head_->sz_freenode_ == 0 )
        {
            return true;
        }

        return false;
    }

    //查询相应的Key是否有
    iterator find(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        size_t first = *(index_base_ +  idx);
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT && !equal_key(get_key(*(data_base_ + first )), key))
        {
            first = *(next_index_ + first );
        }

        return iterator(first, this);
    }
    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    //
    _value_type &find_or_insert(const _value_type &val)
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
    std::pair<iterator, bool> insert_unique(const _value_type &val)
    {
        size_t idx = bkt_num_value(val);
        size_t first_idx = *(index_base_ +  idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t nxt_idx = first_idx;

        while (nxt_idx != _INVALID_POINT )
        {
            //如果找到相同的Key函数
            if (equal_key((get_key(*(data_base_ + nxt_idx ))), (get_key(val))) == true )
            {
                return std::pair<iterator, bool>(iterator(nxt_idx, this), false);
            }

            nxt_idx = *(next_index_ + nxt_idx );
        }

        //没有找到,插入新数据
        size_t newnode = create_node(val);
        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //放入链表中
        *(next_index_ + newnode) = *(index_base_ + idx);
        *(index_base_ + idx) = newnode;

        //如果第一个位置就不是无效的INDEX
        if (first_idx == _INVALID_POINT)
        {
            //记录使用量一个索引
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }


    /*!
    * @brief      插入节点,允许相等（KEY）的节点插入,
    * @return     std::pair<iterator, bool> iterator为返回的迭代器，bool为是否插入成功，
    * @param      val 插入的数据
    */
    std::pair<iterator, bool> insert_equal(const _value_type &val)
    {
        size_t idx = bkt_num_value(val);
        size_t first_idx = *(index_base_ +  idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t nxt_idx = first_idx;

        while (nxt_idx != _INVALID_POINT )
        {
            //如果找到相同的Key函数,会将相同的数据放在一起，便于处理
            if (equal_key((get_key(*(data_base_ + nxt_idx ))), (get_key(val))) == true )
            {
                break;
            }

            nxt_idx = *(next_index_ + nxt_idx );
        }

        //没有找到,插入新数据
        size_t newnode = create_node(val);

        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //没有找到相同KEY的数据
        if (nxt_idx == _INVALID_POINT)
        {
            //放入链表的首部就可以了
            (next_index_ + newnode) = *(index_base_ + idx);
            *(index_base_ + idx) = newnode;
        }
        //如果找到了相同的KEY节点
        else
        {
            //放到这个节点的后面
            *(next_index_ + newnode) = *(next_index_ + nxt_idx );
            *(next_index_ + nxt_idx ) = newnode;
        }

        //如果第一个位置就不是无效的INDEX,记录使用了INDEX
        if (first_idx == _INVALID_POINT)
        {
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    
    /*!
    * @brief      得到KEY相同的元素个数，有点相当于查询操作
    * @return     size_t 数量
    * @param      key    查询的key，
    */
    size_t count(const _key_type &key)
    {
        size_t equal_count = 0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = *(index_base_ +  idx);

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                ++equal_count;
            }

            first = (next_index_ + first );
        }

        return equal_count;
    }


    /*!
    * @brief      根据key删除,
    * @return     bool 是否删除成功
    * @param      key 删除依据的key
    */
    bool erase_unique(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = *(index_base_ +  idx);
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                destroy_node(first);

                //如果INDEX已经被删除了，取消记录
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first );
        }

        return false;
    }


    /*!
    * @brief      使用迭代器删除,尽量高效所以不用简化写法
    * @return     bool 是否删除成功
    * @param      it 删除依据的迭代器，
    */
    bool erase(const iterator &it)
    {
        _extract_key get_key;
        size_t idx = bkt_num_key(get_key(*it));
        size_t first = *(index_base_ +  idx);
        size_t prev = first;
        size_t itseq = it.getserial();

        //
        while (first != _INVALID_POINT )
        {
            if (first == itseq )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                destroy_node(first);

                //如果INDEX已经被删除了，取消记录
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first );
        }

        return false;
    }


    //删除所有相等的KEY的数据,和insert_equal配对使用，返回删除了几个数据
    size_t erase_equal(const _key_type &key)
    {
        size_t erase_count = 0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = *(index_base_ +  idx);
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                //删除的情况下prev不用调整，first向后移动
                size_t del_pos = first;
                first = *(next_index_ + first );

                destroy_node(del_pos);

                //如果INDEX已经被删除了，取消记录
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
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
                first = *(next_index_ + first );
            }
        }

        return erase_count;
    }


    //返回链表中已经有的元素个数
    size_t size()
    {
        return hash_head_->sz_usenode_;
    }

    //返回NODE池子的容量
    size_t capacity()
    {
        return hash_head_->num_of_node_;
    }

    //空闲的节点个数
    size_t sizefreenode()
    {
        return hash_head_->sz_freenode_;
    }

    //使用的索引的个数
    size_t sizeuseindex()
    {
        return hash_head_->sz_useindex_;
    }

protected:

    _shm_hash_table_head         *hash_head_;

    ///所有的指针都是根据基地址计算得到的,保留他们主要用于方便计算,
    ///每次初始化会重新计算,所以不会有地址错误的问题，而且这些值是各自拥有，不共享的，
    //索引数据区,
    size_t                       *index_base_;

    //数据区起始指针,
    _value_type                  *data_base_;

    //这个元素后面跟的开链的索引
    size_t                       *next_index_;
};

template<class _value_type, class _hash_fun = smem_hash<_value_type>, class _equal_key = std::equal_to<_value_type> >
class shm_hashset :
    public shm_hashtable< _value_type, _value_type , _hash_fun, smem_identity<_value_type>, _equal_key  >
{
public:
    //定义迭代器
    //typedef shm_hashtable< _value_type,_value_type ,_hash_fun, _extract_key,_equal_key  >::iterator iterator;
protected:

    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_hashset<_value_type, _hash_fun, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable<_value_type, _value_type , _hash_fun, smem_identity<_value_type>, _equal_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashset<_value_type, _hash_fun, _equal_key >()
    {
    }

public:
    static shm_hashset< _value_type, _hash_fun, _equal_key  >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashset< _value_type, _hash_fun, _equal_key  >*>(
                   shm_hashtable<_value_type, _value_type , _hash_fun, smem_identity<_value_type>, _equal_key>::initialize(numnode, pmmap, if_restore));
    }
};

//HASH MAP
template<class _key_type, class _value_type, class _hash_fun = smem_hash<_key_type>, class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >, class _equal_key = std::equal_to<_key_type> >
class shm_hashmap :
    public shm_hashtable< std::pair <_key_type, _value_type> , _key_type, _hash_fun , _extract_key, _equal_key  >
{

protected:

    //如果在共享内存使用,没有new,所以统一用initialize 初始化
    //这个函数,不给你用,就是不给你用
    shm_hashmap<_key_type, _value_type, _hash_fun, _extract_key, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable< std::pair <_key_type, _value_type> , _key_type, _extract_key, _equal_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashmap<_key_type, _value_type, _hash_fun, _extract_key, _equal_key >()
    {
    }
public:
    static shm_hashmap< _key_type, _value_type, _hash_fun, _extract_key, _equal_key  >*
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashmap< _key_type, _value_type, _hash_fun, _extract_key, _equal_key  >*>(
                   shm_hashtable< std::pair <_key_type, _value_type>, _key_type , _hash_fun, _extract_key, _equal_key>::initialize(numnode, pmmap, if_restore));
    }
    //[]操作符号有优点和缺点，
    _value_type &operator[](const _key_type &key)
    {
        return (find(key)).second;
    }
};

};

#endif //ZCE_LIB_SMEM_HASH_TABLE_H_

