/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_lockfree_deque.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��12��16��
* @brief      һ��һд����������Ϣ���У�lock-free��deque��
*
*
* @details    �������������һ���������ǿ��Ǽ�Ȼ��ʵ��һ��LOCKFREE�Ķ��У������Եö����ˡ������ˡ�
*             2019�ˡ�Ӧ����atomic��дһ�¡�
*
*
* @note
*
*/




#ifndef ZCE_LIB_LOCKFREE_DEQUE_H_
#define ZCE_LIB_LOCKFREE_DEQUE_H_

#include "zce_shm_predefine.h"


namespace zce
{

namespace lockfree
{


/*!
@brief      ���Է���deque��node�ṹ���䳤��ǰ��4���ֽڱ�ʾ���ȣ�
            ��ʵֻ���������ǰ��4���ֽڱ�ʾ���ȣ�ǿת��Ҳ����ʹ��
            ��ע����ĳ��ȱ�ʾ�ֶ�һ��Ҫ�Ƕ�����������size_t��long
            �����ڲ�ͬƽ̨���Ȳ�һ���Ķ�����
*/
class dequechunk_node
{

public:

    /*!
    @brief      ������new���������ڵõ�һ���䳤�õ��ܹ�
    @return     void* operator
    @param      size_t    new��Ĭ�ϲ���
    @param      nodelen   node�ڵ�ĳ���
    */
    void *operator new (size_t, size_t nodelen);

    //���ɺ�ϰ��,дnew,��дdelete.
    //void operator delete(void *ptrframe, size_t);
    void operator delete (void *ptrframe);

public:
    ///
    static void fillin(dequechunk_node *, size_t, char *);

public:

    ///��С��CHUNK NODE���ȣ�4+1
    static const size_t MIN_SIZE_DEQUE_CHUNK_NODE = 5;
    ///ͷ���ĳ��ȣ�
    static const size_t DEQUECHUNK_NODE_HEAD_LEN  = 4;
    // ���곤����unsigned ��һ���ع��Ҹĳ���size_t,�������˺ܶ�ط�
    // �ṹ�õ��ǹ̶����ȣ���ǿתΪdequechunk_node��2�ˡ�

    /// ����Node�ĳ���,����size_of_node_ + chunkdata,
    /// ����ʹ��size_t,long��64λ�»�������
    uint32_t    size_of_node_;


#if defined(ZCE_OS_WINDOWS)
#pragma warning ( disable : 4200)
#endif
    /// �����������ݣ��䳤������
    char            chunk_data_[];
#if defined(ZCE_OS_WINDOWS)
#pragma warning ( default : 4200)
#endif
};





/*!
@brief      ���е�ͷ�����ݣ���ʼ���ķ��ڹ����ڴ���ʼ���֣����ڼ�¼����
            ��һЩ״̬���ؼ����ݣ����ڻָ����ڻָ���ʱ�����бȶԼ��
            �ڲ�ʹ�ýṹ������ǰ�����_
*/
class _shm_dequechunk_head
{
    //
    friend  class shm_dequechunk;

protected:

    ///���캯�����������ṩ��
    _shm_dequechunk_head():
        size_of_mmap_(0),
        size_of_deque_(0),
        max_len_node_(0),
        deque_begin_(0),
        deque_end_(0)
    {
    }
    ///��������
    ~_shm_dequechunk_head()
    {
    }

    //����Ҳ���ṩ�����ڷ��ʣ�shm_dequechunk����
protected:

    ///�ڴ����ĳ���
    size_t               size_of_mmap_;

    ///deque�ĳ���,����>JUDGE_FULL_Iwo1NTERVAL
    size_t               size_of_deque_;

    ///node����󳤶�
    size_t               max_len_node_;


    ///�����ؼ��ڲ�ָ��,����������Ż�
    ///���ζ��п�ʼ�ĵط�������ط������ǻ����ֳ�
    volatile size_t      deque_begin_;
    ///���ж��н����ĵط�������ط������ǻ����ֳ�
    volatile size_t      deque_end_;
};


/*!
@brief      ���1����,1��д���ü���,��Ϊ����Щ����,�м���һ��������,��ֻʹ��pendָ��,дֻʹ��pstartָ��,
            �ж�EMPTY,FULLʹ�ÿ���,
            �����Ҫ������̶�д,Ҫ����,Ҫ�Լ�ʵ����,��ֻ�ṩ��һ������,
            ��������ģ��,��һ���Ƚ����ȳ��Ĵ�������С�����ݿ�Ķ���
            �����Ҫ��������,��smem_list�Լ����,������
*/
class shm_dequechunk : public _shm_memory_base
{


protected:

    ///ֻ���岻ʵ��
    const shm_dequechunk &operator=(const shm_dequechunk & );

    /*!
    @brief      �õ������ؼ�ָ��Ŀ��գ������ж�������������ݻ��ж��٣��Ƿ�Ϊ�����߿�
                ��Щ��������һЩ���ղ���,���������У������������һ��,32λ����ϵͳ�е�
                32λ����������ԭ�Ӳ���
                �����һ��������Ϊʲôһ���˷���,һ���˶�ȡ�ǰ�ȫ��,��Ϊһ����ͬʱֻ
                ʹ��1��,��ʹ����Ҳ��һ�����մ���
    @param      pstart  ���ص�ѭ��������ʼλ��
    @param      pend    ���ص�ѭ�����н���λ��
    */
    void snap_getpoint(size_t &pstart, size_t &pend);

    ///���캯������protected����������������
protected:
    shm_dequechunk();
public:
    ///��������
    ~shm_dequechunk();

public:


    /*!
    @brief      �õ�Ҫ����Ŀռ�Ĵ�С��
    @return     size_t         ʵ������Ŀռ䳤�ȣ��������ĳ��ȣ�
    @param      size_of_deque  ���������deque����,�㰴�����С�����ڴ�
    */
    static size_t getallocsize(const size_t size_of_deque);


    /*!
    @brief      ���ݲ�����ʼ��
    @return     shm_dequechunk * ���صĳ�ʼ����ָ�룬��Ҫ���٣�����delete�Ϳ��ԣ����ߵ���
    @param      size_of_deque    deque�ĳ��ȣ�(���� getallocsize �Ĳ��������Ƿ���ֵѽ)
    @param      max_len_node     �����note��󳤶ȣ��һ������һ��
    @param      pmmap            �ڴ��ָ�룬�����ڴ�Ҳ���ԣ���ͨ�ڴ�Ҳ����
    @param      if_restore       �Ƿ��ǽ��лָ�����������ǣ��ᱣ��ԭ�������ݣ�������ǣ������clear����
    @note
    */
    static shm_dequechunk *initialize(size_t size_of_deque,
                                      size_t max_len_node,
                                      char *pmmap,
                                      bool if_restore = false
                                     );




    /*!
    @brief      ���ٳ�ʼ�� initialize �õ���ָ��
    @param      deque_ptr  ���ٵ�ָ�룬
    */
    static void finalize(shm_dequechunk *deque_ptr);

    ///�����û��ʹ�ù���״̬
    void clear();

    /*!
    @brief      ��һ��NODE����β��
    @return     bool
    @param      node
    */
    bool push_end(const dequechunk_node *node);



    /*!
    @brief      ������һ��NODE�Ӷ��ײ�ȡ��,Ҫ��node!=NULL,�Ѿ��������������
    @return     bool  true��ʾ�ɹ�ȡ���������ʾû��ȡ��
    @param      node  ����pop ���ݵĵ�buffer��
    */
    bool pop_front(dequechunk_node *const node);

    /*!
    @brief      ��ȡ���еĵ�һ��NODE,���ǲ�ȡ��,Ҫ��node!=NULL,����Ϊ���Ѿ��������������
    @return     bool  true��ʾ�ɹ���ȡ
    @param      node  ����read ���ݵĵ�buffer��
    */
    bool read_front(dequechunk_node *const node);

    /*!
    @brief      ������һ��NODE�Ӷ��ײ�ȡ��,�Ҹ���node�ĳ��Ȱ������ռ�,
                Ҫ��new_node=NULL,��ʾ��Ҫ����������仺��,
    @return     bool      true��ʾ�ɹ���ȡ
    @param      new_node  ������ݵ�ָ�룬���������Ҫ�Լ��ͷţ��ҸŲ�������
    */
    bool pop_front_new(dequechunk_node *&new_node);

    /*!
    @brief      ��ȡ���еĵ�һ��NODE���Ҹ���node�ĳ��Ȱ������ռ�,Ҫ��new_node=NULL,��ʾ��Ҫ����������仺��,
    @return     bool      true��ʾ�ɹ���ȡ
    @param      new_node
    */
    bool read_front_new(dequechunk_node *&new_node);

    /*!
    @brief      ��ȡ���еĵ�һ��NODE��ָ�룩��ַ������������е����ݻ����⴦��
                ��ĳЩ���������һ��Memcopy��׷�������ܵ�ʱ����ʹ��
    @return     bool     true��ʾ�ɹ���ȡ
    @param      node_ptr ��ŵ�ַ��ָ��
    */
    bool read_front_ptr(const dequechunk_node *&node_ptr);

    /*!
    @brief      ��������ǰ��ĵ�һ��NODE
    @return     bool �Ƿ����ɹ�
    */
    bool discard_frond();

    ///ȡ����ͷ��buffer����,�������ȷ��pipe���������ݲ��ܵ�������������������Ը���
    inline size_t get_front_len();

    ///�õ�FREE�ռ�Ŀ���
    size_t free_size();

    ///����
    size_t capacity();

    ///�õ��Ƿ����Ŀ���
    bool empty();

    ///�õ��Ƿ�յĿ���
    bool full();

protected:

    ///�ж��Ƿ�Ϊ���ļ�����������Ϊ���ζ��л���һ��ǰ�պ󿪵Ľṹ
    ///deque_begin_ = deque_end_ ��ʾ����ΪNULL
    ///deque_begin_ = deque_end_ + JUDGE_FULL_INTERVAL ��ʾ������
    static const size_t   JUDGE_FULL_INTERVAL = 8;

protected:

    ///�ڴ��ͷ��
    _shm_dequechunk_head     *dequechunk_head_;

    ///��������ͷָ��,�������
    char                     *dequechunk_database_;

    ///�����Ҫ��ȡnode�ĵ�ַ����ȡ�����ݣ�����ô�����������������Ҫ����
    dequechunk_node          *line_wrap_nodeptr_;
};




//ȡ����ͷ��buffer����,�������ȷ��pipe���������ݲ��ܵ�������������������Ը���
//��Ϊ���������ʹ���ﾳ�󲿷���empty֮��
inline size_t shm_dequechunk::get_front_len()
{
    //����Ҫ���ĳ��Ƚض�2��,ͷ��,ͷ��,��д�ö����
    char *tmp1 = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = 0;
    char *tmp2 = reinterpret_cast<char *>(&tmplen);

    //����ܵ��ĳ���Ҳ��Ȧ������Ұ���ķ��ӵõ�����
    if ( tmp1 + dequechunk_node::DEQUECHUNK_NODE_HEAD_LEN > dequechunk_database_ + dequechunk_head_->size_of_deque_ )
    {
        //һ�����ֽڶ�ȡ����
        for (size_t i = 0; i < sizeof (uint32_t); ++i)
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

};

#endif //ZCE_LIB_LOCKFREE_DEQUE_H_




