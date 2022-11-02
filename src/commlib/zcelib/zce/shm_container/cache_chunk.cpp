#include "zce/predefine.h"
#include "zce/shm_container/cache_chunk.h"

namespace zce
{
//得到初始化所需内存的大小
size_t shm_cachechunk::getallocsize(const size_t numnode,
                                    const size_t numchunk,
                                    const size_t szchunk)
{
    return  sizeof(_shm_cachechunk_head)
        + numnode * sizeof(cachechunk_node_index)
        + numchunk * sizeof(size_t)
        + numchunk * szchunk;
}

//根据参数初始化
shm_cachechunk* shm_cachechunk::initialize(const size_t numnode,
                                           const size_t numchunk,
                                           const size_t szchunk,
                                           char* pmmap,
                                           bool if_restore)
{
    //
    _shm_cachechunk_head* cachechunk_head = reinterpret_cast<_shm_cachechunk_head*>(pmmap);
    //

    //如果采用恢复的形式,
    if (if_restore == true)
    {
        //检查所有的尺寸,如果有不对的地方返回nullptr,这样可以保证不出现错误
        if (cachechunk_head->size_of_mmap_ != getallocsize(numnode, numchunk, szchunk) ||
            cachechunk_head->num_of_node_ != numnode ||
            cachechunk_head->num_of_chunk_ != numchunk ||
            cachechunk_head->size_of_chunk_ != szchunk)
        {
            return nullptr;
        }
    }

    //得到空间大小
    cachechunk_head->size_of_mmap_ = getallocsize(numnode, numchunk, szchunk);
    cachechunk_head->num_of_node_ = numnode;
    cachechunk_head->num_of_chunk_ = numchunk;
    cachechunk_head->size_of_chunk_ = szchunk;

    //处理一些偏移量的指针
    shm_cachechunk* cachechunk = new shm_cachechunk();
    cachechunk->smem_base_ = pmmap;
    cachechunk->cachechunk_head_ = cachechunk_head;
    cachechunk->cachenode_base_ = reinterpret_cast<cachechunk_node_index*>(pmmap + sizeof(_shm_cachechunk_head));
    cachechunk->chunkindex_base_ = reinterpret_cast<size_t*>(pmmap + sizeof(_shm_cachechunk_head) + numnode * sizeof(cachechunk_node_index));
    cachechunk->chunkdata_base_ = pmmap + sizeof(_shm_cachechunk_head) + numnode * sizeof(cachechunk_node_index) + numchunk * sizeof(size_t);

    //初始化所有的内存
    if (false == if_restore)
    {
        cachechunk->clear();
    }

    return cachechunk;
}

//
void shm_cachechunk::clear()
{
    //初始化free数据区
    for (size_t i = 0; i < cachechunk_head_->num_of_node_; ++i)
    {
        cachenode_base_[i].chunk_index_ = i + 1;
        cachenode_base_[i].size_of_node_ = 0;

        if (i == cachechunk_head_->num_of_node_ - 1)
        {
            cachenode_base_[i].chunk_index_ = SHM_CNTR_INVALID_POINT;
        }
    }

    cachechunk_head_->free_node_head_ = 0;

    //清理FREELIST的单向NODE,
    for (size_t i = 0; i < cachechunk_head_->num_of_chunk_; ++i)
    {
        chunkindex_base_[i] = i + 1;

        if (i == cachechunk_head_->num_of_chunk_ - 1)
        {
            chunkindex_base_[i] = SHM_CNTR_INVALID_POINT;
        }
    }

    //
    cachechunk_head_->free_chunk_head_ = 0;

    //将可用的数量进行一下修正
    cachechunk_head_->usable_of_node_ = cachechunk_head_->num_of_node_;
    cachechunk_head_->usable_of_chunk_ = cachechunk_head_->num_of_chunk_;
}

//根据申请的空间,分配一个NODE,
bool shm_cachechunk::create_node(const size_t szdata, size_t& node_index)
{
    //如果没有了NODE,或者没有了CHUNK
    bool bret = check_enough(szdata);

    if (false == bret)
    {
        return false;
    }

    //检查剩余的空间十分足够
    size_t chunkinx = (cachechunk_head_->free_chunk_head_);
    size_t tmpszdata = szdata;

    //循环检查空间是否足够的看似也许是低效的,但是你必须得到队尾,所以必须有个循环过程,

    //use_chunk为使用的CHUNK数量，
    size_t use_chunk = 1;

    for (; tmpszdata > cachechunk_head_->size_of_chunk_; ++use_chunk)
    {
        tmpszdata -= cachechunk_head_->size_of_chunk_;

        //根据前面的判断，这个
        if (chunkinx == SHM_CNTR_INVALID_POINT)
        {
            return false;
        }

        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    node_index = cachechunk_head_->free_node_head_;
    cachechunk_head_->free_node_head_ = (*(cachenode_base_ + node_index)).chunk_index_;

    (*(cachenode_base_ + node_index)).chunk_index_ = cachechunk_head_->free_chunk_head_;
    (*(cachenode_base_ + node_index)).size_of_node_ = szdata;

    //将FREE链子的头节点放入
    cachechunk_head_->free_chunk_head_ = *(chunkindex_base_ + chunkinx);

    *(chunkindex_base_ + chunkinx) = SHM_CNTR_INVALID_POINT;

    //记录使用的数据节点数量
    --(cachechunk_head_->usable_of_node_);
    cachechunk_head_->usable_of_chunk_ -= use_chunk;

    return true;
}

//释放一个NODE,将其的桶归还给FREELIST
void shm_cachechunk::destroy_node(const size_t node_index)
{
    //归还CHUNK
    size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    size_t fchunkinx = chunkinx;
    //循环释放CHUNK
    //free_chunk为释放的CHUNK数量，
    size_t free_chunk = 1;

    for (; *(chunkindex_base_ + chunkinx) != SHM_CNTR_INVALID_POINT; ++free_chunk)
    {
        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    *(chunkindex_base_ + chunkinx) = cachechunk_head_->free_chunk_head_;
    cachechunk_head_->free_chunk_head_ = fchunkinx;

    //归还NODE
    (*(cachenode_base_ + node_index)).chunk_index_ = cachechunk_head_->free_node_head_;
    (*(cachenode_base_ + node_index)).size_of_node_ = 0;

    cachechunk_head_->free_node_head_ = node_index;

    //注意下面改的几个内部变量是可用的数量，不是已经用的。
    ++(cachechunk_head_->usable_of_node_);
    cachechunk_head_->usable_of_chunk_ += free_chunk;
}

//取剩余空间的大小
void shm_cachechunk::free(size_t& free_node, size_t& free_chunk, size_t& max_room)
{
    free_node = cachechunk_head_->usable_of_node_;
    free_chunk = cachechunk_head_->usable_of_chunk_;
    max_room = cachechunk_head_->usable_of_chunk_ * cachechunk_head_->size_of_chunk_;
}

//检查是否可以放入一个这样大小的数据
bool shm_cachechunk::check_enough(size_t szdata)
{
    if (cachechunk_head_->usable_of_node_ == 0 ||
        cachechunk_head_->usable_of_chunk_ == 0 ||
        (cachechunk_head_->usable_of_chunk_) * cachechunk_head_->size_of_chunk_ <= szdata)
    {
        return false;
    }

    return true;
}

//将一个NODE放入Cache中，
bool shm_cachechunk::push_node(const size_t szdata,
                               const char* indata,
                               size_t& node_index)
{
    size_t tmpindex;
    bool bret = create_node(szdata, tmpindex);

    if (bret == false)
    {
        return false;
    }

    node_index = tmpindex;
    size_t chunkinx = (*(cachenode_base_ + tmpindex)).chunk_index_;

    size_t szonce = 0, szcpy = 0;
    size_t tmpszdata = szdata;

    //最后tmpszdata ==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata > cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        memcpy(chunkdata_base_ + chunkinx * cachechunk_head_->size_of_chunk_, indata + szcpy, szonce);
        szcpy += szonce;
        tmpszdata -= szonce;

        //准备下个CHUNK
        if (tmpszdata > 0)
        {
            chunkinx = (*(chunkindex_base_ + chunkinx));
        }
    }

    return true;
}

//得到NODE的大小
size_t shm_cachechunk::node_size(const size_t node_index)
{
    return (*(cachenode_base_ + node_index)).size_of_node_;
}

//得到某个NODE的尺寸,以及相应的chunk的数量
void shm_cachechunk::node_size(const size_t node_index, size_t& node_size, size_t& chunknum)
{
    node_size = (*(cachenode_base_ + node_index)).size_of_node_;

    if (0 == (node_size % (cachechunk_head_->size_of_chunk_)))
    {
        chunknum = node_size / (cachechunk_head_->size_of_chunk_);
    }
    else
    {
        chunknum = node_size / (cachechunk_head_->size_of_chunk_) + 1;
    }
}

//得到CHUNK的大小,注意这是CHUNK的容量不是里面数据的大小
size_t shm_cachechunk::chunk_capacity()
{
    return cachechunk_head_->size_of_chunk_;
}

//取回NODE的数据，以及大小
void shm_cachechunk::pull_node(const size_t node_index, size_t& szdata, char* outdata)
{
    size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    size_t tmpszdata = (*(cachenode_base_ + node_index)).size_of_node_;
    szdata = tmpszdata;

    size_t szonce = 0, szcpy = 0;

    //szdata 最后==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata >= cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        memcpy(outdata + szcpy, chunkdata_base_ + chunkinx * (cachechunk_head_->size_of_chunk_), szonce);
        szcpy += szonce;
        tmpszdata -= szonce;

        if (tmpszdata > 0)
        {
            chunkinx = (*(chunkindex_base_ + chunkinx));
        }
    }
}

//释放NODE的索引对应的所有CHUNK，
void shm_cachechunk::free_node(const size_t node_index)
{
    destroy_node(node_index);
}

//当需要一个个CHUNK取出数据时，得到一个NODE的第N个CHUNK的数据
void shm_cachechunk::get_chunk(const size_t node_index, size_t chunk_no, size_t& szdata, char* outdata)
{
    size_t chunk_num_of_node, size_of_node;
    node_size(node_index, size_of_node, chunk_num_of_node);

    size_t size_of_chunk = cachechunk_head_->size_of_chunk_;
    size_t leftsize = size_of_node;

    //
    size_t inxchunk = (*(cachenode_base_ + node_index)).chunk_index_;

    for (size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    memcpy(outdata, chunkdata_base_ + inxchunk * size_of_chunk, szdata);
}

//根据数据的起始位置，取得这个位置所在CHUNK的数据,（注意只拷贝一个CHUNK的数据）
//如果不是数据的起始位置开始，而是在CHUNK中间（不是0），拷贝回来的数据从data_start开始
void shm_cachechunk::get_chunkdata(const size_t node_index,
                                   const size_t data_start,
                                   size_t& chunk_no,
                                   size_t& szdata,
                                   char* outdata)
{
    chunk_no = 0;
    size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    size_t tmpszdata = data_start;

#if defined _DEBUG || defined DEBUG
    assert(data_start < (*(cachenode_base_ + node_index)).size_of_node_);
#endif

    size_t szonce = 0;

    //tmpszdata 最后==0
    while (tmpszdata >= cachechunk_head_->size_of_chunk_)
    {
        szonce = (tmpszdata > cachechunk_head_->size_of_chunk_) ? cachechunk_head_->size_of_chunk_ : tmpszdata;
        tmpszdata -= szonce;
        chunkinx = (*(chunkindex_base_ + chunkinx));
        ++chunk_no;
    }

    //
    szdata = cachechunk_head_->size_of_chunk_ - tmpszdata;
    memcpy(outdata, chunkdata_base_ + chunkinx * (cachechunk_head_->size_of_chunk_), szdata);
}

//用于每次取一个CHUNK的指针操作，根据NODE索引，第几个CHUNK,返回
//  CHUNK的指针以及相应的长度,注意指针的生命周期,
void shm_cachechunk::get_chunk_point(const size_t node_index,
                                     size_t chunk_no,
                                     size_t& szdata,
                                     char*& chunk_point)
{
    size_t chunk_num_of_node, size_of_node;
    node_size(node_index, size_of_node, chunk_num_of_node);

    size_t size_of_chunk = cachechunk_head_->size_of_chunk_;
    size_t leftsize = size_of_node;

    //
    size_t inxchunk = (*(cachenode_base_ + node_index)).chunk_index_;

    for (size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    chunk_point = chunkdata_base_ + inxchunk * size_of_chunk;
}

//用于根据数据的起始位置，取得这个位置所在CHUNK的指针,以及取得
//在这个CHUNK里面的剩余的数据时
void shm_cachechunk::get_chunkdata_point(const size_t node_index,
                                         const size_t data_start,
                                         size_t& chunk_no,
                                         size_t& szdata,
                                         char*& chunk_data_point)
{
    size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    size_t szofchunk = cachechunk_head_->size_of_chunk_;
    size_t size_of_node_ = (*(cachenode_base_ + node_index)).size_of_node_;

#if defined _DEBUG || defined DEBUG
    assert(data_start < size_of_node_);
#endif

    //不是最后一块
    if (size_of_node_ / szofchunk != data_start / szofchunk)
    {
        szdata = szofchunk - (data_start % szofchunk);
    }
    else
    {
        szdata = szofchunk - (data_start % szofchunk) -
            ((size_of_node_ % szofchunk) == 0 ? 0 : (szofchunk - (size_of_node_ % szofchunk)));
    }

    chunk_no = data_start / szofchunk;

    //找到CHUNK的索引编号
    for (size_t i = 0; i < chunk_no; ++i)
    {
        chunkinx = (*(chunkindex_base_ + chunkinx));
    }

    chunk_data_point = chunkdata_base_ + chunkinx * (cachechunk_head_->size_of_chunk_) + (data_start % szofchunk);
}
};