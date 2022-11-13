/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/hash_rehash.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2010年04月6日
* @brief      多次HASH解决冲突的HASH实现，同时也是安全的HASH,设计目的是让这个HASH
*             映射共享内存文件可以采用COPY的方式备份,
*             这个算法的最大好处是数据全部是连续存放的，可以对数据进行直接的备份和操作,
*             所以我加入了一个SAFE标示
*             HASH取模的总次数我采用24次,为什么这样呢，因为我做过一些测试，在HASH的
*             次数在24次左右时，第一次出现冲突的情况的负载率大约是85%以上
*             http://blog.csdn.net/fullsail/article/details/6328702
*
* @details    这个算法的基本思路来自即通的几个同事,感谢icezhuang，mikewei，
*             spraydong等同事，本质上他是一个用再HASH的方式解决冲突的算法，但为了避
*             免麻烦，我仍然声明我拥有100%的版权。我所做的事情是用模板，对于整个数据
*             管理进行封装, 而且做到了动态HASH次数，质数选择，默认值管理等，
*             而多次HASH解决冲突的思路在《数据结构》严蔚敏的书上也有涉及，只是当年完全
*             忽视了这段
*             这个算法的不足之处来自，在极端情况下，其可能出现无法负载的情况，
*             但是即通的同事说他们测试，这种算法的负载能力能达到95%,正常情况，他们保
*             证在80%的负载能力的时候进行扩容,（他们的算法是32阶）
*             你可以估算一下，这个算法出现冲突的情况应该是非常低的
*             但是要注意的是，如果多人操作读写（包括备份），还是会出现拷贝的数据只有半
*             截的问题,这个无法避免,但是由于其中没有链，所以整个数据还是可以使用的，这
*             就是好处,
* @note       代码的一些历史，
*             1.Date  : 2010年04月12日
*             Author  : Sailzeng
*             Modification  : 代码级别完成，开始测试,
*
*             1.Date  : 2010年04月12日
*             Author  : Sailzeng
*             Modification  : 性能测试结果对比HASH的开链方式和多次HASH方式,多次
*             HASH的性能要慢不少，但1s完成上千万次查询还是可以的,
*
*             2.Date  : 2010年04月20日
*             Author  : Sailzeng
*             Modification  :本来不打算加入一个淘汰算法的，因为思前想后，除非在处
*             理的几次取模的过程中进行淘汰，否则意义不大,后来被kliu同学挑战，重新思
*             考了一下，即使队列的数量大约1亿个，全部扫描要淘汰的数据的时间也用不了
*             1s，真的我那么在乎成本吗？既然被挑战，我就提供一个吧。反正也是手到擒来
*             的事情,另外，为什么我不另起炉灶呢,因为本来就都是数组,
*
*             2.Date  : 2010年04月21日
*             Author  : Sailzeng
*             Modification  :思考再三，我还是将_equal都删除了，因为REHASH对于
*             insert_equal支持并不好，为什么呢，他的最大支持相等的对象个数就是质
*             数列表的长度,提供这个仅限于艺术的美感和YY的心理，你一定要用，我先告诉
*             你如果是我自己的代码我是会慎重的其实很痛心，大量的代码被删除了，但丑陋
*             的东西不出现也许是对的呢
*/

#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
//为了方便编译，预先定义一下
template < class T,
    class Key,
    class Hash,
    class Extract,
    class KeyEqual,
    class Washout > class shm_rehash_hashtable;

//SAFE HASH 迭代器，注意这儿是SAFE HASH，不是SAFE iterator，我吧HASH放前面纯粹是方便几个文件
//放在一起
template < class T,
    class Key,
    class Hash,
    class Extract,
    class KeyEqual,
    class Washout >
class _ht_rehash_iter
{
private:
    //HASH TABLE的定义
    typedef shm_rehash_hashtable < T, Key, Hash, Extract, KeyEqual, Washout > rehash_hashtable_t;
    //定义迭代器
    typedef _ht_rehash_iter < T, Key, Hash, Extract, KeyEqual, Washout > iterator;
public:
    typedef shmc_size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    //迭代器萃取器所有的东东
    typedef std::bidirectional_iterator_tag iterator_category;
public:
    _ht_rehash_iter() = default;
    ~_ht_rehash_iter() = default;

