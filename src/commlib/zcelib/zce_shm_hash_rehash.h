/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_rehash.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2010年04月6日
* @brief      多次HASH解决冲突的HASH实现，同时也是安全的HASH,设计目的是让这个HASH
*             映射共享内存文件可以采用COPY的方式备份,
*             这个算法的最大好处是数据全部是连续存放的，可以对数据进行直接的备份和操作,
*             所以我加入了一个SAFE标示
*             HASH取模的总次数我采用24次,为什么这样呢，因为我做过一些测试，在HASH的次数
*             在24次左右时，第一次出现冲突的情况的负载率大约是85%以上
*             http://blog.csdn.net/fullsail/article/details/6328702
* 
* @details    这个算法的基本思路来自即通的几个同事,感谢icezhuang，mikewei，spraydong等
*             同事，本质上他是一个用再HASH的方式解决冲突的算法，
*             我所做的事情是用模板，对于整个数据管理进行封装,
*             这个算法的不足之处来自，在极端情况下，其可能出现无法负载的情况，
*             但是即通的同事说他们测试，这种算法的负载能力能达到95%,正常情况，他们保证
*             在80%的负载能力的时候进行扩容,（他们的算法是32阶）
*             你可以估算一下，这个算法出现冲突的情况应该是非常低的
*             但是要注意的是，如果多人操作读写（包括备份），还是会出现拷贝的数据只有半截的问题,
*             这个无法避免,但是由于其中没有链，所以整个数据还是可以使用的，这就是好处,
* @note       代码的一些历史，
*             1.Date  : 2010年04月12日
*             Author  : Sailzeng
*             Modification  : 代码级别完成，开始测试,
* 
*             1.Date  : 2010年04月12日
*             Author  : Sailzeng
*             Modification  : 性能测试结果对比HASH的开链方式和多次HASH方式,多次HASH的性能
*             要慢不少，但1s完成上千万次查询还是可以的,
* 
*             2.Date  : 2010年04月20日
*             Author  : Sailzeng
*             Modification  :本来不打算加入一个淘汰算法的，因为思前想后，除非在处理的几次取模的过程中进行淘汰，否则意义不大,
*             后来被kliu同学挑战，重新思考了一下，即使队列的数量大约1亿个，全部扫描要淘汰的数据的时间也用不了1s，真的我那么在乎成本吗？
*             既然被挑战，我就提供一个吧。反正也是手到擒来的事情,另外，为什么我不另起炉灶呢,因为本来就都是数组,
* 
*             2.Date  : 2010年04月21日
*             Author  : Sailzeng
*             Modification  :思考再三，我还是将_equal都删除了，因为REHASH对于insert_equal支持并不好，为什么呢，他的最大支持相等的对象个数
*             就是质数列表的长度,提供这个仅限于艺术的美感和YY的心理，你一定要用，我先告诉你如果是我自己的代码我是会慎重的
*             其实很痛心，大量的代码被删除了，但丑陋的东西不出现也许是对的呢
*/

#ifndef ZCE_SHM_HASH_TABLE_REHASH_H_
#define ZCE_SHM_HASH_TABLE_REHASH_H_

#include "zce_shm_predefine.h"

namespace ZCE_LIB
{

//为了方便编译，预先定义一下
template < class _value_type,
         class _key_type,
         class _hash_fun,
         class _extract_key,
         class _equal_key,
         class _washout_fun > class shm_hash_rehash;

//SAFE HASH 迭代器，注意这儿是SAFE HASH，不是SAFE iterator，我吧HASH放前面纯粹是方便几个文件
//放在一起
template < class _value_type,
         class _key_type,
         class _hashfun,
         class _extract_key,
         class _equal_key,
         class _washout_fun >
class _hash_rehash_iterator
{
protected:

    //HASH TABLE的定义
    typedef shm_hash_rehash < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key ,
            _washout_fun > _hashtable_rehash;

    //定义迭代器
    typedef _hash_rehash_iterator < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

protected:
    //序列号
    size_t                 serial_;
    //
    _hashtable_rehash     *ht_rehash_instance_;

public:
    _hash_rehash_iterator():
        serial_(_shm_memory_base::_INVALID_POINT),
        ht_rehash_instance_(NULL)
    {
    }

