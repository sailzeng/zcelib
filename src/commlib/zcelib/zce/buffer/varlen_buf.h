#pragma once

namespace zce
{
template<typename T>
concept integral_t = std::is_integral<T>::value;
/*!
* @brief      Variable-length chunk
*             可以放如deque的node结构，变长，前面若干个字节表示长度，
*             长度用模版参数 T 控制
*             外部使用时，只要你BUFFER最开始标识长度和INTEGRAL_T一致，
*             就可以强转成指针使用
*
*/
template <typename T> requires integral_t<T>
class varlen_buf
{
protected:
    varlen_buf() = delete;
    varlen_buf& operator=(const varlen_buf & others) = delete;
    ~varlen_buf() = delete;
public:

    ///*!
    //@brief      重载了new操作，用于得到一个变长得到架构
    //@return     void* operator
    //@param      size_t    new的默认参数
    //@param      node_len   node节点的长度
    //*
    static varlen_buf* new_node(size_t node_len)
    {
        static_assert(std::is_integral<T>::value, "Not integral!");
        assert(node_len > sizeof(T) &&
               node_len <= static_cast<size_t>(std::numeric_limits<int>::max()));
        if (node_len <= sizeof(T) ||
            node_len > static_cast<size_t>(std::numeric_limits<int>::max()))

        {
            return nullptr;
        }
        char* ptr = ::new char[node_len];

#ifdef  DEBUG
        //检查帧的哪个地方出现问题，还是这样好一点
        memset(ptr, 0, node_len);
#endif
        //
        ((varlen_buf*)ptr)->size_of_buf_ = (T)node_len;

        return ((varlen_buf*)ptr);
    }

    ///养成好习惯,写new,就写delete.
    static void delete_node(varlen_buf * node)
    {
        char* ptr = (char*)node;
        delete[] ptr;
    }
public:

    ///头部的长度，
    static const size_t NODE_HEAD_LEN = sizeof(T);

    ///最小的CHUNK NODE长度，NODE_HEAD_LEN+1
    static const size_t MIN_SIZE_BUF_NODE = NODE_HEAD_LEN + 1;

    ///最大的CHUNK NODE长度，
    static const size_t MAX_SIZE_BUF_NODE = std::numeric_limits<T>::max();

    /// 整个Node的长度,包括size_of_buf_ + chunkdata, 你可以用模版描述这个长度是多少
    /// 这里使用size_t,long在64位下会有问题
    T    size_of_buf_;

    /// 数据区的数据，变长的数据,1只是占位符号
    char buf_data_[1];
};
}