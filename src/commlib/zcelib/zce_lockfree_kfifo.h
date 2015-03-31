/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lockfree_kfifo.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005年12月16日
* @brief      一读一写不加锁的消息队列，lock-free的deque，
*
*
* @details    本来在里面放了一把锁，但是考虑既然是实现一个LOCKFREE的队列，锁就显得多余了。放弃了。
*
*
*
* @note
*
*/




#ifndef ZCE_LIB_SHM_LOCKFREE_DEQUE_H_
#define ZCE_LIB_SHM_LOCKFREE_DEQUE_H_

#include "zce_shm_predefine.h"


namespace ZCE_LIB
{

/*!
@brief      可以放如deque的node结构，变长，前面4个字节表示长度，
            其实只需你的数据前面4个字节表示长度，强转了也可以使用
            但注意你的长度表示字段一定要是定长，不能是size_t，long
            这种在不同平台长度不一样的东东。
*/
class dequechunk_node
{

public:

    /*!
    @brief      重载了new操作，用于得到一个变长得到架构
    @return     void* operator
    @param      size_t    new的默认参数
    @param      nodelen   node节点的长度
    */
    void *operator new (size_t , size_t nodelen);

public:
    ///
    static void fillin(dequechunk_node *, size_t , char *);

public:

    ///最小的CHUNK NODE长度，4+1
    static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = 5;
    ///头部的长度，
    static const size_t DEQUECHUNK_NODE_HEAD_LEN  = 4;
    // 早年长度是unsigned ，一次重构我改成了size_t,但忘记了很多地方
    // 结构用的是固定长度，会强转为dequechunk_node，2了。

    /// 整个Node的长度,包括size_of_node_ + chunkdata,
    /// 这里使用size_t,long在64位下会有问题
    unsigned int    size_of_node_;


#if defined(ZCE_OS_WINDOWS)
#pragma warning ( disable : 4200)
#endif
    /// 数据区的数据，变长的数据
    char            chunk_data_[];
#if defined(ZCE_OS_WINDOWS)
#pragma warning ( default : 4200)
#endif
};





/*!
@brief      队列的头部数据，初始化的放在共享内存的最开始部分，用于记录队列
            的一些状态，关键数据，用于恢复，在恢复的时候会进行比对检查
            内部使用结构，所以前面加了_
*/
class _shm_dequechunk_head
{
    //
    friend  class shm_dequechunk;

protected:

    ///构造函数，不对外提供，
    _shm_dequechunk_head():
        size_of_mmap_(0),
        size_of_deque_(0),
        max_len_node_(0),
        deque_begin_(0),
        deque_end_(0)
    {
    }
    ///析构函数
    ~_shm_dequechunk_head()
    {
    }

    //数据也不提供给大众访问，shm_dequechunk除外
protected:

    ///内存区的长度
    size_t               size_of_mmap_;

    ///deque的长度,必须>JUDGE_FULL_Iwo1NTERVAL
    size_t               size_of_deque_;

    ///node的最大长度
    size_t               max_len_node_;


    ///两个关键内部指针,避免编译器优化
    ///环形队列开始的地方，这个地方必现是机器字长
    volatile size_t      deque_begin_;
    ///环行队列结束的地方，这个地方必现是机器字长
    volatile size_t      deque_end_;
};


/*!
@brief      如果1个读,1个写不用加锁,因为用了些技巧,中间有一个空闲区,读只使用pend指针,写只使用pstart指针,
            判断EMPTY,FULL使用快照,
            如果非要多个进程读写,要加锁,要自己实现锁,我只提供了一个基类,
            不是容器模版,是一个先进，先出的存放任意大小的数据快的队列
            如果非要容器队列,用smem_list自己解决,很容易
*/
class shm_dequechunk : public _shm_memory_base
{


protected:

    ///只定义不实现
    const shm_dequechunk &operator=(const shm_dequechunk & );

    /*!
    @brief      得到两个关键指针的快照，用于判定队列里面的数据还有多少，是否为满或者空
                这些操作都是一些快照操作,不加锁进行，这个操作基于一点,32位操作系统中的
                32位整数操作是原子操作
                这儿费一点口舌解释为什么一个人放入,一个人读取是安全的,因为一个人同时只
                使用1个,即使错误也是一个快照错误
    @param      pstart  返回的循环队列起始位置
    @param      pend    返回的循环队列结束位置
    */
    void snap_getpoint(size_t &pstart, size_t &pend);

    ///构造函数，用protected保护，避免你用了
protected:
    shm_dequechunk();
public:
    ///析构函数
    ~shm_dequechunk();

public:


    /*!
    @brief      得到要分配的空间的大小，
    @return     size_t         实际所需的空间长度，会大于你的长度，
    @param      size_of_deque  你所必须的deque长度,你按这个大小申请内存
    */
    static size_t getallocsize(const size_t size_of_deque);


