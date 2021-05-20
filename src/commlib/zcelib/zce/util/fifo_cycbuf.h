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
        //@brief      ������new���������ڵõ�һ���䳤�õ��ܹ�
        //@return     void* operator
        //@param      size_t    new��Ĭ�ϲ���
        //@param      node_len   node�ڵ�ĳ���
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

        ///���ɺ�ϰ��,дnew,��дdelete.
        static void cycbuf_node(cycbuf_node* node)
        {
            char* ptr = (char*)node;
            delete[] ptr;
        }

    public:

        ///ͷ���ĳ��ȣ�
        static const size_t KFIFO_NODE_HEAD_LEN = sizeof(uint32_t);

        ///��С��CHUNK NODE���ȣ�4+1
        static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = KFIFO_NODE_HEAD_LEN + 1;

        // ���곤����unsigned ��һ���ع��Ҹĳ���size_t,�������˺ܶ�ط�
        // �ṹ�õ��ǹ̶����ȣ���ǿתΪdequechunk_node��2�ˡ�

        /// ����Node�ĳ���,����size_of_node_ + chunkdata,
        /// ����ʹ��size_t,long��64λ�»�������
        INTEGRAL_T    size_of_node_;

#if defined(ZCE_OS_WINDOWS)
#pragma warning ( disable : 4200)
#endif
        /// �����������ݣ��䳤������
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

    ///�ڴ����ĳ���
    size_t               size_of_mmap_ = 0;

    ///deque�ĳ���,����>JUDGE_FULL_INTERVAL
    size_t               size_of_deque_ = 0;

    ///node����󳤶�
    size_t               max_len_node_ = 0;

    ///�����ؼ��ڲ�ָ��,����������Ż�
    ///���ζ��п�ʼ�ĵط�������ط������ǻ����ֳ�
    size_t               deque_begin_ = 0;
    ///���ж��н����ĵط�������ط������ǻ����ֳ�
    size_t               deque_end_ = 0;

    char* cycbuf_;
};
}