    _hash_rehash_iterator(size_t serial, _hashtable_rehash *ht_safe_inst):
        serial_(serial),
        ht_rehash_instance_(ht_safe_inst)
    {
    }

    ~_hash_rehash_iterator()
    {
    }

    _value_type &operator*() const
    {
        return ht_rehash_instance_->value_base_ [serial_];
    }

    _value_type *operator->() const
    {
        return ht_rehash_instance_->value_base_ + serial_;
    }

    //本来只提供前向迭代器，曾经以为使用可以使用LIST保证迭代的高效，发现不行，
    //可能要提供另外的函数
    //前向迭代器
    iterator &operator++()
    {
        _extract_key get_key;
        _equal_key   equal_key;
        size_t old_serial = serial_ + 1;

        for (; old_serial < ht_rehash_instance_->hash_safe_head_->num_of_node_ ; ++old_serial)
        {
            //这个用无效值对象，对所有元素进行赋值，为什么使用placement new，因为更加上流一点
            //如果不是一个无效值
            if (false == equal_key( get_key (*(ht_rehash_instance_->value_base_ + old_serial)),
                                    get_key(ht_rehash_instance_->invalid_data_)) )
            {
                serial_ = old_serial;
                break;
            }
        }

        //如果没有发现数据
        if (old_serial == ht_rehash_instance_->hash_safe_head_->num_of_node_)
        {
            serial_ = _shm_memory_base::_INVALID_POINT;
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
        if (ht_rehash_instance_ == it.ht_rehash_instance_ &&
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
class _hashtable_rehash_head
{
protected:
    //构造析构函数
    _hashtable_rehash_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        len_of_primes_list_(0),
        expire_start_(0)
    {
        //清0
        memset(primes_list_, 0, sizeof(primes_list_));
    }
    ~_hashtable_rehash_head()
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

    //质数列表的个数
    size_t           len_of_primes_list_;

    //
    size_t           primes_list_[NUM_PRIMES_LIST_ELEMENT];

    //淘汰其实的位置，为了避免每次淘汰都从一个地方开始
    size_t           expire_start_;

};

template < class _value_type,
         class _key_type,
         class _hash_fun = smem_hash<_key_type>,
         class _extract_key = smem_identity<_value_type>,
         class _equal_key = std::equal_to<_key_type>,
         class _washout_fun = _default_washout_fun<_value_type> >
class shm_hash_rehash : public  _shm_memory_base
{
public:

    //定义迭代器
    typedef _hash_rehash_iterator < _value_type,
            _key_type,
            _hash_fun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

    //声明迭代器是友元
    friend class _hash_rehash_iterator < _value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key ,
            _washout_fun >;

protected:

    //无效的数据，比如这个数据里面的KEY是永远不会出现的，一般是比如0，或者-1
    //这个地方其实是偷懒的表现，用于简化处理某个空间，是否有效
    _value_type              invalid_data_;

    //放在共享内存头部的指针
    _hashtable_rehash_head  *hash_safe_head_;

    //数据区指针
    _value_type             *value_base_;

    //优先级的数据指针,用32位的数据保存优先级
    unsigned int            *priority_base_;

protected:

    shm_hash_rehash():
        hash_safe_head_(NULL),
        value_base_(NULL),
        priority_base_(NULL)
    {
    }

public:

    ~shm_hash_rehash()
    {
    }

protected:

    //返回大于N的一个质数,(最后一个例外),使用质数作为取模的
    static size_t get_next_prime(const size_t n)
    {
        //为什么不直接在模板里面写static数组？好像是那个编译器不支持,已经忘记过程了
        //最后一个是总和
        const size_t primes_matrix[NUM_HASH_SAFE_PRIMES_LIST][NUM_PRIMES_LIST_ELEMENT + 1] = __HASH_SAFE_PRIME_LIST_BODY;

        size_t i = 0;

        //
        for (; i < NUM_HASH_SAFE_PRIMES_LIST ; ++i)
        {
            if (primes_matrix[i][NUM_PRIMES_LIST_ELEMENT] >= n)
            {
                break;
            }
        }

        //如果大于最后一个调整一下,
        return (i == NUM_HASH_SAFE_PRIMES_LIST) ? primes_matrix[i - 1][NUM_PRIMES_LIST_ELEMENT] : primes_matrix[i][NUM_PRIMES_LIST_ELEMENT];
    }

    //从value中取值
    size_t bkt_num_value(const _value_type &obj, size_t one_primes) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj), one_primes));
    }