    _ht_rehash_iter(size_type serial, rehash_hashtable_t* ht_safe_inst) :
        serial_(serial),
        ht_rehash_instance_(ht_safe_inst)
    {
    }

    T& operator*() const
    {
        return ht_rehash_instance_->value_base_[serial_];
    }

    T* operator->() const
    {
        return ht_rehash_instance_->value_base_ + serial_;
    }

    //本来只提供前向迭代器，曾经以为使用可以使用LIST保证迭代的高效，发现不行，
    //可能要提供另外的函数
    //前向迭代器
    iterator& operator++()
    {
        Extract get_key;
        KeyEqual   equal_key;
        size_type old_serial = serial_ + 1;

        for (; old_serial < ht_rehash_instance_->hash_safe_head_->num_of_node_; ++old_serial)
        {
            //这个用无效值对象，对所有元素进行赋值，为什么使用placement new，因为更加上流一点
            //如果不是一个无效值
            if (false == equal_key(get_key(*(ht_rehash_instance_->value_base_ + old_serial)),
                get_key(ht_rehash_instance_->invalid_data_)))
            {
                serial_ = old_serial;
                break;
            }
        }

        //如果没有发现数据
        if (old_serial == ht_rehash_instance_->hash_safe_head_->num_of_node_)
        {
            serial_ = zce::SHMC_INVALID_POINT;
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
        if (ht_rehash_instance_ == it.ht_rehash_instance_ &&
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
    size_type           serial_ = zce::SHMC_INVALID_POINT;
    //具体Hash table的实例对象指针
    rehash_hashtable_t* ht_rehash_instance_ = nullptr;
};

//==================================================================================================
//每行是16,24,32列
//目前使用24列，列越多，冲突可能会越小
static const std::size_t DEF_PRIMES_LIST_NUM = 24;

static const std::size_t MIN_PRIMES_LIST_NUM = 8;

static const std::size_t MAX_PRIMES_LIST_NUM = 64;

template < class T,
    class Key,
    class Hash = std::hash<Key>,
    class Extract = shm_identity<T>,
    class KeyEqual = std::equal_to<Key>,
    class Washout = default_washout<T> >
class shm_rehash_hashtable
{
    //声明迭代器是友元
    friend class _ht_rehash_iter <T, Key, Hash, Extract, KeyEqual, Washout >;
private:
    //定义自己
    typedef shm_rehash_hashtable <T, Key, Hash, Extract, KeyEqual, Washout > self;
public:
    //定义typedef
    typedef T value_type;
    typedef Key key_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef _ht_rehash_iter <T, Key, Hash, Extract, KeyEqual, Washout> iterator;
    typedef const iterator const_iterator;
    typedef iterator::iterator_category iterator_category;
    typedef shmc_size_type size_type;

protected:

    //头部，LRU_HASH的头部结构，放在LRUHASH内存的前面
    class _ht_rehash_head
    {
    protected:
        //构造析构函数
        _ht_rehash_head() = default;
        ~_ht_rehash_head() = default;

    public:
        //内存区的长度,它可能会超过32bit，所以这儿用size_t
        std::size_t         size_of_mem_ = 0;

        //NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
        size_type           num_of_node_ = 0;

        //FREE的NODE个数
        size_type           sz_freenode_ = 0;
        //USE的NODE个数
        size_type           sz_usenode_ = 0;

        //质数列表的列数量
        size_type           row_primes_ary_ = 0;

        //
        size_type           primes_ary_[MAX_PRIMES_LIST_NUM] = { 0 };

        //淘汰起始的位置，为了避免每次淘汰都从一个地方开始
        size_type           expire_start_ = 0;
    };

public:
    //不给你使用。请使用initialize 获取实例
    shm_rehash_hashtable() = default;
    //不实现，避免误用
    shm_rehash_hashtable(const shm_rehash_hashtable&) = delete;
    const self& operator=(const self& others) = delete;
    //析构
    ~shm_rehash_hashtable()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }

protected:

    //从value中取值
    size_type bkt_num_value(const T& obj, size_type one_primes) const
    {
        Extract get_key;
        return static_cast<size_type>(bkt_num_key(get_key(obj), one_primes));
    }

    //为什么不能重载上面的函数,自己考虑一下,
    //重载的话，如果_value_type和_key_type一样，就等着哭吧 ---inmore
    size_type bkt_num_key(const Key& key, size_type one_primes) const
    {
        Hash hash_fun;
        return static_cast<size_type>(hash_fun(key) % one_primes);
    }

    //取出无效数据，在迭代器使用的时候需要
    T get_invalid_data()
    {
        return invalid_data_;
    }

    //用于自己的内部的初始化处理
    bool initialize_i(std::size_t row_primes_ary,
                      const size_type primes_list[],
                      size_type num_node,
                      std::size_t sz_alloc,
                      char* mem_addr,
                      const T& invalid_data,
                      bool if_expire,
                      bool if_restore)
    {
        _ht_rehash_head* hashhead = reinterpret_cast<_ht_rehash_head*>(mem_addr);
        //如果是恢复,数据都在内存中,对数据进行检查
        if (if_restore == true)
        {
            //是否要检查质数表和现在的表是否一致呢?我暂时选择了进行检查，
            //但如果不进行这样，即使如果质数的矩阵改变了，还是可以用共享内存中间的数据继续跑，我选择了严谨，而不是

            //质数列表的个数应该一致
            if (hashhead->row_primes_ary_ != row_primes_ary)
            {
                return false;
            }

            //检查质数队列是否一致
            size_t real_num = 0;
            for (size_type y = 0; y < row_primes_ary; ++y)
            {
                real_num += primes_list[y];
                if (hashhead->primes_ary_[y] != primes_list[y])
                {
                    return false;
                }
            }

            //检查一下恢复的内存是否正确,
            if (sz_alloc == hashhead->size_of_mem_ ||
                real_num != hashhead->num_of_node_)
            {
                return false;
            }
        }
        mem_addr_ = mem_addr;
        char* tmp_base = mem_addr_;
        hash_safe_head_ = reinterpret_cast<_ht_rehash_head*>(tmp_base);
        tmp_base = tmp_base + sizeof(_ht_rehash_head);

        value_base_ = reinterpret_cast<T*>(tmp_base);
        tmp_base = tmp_base + (sizeof(T) * (num_node));

        //初始化尺寸,如果是恢复，刚才已经比较过了
        hash_safe_head_->size_of_mem_ = sz_alloc;
        hash_safe_head_->num_of_node_ = num_node;

        //每次都将将淘汰开始的地方记录为0
        hash_safe_head_->expire_start_ = 0;

        if (if_expire)
        {
            priority_base_ = reinterpret_cast<uint32_t*>(tmp_base);
            tmp_base = tmp_base + (sizeof(uint32_t) * (num_node));
        }
        else
        {
            //还是强调一次成nullptr
            priority_base_ = nullptr;
        }

        invalid_data_ = invalid_data;

        //质数表填写进去
        hash_safe_head_->row_primes_ary_ = row_primes_ary;

        for (size_type y = 0; y < row_primes_ary; ++y)
        {
            hash_safe_head_->primes_ary_[y] = primes_list[y];
        }
        //把多余的地方填写成0
        for (size_type y = row_primes_ary; y < MAX_PRIMES_LIST_NUM; ++y)
        {
            hash_safe_head_->primes_ary_[y] = 0;
        }

        //如果不进行恢复处理
        if (if_restore == false)
        {
            //清理初始化所有的内存,有的节点为FREE
            clear(if_expire);
        }

        return true;
    }

    template<typename U>
    std::pair<iterator, bool> insert_i(U&& val,
                                       uint32_t priority,
                                       uint32_t expire_priority)
    {
        //使用函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        KeyEqual   equal_key;

        size_type idx_count = 0;
        size_type idx_no_use = SHMC_INVALID_POINT;

        //循环进行N此取模操作，
        for (size_type i = 0; i < hash_safe_head_->row_primes_ary_; ++i)
        {
            size_type idx = bkt_num_value(std::forward<U>(val), hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), (get_key(std::forward<U>(val)))))
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //如果是一个无效数据，表示一个空位置
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == SHMC_INVALID_POINT)
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
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        //如果每一列对应的位置都被流氓占用了,返回一个特殊的迭代器end,告诉前段，空间危险了
        if (SHMC_INVALID_POINT == idx_no_use)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //使用placement new进行赋值
        new (value_base_ + idx_no_use)T(std::forward<U>(val));

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        //如果插入成功了记录优先级
        priority_base_[idx_no_use] = priority;

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }

