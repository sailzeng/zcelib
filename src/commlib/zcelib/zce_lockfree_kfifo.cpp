



#include "zce_predefine.h"
#include "zce_lockfree_kfifo.h"


namespace zce::lockfree
{

/*********************************************************************************
class dequechunk_node
*********************************************************************************/


//nodelen ���ĳ���,����sizeofnode�ĳ���
//����new�����,�õ�һ���䳤������
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
    //���֡���ĸ��ط��������⣬����������һ��
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
//���캯�����������������Ǵ������ʹ�õ�,
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


//���ݲ�����ʼ��
shm_dequechunk *shm_dequechunk::initialize(size_t size_of_deque,
                                           size_t max_len_node,
                                           char *pmmap,
                                           bool if_restore )
{
    //������ڼ������
    if (size_of_deque <= sizeof(size_t) + 16)
    {
        return NULL;
    }

    //
    _shm_dequechunk_head *dequechunk_head = reinterpret_cast<_shm_dequechunk_head *>(pmmap);

    //����ǻָ�����鼸��ֵ�Ƿ����
    if (if_restore == true)
    {
        //��ֹؼ������Ƿ�һ��
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

    //�õ��ռ��С
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

//�����û��ʹ�ù���״̬
void shm_dequechunk::clear()
{
    //
    dequechunk_head_->deque_begin_ = 0;
    dequechunk_head_->deque_end_ = 0 ;
    memset(dequechunk_database_, 0, dequechunk_head_->size_of_deque_);
}


//�õ������ؼ�ָ��Ŀ���
//����������Բ��ü�������һ��,32λ����ϵͳ�е�32λ����������ԭ�Ӳ���
void shm_dequechunk::snap_getpoint(size_t &pstart, size_t &pend)
{
    pstart = dequechunk_head_->deque_begin_;
    pend   = dequechunk_head_->deque_end_;
    return;
}



//��һ��NODE����β��
bool shm_dequechunk::push_end(const dequechunk_node *node)
{
    //���Եļ��,������Ȳ��ϸ�,���ز��ɹ�
    if (node->size_of_node_ < dequechunk_node::MIN_SIZE_DEQUE_CHUNK_NODE ||
        node->size_of_node_ > dequechunk_head_->max_len_node_ )
    {
        return false;
    }

    //�����еĿռ��Ƿ���
    if (free_size() < node->size_of_node_ )
    {
        return false;
    }

    //����ռ��㹻
    char *pend = dequechunk_database_ + dequechunk_head_->deque_end_;

    //�����Ȧ
    if (pend + node->size_of_node_ >= dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_end_ ;
        size_t second = node->size_of_node_ - first;
        memcpy(pend, reinterpret_cast<const char *>(node), first);
        memcpy(dequechunk_database_, reinterpret_cast<const char *>(node) + first, second);
        dequechunk_head_->deque_end_ = second;
    }
    //�������һ�ο������
    else
    {
        memcpy(pend, reinterpret_cast<const char *>(node), node->size_of_node_);
        dequechunk_head_->deque_end_ += node->size_of_node_;
    }

    return true;
}



//������һ��NODE����ȡ��,
//��������Լ�����,���׼��һ�����õĻ���ʹ��
//���صĽڵ���,Ҫ��node!=NULL,�Ѿ��������������
bool shm_dequechunk::pop_front(dequechunk_node *const node)
{
    assert(node != NULL);

    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    assert(tmplen > 0);
    assert(tmplen < 64 * 1024);
    assert(dequechunk_head_->deque_begin_ <= dequechunk_head_->size_of_deque_);

    //�������Ϊ2��
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




//������һ��NODE�Ӷ��ײ�ȡ��,�Ҹ���node�ĳ��Ȱ������ռ�,Ҫ��new_node=NULL,��ʾ��Ҫ����������仺��,
bool shm_dequechunk::pop_front_new(dequechunk_node *&new_node)
{
    assert(new_node == NULL);

    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    size_t tmplen = get_front_len();
    new_node = new (tmplen) dequechunk_node;

    //����д����һЩ�ظ����ã������Ҿ�������ط����ܲ��������⡣
    return pop_front(new_node);
}



//������һ��NODE��ȡ���Ƴ���,���ǲ���ȡ����
bool shm_dequechunk::read_front(dequechunk_node *const node)
{
    assert(node != NULL);

    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    //����ռ��㹻
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //�������Ϊ2��
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


//��ȡ���еĵ�һ��NODE���Ҹ���node�ĳ��Ȱ������ռ�,Ҫ��new_node=NULL,��ʾ��Ҫ����������仺��,
bool shm_dequechunk::read_front_new(dequechunk_node *&new_node)
{
    assert(new_node == NULL);

    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    size_t tmplen = get_front_len();
    new_node = new (tmplen) dequechunk_node;

    return read_front(new_node);
}

//��ȡ���еĵ�һ��NODE��ָ�룬��������е����ݻ����⴦��
bool shm_dequechunk::read_front_ptr(const dequechunk_node *&node_ptr)
{
    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    //����ռ��㹻
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //�������Ϊ2��,�����ˣ���line_wrap_nodeptr_���������ݣ��ύ���ϲ㣬
    if (pbegin + tmplen > dequechunk_database_ + dequechunk_head_->size_of_deque_)
    {
        size_t first = dequechunk_head_->size_of_deque_ - dequechunk_head_->deque_begin_ ;
        size_t second = tmplen - first;

        //���line_wrap_nodeptr_û�пռ䣬�ַ���
        if (line_wrap_nodeptr_ == NULL)
        {
            line_wrap_nodeptr_ = new (dequechunk_head_->max_len_node_) dequechunk_node;
        }

        //���������ݱ��浽line_wrap_nodeptr_�У����ϲ�������ã����ϲ���Ȼʹ��һ�������Ŀռ�
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

//��������ǰ��ĵ�һ��NODE
bool shm_dequechunk::discard_frond()
{

    //����Ƿ�Ϊ��
    if (empty() == true)
    {
        return false;
    }

    //����ռ��㹻
    char *pbegin = dequechunk_database_ + dequechunk_head_->deque_begin_;
    size_t tmplen = get_front_len();

    //���Ҫ���ʹ���߷���,�м��ͷ�,

    //�������Ϊ2��
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


//FREE�ĳߴ�,���еĿռ��ж���
size_t shm_dequechunk::free_size()
{
    //ȡ����
    size_t pstart, pend, szfree;
    snap_getpoint(pstart, pend);

    //����ߴ�
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

    //��Ҫ��FREE����Ӧ�ü�ȥԤ�����ֳ��ȣ���֤��β�������
    szfree -= JUDGE_FULL_INTERVAL;

    return szfree;
}

//����
size_t shm_dequechunk::capacity()
{
    return dequechunk_head_->size_of_mmap_;
}


//�õ�ĳ1ʱ�̵Ŀ����Ƿ�ΪEMPTY
bool shm_dequechunk::empty()
{
    return free_size() == dequechunk_head_->size_of_deque_ - JUDGE_FULL_INTERVAL;
}

//�õ�ĳ1ʱ�̵Ŀ����Ƿ�ΪFULL
bool shm_dequechunk::full()
{
    return free_size() == 0;
}



}; 


