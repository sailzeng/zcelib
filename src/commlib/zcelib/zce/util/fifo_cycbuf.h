#pragma once

namespace zce
{
template <typename INTEGRAL_T>
class fifo_cycbuf
{
public:

    class cycbuf_node
    {
    public:

        ///*!
        //@brief      重载了new操作，用于得到一个变长得到架构
        //@return     void* operator
        //@param      size_t    new的默认参数
        //@param      node_len   node节点的长度
        //*
        static cycbuf_node* new_node(size_t node_len)
        {
            static_assert(std::is_integral<INTEGRAL_T>::value, "Not integral!");
            assert(node_len > sizeof(INTEGRAL_T));
            if (node_len < sizeof(cycbuf_node))
            {
                return nullptr;
            }
            char* ptr = ::new char[node_len];

#ifdef  DEBUG
            memset(ptr, 0, nodelen);
#endif
            //
            ((cycbuf_node*)ptr)->size_of_node_ = (INTEGRAL_T)node_len;
            return ((cycbuf_node*)ptr);
        }

        ///养成好习惯,写new,就写delete.
        static void cycbuf_node(cycbuf_node* node)
        {
            char* ptr = (char*)node;
            delete[] ptr;
        }

    public:

        ///头部的长度，
        static const size_t KFIFO_NODE_HEAD_LEN = sizeof(uint32_t);

        ///最小的CHUNK NODE长度，4+1
        static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = KFIFO_NODE_HEAD_LEN + 1;

        // 早年长度是unsigned ，一次重构我改成了size_t,但忘记了很多地方
        // 结构用的是固定长度，会强转为dequechunk_node，2了。

        /// 整个Node的长度,包括size_of_node_ + chunkdata,
        /// 这里使用size_t,long在64位下会有问题
        INTEGRAL_T    size_of_node_;

#if defined(ZCE_OS_WINDOWS)
#pragma warning ( disable : 4200)
#endif
        /// 数据区的数据，变长的数据
        char            node_data_[];
#if defined(ZCE_OS_WINDOWS)
#pragma warning ( default : 4200)
#endif
    };

public:

    fifo_cycbuf(size_t size_of_deque,
                size_t max_len_node)
    {
    }

    ~fifo_cycbuf()
    {
    }

protected:

    ///内存区的长度
    size_t               size_of_mmap_ = 0;

    ///deque的长度,必须>JUDGE_FULL_INTERVAL
    size_t               size_of_deque_ = 0;

    ///node的最大长度
    size_t               max_len_node_ = 0;

    ///两个关键内部指针,避免编译器优化
    ///环形队列开始的地方，这个地方必现是机器字长
    size_t               deque_begin_ = 0;
    ///环行队列结束的地方，这个地方必现是机器字长
    size_t               deque_end_ = 0;

    char* cycbuf_;
};
}