    //插入节点,内部实现
    template<typename U>
    std::pair<iterator, bool> insert_i(U&& val)
    {
        //使用函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        KeyEqual   equal_key;

        size_type idx_count = 0;
        size_type idx_no_use = SHMC_INVALID_POINT;

        //循环进行N此取模操作，
        for (size_type i = 0; i < hash_safe_head_->row_primes_ary_; ++i)
        {
            size_type idx = bkt_num_value(std::forward<U>(val), hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), (get_key(std::forward<U>(val)))))
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //如果是一个无效数据，表示一个空位置
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == SHMC_INVALID_POINT)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }

            //准备在下一个质数列里面找
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        //如果每一列对应的位置都被流氓占用了,返回一个特殊的迭代器end,告诉前段，空间危险了
        if (SHMC_INVALID_POINT == idx_no_use)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //使用placement new进行赋值
        new (value_base_ + idx_no_use)T(std::forward<U>(val));

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }
public:

    /*!
    * @brief      根据你要分配NODE数量，返回需要分配的空间大小，以及相应的质数数组信息
    * @return     std::size_t 返回值为需要分配的空间大小,因为空间可能超过4G(32bit),所以这儿用size_t
    * @param[in]  req_num     表示所需要的节点个数，
    * @param[out] real_num    最后实际分配的节点数量，注意返回的是实际INDEX长度,会增加一些
    * @param[out] prime_ary   返回的，质数数组，用于多次REHASH处理
    * @param[in]  if_expire   是否使用超时处理
    * @param[in]  row_prime_ary 质数数值的列数量,放在最后一个地方，是为了方便默认值
    * @note       内存区的构成为 define区,index区,data区,返回所需要的长度,
    */
    static std::size_t alloc_size(size_type req_num,
                                  size_type& real_num,
                                  size_type prime_ary[],
                                  bool if_expire,
                                  size_type row_prime_ary = DEF_PRIMES_LIST_NUM)
    {
        ZCE_ASSERT(row_prime_ary >= MIN_PRIMES_LIST_NUM && row_prime_ary <= MAX_PRIMES_LIST_NUM);

        std::size_t sz_alloc = 0;
        //计算得到一个Hash数组
        zce::hash_prime_ary(req_num, real_num, row_prime_ary, prime_ary);
        sz_alloc += sizeof(_ht_rehash_head);
        //给出结构的数量
        sz_alloc += sizeof(T) * (real_num);
        if (if_expire)
        {
            sz_alloc += (sizeof(uint32_t) * (real_num));
        }

        return sz_alloc;
    }

    /*!
    * @brief      返回值为需要分配的空间大小，如果是输入质数队列，用这个函数得到空间,
    * @return     std::size_t    返回值为需要分配的空间大小，用size_t原因见上
    * @param[in]  row_prime_ary  参数质数队列长度,
    * @param[in]  prime_ary    参数质数队列，
    * @param[in]  if_expire      是否超时处理
    * @param[out] real_num     返回参数,总节点个数,
    * @note
    */
    static std::size_t alloc_size(size_type row_prime_ary,
                                  const size_type prime_ary[],
                                  bool if_expire,
                                  size_type& real_num)
    {
        //列表的最大长度不能大于MAX_PRIMES_LIST_ELEMENT
        ZCE_ASSERT(row_prime_ary <= DEF_PRIMES_LIST_NUM);

        //计算总容量
        real_num = 0;
        for (size_type i = 0; i < row_prime_ary; ++i)
        {
            real_num += prime_ary[i];
        }

        ZCE_ASSERT(real_num > 0);
        //
        std::size_t sz_alloc = 0;
        //计算空间，根据头，质数队列,结构数量，优先级空间
        sz_alloc += sizeof(_ht_rehash_head);

        sz_alloc += sizeof(T) * (real_num);

        if (if_expire)
        {
            sz_alloc += sizeof(uint32_t) * (real_num);
        }

        return sz_alloc;
    }

    /*!
    * @brief      初始化，返回函数对象的指针，以后就通过这个指针操作，为什么不直接用构造函数呢，我很难回答，可能是通过alloc_size一脉相承？
    * @return     bool 是否初始化成功
    * @param[in]  req_num        表示所需要的节点个数，你需要放几个元素给你，
    * @param[out] real_num       注意这个参数会返回一个实际我分配多少尺寸给你
    * @param[in]  mem_addr       传递进来的空间指针，空间的大小通过alloc_size得到.
    * @param[in]  invalid_data   一个无效的数据数据值，因为我懒得给你开辟一个地方
    *                            记录某个数据是否使用了.所以我会将所有的数据都初始
    *                            化无效结构，无效结构，我会视作这个空间没有使用
    * @param[in]  if_expire      是否要使用淘汰功能,如果不适用，空间可以更加小一些
    * @param[in]  if_restore     是否是从一个内存中恢复空间，比如共享内存之类的恢复
    * @note       推荐使用这个函数,你做的事情要少很多
    */
    bool initialize(size_type req_num,
                    size_type& real_num,
                    char* mem_addr,
                    const T& invalid_data,
                    bool if_expire,
                    std::size_t row_prime_ary = DEF_PRIMES_LIST_NUM,
                    bool if_restore = false)
    {
        ZCE_ASSERT(mem_addr != nullptr
                   && req_num > 0
                   && row_prime_ary <= MAX_PRIMES_LIST_NUM);

        //调整,根据你的尺寸，向上找一个合适的空间
        size_type prime_ary[MAX_PRIMES_LIST_NUM];
        std::size_t sz_alloc = alloc_size(req_num,
                                          real_num,
                                          prime_ary,
                                          if_expire,
                                          row_prime_ary);

        return initialize_i(row_prime_ary,
                            prime_ary,
                            real_num,
                            sz_alloc,
                            mem_addr,
                            invalid_data,
                            if_expire,
                            if_restore);
    }

    //! 自己分配内存的初始化，参数参考上面
    bool initialize(size_type req_num,
                    size_type& real_num,
                    const T& invalid_data,
                    bool if_expire,
                    std::size_t row_prime_ary = DEF_PRIMES_LIST_NUM)
    {
        ZCE_ASSERT(req_num > 0
                   && row_prime_ary <= MAX_PRIMES_LIST_NUM);

        //调整,根据你的尺寸，向上找一个合适的空间
        size_type prime_ary[MAX_PRIMES_LIST_NUM];
        std::size_t sz_alloc = alloc_size(req_num,
                                          real_num,
                                          prime_ary,
                                          if_expire,
                                          row_prime_ary);
        char *mem_addr = new char[sz_alloc];
        slef_alloc_ = true;
        return initialize_i(row_prime_ary,
                            prime_ary,
                            real_num,
                            sz_alloc,
                            mem_addr,
                            invalid_data,
                            if_expire,
                            false);
    }

    /*!
     * @brief 你也可以传递一个质数队列，作为进行多轮HASH取模的质数队列,
     * @param row_prime_ary 质数队列数量
     * @param prime_ary     指数队列
     * @return
    */
    bool initialize(std::size_t row_prime_ary,
                    size_type prime_ary[],
                    size_type& real_num,
                    char* mem_addr,
                    const T& invalid_data,
                    bool if_expire,
                    bool if_restore = false)
    {
        ZCE_ASSERT(mem_addr != nullptr &&
                   row_prime_ary > 0);
        real_num = 0;
        std::size_t sz_alloc = alloc_size(row_prime_ary,
                                          prime_ary,
                                          if_expire,
                                          real_num);

        return initialize_i(row_prime_ary,
                            prime_ary,
                            real_num,
                            sz_alloc,
                            mem_addr,
                            invalid_data,
                            if_expire,
                            if_restore);
    }

    //! 自己分配内存的初始化，参数参考上面
    bool initialize(std::size_t row_prime_ary,
                    size_type prime_ary[],
                    size_type& real_num,
                    const T& invalid_data,
                    bool if_expire)
    {
        ZCE_ASSERT(row_prime_ary > 0);
        real_num = 0;
        std::size_t sz_alloc = alloc_size(row_prime_ary,
                                          prime_ary,
                                          if_expire,
                                          real_num);
        //自己分配一个空间，自己使用
        char *mem_addr = new char[sz_alloc];
        slef_alloc_ = true;
        return initialize_i(row_prime_ary,
                            prime_ary,
                            real_num,
                            sz_alloc,
                            mem_addr,
                            invalid_data,
                            if_expire,
                            false);
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear(bool if_expire)
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        hash_safe_head_->sz_freenode_ = hash_safe_head_->num_of_node_;
        hash_safe_head_->sz_usenode_ = 0;

        //初始化free数据区，将所有的数据初始化成无效值，用于表示一个记录是否修正了
        for (size_type i = 0; i < hash_safe_head_->num_of_node_; ++i)
        {
            //这个用无效值对象，对所有元素进行赋值，为什么使用placement new，因为更加上流一点
            new (value_base_ + i)T(invalid_data_);
        }

        //如果要记录淘汰信息
        if (if_expire)
        {
            memset(priority_base_, 0, (sizeof(uint32_t) * (hash_safe_head_->num_of_node_)));
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
            (value_base_ + pos)->~T();
        }
        bool if_expire = false;
        if (priority_base_)
        {
            if_expire = true;
        }
        clear(if_expire);
        if (slef_alloc_)
        {
            delete[] mem_addr_;
            mem_addr_ = nullptr;
        }
    }

    //得到开始的迭代器的位置，其实并不高效，少用呀，哥们，
    iterator begin()
    {
        Extract get_key;
        KeyEqual   equal_key;

        //找到第一个已经填写数据的对象，作为begin
        for (size_type i = 0; i < hash_safe_head_->num_of_node_; ++i)
        {
            //如果不是一个无效值
            if (false == equal_key(get_key(*(value_base_ + i)), get_key(invalid_data_)))
            {
                return iterator(i, this);
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
        return hash_safe_head_->sz_usenode_;
    }
    //得到容量
    size_type capacity() const
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

public:
    //插入节点
    std::pair<iterator, bool> insert(const T& val)
    {
        return insert_i(val);
    }
    std::pair<iterator, bool> insert(T&& val)
    {
        return insert_i(val);
    }

    //带优先级的插入，开始初始化的时候必须if_expire == true
    //@const _value_type &val 插入的数据
    //@uint32_t priority  插入数据优先级，
    //@uint32_t expire_priority = static_cast<uint32_t>(-1)，淘汰的优先级，默认为最大值，不进行不淘汰，这个修正来自djiang的好建议
    std::pair<iterator, bool> insert(const T& val,
                                     uint32_t priority,
                                     uint32_t expire_priority = 0)
    {
        return insert_i(val, priority, expire_priority);
    }
    std::pair<iterator, bool> insert(T&& val,
                                     uint32_t priority,
                                     uint32_t expire_priority = 0)
    {
        return insert_i(val, priority, expire_priority);
    }

public:
    //查询相应的Key是否有,返回迭代器
    //这个地方有一个陷阱,这个地方返回的迭代器++，不能给你找到相同的key的数据,而开链的HASH实现了这个功能
    iterator find(const Key& key)
    {
        //使用量函数对象,一个类单独定义一个是否更好?
        Extract get_key;
        KeyEqual equal_key;

        size_type idx_count = 0;

        //循环进行N此取模操作，
        for (size_type i = 0; i < hash_safe_head_->row_primes_ary_; ++i)
        {
            //将val取出key，取模
            size_type idx = bkt_num_key(key, hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //如果找到相同的Key函数,这个函数的语义不能这样
            if (equal_key((get_key(value_base_[idx_count])), key))
            {
                return iterator(idx_count, this);
            }

            //准备在下一个质数列里面找
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        return end();
    }

    //
    iterator find_value(const T& val)
    {
        Extract get_key;
        return find(get_key(val));
    }

    bool erase(const Key& key)
    {
        iterator iter_temp = find(key);

        if (iter_temp == end())
        {
            return false;
        }

        return erase(iter_temp);
    }

    //使用迭代器删除,尽量高效所以不用简化写法
    bool erase(const iterator& it_del)
    {
        //我不做过多的检查
        if (it_del != end())
        {
            //调用析构函数,注意这个调用只能通过指针进行，对于int，之类的类型
            (value_base_ + it_del.getserial())->~T();
            //恢复成无效数据
            new (value_base_ + it_del.getserial())T(invalid_data_);
            --(hash_safe_head_->sz_usenode_);
            ++(hash_safe_head_->sz_freenode_);

            return true;
        }

        return false;
    }

    //删除某个值
    bool erase_value(const T& val)
    {
        Extract get_key;
        return erase(get_key(val));
    }

    //激活,将激活的数据挂到LIST的最开始,淘汰使用expire,disuse
    //优先级参数可以使用当前的时间
    bool active(const Key& key,
                uint32_t priority)
    {
        iterator  iter_tmp = find(key);

        //如果找到了对应的KEY
        if (end() != iter_tmp)
        {
            priority_base_[iter_tmp.getserial()] = priority;
            return true;
        }

        return false;
    }

    //
    bool active_value(const T& val,
                      uint32_t priority)
    {
        Extract get_key;
        return active(get_key(val), priority);
    }

    //淘汰过期的数据,假设LIST中间的数据是按照过期实际排序的，这要求你传入的优先级最好是时间
    //小于等于这个优先级的数据将被淘汰
    //hope_expire_num表示你希望删除多少个值，默认为最大值,全部淘汰
    size_type expire(uint32_t expire_time,
                     size_type hope_expire_num = static_cast<size_type>(-1))
    {
        //从尾部开始检查，
        size_type expire_num = 0;
        size_type i = 0;

        for (; i < hash_safe_head_->num_of_node_ && expire_num < hope_expire_num; ++i)
        {
            size_type del_iter = (hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

            //小于等于
            if (priority_base_[del_iter] <= expire_time)
            {
                Washout wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
        }

        //从一个特殊的地方上开始
        hash_safe_head_->expire_start_ = (hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

        return expire_num;
    }

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //内存基础地址
    char* mem_addr_ = nullptr;

    //无效的数据，比如这个数据里面的KEY是永远不会出现的，一般是比如0，或者-1
    //这个地方其实是偷懒的表现，用于简化处理某个空间，是否有效
    T              invalid_data_;

    //放在共享内存头部的指针
    _ht_rehash_head* hash_safe_head_ = nullptr;

    //数据区指针
    T* value_base_ = nullptr;

    //优先级的数据指针,用32位的数据保存优先级
    uint32_t* priority_base_ = nullptr;
};

template < class T, class Key, class Hash, class keyEqual, class Washout >
using static_rehash_hashtable = zce::shm_rehash_hashtable<T, Key, Hash, keyEqual, Washout>;
}
