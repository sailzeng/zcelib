/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/list.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2008年1月18日
* @brief      可以放在共享内存里面的容器，不可以放，而且可以用来恢复。甚至共享
*             STL因为做不到这点的。哈哈。
*
* @details    写的时候参考了侯捷的《STL源码剖析》，好书，
*             建议看看。一些共用的说明在@ref zce/shm_container/common.h 文件里面，
*             请自己阅读，
*
* @note       frost教导我说，你丫赶快去谈场恋爱，我正好在听这首歌，
* 拿来记录写了17年的代码，这个感觉有点类似，所以摘抄，
*
* 《爱情有什么道理》，莫文蔚
* 其实一个人的生活也不算太坏
* 偶而有些小小的悲哀
* 我想别人也看不出来
* 即使孤单会使我伤怀
* 也会试着让自己想得开
* 对你 不知道是已经习惯还是爱
* 当初所坚持的心情
* 是不是还依然存在
* 眼看这一季就要过去
* 我的春天还没有来
* 你为何不掉过头去
* 让我自己去面对问题
* 你尝试着不露痕迹
* 告诉我爱情的道理
* 你认为值得努力的
* 是我俩之间的距离
* 喔 这一季 总算有些值得回忆
*
* 再对暗念俺女（男）程序员说明，我已经结婚。
*/

#pragma once

#include "zce/shm_container/common.h"

namespace zce
{
//============================================================================================

template <class T> class shm_list;

/*!
@brief      迭代器的封装，双向迭代器，为shm_list 提供迭起器。
            内部通过序列号，以及对象指针对进行迭代器的判等等。
@tparam     _value_type 容器处理的数据类型
*/
template <class T> class _shm_list_iterator
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
    typedef _shm_list_iterator<T> iterator;
    typedef shm_list<T> shm_list_t;
public:

    /*!
    @brief      构造函数
    @param      seq      序号
    @param      instance LIST的实例
    @note
    */
    _shm_list_iterator(size_type seq, shm_list_t* instance) :
        serial_(seq),
        list_instance_(instance)
    {
    }

    ///构造函数
    _shm_list_iterator() = default;
    ///析构函数
    ~_shm_list_iterator() = default;

    ///初始化，
    void initialize(size_type seq, shm_list_t* instance)
    {
        serial_ = seq;
        list_instance_ = instance;
    }

    ///保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_type getserial() const
    {
        return serial_;
    }

    ///迭代器的判等，
    bool operator==(const iterator& x) const
    {
        return (serial_ == x.serial_ && list_instance_ == x.list_instance_);
    }

    /// 迭代器的判定不等
    bool operator!=(const iterator& x) const
    {
        return !(*this == x);
    }

    ///提领操作
    T& operator*() const
    {
        return *(operator->());
    }
    //在多线程的环境下提供这个运送符号是不安全的,我没有加锁,原因如说明
    T* operator->() const
    {
        //
        return list_instance_->data_base() + serial_;
    }

    ///++iter，迭代器后向移动操作
    iterator& operator++()
    {
        serial_ = (list_instance_->index_base() + serial_)->idx_next_;
        return *this;
    }

    ///iter++操作，
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    /// --iter操作
    iterator& operator--()
    {
        serial_ = (list_instance_->index_base() + serial_)->idx_prev_;
        return *this;
    }

    /// iter--操作
    iterator operator--(int)
    {
        iterator tmp = *this;
        --(*this);
        return tmp;
    }

protected:

    //序列号，相对于数组下标
    size_type    serial_ = zce::SHMC_INVALID_POINT;
    //对应的list对象指针
    shm_list_t*  list_instance_ = nullptr;
};

//============================================================================================

/*!
@brief      双向链表，可以在共享内存（普通内存也行），使用，
            也可以多进程共享，（当然同步层面的事情，你自己考虑）

            额外需要的地址空间大小说明，每个node额外需要2个shm_index_t大小的空间，

@tparam     _value_type 元素类型
*/
template <class T>
class shm_list
{
    //某些函数提供给迭代器用
    friend class _shm_list_iterator<T>;
private:
    //定义自己
    typedef shm_list<T> self;
public:
    typedef _shm_list_iterator<T> iterator;
    typedef const iterator const_iterator;
    typedef iterator::iterator_category iterator_category;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef shmc_size_type size_type;

protected:
    /*!
    @brief      LIST数据区的头部，用于存放此次，对象数量，等数据
                使用的NODE数量，FREE的NODE数量等
    */
    class _shm_list_head
    {
    protected:

        ///构造函数
        _shm_list_head() = default;
        ~_shm_list_head() = default;

        //我不能对模版搞个友元，算了，开始开放出来把
    public:

        ///内存区的长度
        std::size_t      size_of_mem_ = 0;
        ///NODE结点个数
        size_type        num_of_node_ = 0;

        ///FREE的NODE个数
        size_type        size_free_node_ = 0;
        ///USE的NODE个数
        size_type        size_use_node_ = 0;
    };

public:
    //构造函数和析构函数
    shm_list() = default;
    //只定义,不实现,
    shm_list(const shm_list&) = delete;
    const self operator=(const self& others) = delete;
    ~shm_list()
    {
        if (slef_alloc_)
        {
            terminate();
        }
    }
protected:

    //得到索引的基础地址
    inline _shm_list_index* index_base()
    {
        return index_base_;
    }
    //得到数据区的基础地质
    inline  T* data_base()
    {
        return data_base_;
    }

    //分配一个NODE,将其从FREELIST中取出
    template<typename U>
    size_type create_node(U&& val)
    {
        //如果没有空间可以分配
        if (list_head_->size_free_node_ == 0)
        {
            return SHMC_INVALID_POINT;
        }

        //从链上取1个下来
        size_type node = freenode_->idx_next_;

        //从FREE链表上摘下来
        freenode_->idx_next_ = (index_base_ + node)->idx_next_;
        (index_base_ + freenode_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //用placement new生产对象
        new (data_base_ + node) T(std::forward<U>(val));

        list_head_->size_use_node_++;
        list_head_->size_free_node_--;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);

        return node;
    }
    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_type pos)
    {
        size_type freenext = freenode_->idx_next_;

        (index_base_ + pos)->idx_next_ = freenext;
        (index_base_ + pos)->idx_prev_ = (index_base_ + freenext)->idx_prev_;

        freenode_->idx_next_ = pos;

        (index_base_ + freenext)->idx_prev_ = pos;

        //调用显式的析构函数
        (data_base_ + pos)->~T();

        list_head_->size_use_node_--;
        list_head_->size_free_node_++;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);
    }

    //通过偏移序列号插入,如果你胡乱使用,不是非常安全,FREENODE也是有POS的.
    //插入在这个POS节点的前面
    template<typename U>
    size_type insert_i(size_type pos,
                       U&& val)
    {
        size_type node = create_node(std::forward<U>(val));
        if (node == SHMC_INVALID_POINT)
        {
            return SHMC_INVALID_POINT;
        }

        //将新结点挂接到队列中
        (index_base_ + node)->idx_next_ = pos;
        (index_base_ + node)->idx_prev_ = (index_base_ + pos)->idx_prev_;

        (index_base_ + (index_base_ + pos)->idx_prev_)->idx_next_ = node;
        (index_base_ + pos)->idx_prev_ = node;

        return node;
    }

    //通过迭代器插入,推荐使用这个函数,
    //插入在这个迭代器节点的前面
    template<typename U>
    std::pair<iterator, bool> insert_i(const iterator& pos, U&& val)
    {
        size_type tmp = insert_i(pos.getserial(),
                                 std::forward<U>(val));

        //插入失败
        if (SHMC_INVALID_POINT == tmp)
        {
            return std::pair<iterator, bool>(end(), false);
        }
        else
        {
            return std::pair<iterator, bool>(iterator(tmp, this), true);
        }
    }

    //通过偏移序列号删除,危险函数,自己包装正确使用
    size_type erase_i(size_type pos)
    {
        size_type nextnode = (index_base_ + pos)->idx_next_;
        size_type prevnode = (index_base_ + pos)->idx_prev_;

        (index_base_ + prevnode)->idx_next_ = nextnode;
        (index_base_ + nextnode)->idx_prev_ = prevnode;

        destroy_node(pos);

        return nextnode;
    }