    //为什么不能重载上面的函数,自己考虑一下,
    //重载的话，如果_value_type和_key_type一样，就等着哭吧 ---inmore
    size_t bkt_num_key(const _key_type &key, size_t one_primes) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % one_primes);
    }

    //取出无效数据，在迭代器使用的时候需要
    _value_type get_invalid_data()
    {
        return invalid_data_;
    }

    //用于自己的内部的初始化处理
    static shm_hash_rehash < _value_type,
           _key_type,
           _hash_fun,
           _extract_key,
           _equal_key > *
           initialize_i(size_t primes_number,
                        const size_t primes_list[],
                        size_t num_node,
                        size_t sz_alloc,
                        char *pmmap,
                        const _value_type &invalid_data,
                        bool if_expire,
                        bool if_restore)
    {

        shm_hash_rehash< _value_type, _key_type , _hash_fun, _extract_key, _equal_key >* instance
            = new shm_hash_rehash< _value_type, _key_type , _hash_fun, _extract_key, _equal_key >();

        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;
        instance->hash_safe_head_ = reinterpret_cast<_hashtable_rehash_head *>(tmp_base);
        tmp_base = tmp_base + sizeof(_hashtable_rehash_head);

        instance->value_base_ = reinterpret_cast<_value_type *>(tmp_base);
        tmp_base = tmp_base + (sizeof(_value_type) * (num_node));

        //初始化尺寸,如果是恢复，刚才已经比较过了
        instance->hash_safe_head_->size_of_mmap_ = sz_alloc;
        instance->hash_safe_head_->num_of_node_ = num_node;

        //每次都将将淘汰开始的地方记录为0
        instance->hash_safe_head_->expire_start_ = 0;

        if (if_expire)
        {
            instance->priority_base_ = reinterpret_cast<unsigned int *>(tmp_base);
            tmp_base = tmp_base + (sizeof(unsigned int) * (num_node));
        }
        else
        {
            //还是强调一次成NULL
            instance->priority_base_ = NULL;
        }

        instance->invalid_data_ = invalid_data;

        //质数表填写进去
        instance->hash_safe_head_->len_of_primes_list_ = primes_number;

        for (size_t y = 0; y < primes_number; ++y)
        {
            instance->hash_safe_head_->primes_list_[y] = primes_list[y];
        }

        //如果不进行恢复处理
        if (if_restore == false)
        {
            //清理初始化所有的内存,有的节点为FREE
            instance->clear(if_expire);
        }

        return instance;
    }

