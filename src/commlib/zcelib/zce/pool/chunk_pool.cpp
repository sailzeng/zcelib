#include "zce/predefine.h"
#include "zce/pool/chunk_pool.h"

namespace zce
{
//得到初始化所需内存的大小
std::size_t chunk_pool::alloc_size(const std::size_t numnode,
                                   const std::size_t numchunk,
                                   const std::size_t szchunk)
{
    return  sizeof(_chunkpool_head)
        + numnode * sizeof(chunkpool_node_index)
        + numchunk * sizeof(std::size_t)
        + numchunk * szchunk;
}

//根据参数初始化
int chunk_pool::initialize(std::size_t numnode,
                           std::size_t numchunk,
                           std::size_t szchunk,
                           char* mem_addr,
                           bool if_restore)
{
    //
    _chunkpool_head* chunk_head = reinterpret_cast<_chunkpool_head*>(mem_addr);
    //如果采用恢复的形式,
    if (if_restore == true)
    {
        //检查所有的尺寸,如果有不对的地方返回nullptr,这样可以保证不出现错误
        if (chunk_head->size_of_mmap_ != alloc_size(numnode, numchunk, szchunk) ||
            chunk_head->num_of_node_ != numnode ||
            chunk_head->num_of_chunk_ != numchunk ||
            chunk_head->size_of_chunk_ != szchunk)
        {
            return -1;
        }
    }

    //得到空间大小
    chunk_head->size_of_mmap_ = alloc_size(numnode, numchunk, szchunk);
    chunk_head->num_of_node_ = numnode;
    chunk_head->num_of_chunk_ = numchunk;
    chunk_head->size_of_chunk_ = szchunk;

    //处理一些偏移量的指针
    mem_addr_ = mem_addr;
    chunkpool_head_ = chunk_head;
    cachenode_base_ =
        reinterpret_cast<chunkpool_node_index*>(mem_addr + sizeof(_chunkpool_head));
    chunkindex_base_ =
        reinterpret_cast<std::size_t*>(mem_addr + sizeof(_chunkpool_head) \
                                       + numnode * sizeof(chunkpool_node_index));
    chunkdata_base_ =
        mem_addr + sizeof(_chunkpool_head) + \
        numnode * sizeof(chunkpool_node_index) + numchunk * sizeof(std::size_t);

    //初始化所有的内存
    if (false == if_restore)
    {
        clear();
    }
    return 0;
}

int chunk_pool::initialize(std::size_t numnode,
                           std::size_t numchunk,
                           std::size_t szchunk)
{
    std::size_t sz_pool = alloc_size(numnode, numchunk, szchunk);
    char *pool_area = new char[sz_pool];
    self_alloc_mem_ = true;
    return initialize(numnode, numchunk, szchunk, pool_area, false);
}

//! 销毁
void chunk_pool::terminate()
{
    if (self_alloc_mem_)
    {
        delete[] mem_addr_;
    }
}

//
void chunk_pool::clear()
{
    //初始化free数据区
    for (std::size_t i = 0; i < chunkpool_head_->num_of_node_; ++i)
    {
        cachenode_base_[i].chunk_index_ = i + 1;
        cachenode_base_[i].size_of_node_ = 0;

        if (i == chunkpool_head_->num_of_node_ - 1)
        {
            cachenode_base_[i].chunk_index_ = INDEX_INVALID;
        }
    }

    chunkpool_head_->free_node_head_ = 0;

    //清理FREELIST的单向NODE,
    for (std::size_t i = 0; i < chunkpool_head_->num_of_chunk_; ++i)
    {
        chunkindex_base_[i] = i + 1;

        if (i == chunkpool_head_->num_of_chunk_ - 1)
        {
            chunkindex_base_[i] = INDEX_INVALID;
        }
    }

    //
    chunkpool_head_->free_chunk_head_ = 0;
    //将可用的数量进行一下修正
    chunkpool_head_->usable_of_node_ = chunkpool_head_->num_of_node_;
    chunkpool_head_->usable_of_chunk_ = chunkpool_head_->num_of_chunk_;
}

//根据申请的空间,分配一个NODE,
bool chunk_pool::create_node(std::size_t szdata, std::size_t& node_index)
{
    //如果没有了NODE,或者没有了CHUNK
    bool bret = check_enough(szdata);

    if (false == bret)
    {
        return false;
    }

    //检查剩余的空间十分足够
    std::size_t chunkinx = (chunkpool_head_->free_chunk_head_);
    std::size_t tmpszdata = szdata;

    //循环检查空间是否足够的看似也许是低效的,但是你必须得到队尾,所以必须有个循环过程,

    //use_chunk为使用的CHUNK数量，
    std::size_t use_chunk = 1;

    for (; tmpszdata > chunkpool_head_->size_of_chunk_; ++use_chunk)
    {
        tmpszdata -= chunkpool_head_->size_of_chunk_;

        //根据前面的判断，这个
        if (chunkinx == INDEX_INVALID)
        {
            return false;
        }

        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    node_index = chunkpool_head_->free_node_head_;
    chunkpool_head_->free_node_head_ = (*(cachenode_base_ + node_index)).chunk_index_;

    (*(cachenode_base_ + node_index)).chunk_index_ = chunkpool_head_->free_chunk_head_;
    (*(cachenode_base_ + node_index)).size_of_node_ = szdata;

    //将FREE链子的头节点放入
    chunkpool_head_->free_chunk_head_ = *(chunkindex_base_ + chunkinx);

    *(chunkindex_base_ + chunkinx) = INDEX_INVALID;

    //记录使用的数据节点数量
    --(chunkpool_head_->usable_of_node_);
    chunkpool_head_->usable_of_chunk_ -= use_chunk;

    return true;
}

//释放一个NODE,将其的桶归还给FREELIST
void chunk_pool::destroy_node(const std::size_t node_index)
{
    //归还CHUNK
    std::size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    std::size_t fchunkinx = chunkinx;
    //循环释放CHUNK
    //free_chunk为释放的CHUNK数量，
    std::size_t free_chunk = 1;

    for (; *(chunkindex_base_ + chunkinx) != INDEX_INVALID; ++free_chunk)
    {
        chunkinx = *(chunkindex_base_ + chunkinx);
    }

    *(chunkindex_base_ + chunkinx) = chunkpool_head_->free_chunk_head_;
    chunkpool_head_->free_chunk_head_ = fchunkinx;

    //归还NODE
    (*(cachenode_base_ + node_index)).chunk_index_ = chunkpool_head_->free_node_head_;
    (*(cachenode_base_ + node_index)).size_of_node_ = 0;

    chunkpool_head_->free_node_head_ = node_index;

    //注意下面改的几个内部变量是可用的数量，不是已经用的。
    ++(chunkpool_head_->usable_of_node_);
    chunkpool_head_->usable_of_chunk_ += free_chunk;
}

//取剩余空间的大小
void chunk_pool::free(std::size_t& free_node, std::size_t& free_chunk, std::size_t& max_room)
{
    free_node = chunkpool_head_->usable_of_node_;
    free_chunk = chunkpool_head_->usable_of_chunk_;
    max_room = chunkpool_head_->usable_of_chunk_ * chunkpool_head_->size_of_chunk_;
}

//检查是否可以放入一个这样大小的数据
bool chunk_pool::check_enough(std::size_t szdata)
{
    if (chunkpool_head_->usable_of_node_ == 0 ||
        chunkpool_head_->usable_of_chunk_ == 0 ||
        (chunkpool_head_->usable_of_chunk_) * chunkpool_head_->size_of_chunk_ <= szdata)
    {
        return false;
    }

    return true;
}

//将一个NODE放入Cache中，
bool chunk_pool::push_node(const std::size_t szdata,
                           const char* indata,
                           std::size_t& node_index)
{
    std::size_t tmpindex;
    bool bret = create_node(szdata, tmpindex);

    if (bret == false)
    {
        return false;
    }

    node_index = tmpindex;
    std::size_t chunkinx = (*(cachenode_base_ + tmpindex)).chunk_index_;

    std::size_t szonce = 0, szcpy = 0;
    std::size_t tmpszdata = szdata;

    //最后tmpszdata ==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata > chunkpool_head_->size_of_chunk_) ? chunkpool_head_->size_of_chunk_ : tmpszdata;
        memcpy(chunkdata_base_ + chunkinx * chunkpool_head_->size_of_chunk_, indata + szcpy, szonce);
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
std::size_t chunk_pool::node_size(const std::size_t node_index)
{
    return (*(cachenode_base_ + node_index)).size_of_node_;
}

//得到某个NODE的尺寸,以及相应的chunk的数量
void chunk_pool::node_size(const std::size_t node_index, std::size_t& node_size, std::size_t& chunknum)
{
    node_size = (*(cachenode_base_ + node_index)).size_of_node_;

    if (0 == (node_size % (chunkpool_head_->size_of_chunk_)))
    {
        chunknum = node_size / (chunkpool_head_->size_of_chunk_);
    }
    else
    {
        chunknum = node_size / (chunkpool_head_->size_of_chunk_) + 1;
    }
}

//得到CHUNK的大小,注意这是CHUNK的容量不是里面数据的大小
std::size_t chunk_pool::chunk_capacity()
{
    return chunkpool_head_->size_of_chunk_;
}

//取回NODE的数据，以及大小
void chunk_pool::pull_node(const std::size_t node_index, std::size_t& szdata, char* outdata)
{
    std::size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    std::size_t tmpszdata = (*(cachenode_base_ + node_index)).size_of_node_;
    szdata = tmpszdata;

    std::size_t szonce = 0, szcpy = 0;

    //szdata 最后==0
    while (tmpszdata > 0)
    {
        szonce = (tmpszdata >= chunkpool_head_->size_of_chunk_) ? chunkpool_head_->size_of_chunk_ : tmpszdata;
        memcpy(outdata + szcpy, chunkdata_base_ + chunkinx * (chunkpool_head_->size_of_chunk_), szonce);
        szcpy += szonce;
        tmpszdata -= szonce;

        if (tmpszdata > 0)
        {
            chunkinx = (*(chunkindex_base_ + chunkinx));
        }
    }
}

//释放NODE的索引对应的所有CHUNK，
void chunk_pool::free_node(const std::size_t node_index)
{
    destroy_node(node_index);
}

//当需要一个个CHUNK取出数据时，得到一个NODE的第N个CHUNK的数据
void chunk_pool::get_chunk(const std::size_t node_index, std::size_t chunk_no, std::size_t& szdata, char* outdata)
{
    std::size_t chunk_num_of_node, size_of_node;
    node_size(node_index, size_of_node, chunk_num_of_node);

    std::size_t size_of_chunk = chunkpool_head_->size_of_chunk_;
    std::size_t leftsize = size_of_node;

    //
    std::size_t inxchunk = (*(cachenode_base_ + node_index)).chunk_index_;

    for (std::size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    memcpy(outdata, chunkdata_base_ + inxchunk * size_of_chunk, szdata);
}

//根据数据的起始位置，取得这个位置所在CHUNK的数据,（注意只拷贝一个CHUNK的数据）
//如果不是数据的起始位置开始，而是在CHUNK中间（不是0），拷贝回来的数据从data_start开始
void chunk_pool::get_chunkdata(const std::size_t node_index,
                               const std::size_t data_start,
                               std::size_t& chunk_no,
                               std::size_t& szdata,
                               char* outdata)
{
    chunk_no = 0;
    std::size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    std::size_t tmpszdata = data_start;

#if defined _DEBUG || defined DEBUG
    assert(data_start < (*(cachenode_base_ + node_index)).size_of_node_);
#endif

    std::size_t szonce = 0;

    //tmpszdata 最后==0
    while (tmpszdata >= chunkpool_head_->size_of_chunk_)
    {
        szonce = (tmpszdata > chunkpool_head_->size_of_chunk_) ? chunkpool_head_->size_of_chunk_ : tmpszdata;
        tmpszdata -= szonce;
        chunkinx = (*(chunkindex_base_ + chunkinx));
        ++chunk_no;
    }

    //
    szdata = chunkpool_head_->size_of_chunk_ - tmpszdata;
    memcpy(outdata, chunkdata_base_ + chunkinx * (chunkpool_head_->size_of_chunk_), szdata);
}

//用于每次取一个CHUNK的指针操作，根据NODE索引，第几个CHUNK,返回
//  CHUNK的指针以及相应的长度,注意指针的生命周期,
void chunk_pool::get_chunk_point(const std::size_t node_index,
                                 std::size_t chunk_no,
                                 std::size_t& szdata,
                                 char*& chunk_point)
{
    std::size_t chunk_num_of_node, size_of_node;
    node_size(node_index, size_of_node, chunk_num_of_node);

    std::size_t size_of_chunk = chunkpool_head_->size_of_chunk_;
    std::size_t leftsize = size_of_node;

    //
    std::size_t inxchunk = (*(cachenode_base_ + node_index)).chunk_index_;

    for (std::size_t i = 0; i < chunk_no; ++i)
    {
        inxchunk = (*(chunkindex_base_ + inxchunk));
        leftsize -= size_of_chunk;
    }

    szdata = chunk_no == (chunk_num_of_node - 1) ? leftsize : size_of_chunk;
    chunk_point = chunkdata_base_ + inxchunk * size_of_chunk;
}

//用于根据数据的起始位置，取得这个位置所在CHUNK的指针,以及取得
//在这个CHUNK里面的剩余的数据时
void chunk_pool::get_chunkdata_point(const std::size_t node_index,
                                     const std::size_t data_start,
                                     std::size_t& chunk_no,
                                     std::size_t& szdata,
                                     char*& chunk_data_point)
{
    std::size_t chunkinx = (*(cachenode_base_ + node_index)).chunk_index_;
    std::size_t szofchunk = chunkpool_head_->size_of_chunk_;
    std::size_t size_of_node_ = (*(cachenode_base_ + node_index)).size_of_node_;

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
    for (std::size_t i = 0; i < chunk_no; ++i)
    {
        chunkinx = (*(chunkindex_base_ + chunkinx));
    }

    chunk_data_point = chunkdata_base_ + chunkinx * (chunkpool_head_->size_of_chunk_) + (data_start % szofchunk);
}
};