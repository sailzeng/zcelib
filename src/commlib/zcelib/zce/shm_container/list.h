/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_list.h
* @author     Sailzeng <sailerzeng@gmail.com>
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

#ifndef ZCE_LIB_SHARE_MEM_LIST_H_
#define ZCE_LIB_SHARE_MEM_LIST_H_

#include "zce/shm_container/common.h"

namespace zce
{

//============================================================================================

template <class _value_type> class smem_list;

/*!
@brief      迭代器的封装，双向迭代器，为smem_list 提供迭起器。
            内部通过序列号，以及对象指针对进行迭代器的判等等。
@tparam     _value_type 容器处理的数据类型
*/
template <class _value_type> class _shm_list_iterator
{
    typedef _shm_list_iterator<_value_type> iterator;

    //迭代器萃取器所有的东东
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:

    /*!
    @brief      构造函数
    @param      seq      序号
    @param      instance LIST的实例
    @note
    */
    _shm_list_iterator<_value_type>(size_t seq, smem_list<_value_type> *instance):
        serial_(seq),
        list_instance_(instance)
    {
    }

    ///构造函数
    _shm_list_iterator<_value_type>():
        serial_(_shm_memory_base::_INVALID_POINT),
        list_instance_(NULL)
    {
    }
    ///析构函数
    ~_shm_list_iterator<_value_type>()
    {
    }

    ///初始化，
    void initialize(size_t seq, smem_list<_value_type> *instance)
    {
        serial_ = seq;
        list_instance_ = instance;
    }

    ///保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

    ///迭代器的判等，
    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && list_instance_ == x.list_instance_ );
    }

    /// 迭代器的判定不等
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    ///提领操作
    _value_type &operator*() const
    {
        return *(operator->());
    }
    //在多线程的环境下提供这个运送符号是不安全的,我没有加锁,原因如说明
    _value_type *operator->() const
    {
        //
        return list_instance_->getdatabase() + serial_ ;
    }

    ///++iter，迭代器后向移动操作
    iterator &operator++()
    {
        serial_ = (list_instance_->getindexbase() + serial_)->idx_next_;
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
    iterator &operator--()
    {
        serial_ = (list_instance_->getindexbase() + serial_)->idx_prev_;
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
    size_t                  serial_;
    //对应的list对象指针
    smem_list<_value_type> *list_instance_;
};

//============================================================================================

/*!
@brief      LIST数据区的头部，用于存放此次，对象数量，等数据
            使用的NODE数量，FREE的NODE数量等
*/
class _shm_list_head
{
protected:

    ///构造函数
    _shm_list_head():
        size_of_mmap_(0),
        num_of_node_(0),
        size_free_node_(0),
        size_use_node_(0)
    {
    }

    //析构函数
    ~_shm_list_head()
    {
    }

    //我不能对模版搞个友元，算了，开始开放出来把
public:

    ///内存区的长度
    size_t               size_of_mmap_;
    ///NODE结点个数
    size_t               num_of_node_;

    ///FREE的NODE个数
    size_t               size_free_node_;
    ///USE的NODE个数
    size_t               size_use_node_;
};

//============================================================================================

/*!
@brief      双向链表，可以在共享内存（普通内存也行），使用，
            也可以多进程共享，（当然同步层面的事情，你自己考虑）

            额外需要的地址空间大小说明，每个node额外需要2个shm_index_t大小的空间，

@tparam     _value_type 元素类型
*/
template <class _value_type> class smem_list :
    public _shm_memory_base
{
public:

    ///定义迭代器
    typedef _shm_list_iterator<_value_type> iterator;

    //某些函数提供给迭代器用
    friend class _shm_list_iterator<_value_type>;

public:

    ///如果在共享内存使用,没有new,所以统一用initialize 初始化
    ///这个函数,不给你用,就是不给你用
    smem_list<_value_type>(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(pmmap),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    smem_list<_value_type>():
        _shm_memory_base(NULL),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    ~smem_list<_value_type>()
    {
    }

    //只定义,不实现,
    const smem_list<_value_type> &operator=(const smem_list<_value_type> &others);

protected:

    //得到索引的基础地址
    inline _shm_list_index *getindexbase()
    {
        return index_base_;
    }
    //得到数据区的基础地质
    inline  _value_type *getdatabase()
    {
        return data_base_;
    }

    //分配一个NODE,将其从FREELIST中取出
    size_t create_node(const _value_type &val)
    {
        //如果没有空间可以分配
        if (list_head_->size_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //从链上取1个下来
        size_t node = freenode_->idx_next_;

        //从FREE链表上摘下来
        freenode_->idx_next_ = (index_base_ + node)->idx_next_;
        (index_base_ + freenode_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //用placement new生产对象
        new (data_base_ + node) _value_type(val) ;

        list_head_->size_use_node_  ++;
        list_head_->size_free_node_ --;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);

        return node;
    }
    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = freenode_->idx_next_;

        (index_base_ + pos)->idx_next_ = freenext;
        (index_base_ + pos)->idx_prev_ = (index_base_ + freenext)->idx_prev_;

        freenode_->idx_next_ = pos;

        (index_base_ + freenext)->idx_prev_ = pos;

        //调用显式的析构函数
        (data_base_ + pos)->~_value_type();

        list_head_->size_use_node_  --;
        list_head_->size_free_node_ ++;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);
    }

public:

    //内存区的构成为 定义区,index区,data区,返回所需要的长度,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_list_head)  + sizeof(_shm_list_index) * (numnode + ADDED_NUM_OF_INDEX) + sizeof(_value_type) * numnode ;
    }

    smem_list<_value_type> *getinstance()
    {
        return this;
    }

    //初始化
    static smem_list<_value_type> *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_list_head *listhead = reinterpret_cast<_shm_list_head *>(pmmap);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (getallocsize(numnode) != listhead->size_of_mmap_ ||
                numnode != listhead->num_of_node_ )
            {
                return NULL;
            }
        }

        //初始化尺寸
        listhead->size_of_mmap_ = getallocsize(numnode);
        listhead->num_of_node_ = numnode;

        smem_list<_value_type> *instance = new smem_list<_value_type>();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        instance->smem_base_ = pmmap;
        instance->list_head_ = listhead;
        instance->index_base_ = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head));
        instance->data_base_  = reinterpret_cast<_value_type *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode + ADDED_NUM_OF_INDEX) );

        //这两个家伙用于FREENODE,USENODE的使用
        instance->freenode_   = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode ));
        instance->usenode_    = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode + 1));

        //
        if (if_restore == false)
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }

        assert(listhead->size_use_node_ + listhead->size_free_node_ == listhead->num_of_node_);

        //打完收工
        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        list_head_->size_free_node_ = list_head_->num_of_node_;
        list_head_->size_use_node_ = 0;

        //将两个队列都清理为NULL,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        freenode_->idx_next_ = list_head_->num_of_node_ ;
        freenode_->idx_prev_ = list_head_->num_of_node_ ;

        usenode_->idx_next_ = list_head_->num_of_node_ + 1;
        usenode_->idx_prev_ = list_head_->num_of_node_ + 1;

        _shm_list_index *pindex = index_base_;

        //初始化free数据区
        for (size_t i = 0; i < list_head_->num_of_node_ ; ++i )
        {

            pindex->idx_next_ = (i + 1) ;
            pindex->idx_prev_ = (i - 1) ;

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
        if (list_head_->size_free_node_ == 0 )
        {
            return true;
        }

        return false;
    };