public:

    //内存区的构成为 define区,index区,data区,返回所需要的长度,

    //返回值为需要分配的空间大小
    //第一个参数表示所需要的节点个数，注意返回的是实际INDEX长度,会增加一些
    //第二个参数表示是否使用超时处理
    static size_t getallocsize(size_t &num_node, bool if_expire)
    {
        num_node = get_next_prime(num_node);
        size_t sz_alloc =  0;
        //
        sz_alloc += sizeof(_hashtable_rehash_head);

        sz_alloc += sizeof(size_t) * NUM_PRIMES_LIST_ELEMENT;
        //给出结构的数量
        sz_alloc += sizeof(_value_type) * (num_node);

        if (if_expire)
        {
            sz_alloc += (sizeof(unsigned int) * (num_node));
        }

        return sz_alloc;
    }

    //返回值为需要分配的空间大小
    //如果是输入质数队列，用这个函数得到空间,
    //第一个参数质数队列长度,
    //第二个参数质数队列，
    //第三个参数是是否超时，
    //第三个参数返回参数,总节点个数,
    static size_t getallocsize(size_t primes_number, const size_t primes_list[], bool if_expire, size_t &node_count)
    {
        //列表的最大长度不能大于MAX_PRIMES_LIST_ELEMENT
        assert(primes_number <= NUM_PRIMES_LIST_ELEMENT);

        //计算总容量
        node_count = 0;

        for (size_t i = 0; i < primes_number; ++i)
        {
            node_count += primes_list[i];
        }

        assert(node_count > 0);

        //
        size_t sz_alloc =  0;

        //计算空间，根据头，质数队列,结构数量，优先级空间
        sz_alloc += sizeof(_hashtable_rehash_head);

        sz_alloc += sizeof(_value_type) * (node_count);

        if (if_expire)
        {
            sz_alloc += sizeof(unsigned int) * (node_count);
        }

        return sz_alloc;
    }

    //推荐使用这个函数,你做的事情要少很多
    //初始化，返回函数对象的指针，以后就通过这个指针操作，为什么不直接用构造函数呢，我很难回答，可能是通过getallocsize一脉相承？
    //第1个参数，尺寸，你需要放几个元素给你，注意这个参数会返回一个实际我分配多少尺寸给你
    //第2个参数，传递进来的空间指针，空间的大小通过getallocsize得到.
    //第3个参数，一个无效的数据数据值，因为我懒得给你开辟一个地方记录某个数据是否使用了.所以我会将所有的数据都初始化无效结构，无效结构，我会记录成这个空间没有使用
    //第4个参数，是否要使用淘汰功能,如果不适用，空间可以更加小一些
    //第5个参数，是否是从一个内存中恢复空间，比如共享内存之类的恢复
    static shm_hash_rehash < _value_type,
    _key_type,
    _hash_fun,
    _extract_key,
    _equal_key > *
    initialize(size_t &num_node, char *pmmap, const _value_type &invalid_data, bool if_expire, bool if_restore = false)
    {
        assert(pmmap != NULL && num_node > 0 );

        //调整,根据你的尺寸，向上找一个合适的空间
        num_node = get_next_prime(num_node);

        size_t sz_alloc = getallocsize(num_node, if_expire);

        _hashtable_rehash_head *hashhead =  reinterpret_cast< _hashtable_rehash_head * >(pmmap);

        const size_t primes_matrix[NUM_HASH_SAFE_PRIMES_LIST][NUM_PRIMES_LIST_ELEMENT + 1] = __HASH_SAFE_PRIME_LIST_BODY;

        size_t primes_list_no = 0;

        //找到选择的HASH列，直接将这一列的数据放入内存，方便后面处理
        for (size_t x = 0; x < NUM_HASH_SAFE_PRIMES_LIST; ++x)
        {
            //最后一个元素是前面所有质数列表的和，而NODE的个数也应该是质数列表的和
            if (num_node == primes_matrix[x][NUM_PRIMES_LIST_ELEMENT] )
            {
                primes_list_no = x;
            }
        }

        //如果是恢复,数据都在内存中,对数据进行检查
        if (if_restore == true)
        {
            //是否要检查质数表和现在的表是否一致呢?我暂时选择了进行检查，
            //但如果不进行这样，即使如果质数的矩阵改变了，还是可以用共享内存中间的数据继续跑，我选择了严谨，而不是

            //检查一下恢复的内存是否正确,
            if ( sz_alloc != hashhead->size_of_mmap_ ||
                 num_node != hashhead->num_of_node_ )
            {
                return NULL;
            }

            //质数列表的个数应该一致
            if ( hashhead->len_of_primes_list_ != NUM_PRIMES_LIST_ELEMENT )
            {
                return NULL;
            }

            //HASH列表的个数，应该等于元素的总和，是不是应该将每个数据都拿出来比较一下呢
            size_t num_node_count = 0;

            for (size_t p = 0; p < hashhead->len_of_primes_list_; ++p)
            {
                num_node_count += hashhead->primes_list_[p];
            }

            if (num_node_count != hashhead->num_of_node_)
            {
                return NULL;
            }

            //检查质数队列是否一致
            for (size_t y = 0; y < NUM_PRIMES_LIST_ELEMENT; ++y)
            {
                if (hashhead->primes_list_[y] != primes_matrix[primes_list_no][y])
                {
                    return NULL;
                }
            }

        }

        //打完收工
        return initialize_i(NUM_PRIMES_LIST_ELEMENT,
                            primes_matrix[primes_list_no],
                            num_node,
                            sz_alloc,
                            pmmap,
                            invalid_data,
                            if_expire,
                            if_restore);

    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear(bool if_expire)
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        hash_safe_head_->sz_freenode_ = hash_safe_head_->num_of_node_;
        hash_safe_head_->sz_usenode_ = 0;

        //初始化free数据区，将所有的数据初始化成无效值，用于表示一个记录是否修正了
        for (size_t i = 0; i < hash_safe_head_->num_of_node_ ; ++i )
        {
            //这个用无效值对象，对所有元素进行赋值，为什么使用placement new，因为更加上流一点
            new (value_base_ + i)_value_type(invalid_data_);
        }

        //如果要记录淘汰信息
        if (if_expire)
        {
            memset(priority_base_, 0, (sizeof(unsigned int) * (hash_safe_head_->num_of_node_)));
        }
    }

    //你也可以传递一个质数队列，作为进行多轮HASH取模的质数队列,
    static shm_hash_rehash < _value_type,
    _key_type,
    _hash_fun,
    _extract_key,
    _equal_key > *
    initialize(size_t primes_number, size_t primes_list[], char *pmmap, _value_type invalid_data, bool if_expire, bool if_restore = false)
    {
        assert(pmmap != NULL );

        _hashtable_rehash_head *hashhead =  reinterpret_cast< _hashtable_rehash_head * >(pmmap);

        size_t node_count = 0;
        size_t sz_alloc = getallocsize(primes_number, primes_list, if_expire, node_count);

        //如果是恢复,数据都在内存中,对数据进行检查
        if (if_restore == true)
        {
            //是否要检查质数表和现在的表是否一致呢?我暂时选择了进行检查，
            //但如果不进行这样，即使如果质数的矩阵改变了，还是可以用共享内存中间的数据继续跑，我选择了严谨，而不是

            //检查一下恢复的内存是否正确,
            if (sz_alloc = hashhead->size_of_mmap_ ||
                           node_count != hashhead->num_of_node_ )
            {
                return NULL;
            }

            //质数列表的个数应该一致
            if ( hashhead->len_of_primes_list_ != primes_number )
            {
                return NULL;
            }

            //检查质数队列是否一致
            for (size_t y = 0; y < primes_number; ++y)
            {
                if (hashhead->primes_list_[y] != primes_list[y])
                {
                    return NULL;
                }
            }
        }

        //打完收工
        return initialize_i(primes_number,
                            primes_list,
                            node_count,
                            sz_alloc,
                            pmmap,
                            invalid_data,
                            if_expire,
                            if_restore);
    }