    /*!
    @brief      根据参数初始化
    @return     shm_dequechunk * 返回的初始化的指针，需要销毁，你用delete就可以，或者调用
    @param      size_of_deque    deque的长度，(就是 getallocsize 的参数，不是返回值呀)
    @param      max_len_node     放入的note最大长度，我会帮你检查一下
    @param      pmmap            内存的指针，共享内存也可以，普通内存也可以
    @param      if_restore       是否是进行恢复操作，如果是，会保留原来的数据，如果不是，会调用clear清理
    @note
    */
    static shm_dequechunk *initialize(size_t size_of_deque,
                                      size_t max_len_node,
                                      char *pmmap,
                                      bool if_restore = false
                                     );




    /*!
    @brief      销毁初始化 initialize 得到的指针
    @param      deque_ptr  销毁的指针，
    */
    static void finalize(shm_dequechunk *deque_ptr);

    ///清理成没有使用过的状态
    void clear();

    /*!
    @brief      将一个NODE放入尾部
    @return     bool
    @param      node
    */
    bool push_end(const dequechunk_node *node);



    /*!
    @brief      将队列一个NODE从队首部取出,要求node!=NULL,已经分配好了数据区
    @return     bool  true表示成功取出，否则表示没有取出
    @param      node  保存pop 数据的的buffer，
    */
    bool pop_front(dequechunk_node *const node);

    /*!
    @brief      读取队列的第一个NODE,但是不取出,要求node!=NULL,我认为你已经分配好了数据区
    @return     bool  true表示成功读取
    @param      node  保存read 数据的的buffer，
    */
    bool read_front(dequechunk_node *const node);

    /*!
    @brief      将队列一个NODE从队首部取出,我根据node的长度帮你分配空间,
                要求new_node=NULL,表示你要函数帮你分配缓冲,
    @return     bool      true表示成功读取
    @param      new_node  获得数据的指针，这个数据你要自己释放，我概不负责了
    */
    bool pop_front_new(dequechunk_node *&new_node);

    /*!
    @brief      读取队列的第一个NODE，我根据node的长度帮你分配空间,要求new_node=NULL,表示你要函数帮你分配缓冲,
    @return     bool      true表示成功读取
    @param      new_node
    */
    bool read_front_new(dequechunk_node *& new_node);

    /*!
    @brief      读取队列的第一个NODE（指针）地址，，如果是折行的数据会特殊处理
                在某些情况下少用一次Memcopy，追求极致性能的时候考虑使用
    @return     bool     true表示成功读取
    @param      node_ptr 存放地址的指针
    */
    bool read_front_ptr(const dequechunk_node *&node_ptr);

    /*!
    @brief      丢弃队列前面的第一个NODE
    @return     bool 是否丢弃成功
    */
    bool discard_frond();

    ///取队列头的buffer长度,你必须在确认pipe里面有数据才能调用这个函数，否则后果自负。
    inline size_t get_front_len();

    ///得到FREE空间的快照
    size_t free_size();

    ///容量
    size_t capacity();

    ///得到是否满的快照
    bool empty();

    ///得到是否空的快照
    bool full();

protected:

    ///判断是非为满的间隔，你可以认为环形队列还是一个前闭后开的结构
    ///deque_begin_ = deque_end_ 表示队列为NULL
    ///deque_begin_ = deque_end_ + JUDGE_FULL_INTERVAL 表示队列满
    static const size_t   JUDGE_FULL_INTERVAL = 8;

protected:

    ///内存的头部
    _shm_dequechunk_head     *dequechunk_head_;

    ///数据区的头指针,方便计算
    char                     *dequechunk_database_;

    ///如果需要读取node的地址（不取出数据），那么有种特殊情况，折行要考虑
    dequechunk_node          *line_wrap_nodeptr_;
};




//取队列头的buffer长度,你必须在确认pipe里面有数据才能调用这个函数，否则后果自负。
//因为这个函数的使用语境大部分是empty之后，
inline size_t shm_dequechunk::get_front_len()
{
    //还是要担心长度截断2节,头大,头大,多写好多代码
    char *tmp1 = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = 0;
    char *tmp2 = reinterpret_cast<char *>(&tmplen);

    //如果管道的长度也绕圈，采用野蛮的法子得到长度
    if ( tmp1 + dequechunk_node::DEQUECHUNK_NODE_HEAD_LEN > dequechunk_database_ + dequechunk_head_->size_of_deque_ )
    {
        //一个个字节读取长度
        for (size_t i = 0; i < sizeof (unsigned int); ++i)
        {
            if ( tmp1 >= dequechunk_database_ + dequechunk_head_->size_of_deque_ )
            {
                tmp1 = dequechunk_database_;
            }

            *tmp2 = *tmp1;
            ++tmp1 ;
            ++tmp2 ;
        }
    }
    //
    else
    {
        tmplen = *(reinterpret_cast<unsigned int *>(tmp1));
    }

    return tmplen;
}

};

#endif //ZCE_LIB_SHM_LOCKFREE_DEQUE_H_




