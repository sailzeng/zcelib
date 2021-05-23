/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/cache_chunk.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年12月16日
* @brief      一个简单的存放变长数据的Cache，用于大小不太固定的数据的缓冲，
*             用最小的内存，存放最大的数据，空间浪费小。
*             将内存分割成若干个大小的桶， 每个数据用N(N>=1)个桶存放，
*
* @details    桶的大小设计毕竟比较有讲究，太大不好，还是会造成依稀浪费，
*             太小也不好，会导致一个数据存放在太多的桶内。
*             负载比，平均数据大小和桶的关系，一般情况大致在2-4比较好，
*
* @note       整体设计思路其实和FAT基本一致，有兴趣的去怀古一下，
*             这个工具在当年小熊的下载器里面被第一次尝试使用，当时的jovi
*             应该帮过不少忙，
*
*/
#ifndef ZCE_LIB_SHM_CACHE_CHUNK_H_
#define ZCE_LIB_SHM_CACHE_CHUNK_H_

#include "zce/shm_container/common.h"

namespace zce
{
/*!
@brief      NODE的信息，NODE就是一个数据，NODE内部会存放最开始的CHUNK，
            （后面的CHUNK会形成一条链）
            NODE的大小，（帮助我们知道数据的实际长度）
*/
class cachechunk_node_index
{
public:
    ///NODE链表的起始桶索引
    size_t     chunk_index_;
    ///NODE的实际尺寸
    size_t     size_of_node_;
};

/*!
@brief      放在内存的头部数据区，用于标识还有多少空闲的NODE，CHUNK等
            信息，也会记录FREENODE，FREECHUNK的起始节点等。
            内部结构，外部不要使用
*/
class _shm_cachechunk_head
{
    ///通过友元让其他人使用
    friend class shm_cachechunk;
private:
    ///构造函数
    _shm_cachechunk_head() :
        size_of_mmap_(0),
        num_of_node_(0),
        usable_of_node_(0),
        num_of_chunk_(0),
        usable_of_chunk_(0),
        size_of_chunk_(0),
        free_node_head_(0),
        free_chunk_head_(0)
    {
    }
    ///析构函数
    ~_shm_cachechunk_head()
    {
    }

private:
    ///内存区的长度
    size_t               size_of_mmap_;

    ///NODE的数量
    size_t               num_of_node_;
    ///还可以使用的NODE的数量，注意是可用，不是已经使用
    size_t               usable_of_node_;

    ///Chunk的数量
    size_t               num_of_chunk_;
    ///还可以使用的CHUNK的数量
    size_t               usable_of_chunk_;

    ///chunk尺寸
    size_t               size_of_chunk_;

    ///FREE NODE的起始节点
    size_t               free_node_head_;
    ///FREE CHUNK的起始节点
    size_t               free_chunk_head_;
};

/*!
* @brief      一个简单的存放变长数据的Cache，用于大小不太固定的数据的缓冲，
*             用最小的内存，存放最大的数据，空间浪费小。
*
*/
class shm_cachechunk : public shm_container
{
protected:
    //构造函数,
    shm_cachechunk();
    //析构函数,
    ~shm_cachechunk();

protected:

    /*!
    * @brief      根据申请的空间,分配一个NODE,
    * @return     bool    是否成功申请
    * @param[in]  size_t    希望申请放入的NODE的长度
    * @param[out] nodeindex 返回参数，申请到的NODE的索引
    */
    bool create_node(size_t, size_t& nodeindex);

    /*!
    * @brief      释放一个NODE,将其归还给FREELIST
    * @param      nodeindex 释放的NODE索引
    */
    void destroy_node(const size_t nodeindex);

public:

    ///@brief      清理所有的数据,将Cache还原成初始化，没有任何数据的样子
    void clear();

    /*!
    * @brief      取剩余空间的大小
    * @param[out] free_node    剩余的NODE数量，还可以放多少个数据
    * @param[out] free_chunk   剩余的桶的数量，
    * @param[out] max_room     剩余的空间，最大可以放多大的数据
    */
    void free(size_t& free_node,
              size_t& free_chunk,
              size_t& max_room);

    /*!
    * @brief      检查是否有足够空间存放一个数据
    * @return     bool   返回值，是否可以放入，
    * @param[in]  szdata 要放入的数据大小
    */
    bool check_enough(size_t szdata);

    /*!
    * @brief      放入一个NODE数据，
    * @return     bool       是否成功放入
    * @param[in]  szdata     数据的大小
    * @param[in]  indata     数据指针
    * @param[out] nodeindex  NODE放入的NODE的索引，根据这个可以找到这个NODE
    */
    bool set_node(const size_t szdata,
                  const char* indata,
                  size_t& nodeindex);

    /*!
    * @brief      得到某个NODE的尺寸
    * @return     size_t    返回NODE的尺寸
    * @param[in]  nodeindex NODE的索引，拜托你传递一个正确的参数，否则行为未定义
    */
    size_t nodesize(const size_t nodeindex);

    /*!
    * @brief      得到某个NODE的尺寸,桶数量，本来打算用一个返回值表示是否取值成功的，
    *             但后来想想我是数组下标操作，还是你来保证参数吧。
    * @param[in]  nodeindex  NODE的索引，拜托你传递一个正确的参数，否则行为未定义
    * @param[out] nodesize   返回参数，NODE的尺寸
    * @param[out] chunknum   返回参数，存放所用的CHUNK的数量
    */
    void nodesize(const size_t nodeindex,
                  size_t& nodesize,
                  size_t& chunknum);

