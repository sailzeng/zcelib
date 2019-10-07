



#include "zce_predefine.h"
#include "zce_lockfree_kfifo.h"


namespace zce::lockfree
{

/*********************************************************************************
class dequechunk_node
*********************************************************************************/


//nodelen 结点的长度,包括sizeofnode的长度
//重载new运算符,得到一个变长的数据
void   *dequechunk_node::operator new   (size_t, size_t nodelen)
{
    //assert(nodelen > sizeof (dequechunk_node));
    if (nodelen < sizeof (dequechunk_node))
    {
        nodelen = sizeof (dequechunk_node);
    }

    //
    void *ptr = ::new unsigned char [nodelen ];

#ifdef  DEBUG
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr, 0, nodelen);
#endif
    //
    ((dequechunk_node *)ptr)->size_of_node_ = (unsigned int)nodelen;

    return ptr;
};


void dequechunk_node::operator delete (void *ptrframe)
{
    unsigned char *ptr = (unsigned char *)ptrframe;
    delete[] ptr;
}

/*********************************************************************************
class shm_dequechunk
*********************************************************************************/
//构造函数和析构函数都不是打算给你使用的,
shm_dequechunk::shm_dequechunk():
    dequechunk_head_(NULL),
    dequechunk_database_(NULL),
    line_wrap_nodeptr_(NULL)
{
}

shm_dequechunk::~shm_dequechunk()
{
    if (line_wrap_nodeptr_)
    {
        delete line_wrap_nodeptr_;
        line_wrap_nodeptr_ = NULL;
    }
}


size_t shm_dequechunk::getallocsize(const size_t szdeque)
{
    return  sizeof(_shm_dequechunk_head) + szdeque + JUDGE_FULL_INTERVAL ;
}


//根据参数初始化
shm_dequechunk *shm_dequechunk::initialize(size_t size_of_deque,
                                           size_t max_len_node,
                                           char *pmmap,
                                           bool if_restore )
{
    //必须大于间隔长度
    if (size_of_deque <= sizeof(size_t) + 16)
    {
        return NULL;
    }

    //
    _shm_dequechunk_head *dequechunk_head = reinterpret_cast<_shm_dequechunk_head *>(pmmap);

    //如果是恢复，检查几个值是否相等
    if (if_restore == true)
    {
        //坚持关键数据是否一致
        if (dequechunk_head->size_of_mmap_ != getallocsize(size_of_deque)
            || dequechunk_head->size_of_deque_ != size_of_deque + JUDGE_FULL_INTERVAL
            || dequechunk_head->max_len_node_ != max_len_node )
        {
            return NULL;
        }

    }

    dequechunk_head->size_of_mmap_ = getallocsize(size_of_deque);
    dequechunk_head->size_of_deque_ = size_of_deque + JUDGE_FULL_INTERVAL;
    dequechunk_head->max_len_node_ = max_len_node;

    shm_dequechunk *dequechunk  = new shm_dequechunk();

    //得到空间大小
    dequechunk->smem_base_ = pmmap;

    //
    dequechunk->dequechunk_head_ = dequechunk_head;
    //
    dequechunk->dequechunk_database_ = pmmap + sizeof(_shm_dequechunk_head);

    if (if_restore == false)
    {
        dequechunk->clear();
    }

    return dequechunk;
}

//清理成没有使用过的状态
void shm_dequechunk::clear()
{
    //
    dequechunk_head_->deque_begin_ = 0;
    dequechunk_head_->deque_end_ = 0 ;
    memset(dequechunk_database_, 0, dequechunk_head_->size_of_deque_);
}


//得到两个关键指针的快照
//这个操作可以不用加锁基于一点,32位操作系统中的32位整数操作是原子操作
void shm_dequechunk::snap_getpoint(size_t &pstart, size_t &pend)
{
    pstart = dequechunk_head_->deque_begin_;
    pend   = dequechunk_head_->deque_end_;
    return;
}



//将一个NODE放入尾部
bool shm_dequechunk::push_end(const dequechunk_node *node)
{
    //粗略的检查,如果长度不合格,返回不成功
    if (node->size_of_node_ < dequechunk_node::MIN_SIZE_DEQUE_CHUNK_NODE ||
        node->size_of_node_ > dequechunk_head_->max_len_node_ )
    {
        return false;
    }

    //检查队列的空间是否够用
    if (free_size() < node->size_of_node_ )
    {
        return false;
    }

    //如果空间足够
    char *pend = dequechunk_database_ + dequechunk_head_->deque_end_;

    //如果绕圈
    if (pend + node->size_of_node_ >= dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_end_ ;
        size_t second = node->size_of_node_ - first;
        memcpy(pend, reinterpret_cast<const char *>(node), first);
        memcpy(dequechunk_database_, reinterpret_cast<const char *>(node) + first, second);
        dequechunk_head_->deque_end_ = second;
    }
    //如果可以一次拷贝完成
    else
    {
        memcpy(pend, reinterpret_cast<const char *>(node), node->size_of_node_);
        dequechunk_head_->deque_end_ += node->size_of_node_;
    }

    return true;
}