protected:
    //通过偏移序列号插入,如果你胡乱使用,不是非常安全,FREENODE也是有POS的.
    //插入在这个POS节点的前面
    size_t insert(size_t pos, const _value_type &val)
    {
        size_t node = create_node(val);

        if (node == _INVALID_POINT)
        {
            return _INVALID_POINT;
        }

        //将新结点挂接到队列中
        (index_base_ + node)->idx_next_ = pos;
        (index_base_ + node)->idx_prev_ = (index_base_ + pos)->idx_prev_;

        (index_base_ + (index_base_ + pos)->idx_prev_)->idx_next_ = node;
        (index_base_ + pos)->idx_prev_ = node;

        return node;
    }

public:

    //通过迭代器插入,推荐使用这个函数,
    //插入在这个迭代器节点的前面
    std::pair<iterator, bool> insert(const iterator &pos, const _value_type &val)
    {
        size_t tmp = insert(pos.getserial(), val);

        //插入失败
        if (_INVALID_POINT == tmp)
        {
            return std::pair<iterator, bool>(end(), false);
        }
        else
        {
            return std::pair<iterator, bool>(iterator(tmp, this), true);
        }
    }

protected:

    //通过偏移序列号删除,危险函数,自己包装正确使用
    size_t erase(size_t pos)
    {
        size_t nextnode = (index_base_ + pos)->idx_next_;
        size_t prevnode = (index_base_ + pos)->idx_prev_;

        (index_base_ + prevnode)->idx_next_ = nextnode;
        (index_base_ + nextnode)->idx_prev_ = prevnode;

        destroy_node(pos);

        return nextnode;
    }