public:

    //内存区的构成为 定义区,index区,data区,返回所需要的长度,
    static std::size_t alloc_size(const size_type numnode)
    {
        return  sizeof(_shm_list_head) +
            sizeof(_shm_list_index) * (numnode + ADDED_NUM_OF_INDEX) + sizeof(T) * numnode;
    }

    self* getinstance()
    {
        return this;
    }

    /*!
     * @brief 初始化，得到一个self的对象指针，
     * @param numnode    对象数量
     * @param mem_addr   内心地址指针，可以是共享内存指针
     * @param if_restore 如果是共享内存，可以尝试恢复
     * @return self，自己的对象指针
    */
    bool initialize(size_type num_node, char* mem_addr, bool if_restore = false)
    {
        ZCE_ASSERT(num_node > 0 && mem_addr_ == nullptr);
        mem_addr_ = mem_addr;
        _shm_list_head* list_head = reinterpret_cast<_shm_list_head*>(mem_addr);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(num_node) != list_head->size_of_mem_ ||
                num_node != list_head->num_of_node_)
            {
                return false;
            }
        }

        //初始化尺寸
        list_head->size_of_mem_ = alloc_size(num_node);
        list_head->num_of_node_ = num_node;

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        mem_addr_ = mem_addr;
        list_head_ = list_head;
        index_base_ = reinterpret_cast<_shm_list_index*>(
            mem_addr_ +
            sizeof(_shm_list_head));
        data_base_ = reinterpret_cast<T*>(
            mem_addr_ +
            sizeof(_shm_list_head) +
            sizeof(_shm_list_index) * (num_node + ADDED_NUM_OF_INDEX));

        //这两个家伙用于FREENODE,USENODE的使用
        freenode_ = reinterpret_cast<_shm_list_index*>
            (mem_addr_ + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (num_node));
        usenode_ = reinterpret_cast<_shm_list_index*>
            (mem_addr_ + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (num_node + 1));

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
        ZCE_ASSERT(list_head->size_use_node_ + list_head->size_free_node_ == list_head->num_of_node_);

        //打完收工
        return true;
    }

    bool initialize(size_type num_node)
    {
        std::size_t sz_alloc = alloc_size(num_node);
        //自己分配一个空间，自己使用
        char *mem_addr = new char[sz_alloc];
        slef_alloc_ = true;
        return initialize(num_node, mem_addr, false);
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

    //!清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        list_head_->size_free_node_ = list_head_->num_of_node_;
        list_head_->size_use_node_ = 0;

        //将两个队列都清理为nullptr,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        freenode_->idx_next_ = list_head_->num_of_node_;
        freenode_->idx_prev_ = list_head_->num_of_node_;

        usenode_->idx_next_ = list_head_->num_of_node_ + 1;
        usenode_->idx_prev_ = list_head_->num_of_node_ + 1;

        _shm_list_index* pindex = index_base_;

        //初始化free数据区
        for (size_type i = 0; i < list_head_->num_of_node_; ++i)
        {
            pindex->idx_next_ = (i + 1);
            pindex->idx_prev_ = (i - 1);

            //将所有的数据用FREENODE串起来
            if (0 == i)
            {
                pindex->idx_prev_ = freenode_->idx_next_;
                freenode_->idx_next_ = 0;
            }

            if (i == list_head_->num_of_node_ - 1)
            {
                pindex->idx_next_ = freenode_->idx_prev_;
                freenode_->idx_prev_ = list_head_->num_of_node_ - 1;
            }

            pindex++;
        }
    }

    //
    iterator begin()
    {
        return iterator(usenode_->idx_next_, this);
    };
    //容器应该是前闭后开的,usenode_视为为最后一个index
    iterator end()
    {
        return iterator(list_head_->num_of_node_ + 1, this);
    }

    //对不起,我不提供反向迭代器,STL的反向迭代器太精彩了,但如果我要用必须定义
    //你可以自己拿end,--折腾吧.

    //
    bool empty()
    {
        if (list_head_->size_free_node_ == list_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }
    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (list_head_->size_free_node_ == 0)
        {
            return true;
        }

        return false;
    };

    //!通过迭代器插入
    std::pair<iterator, bool> insert(const iterator& pos, const T& val)
    {
        return insert_i(pos, val);
    }
    std::pair<iterator, bool> insert(const iterator& pos, T&& val)
    {
        return insert_i(pos, val);
    }

    //通过迭代器删除
    iterator erase(const iterator& pos)
    {
        size_type tmp = erase_i(pos.getserial());
        return iterator(tmp, this);
    }

    //有了迭代器,这些函数居然如此简单,想不到吧
    bool push_front(const T& x)
    {
        std::pair<iterator, bool> result = insert(begin(), x);
        return result.second;
    }
    bool push_front(T&& x)
    {
        std::pair<iterator, bool> result = insert(begin(), std::move(x));
        return result.second;
    }
    bool push_back(const T& x)
    {
        std::pair<iterator, bool> result = insert(end(), x);
        return result.second;
    }
    bool push_back(T&& x)
    {
        std::pair<iterator, bool> result = insert(end(), std::move(x));
        return result.second;
    }
    void pop_front()
    {
        erase(begin());
    }
    void pop_back()
    {
        iterator tmp = end();
        erase(--tmp);
    }

    //将[first,last)的队列移动到pos的位置,注意:pos,first,last必须是同一个对象的数据.!!!
    //另外不要有交叉.
    void transfer(const iterator& pos, const iterator& first, const iterator& last)
    {
        //实在不愿意想,
        if (pos != last && pos != first)
        {
            size_type sqpos = pos.getserial();
            size_type sqfirst = first.getserial();
            size_type sqlast = last.getserial();
            size_type sqpos_prev = (index_base_ + sqpos)->idx_prev_;
            size_type sqfirst_prev = (index_base_ + sqfirst)->idx_prev_;
            size_type sqlast_prev = (index_base_ + sqlast)->idx_prev_;

            (index_base_ + sqlast)->idx_prev_ = sqfirst_prev;
            (index_base_ + sqfirst_prev)->idx_next_ = sqlast;
            (index_base_ + sqfirst)->idx_prev_ = sqpos_prev;
            (index_base_ + sqpos_prev)->idx_next_ = sqfirst;
            (index_base_ + sqpos)->idx_prev_ = sqlast_prev;
            (index_base_ + sqlast_prev)->idx_next_ = sqpos;
        }
    }

    void move_begin(const iterator& first, const iterator& last)
    {
        transfer(begin(), first, last);
    }

    void move_end(const iterator& first, const iterator& last)
    {
        transfer(end(), first, last);
    }

    void move_begin(const iterator& itr)
    {
        move_begin(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    void move_end(const iterator& itr)
    {
        move_end(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    //返回链表中已经有的元素个数
    size_type size()
    {
        return list_head_->size_use_node_;
    }
    //返回链表池子的容量
    size_type max_size()
    {
        return list_head_->num_of_node_;
    }
    //
    size_type free()
    {
        return list_head_->size_free_node_;
    }

    void dump()
    {
        iterator iter_tmp = begin();
        iterator iter_end = end();

        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            /*std::cout<<"getserial:"<<static_cast<unsigned int>(iter_tmp.getserial())
                << "idx_next_:"<<static_cast<unsigned int>((index_base_ + iter_tmp.getserial())->idx_next_ )
                << "idx_prev_:"<<static_cast<unsigned int>((index_base_ + iter_tmp.getserial())->idx_prev_ )<<std::endl;*/
        }
    }

protected:

    //index区要增加两个数据,一个表示
    static const size_type ADDED_NUM_OF_INDEX = 2;

protected:
    //mem_addr_是否是自己分配的，如果是自己分配的，自己负责释放
    bool slef_alloc_ = false;
    //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
    //内存基础地址
    char* mem_addr_ = nullptr;

    //LIST的头部区指针
    _shm_list_head* list_head_ = nullptr;
    //索引数据区指针,
    _shm_list_index* index_base_ = nullptr;
    //数据区起始指针,
    T* data_base_ = nullptr;

    //FREE NODE的头指针,N+1个索引位表示
    _shm_list_index* freenode_ = nullptr;
    //USE NODE的头指针,N+2个索引位表示
    _shm_list_index* usenode_ = nullptr;
};

//定义一个别名，shm_list也可以作为一个static_list用
template<typename T>
using static_list = zce::shm_list<T>;
}