public:

    //得到开始的迭代器的位置，其实并不高效，少用呀，哥们，
    iterator begin()
    {
        _extract_key get_key;
        _equal_key   equal_key;

        //找到第一个已经填写数据的对象，作为begin
        for (size_t i = 0; i < hash_safe_head_->num_of_node_; ++i)
        {
            //如果不是一个无效值
            if (false == equal_key(get_key(*(value_base_ + i)), get_key(invalid_data_)) )
            {
                return iterator( i, this);
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
        return hash_safe_head_->sz_usenode_;
    }
    //得到容量
    size_t capacity() const
    {
        return hash_safe_head_->num_of_node_;
    }
    //是否为空
    bool empty() const
    {
        return (hash_safe_head_->sz_freenode_ == hash_safe_head_->num_of_node_);
    }
    //是否空间已经满了
    bool full() const
    {
        return (hash_safe_head_->sz_freenode_ == 0);
    }

    //插入节点
    std::pair<iterator, bool> insert(const _value_type &val )
    {
        //使用函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;
        size_t idx_no_use = _INVALID_POINT;

        //循环进行N此取模操作，
        for (size_t i = 0; i < hash_safe_head_->len_of_primes_list_; ++i )
        {
            size_t idx = bkt_num_value(val, hash_safe_head_->primes_list_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), (get_key(val))) )
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //如果是一个无效数据，表示一个空位置
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == _INVALID_POINT)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }

            //准备在下一个质数列里面找
            idx_count += (hash_safe_head_->primes_list_[i] - idx);
        }

        //如果每一列对应的位置都被流氓占用了,返回一个特殊的迭代器end,告诉前段，空间危险了
        if (_INVALID_POINT == idx_no_use )
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //使用placement new进行赋值
        new (value_base_ + idx_no_use)_value_type(val);

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }

    //带优先级的插入，开始初始化的时候必须if_expire == true
    //@const _value_type &val 插入的数据
    //@unsigned int priority  插入数据优先级，
    //@unsigned int expire_priority = static_cast<unsigned int>(-1)，淘汰的优先级，默认为最大值，不进行不淘汰，这个修正来自djiang的好建议
    std::pair<iterator, bool> insert(const _value_type &val,
                                     unsigned int priority,
                                     unsigned int expire_priority = 0)
    {

        //使用函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;
        size_t idx_no_use = _INVALID_POINT;

        //循环进行N此取模操作，
        for (size_t i = 0; i < hash_safe_head_->len_of_primes_list_; ++i )
        {
            size_t idx = bkt_num_value(val, hash_safe_head_->primes_list_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), (get_key(val))) )
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //如果是一个无效数据，表示一个空位置
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == _INVALID_POINT)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }
            //如果这个位置已经有数据，可以检查一下这个数据是否过期
            else
            {
                //这儿是跟进djiang的建议进行的一个补充功能，在插入的时候就淘汰掉过期的数据，
                //如果优先级时间已经过期，就可以直接利用这个位置
                if (priority_base_[idx_count] <= expire_priority)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }

            //准备在下一个质数列里面找
            idx_count += (hash_safe_head_->primes_list_[i] - idx);
        }

        //如果每一列对应的位置都被流氓占用了,返回一个特殊的迭代器end,告诉前段，空间危险了
        if (_INVALID_POINT == idx_no_use )
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //使用placement new进行赋值
        new (value_base_ + idx_no_use)_value_type(val);

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        //如果插入成功了记录优先级
        priority_base_[idx_no_use] = priority;

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }

    //查询相应的Key是否有,返回迭代器
    //这个地方有一个陷阱,这个地方返回的迭代器++，不能给你找到相同的key的数据,而开链的HASH实现了这个功能
    iterator find(const _key_type &key)
    {
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;

        //循环进行N此取模操作，
        for (size_t i = 0; i < hash_safe_head_->len_of_primes_list_; ++i )
        {
            //将val取出key，取模
            size_t idx = bkt_num_key(key, hash_safe_head_->primes_list_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), key) )
            {
                return iterator(idx_count, this);
            }

            //准备在下一个质数列里面找
            idx_count += (hash_safe_head_->primes_list_[i] - idx);
        }

        return end();
    }

    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    bool erase(const _key_type &key)
    {
        iterator iter_temp = find(key);

        if (iter_temp == end())
        {
            return false;
        }

        return erase(iter_temp);
    }

    //使用迭代器删除,尽量高效所以不用简化写法
    bool erase(const iterator &it_del)
    {
        //我不做过多的检查
        if (it_del != end())
        {
            //调用析构函数,注意这个调用只能通过指针进行，对于int，之类的类型
            (value_base_ + it_del.getserial())->~_value_type();
            //恢复成无效数据
            new (value_base_ + it_del.getserial() )_value_type(invalid_data_);
            --(hash_safe_head_->sz_usenode_);
            ++(hash_safe_head_->sz_freenode_);

            return true;
        }

        return false;
    }

    //删除某个值
    bool erase_value(const _value_type &val )
    {
        _extract_key get_key;
        return erase( get_key(val));
    }

    //激活,将激活的数据挂到LIST的最开始,淘汰使用expire,disuse
    //优先级参数可以使用当前的时间
    bool active(const _key_type &key,
                unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        iterator  iter_tmp = find(key);

        //如果找到了对应的KEY
        if ( end()  != iter_tmp )
        {
            priority_base_[iter_tmp.getserial()] = priority;
            return true;
        }

        return false;
    }

    //
    bool active_value(const _value_type &val,
                      unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        _extract_key get_key;
        return active(get_key(val), priority);
    }

    //淘汰过期的数据,假设LIST中间的数据是按照过期实际排序的，这要求你传入的优先级最好是时间
    //小于等于这个优先级的数据将被淘汰
    //hope_expire_num表示你希望删除多少个值，默认为最大值,全部淘汰
    size_t expire(unsigned int expire_time, size_t hope_expire_num = static_cast<size_t>(-1))
    {
        //从尾部开始检查，
        size_t expire_num = 0;
        size_t i = 0;

        for (; i < hash_safe_head_->num_of_node_ && expire_num < hope_expire_num ; ++i)
        {
            size_t del_iter = ( hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

            //小于等于
            if (priority_base_[del_iter] <= expire_time)
            {

                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
        }

        //从一个特殊的地方上开始
        hash_safe_head_->expire_start_ = ( hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

        return expire_num;
    }

};

};

#endif //#ifndef ZCE_SHM_HASH_TABLE_REHASH_H_

