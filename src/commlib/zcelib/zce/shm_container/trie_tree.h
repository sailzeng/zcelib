/*!
* @copyright  2004-2018  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_trie_tree.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2018年9月9日
* @brief      一个字典树(OR前缀树)的实现。字典树在在HW的计费的时候实现过。当时主要为了做计费。
*             时隔10年后，在互动影视有发现了一次运用场景。而且我按目标来看。可能也需要一个长度
*             限制。才好用。
*
* @details
*
*
*
* @note
*
*   《橄榄树》 齐豫
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   流浪远方 流浪
*   为了天空飞翔的小鸟
*   为了山间轻流的小溪
*   为了宽阔的草原
*   流浪远方 流浪
*   还有还有
*   为了梦中的橄榄树 橄榄树
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   为什么流浪远方
*   为了我梦中的橄榄树
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   流浪远方 流浪 [4]
*
*/

#pragma once



namespace zce
{

//============================================================================================

/*!
@brief      TRIE TREE数据区的头部，用于存放此次，对象数量，等数据
            使用的NODE数量，FREE的NODE数量等
*/
struct _shm_trie_tree_head
{
    //我不能对模版搞个友元，算了，开始开放出来把
public:

    ///内存区的长度
    uint32_t         size_of_mmap_ = 0;
    ///NODE结点个数
    uint32_t         num_of_node_ = 0;


    //FREE NODE的头指针,N+1个索引位表示
    uint32_t         free_index_start_ = static_cast<uint32_t>(-1);

    uint32_t         use_index_start_ = static_cast<uint32_t>(-1);

    ///FREE的NODE个数
    uint32_t         size_free_node_ = 0;
    ///USE的NODE个数
    uint32_t         size_use_node_ = 0;
};

//============================================================================================

///TRIE TREE的树形NODE的索引
template <class _meta_type>
class _shm_trie_tree_index
{
public:
    //数据区起始指针,
    _meta_type  meta_data_;
    ///兄节点
    uint32_t    idx_brother_ = static_cast<uint32_t>(-1);
    ///子节点
    uint32_t    idx_children_ = static_cast<uint32_t>(-1);

};


//============================================================================================
/*!
@brief      字典树(OR前缀树)，可以在共享内存（普通内存也行），一块内存中，使用，
            也可以多进程共享，（当然同步层面的事情，你自己考虑）

            额外需要的地址空间大小说明，每个node额外需要2个shm_index_t大小的空间，

@tparam     _meta_type 元类型，每个NODE包括的数据
*/
template <class _meta_type> class smem_trie_tree
{
protected:

    //index区要增加两个数据,一个表示
    static const size_t ADDED_NUM_OF_ROOT = 1;

protected:


    //LIST的头部区指针
    _shm_trie_tree_head             *trie_head_;

    _shm_trie_tree_index            *trie_tree_base_;




public:

    ///如果在共享内存使用,没有new,所以统一用initialize 初始化
    ///这个函数,不给你用,就是不给你用
    smem_trie_tree<_meta_type>(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(pmmap),
        trie_head_(NULL),
        trie_tree_base_(NULL)
    {
    }

    smem_trie_tree<_meta_type>():
        _shm_memory_base(NULL),
        trie_head_(NULL),
        trie_tree_base_(NULL)
    {
    }

    ~smem_trie_tree<_meta_type>()
    {
    }

    //只定义,不实现,
    const smem_trie_tree<_meta_type>& operator=(const smem_trie_tree<_meta_type>& others) = delete;

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
    size_t create_node(const _meta_type &val)
    {
        //如果没有空间可以分配
        if (list_head_->size_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //从链上取1个下来
        size_t node = freenode_->idx_next_;

        freenode_->idx_next_ = (index_base_ + node)->idx_next_;
        //
        (index_base_ + freenode_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //用placement new生产对象
        new (data_base_ + node) _meta_type(val) ;

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
    }




};

};

#endif //ZCE_LIB_SHARE_MEM_TRIE_TREE_H_

