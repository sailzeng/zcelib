
#include "zce_predefine.h"
#include "zce_shm_cache_chunk.h"

namespace zce
{

shm_cachechunk::shm_cachechunk()
{
}

shm_cachechunk::~shm_cachechunk()
{
}

//�õ���ʼ�������ڴ�Ĵ�С
size_t shm_cachechunk::getallocsize(const size_t numnode,
                                    const size_t numchunk,
                                    const size_t szchunk)
{
    return  sizeof(_shm_cachechunk_head)
            + numnode * sizeof(cachechunk_node_index)
            +  numchunk * sizeof(size_t)
            + numchunk * szchunk ;
}

//���ݲ�����ʼ��
shm_cachechunk *shm_cachechunk::initialize(const size_t numnode,
                                           const size_t numchunk,
                                           const size_t szchunk,
                                           char *pmmap,
                                           bool if_restore)
{
    //
    _shm_cachechunk_head *cachechunk_head = reinterpret_cast<_shm_cachechunk_head *>(pmmap);
    //

    //������ûָ�����ʽ,
    if (if_restore == true)
    {
        //������еĳߴ�,����в��Եĵط�����NULL,�������Ա�֤�����ִ���
        if (cachechunk_head->size_of_mmap_ != getallocsize(numnode, numchunk, szchunk) ||
            cachechunk_head->num_of_node_ != numnode ||
            cachechunk_head->num_of_chunk_ != numchunk ||
            cachechunk_head->size_of_chunk_ != szchunk )
        {
            return NULL;
        }
    }

    //�õ��ռ��С
    cachechunk_head->size_of_mmap_ = getallocsize(numnode, numchunk, szchunk);
    cachechunk_head->num_of_node_ = numnode;
    cachechunk_head->num_of_chunk_ = numchunk;
    cachechunk_head->size_of_chunk_ = szchunk;

    //����һЩƫ������ָ��
    shm_cachechunk *cachechunk = new shm_cachechunk();
    cachechunk->smem_base_ = pmmap;
    cachechunk->cachechunk_head_ = cachechunk_head;
    cachechunk->cachenode_base_ = reinterpret_cast<cachechunk_node_index *>(pmmap + sizeof(_shm_cachechunk_head));
    cachechunk->chunkindex_base_ = reinterpret_cast<size_t *>(pmmap + sizeof(_shm_cachechunk_head) + numnode * sizeof(cachechunk_node_index));
    cachechunk->chunkdata_base_  =  pmmap + sizeof(_shm_cachechunk_head) + numnode * sizeof(cachechunk_node_index) +  numchunk * sizeof(size_t);

    //��ʼ�����е��ڴ�
    if ( false == if_restore )
    {
        cachechunk->clear();
    }

    return cachechunk;
}

//
void shm_cachechunk::clear()
{
    //��ʼ��free������
    for (size_t i = 0; i < cachechunk_head_->num_of_node_ ; ++i )
    {
        cachenode_base_[i].chunk_index_ = i + 1;
        cachenode_base_[i].size_of_node_ = 0;

        if (i == cachechunk_head_->num_of_node_ - 1)
        {
            cachenode_base_[i].chunk_index_ = _INVALID_POINT;
        }
    }

    cachechunk_head_->free_node_head_ = 0;

    //����FREELIST�ĵ���NODE,
    for (size_t i = 0; i < cachechunk_head_->num_of_chunk_ ; ++i )
    {
        chunkindex_base_[i] = i + 1;

        if (i == cachechunk_head_->num_of_chunk_ - 1)
        {
            chunkindex_base_[i] = _INVALID_POINT;
        }
    }

    //
    cachechunk_head_->free_chunk_head_ = 0;

    //�����õ���������һ������
    cachechunk_head_->usable_of_node_ = cachechunk_head_->num_of_node_;
    cachechunk_head_->usable_of_chunk_ = cachechunk_head_->num_of_chunk_;

}

//��������Ŀռ�,����һ��NODE,
bool shm_cachechunk::create_node(const size_t szdata, size_t &nodeindex)
{
    //���û����NODE,����û����CHUNK
    bool bret = check_enough(szdata);

    if ( false == bret  )
    {
        return false;
    }

    //���ʣ��Ŀռ�ʮ���㹻
    size_t chunkinx = ( cachechunk_head_->free_chunk_head_);
    size_t tmpszdata = szdata;

    //ѭ�����ռ��Ƿ��㹻�Ŀ���Ҳ���ǵ�Ч��,���������õ���β,���Ա����и�ѭ������,

    //use_chunkΪʹ�õ�CHUNK������
    size_t use_chunk = 1;

    for (; tmpszdata > cachechunk_head_->size_of_chunk_; ++use_chunk)
    {
        tmpszdata -= cachechunk_head_->size_of_chunk_;

        //����ǰ����жϣ����
        if (chunkinx == _INVALID_POINT)
        {
            return false;
        }

        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    nodeindex = cachechunk_head_->free_node_head_;
    cachechunk_head_->free_node_head_ = (*(cachenode_base_ + nodeindex)).chunk_index_;

    (*(cachenode_base_ + nodeindex)).chunk_index_ =  cachechunk_head_->free_chunk_head_;
    (*(cachenode_base_ + nodeindex)).size_of_node_ =  szdata;

    //��FREE���ӵ�ͷ�ڵ����
    cachechunk_head_->free_chunk_head_  = *(chunkindex_base_ + chunkinx);

    *(chunkindex_base_ + chunkinx) = _INVALID_POINT;

    //��¼ʹ�õ����ݽڵ�����
    --(cachechunk_head_->usable_of_node_) ;
    cachechunk_head_->usable_of_chunk_ -= use_chunk;

    return true;
}

//�ͷ�һ��NODE,�����Ͱ�黹��FREELIST
void shm_cachechunk::destroy_node(const size_t nodeindex)
{
    //�黹CHUNK
    size_t chunkinx = (*(cachenode_base_ + nodeindex)).chunk_index_;
    size_t fchunkinx = chunkinx;
    //ѭ���ͷ�CHUNK
    //free_chunkΪ�ͷŵ�CHUNK������
    size_t free_chunk = 1;

    for (; * (chunkindex_base_ + chunkinx) != _INVALID_POINT; ++free_chunk)
    {
        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    *(chunkindex_base_ + chunkinx) = cachechunk_head_->free_chunk_head_;
    cachechunk_head_->free_chunk_head_ = fchunkinx;

    //�黹NODE
    (*(cachenode_base_ + nodeindex)).chunk_index_ = cachechunk_head_->free_node_head_;
    (*(cachenode_base_ + nodeindex)).size_of_node_    = 0;

    cachechunk_head_->free_node_head_ = nodeindex;

    //ע������ĵļ����ڲ������ǿ��õ������������Ѿ��õġ�
    ++(cachechunk_head_->usable_of_node_) ;
    cachechunk_head_->usable_of_chunk_ += free_chunk;

}

//ȡʣ��ռ�Ĵ�С
void shm_cachechunk::free_size(size_t &free_node, size_t &free_chunk, size_t &max_room)
{
    free_node = cachechunk_head_->usable_of_node_;
    free_chunk = cachechunk_head_->usable_of_chunk_;
    max_room = cachechunk_head_->usable_of_chunk_ * cachechunk_head_->size_of_chunk_;
}

//����Ƿ���Է���һ��������С������
bool shm_cachechunk::check_enough(size_t szdata)
{
    if ( cachechunk_head_->usable_of_node_ == 0 ||
         cachechunk_head_->usable_of_chunk_ == 0 ||
         (cachechunk_head_->usable_of_chunk_) * cachechunk_head_->size_of_chunk_ <= szdata )
    {
        return false;
    }

    return true;
}

//��һ��NODE����Cache�У�
bool shm_cachechunk::set_node(const size_t szdata, const char *indata, size_t &nodeindex)
{
    size_t tmpindex;
    bool bret = create_node(szdata, tmpindex);

    if (bret == false)
    {
        return false;
    }

    nodeindex = tmpindex;
    size_t chunkinx = (*(cachenode_base_ + tmpindex)).chunk_index_;

    size_t szonce = 0, szcpy = 0;
    size_t tmpszdata = szdata;

    //���tmpszdata ==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata > cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        memcpy(chunkdata_base_ +  chunkinx * cachechunk_head_->size_of_chunk_, indata + szcpy, szonce);
        szcpy += szonce;
        tmpszdata -= szonce;

        //׼���¸�CHUNK
        if (tmpszdata > 0)
        {
            chunkinx = (*(chunkindex_base_ + chunkinx));
        }
    }

    return true;
}

//�õ�NODE�Ĵ�С
size_t shm_cachechunk::nodesize(const size_t nodeindex)
{
    return (*(cachenode_base_ + nodeindex)).size_of_node_;
}

//�õ�ĳ��NODE�ĳߴ�,�Լ���Ӧ��chunk������
void shm_cachechunk::nodesize(const size_t nodeindex, size_t &nodesize, size_t &chunknum)
{
    nodesize = (*(cachenode_base_ + nodeindex)).size_of_node_;

    if (0 == (nodesize % (cachechunk_head_->size_of_chunk_)))
    {
        chunknum =  nodesize / (cachechunk_head_->size_of_chunk_);
    }
    else
    {
        chunknum =  nodesize / (cachechunk_head_->size_of_chunk_) + 1;
    }
}

//�õ�CHUNK�Ĵ�С,ע������CHUNK�����������������ݵĴ�С
size_t shm_cachechunk::chunksize()
{
    return cachechunk_head_->size_of_chunk_;
}

//ȡ��NODE�����ݣ��Լ���С
void shm_cachechunk::get_node(const size_t nodeindex, size_t &szdata, char *outdata)
{
    size_t chunkinx = (*(cachenode_base_ + nodeindex)).chunk_index_;
    size_t tmpszdata = (*(cachenode_base_ + nodeindex)).size_of_node_;
    szdata = tmpszdata;

    size_t szonce = 0, szcpy = 0;

    //szdata ���==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata >= cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        memcpy(outdata + szcpy, chunkdata_base_ +  chunkinx * (cachechunk_head_->size_of_chunk_), szonce);
        szcpy += szonce;
        tmpszdata -= szonce;

        if (tmpszdata > 0)
        {
            chunkinx = (*(chunkindex_base_ + chunkinx));
        }
    }
}

//�ͷ�NODE��������Ӧ������CHUNK��
void shm_cachechunk::freenode(const size_t nodeindex)
{
    destroy_node(nodeindex);
}

//����Ҫһ����CHUNKȡ������ʱ���õ�һ��NODE�ĵ�N��CHUNK������
void shm_cachechunk::get_chunk(const size_t nodeindex, size_t chunk_no, size_t &szdata, char *outdata)
{
    size_t chunk_num_of_node, size_of_node;
    nodesize(nodeindex, size_of_node, chunk_num_of_node);

    size_t size_of_chunk = cachechunk_head_->size_of_chunk_;
    size_t leftsize    = size_of_node;

    //
    size_t inxchunk = (*(cachenode_base_ + nodeindex)).chunk_index_;

    for (size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    memcpy(outdata, chunkdata_base_ + inxchunk * size_of_chunk, szdata);
}

//�������ݵ���ʼλ�ã�ȡ�����λ������CHUNK������,��ע��ֻ����һ��CHUNK�����ݣ�
//����������ݵ���ʼλ�ÿ�ʼ��������CHUNK�м䣨����0�����������������ݴ�data_start��ʼ
void shm_cachechunk::get_chunkdata(const size_t nodeindex,
                                   const size_t data_start,
                                   size_t &chunk_no,
                                   size_t &szdata,
                                   char *outdata)
{
    chunk_no = 0;
    size_t chunkinx = (*(cachenode_base_ + nodeindex)).chunk_index_;
    size_t tmpszdata = data_start;

#if defined _DEBUG || defined DEBUG
    assert(data_start < (*(cachenode_base_ + nodeindex)).size_of_node_);
#endif

    size_t szonce = 0 ;

    //tmpszdata ���==0
    while (tmpszdata >= cachechunk_head_->size_of_chunk_)
    {
        szonce = (tmpszdata > cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        tmpszdata -= szonce;
        chunkinx = (*(chunkindex_base_ + chunkinx));
        ++chunk_no;
    }

    //
    szdata = cachechunk_head_->size_of_chunk_ - tmpszdata;
    memcpy(outdata, chunkdata_base_ +  chunkinx * (cachechunk_head_->size_of_chunk_), szdata);
}

//����ÿ��ȡһ��CHUNK��ָ�����������NODE�������ڼ���CHUNK,����
//  CHUNK��ָ���Լ���Ӧ�ĳ���,ע��ָ�����������,
void shm_cachechunk::get_chunk_point(const size_t nodeindex,
                                     size_t chunk_no,
                                     size_t &szdata,
                                     char *&chunk_point)
{
    size_t chunk_num_of_node, size_of_node;
    nodesize(nodeindex, size_of_node, chunk_num_of_node);

    size_t size_of_chunk = cachechunk_head_->size_of_chunk_;
    size_t leftsize    = size_of_node;

    //
    size_t inxchunk = (*(cachenode_base_ + nodeindex)).chunk_index_;

    for (size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    chunk_point = chunkdata_base_ + inxchunk * size_of_chunk;
}

//���ڸ������ݵ���ʼλ�ã�ȡ�����λ������CHUNK��ָ��,�Լ�ȡ��
//�����CHUNK�����ʣ�������ʱ
void shm_cachechunk::get_chunkdata_point(const size_t nodeindex,
                                         const size_t data_start,
                                         size_t &chunk_no,
                                         size_t &szdata,
                                         char *&chunk_data_point)
{

    size_t chunkinx = (*(cachenode_base_ + nodeindex)).chunk_index_;
    size_t szofchunk = cachechunk_head_->size_of_chunk_;
    size_t size_of_node_ = (*(cachenode_base_ + nodeindex)).size_of_node_;

#if defined _DEBUG || defined DEBUG
    assert(data_start < size_of_node_);
#endif

    //�������һ��
    if ( size_of_node_ / szofchunk != data_start / szofchunk )
    {
        szdata = szofchunk - (data_start % szofchunk);
    }
    else
    {
        szdata = szofchunk - (data_start % szofchunk) -
                 ((size_of_node_ % szofchunk) == 0 ? 0 : (szofchunk - (size_of_node_ % szofchunk)) );
    }

    chunk_no = data_start / szofchunk;

    //�ҵ�CHUNK���������
    for (size_t i = 0; i < chunk_no; ++i)
    {
        chunkinx = (*(chunkindex_base_ + chunkinx));
    }

    chunk_data_point = chunkdata_base_ +  chunkinx * (cachechunk_head_->size_of_chunk_) + (data_start % szofchunk);
}

};

