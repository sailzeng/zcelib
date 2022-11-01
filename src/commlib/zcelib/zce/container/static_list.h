namespace zce
{
//无效的容器
constexpr size_t CONTAINER_INVALID_POINT = (size_t)(-1);

class static_list_index
{
public:
    ///LiST后驱索引，
    size_t  idx_next_;
    ///LiST的前驱索引，
    size_t  idx_prev_;

    static_list_index() :
        idx_next_(static_cast<size_t>(-1)),
        idx_prev_(static_cast<size_t>(-1))
    {
    }
    static_list_index(const size_t& nxt, const size_t& prv) :
        idx_next_(nxt),
        idx_prev_(prv)
    {
    }
    ~static_list_index()
    {
    }
};

//============================================================================================

template <class T> class static_list;

/*!
@brief      迭代器的封装，双向迭代器，为static_list 提供迭起器。
            内部通过序列号，以及对象指针对进行迭代器的判等等。
@tparam     T 容器处理的数据类型
*/
template <class T> class static_list_iter
{
    typedef static_list_iter<T> iterator;

    //迭代器萃取器所有的东东
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
    typedef T value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:

    /*!
    @brief      构造函数
    @param      seq      序号
    @param      instance LIST的实例
    @note
    */
    static_list_iter(size_t seq, static_list<T>* instance) :
        serial_(seq),
        list_instance_(instance)
    {
    }

    ///构造函数
    static_list_iter() = default;
    ///析构函数
    ~static_list_iter() = default;

    ///初始化，
    void initialize(size_t seq, static_list<T>* instance)
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
        return list_instance_->getdatabase() + serial_;
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
    size_t                  serial_ = zce::CONTAINER_INVALID_POINT;
    //对应的list对象指针
    static_list<T>*         list_instance_ = nullptr;
};

//============================================================================================

/*!
@brief      双向链表，容量固定，自己内部请内存空间，使用，
            容量固定，避免每次都申请内存。用于一些特殊场景
            多线程使用需要自己考虑同步问题。
            额外需要的地址空间大小说明，需要两个static_list_index，
            每个node额外需要2个size_t大小的空间，

@tparam     T 元素类型
*/
template <class T>
class static_list
{
private:

    //定义自己
    typedef static_list<T> self;

public:
    ///定义迭代器
    typedef static_list_iter<T> iterator;

    //某些函数提供给迭代器用
    friend class static_list_iter<T>;

public:

    ///如果在共享内存使用,没有new,所以统一用initialize 初始化
    ///这个函数,不给你用,就是不给你用
    static_list() = default;
    static_list(size_t num_node)
    {
        assert(num_node > 0);
        initialize(num_node);
    }

    ~static_list()
    {
        terminate();
    }

    //只定义,不实现,
    static_list(const static_list&) = default;
    const self operator=(const self& others) = delete;

public:
    //初始化
    bool initialize(const size_t num_node)
    {
        assert(num_node > 0 && mem_base_ == nullptr);
        //初始化尺寸
        size_of_mem_ = sizeof(static_list_index) * (num_node + TWO_USE) + sizeof(T) * num_node;
        capacity_of_list_ = num_node;

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        mem_base_ = new char[size_of_mem_];
        if (!mem_base_)
        {
            return false;
        }
        index_base_ = reinterpret_cast<static_list_index*>(mem_base_);
        data_base_ = reinterpret_cast<T*>(mem_base_ +
                                          sizeof(static_list_index) * (num_node + TWO_USE));
        free_node_ = &index_base_[num_node];
        use_node_ = &index_base_[num_node + 1];
        clear();
        return true;
    }

    void terminate()
    {
        clear();
        //清理现场
        if (mem_base_)
        {
            delete[]mem_base_;
            mem_base_ = nullptr;
        }
        capacity_of_list_ = 0;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        size_free_node_ = capacity_of_list_;
        size_use_node_ = 0;

        //将两个队列都清理为nullptr,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        free_node_->idx_next_ = capacity_of_list_;
        free_node_->idx_prev_ = capacity_of_list_;

        use_node_->idx_next_ = capacity_of_list_ + 1;
        use_node_->idx_prev_ = capacity_of_list_ + 1;

        static_list_index* pindex = index_base_;

        //初始化free数据区
        for (size_t i = 0; i < capacity_of_list_; ++i)
        {
            pindex->idx_next_ = (i + 1);
            pindex->idx_prev_ = (i - 1);

            //将所有的数据用FREENODE串起来
            if (0 == i)
            {
                pindex->idx_prev_ = free_node_->idx_next_;
                free_node_->idx_next_ = 0;
            }

            if (i == capacity_of_list_ - 1)
            {
                pindex->idx_next_ = free_node_->idx_prev_;
                free_node_->idx_prev_ = capacity_of_list_ - 1;
            }

            pindex++;
        }
    }

    //
    iterator begin()
    {
        return iterator(use_node_->idx_next_, this);
    };
    //容器应该是前闭后开的,use_node_视为为最后一个index
    iterator end()
    {
        return iterator(capacity_of_list_ + 1, this);
    }

    //对不起,我不提供反向迭代器,STL的反向迭代器太精彩了,但如果我要用必须定义
    //你可以自己拿end,--折腾吧.

    //
    bool empty()
    {
        if (size_free_node_ == capacity_of_list_)
        {
            return true;
        }

        return false;
    }
    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (size_free_node_ == 0)
        {
            return true;
        }

        return false;
    };

    //通过迭代器插入,推荐使用这个函数,
    //插入在这个迭代器节点的前面
    std::pair<iterator, bool> insert(const iterator& pos, const T& val)
    {
        size_t tmp = insert(pos.getserial(), val);

        //插入失败
        if (CONTAINER_INVALID_POINT == tmp)
        {
            return std::pair<iterator, bool>(end(), false);
        }
        else
        {
            return std::pair<iterator, bool>(iterator(tmp, this), true);
        }
    }

public:
    //通过迭代器删除
    iterator erase(const iterator& pos)
    {
        size_t tmp = erase(pos.getserial());
        return iterator(tmp, this);
    }

    //有了迭代器,这些函数居然如此简单,想不到吧
    bool push_front(const T& x)
    {
        std::pair<iterator, bool> tmp = insert(begin(), x);
        return tmp.second;
    }

    bool push_back(const T& x)
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
    void transfer(const iterator& pos, const iterator& first, const iterator& last)
    {
        //实在不愿意想,
        if (pos != last && pos != first)
        {
            size_t sqpos = pos.getserial();
            size_t sqfirst = first.getserial();
            size_t sqlast = last.getserial();
            size_t sqpos_prev = (index_base_ + sqpos)->idx_prev_;
            size_t sqfirst_prev = (index_base_ + sqfirst)->idx_prev_;
            size_t sqlast_prev = (index_base_ + sqlast)->idx_prev_;

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
    size_t size()
    {
        return size_use_node_;
    }
    //返回链表池子的容量
    size_t capacity()
    {
        return capacity_of_list_;
    }
    //
    size_t free()
    {
        return size_free_node_;
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

    //分配一个NODE,将其从FREELIST中取出
    size_t create_node(const T& val)
    {
        //如果没有空间可以分配
        if (size_free_node_ == 0)
        {
            return CONTAINER_INVALID_POINT;
        }

        //从链上取1个下来
        size_t node = free_node_->idx_next_;

        //从FREE链表上摘下来
        free_node_->idx_next_ = (index_base_ + node)->idx_next_;
        (index_base_ + free_node_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //用placement new生产对象
        new (data_base_ + node) T(val);

        size_use_node_++;
        size_free_node_--;

        //assert(szusenode_ + szfreenode_ == numofnode_);

        return node;
    }
    //释放一个NODE,将其归还给FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = free_node_->idx_next_;

        (index_base_ + pos)->idx_next_ = freenext;
        (index_base_ + pos)->idx_prev_ = (index_base_ + freenext)->idx_prev_;

        free_node_->idx_next_ = pos;

        (index_base_ + freenext)->idx_prev_ = pos;

        //调用显式的析构函数
        (data_base_ + pos)->~T();

        size_use_node_--;
        size_free_node_++;

        //assert(szusenode_ + szfreenode_ == numofnode_);
    }

    //得到索引的基础地址
    inline static_list_index* index_base()
    {
        return index_base_;
    }
    //得到数据区的基础地质
    inline  T* getdatabase()
    {
        return data_base_;
    }

    //通过偏移序列号插入,如果你胡乱使用,不是非常安全,FREENODE也是有POS的.
    //插入在这个POS节点的前面
    size_t insert(size_t pos, const T& val)
    {
        size_t node = create_node(val);

        if (node == CONTAINER_INVALID_POINT)
        {
            return CONTAINER_INVALID_POINT;
        }

        //将新结点挂接到队列中
        (index_base_ + node)->idx_next_ = pos;
        (index_base_ + node)->idx_prev_ = (index_base_ + pos)->idx_prev_;

        (index_base_ + (index_base_ + pos)->idx_prev_)->idx_next_ = node;
        (index_base_ + pos)->idx_prev_ = node;

        return node;
    }

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

protected:

    //index区要增加两个数据,一个表示FREE，一个标识USE，
    static const size_t TWO_USE = 2;
protected:

    char* mem_base_ = nullptr;

    //!使用的内存大小
    size_t               size_of_mem_ = 0;

    ///NODE结点个数,容量
    size_t               capacity_of_list_ = 0;

    ///FREE的NODE个数
    size_t               size_free_node_ = 0;
    ///USE的NODE个数
    size_t               size_use_node_ = 0;

    //索引数据区指针,
    static_list_index* index_base_ = nullptr;
    //数据区起始指针,
    T* data_base_ = nullptr;

    //FREE NODE的头指针,N+1个索引位表示
    static_list_index* free_node_ = nullptr;
    //USE NODE的头指针,N+2个索引位表示
    static_list_index* use_node_ = nullptr;
};
};