    /*!
    * @brief      取得一个节点的数据
    * @param[in]  nodeindex  存放NODE的索引
    * @param[out] szdata     返回NODE的大小
    * @param[out] outdata    返回的NODE数据数据空间的尺寸你要自己保证喔，
    */
    void get_node(const size_t nodeindex,
                  size_t& szdata,
                  char* outdata);

    /*!
    * @brief      当需要一个个CHUNK取出数据时，得到一个NODE的第N个CHUNK的数据
    * @param[in]  nodeindex  NODE的索引
    * @param[in]  chunk_no   第几个CHUNK，从0开始呀，（注意这不是下标，而是第几个桶）
    * @param[out] szdata     返回参数，返回的数据长度，（小于等于桶长度）
    * @param[out] outdata    返回参数，这个桶的数据，数据空间要大于桶的容量
    */
    void get_chunk(const size_t nodeindex,
                   size_t chunk_no,
                   size_t& szdata,
                   char* outdata);

    /*!
    * @brief      根据数据的起始位置，取得这个位置所在CHUNK的数据,（注意只拷贝一个CHUNK的数据）
    *             如果不是数据的起始位置开始，而是在CHUNK中间（不是0），拷贝回来的数据从data_start
    *             开始
    * @param[in]  nodeindex   NODE的索引
    * @param[in]  data_start  数据的起始位置
    * @param[out] chunk_no    返回参数，这个起始位置，位于第几个桶上
    * @param[out] szdata      返回参数，返回的数据长度
    * @param[out] outdata     返回参数，这个桶的从data_start开始到桶结束位置的数据，
    */
    void get_chunkdata(const size_t nodeindex,
                       const size_t data_start,
                       size_t& chunk_no,
                       size_t& szdata,
                       char* outdata);

    /*!
    * @brief      释放某个NODE节点
    * @param[in]  nodeindex  释放的NODE的索引
    */
    void freenode(const size_t nodeindex);

    /*!
    * @brief      得到CHUNK的定义大小,注意这是CHUNK的容量不是里面数据的大小
    * @return     size_t CHUNK的定义大小
    */
    size_t chunksize();

    /*!
    * @brief      用于每次取一个CHUNK的指针操作，根据NODE索引，第几个CHUNK,返回
    *             CHUNK的指针以及相应的长度,注意指针的生命周期,多线程情况下请注意加锁
    *             多用于一些为了追求极致速度，不希望拷贝数据的地方
    * @param[in]  nodeindex   NODE索引
    * @param[in]  chunk_no    第几个CHUNK，
    * @param[out] szdata      这个CHUNK中数据的长度
    * @param[out] chunk_point 这个CHUNK开始的指针
    */
    void get_chunk_point(const size_t nodeindex,
                         size_t chunk_no,
                         size_t& szdata,
                         char*& chunk_point);

    /*!
    * @brief      用于根据数据的起始位置，取得这个位置所在CHUNK的指针,以及取得
    *             在这个CHUNK里面的剩余的数据时
    *             多用于一些为了追求极致速度，不希望拷贝数据的地方
    * @param[in]  nodeindex        NODE索引
    * @param[in]  data_start       查询的数据的起始位置
    * @param[out] chunk_no         返回参数，这个起始位置，位于第几个桶上
    * @param[out] szdata           返回参数，返回的数据长度，从data_start位置开始，到这个CHUNK结束，的数据长度
    * @param[out] chunk_data_point 返回参数，这个data_start位置在CHUNK中的位置指针
    * @note       注意指针的生命周期，当年jovi用这个好像搞了一套引用技术，呵呵
    */
    void get_chunkdata_point(const size_t nodeindex,
                             const size_t data_start,
                             size_t& chunk_no,
                             size_t& szdata,
                             char*& chunk_data_point);

public:
    /*!
    * @brief      得到初始化所需的内存尺寸，单位字节，你应该根据这个长度去申请内存
    * @return     size_t     所需的内存大小，
    * @param[in]  numnode    NODE节点的个数
    * @param[in]  numchunk   CHUNK节点的个数，多个桶存放一个NODE
    * @param[in]  szchunk    CHUNK的尺寸大小
    */
    static size_t getallocsize(const size_t numnode,
                               const size_t numchunk,
                               const size_t szchunk);

    /*!
    * @brief      根据参数初始化相应的内存，
    * @return     shm_cachechunk* 如果初始化成功，返回指针让你使用
    * @param[in]  numnode     NODE节点的个数
    * @param[in]  numchunk    CHUNK节点的个数
    * @param[in]  szchunk     CHUNK的尺寸大小
    * @param[in]  pmmap       输入的内存指针
    * @param[in]  if_restore  是否恢复原有内存中数据
    */
    static shm_cachechunk* initialize(const size_t numnode,
                                      const size_t numchunk,
                                      const size_t szchunk,
                                      char* pmmap,
                                      bool if_restore = false);

protected:

    ///CACHE的头部，
    _shm_cachechunk_head* cachechunk_head_ = nullptr;

    ///Cache NODE 的BASE指针,NODE表示使用的
    cachechunk_node_index* cachenode_base_ = nullptr;

    ///CHUNK INDEX的BASE指针,
    size_t* chunkindex_base_ = nullptr;

    ///CHUNK DATA数据区的BASE指针
    char* chunkdata_base_ = nullptr;
};
};

#endif //ZCE_LIB_SHM_CACHE_CHUNK_H_