//将队列一个NODE拷贝取出,
//如果缓冲自己分配,最好准备一个够用的缓冲使用
//返回的节点区,要求node!=NULL,已经分配好了数据区
bool shm_dequechunk::pop_front(dequechunk_node *const node)
{
    assert(node != NULL);

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    assert(tmplen > 0);
    assert(tmplen < 64 * 1024);
    assert(dequechunk_head_->deque_begin_ <= dequechunk_head_->size_of_deque_);

    //如果被分为2截
    if (pbegin + tmplen > dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_begin_ ;
        size_t second = tmplen - first;
        memcpy(reinterpret_cast<char *>(node), pbegin, first);
        memcpy(reinterpret_cast<char *>(node) + first, dequechunk_database_, second);
        dequechunk_head_->deque_begin_ = second ;
    }
    else
    {
        memcpy(reinterpret_cast<char *>(node), pbegin, tmplen);
        dequechunk_head_->deque_begin_ += node->size_of_node_;
        assert(dequechunk_head_->deque_begin_ <= dequechunk_head_->size_of_deque_);
    }

    assert(dequechunk_head_->deque_begin_ <= dequechunk_head_->size_of_deque_);

    return true;
}




//将队列一个NODE从队首部取出,我根据node的长度帮你分配空间,要求new_node=NULL,表示你要函数帮你分配缓冲,
bool shm_dequechunk::pop_front_new(dequechunk_node *&new_node)
{
    assert(new_node == NULL);

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    size_t tmplen = get_front_len();
    new_node = new (tmplen) dequechunk_node;

    //这样写会有一些重复调用，但是我觉得这个地方性能不会是问题。
    return pop_front(new_node);
}



//将队列一个NODE读取复制出来,但是不是取出，
bool shm_dequechunk::read_front(dequechunk_node *const node)
{
    assert(node != NULL);

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    //如果空间足够
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //如果被分为2截
    if (pbegin + tmplen > dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_begin_ ;
        size_t second = tmplen - first;
        memcpy(reinterpret_cast<char *>(node), pbegin, first);
        memcpy(reinterpret_cast<char *>(node) + first, dequechunk_database_, second);
    }
    else
    {
        memcpy(reinterpret_cast<char *>(node), pbegin, tmplen);
    }

    return true;
}


//读取队列的第一个NODE，我根据node的长度帮你分配空间,要求new_node=NULL,表示你要函数帮你分配缓冲,
bool shm_dequechunk::read_front_new(dequechunk_node *&new_node)
{
    assert(new_node == NULL);

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    size_t tmplen = get_front_len();
    new_node = new (tmplen) dequechunk_node;

    return read_front(new_node);
}

//读取队列的第一个NODE的指针，如果是折行的数据会特殊处理
bool shm_dequechunk::read_front_ptr(const dequechunk_node *&node_ptr)
{
    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    //如果空间足够
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //如果被分为2截,折行了，用line_wrap_nodeptr_，保存数据，提交给上层，
    if (pbegin + tmplen > dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_begin_ ;
        size_t second = tmplen - first;

        //如果line_wrap_nodeptr_没有空间，现分配
        if (line_wrap_nodeptr_ == NULL)
        {
            line_wrap_nodeptr_ = new (dequechunk_head_->max_len_node_) dequechunk_node;
        }

        //将两截数据保存到line_wrap_nodeptr_中，给上层调用者用，让上层仍然使用一个连续的空间
        memcpy(reinterpret_cast<char *>(line_wrap_nodeptr_), pbegin, first);
        memcpy(reinterpret_cast<char *>(line_wrap_nodeptr_) + first, dequechunk_database_, second);

        node_ptr = reinterpret_cast<const dequechunk_node *>(line_wrap_nodeptr_);
    }
    else
    {
        node_ptr = reinterpret_cast<const dequechunk_node *>(pbegin);
    }

    return true;
}

//丢弃队列前面的第一个NODE
bool shm_dequechunk::discard_frond()
{

    //检查是否为空
    if (empty() == true)
    {
        return false;
    }

    //如果空间足够
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //如果要求帮使用者分配,切记释放,

    //如果被分为2截
    if (pbegin + tmplen > dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_begin_ ;
        size_t second = tmplen - first;
        dequechunk_head_->deque_begin_ = second ;
    }
    else
    {
        dequechunk_head_->deque_begin_ += tmplen;
    }

    return true;
}


//FREE的尺寸,空闲的空间有多少
size_t shm_dequechunk::free_size()
{
    //取快照
    size_t pstart, pend, szfree;
    snap_getpoint(pstart, pend);

    //计算尺寸
    if (pstart == pend )
    {
        szfree = dequechunk_head_->size_of_deque_;
    }
    else if (pstart < pend)
    {
        szfree = dequechunk_head_->size_of_deque_ - (pend - pstart) ;
    }
    else
    {
        szfree = pstart - pend ;
    }

    //重要：FREE长度应该减去预留部分长度，保证首尾不会相接
    szfree -= JUDGE_FULL_INTERVAL;

    return szfree;
}

//容量
size_t shm_dequechunk::capacity()
{
    return dequechunk_head_->size_of_mmap_;
}


//得到某1时刻的快照是否为EMPTY
bool shm_dequechunk::empty()
{
    return free_size() == dequechunk_head_->size_of_deque_ - JUDGE_FULL_INTERVAL;
}

//得到某1时刻的快照是否为FULL
bool shm_dequechunk::full()
{
    return free_size() == 0;
}



}; 