public:
    //通过迭代器删除
    iterator erase(const iterator &pos)
    {
        size_t tmp = erase(pos.getserial());
        return iterator(tmp, this);
    }

    //有了迭代器,这些函数居然如此简单,想不到吧
    bool push_front(const _value_type &x)
    {
        std::pair<iterator, bool> tmp = insert(begin(), x);
        return tmp.second;
    }

    bool push_back(const _value_type &x)
    {
        std::pair<iterator, bool> tmp = insert(end(), x);
        return tmp.second;
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
    void transfer(const iterator &pos, const iterator &first, const iterator &last)
    {
        //实在不愿意想,
        if (pos != last && pos != first )
        {
            size_t sqpos   = pos.getserial();
            size_t sqfirst = first.getserial();
            size_t sqlast = last.getserial();
            size_t sqpos_prev =  (index_base_ + sqpos)->idx_prev_;
            size_t sqfirst_prev = (index_base_ + sqfirst)->idx_prev_;
            size_t sqlast_prev = (index_base_ + sqlast)->idx_prev_;

            (index_base_ + sqlast)->idx_prev_ = sqfirst_prev;
            (index_base_ + sqfirst_prev)->idx_next_  = sqlast;
            (index_base_ + sqfirst)->idx_prev_ = sqpos_prev;
            (index_base_ + sqpos_prev)->idx_next_ = sqfirst;
            (index_base_ + sqpos)->idx_prev_ = sqlast_prev;
            (index_base_ + sqlast_prev)->idx_next_ = sqpos;
        }
    }

    void move_begin(const iterator &first, const iterator &last)
    {
        transfer(begin(), first, last);
    }

    void move_end(const iterator &first, const iterator &last)
    {
        transfer(end(), first, last);
    }

    void move_begin(const iterator &itr)
    {
        move_begin(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    void move_end(const iterator &itr)
    {
        move_end(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    //返回链表中已经有的元素个数
    size_t size()
    {
        return list_head_->size_use_node_;
    }
    //返回链表池子的容量
    size_t capacity()
    {
        return list_head_->num_of_node_;
    }
    //
    size_t sizefreenode()
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
    static const size_t ADDED_NUM_OF_INDEX = 2;

protected:

    //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算

    //LIST的头部区指针
    _shm_list_head      *list_head_;
    //索引数据区指针,
    _shm_list_index     *index_base_;
    //数据区起始指针,
    _value_type          *data_base_;

    //FREE NODE的头指针,N+1个索引位表示
    _shm_list_index     *freenode_;
    //USE NODE的头指针,N+2个索引位表示
    _shm_list_index     *usenode_;
};

};

#endif //ZCE_LIB_SHARE_MEM_LIST_H_

