/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   mmap_pipe.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2010年？
* @brief      共享内存的管道。用于多进程通信。
*
*
* @details
*
*
*
* @note
*
*/

#pragma once

//美国著名计算机专家、C语言及Unix之父、图灵奖获得者丹尼斯·里奇（Dennis Ritchie）
//于2011年10日12日于家中去世，享年70岁。

#include "zce/util/non_copyable.h"
#include "zce/shared_mem/mmap.h"
#include "zce/lockfree/kfifo.h"

namespace zce
{
//
typedef zce::lockfree::kfifo<uint32_t> bus_pipe;
//
typedef zce::lockfree::kfifo<uint32_t>::node bus_node;

//MMAP的管道，你要初始化几条就初始化几条
template<size_t MAX_PIPE>
class mmap_buspipe
{
protected:

    //
    template<size_t PIPE_SIZE>
    struct BUS_PIPE_HEAD
    {
        BUS_PIPE_HEAD() :
            size_of_sizet_(sizeof(size_t))
        {
            memset(size_of_pipe_, 0, sizeof(size_of_pipe_));
            memset(size_of_room_, 0, sizeof(size_of_room_));
        }

        ~BUS_PIPE_HEAD() = default;

        //机器字长
        uint32_t            size_of_sizet_;

        //管道数量
        uint32_t            number_of_pipe_ = PIPE_SIZE;
        //管道配置长度,2个管道的配置长度,
        size_t              size_of_pipe_[PIPE_SIZE];
        //
        size_t              size_of_room_[PIPE_SIZE];
    };

public:
    //构造函数,
    mmap_buspipe()
    {
        memset(bus_pipe_pointer_, 0, sizeof(bus_pipe_pointer_));
    }
    //析购函数
    ~mmap_buspipe()
    {
        mmap_file_.flush();

        for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
        {
            bus_head_.size_of_pipe_[i] = 0;
            bus_head_.size_of_room_[i] = 0;
        }
    }

    mmap_buspipe(const mmap_buspipe &) = delete;
    mmap_buspipe& operator=(const mmap_buspipe&) = delete;

public:

    /*!
    * @brief      初始化，根据参数，
    * @return     int
    * @param      bus_mmap_name
    * @param      number_of_pipe  管道的数量
    * @param      size_of_pipe    管道的大小
    * @param      max_frame_len   最大的帧长度
    * @param      if_restore      是否进行恢复
    * @note
    */
    int initialize(const char* bus_mmap_name,
                   uint32_t number_of_pipe,
                   size_t size_of_pipe[],
                   size_t max_frame_len,
                   bool if_restore);

    ///初始化，只根据文件进行初始化，用于某些工具对MMAP文件进行处理的时候
    int initialize(const char* bus_mmap_name,
                   size_t max_frame_len);

    //-----------------------------------------------------------------
    //怀疑TMD我有强迫症倾向，提供这么多接口干嘛，下面一组足够用了。
    bool is_exist_bus(size_t pipe_id)
    {
        return bus_pipe_pointer_[pipe_id] == nullptr ? false : true;
    }

    //MMAP隐射文件名称
    const char* mmap_file_name();

    //向管道写入帧
    inline int push_back_bus(size_t pipe_id, const bus_node* node)
    {
        //取出一个帧
        bool bret = bus_pipe_pointer_[pipe_id].push_end(node);

        //
        if (!bret)
        {
            ZCE_LOG(RS_ALERT,
                    "[zcelib] %u Pipe is full or data small?,Some data can't put to pipe. "
                    "Please increase and check. nodesize=%lu, freesize=%lu,capacity=%lu",
                    pipe_id,
                    node->size_of_node_,
                    bus_pipe_pointer_[pipe_id].free(),
                    bus_pipe_pointer_[pipe_id].capacity()
            );
            return -1;
        }

        return 0;
    }

    /*!
    * @brief      从管道POP读取帧，(就是拷贝后删除)
    * @return     int
    * @param      pipe_id  管道ID
    * @param      node     准备复制node指针，指针的空间请分配好
    * @note
    */
    inline int pop_front_bus(size_t pipe_id, bus_node* node)
    {
        if (bus_pipe_pointer_[pipe_id].empty())
        {
            return -1;
        }

        //取出一个帧
        bus_pipe_pointer_[pipe_id].pop_front(node);

        return 0;
    }

    /*!
    * @brief      从管道拷贝复制一个帧出来
    * @return     int
    * @param      pipe_id 管道ID
    * @param      node
    * @note
    */
    inline int read_front_bus(size_t pipe_id, bus_node*& node)
    {
        if (bus_pipe_pointer_[pipe_id].empty())
        {
            return -1;
        }

        //取出一个帧
        bus_pipe_pointer_[pipe_id].read_front(node);

        return 0;
    }

    //抛弃一个帧
    inline int pop_front_bus(size_t pipe_id)
    {
        if (bus_pipe_pointer_[pipe_id].empty())
        {
            return -1;
        }

        //取出一个帧
        bus_pipe_pointer_[pipe_id].discard_frond();
        return 0;
    }

    //取管道头的帧长
    inline int get_front_nodesize(size_t pipe_id, size_t& note_size)
    {
        if (bus_pipe_pointer_[pipe_id].empty())
        {
            return -1;
        }

        note_size = bus_pipe_pointer_[pipe_id].get_front_len();
        return 0;
    }

    //管道为满
    inline bool is_full_bus(size_t pipe_id)
    {
        return bus_pipe_pointer_[pipe_id].full();
    }

    //管道是否为空
    inline bool is_empty_bus(size_t pipe_id)
    {
        return bus_pipe_pointer_[pipe_id].empty();
    }

    //管道的空余空间,
    inline void get_bus_freesize(size_t pipe_id,
                                 size_t& pipe_size,
                                 size_t& free)
    {
        pipe_size = bus_head_.size_of_pipe_[pipe_id];
        free = bus_pipe_pointer_[pipe_id].free();
        return;
    }
    //-----------------------------------------------------------------
protected:

    //初始化所有的数据管道
    int init_all_pipe(size_t max_frame_len,
                      bool if_restore);

protected:

    ///BUS文件的头部信息
    BUS_PIPE_HEAD<MAX_PIPE>      bus_head_;

    ///N个管道,比如接收管道,发送管道……,最大MAX_NUMBER_OF_PIPE个
    bus_pipe bus_pipe_pointer_[MAX_PIPE];

    ///MMAP内存文件，
    zce::shm_mmap      mmap_file_;
};

//初始化
template<size_t MAX_PIPE>
int mmap_buspipe<MAX_PIPE>::initialize(const char* bus_mmap_name,
                                       uint32_t number_of_pipe,
                                       size_t size_of_pipe[],
                                       size_t max_frame_len,
                                       bool if_restore)
{
    int ret = 0;

    assert(number_of_pipe > 0);

    if (number_of_pipe == 0)
    {
        return -1;
    }

    bus_head_.number_of_pipe_ = number_of_pipe;

    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        bus_head_.size_of_pipe_[i] = size_of_pipe[i];
    }

    //是否对管道进行加锁

    //Malloc分配器,

    //如果不恢复,干脆删除原有的MMAP文件,避免使用的时候出现问题.
    if (if_restore == false)
    {
        zce::unlink(bus_mmap_name);
    }
    //如果没有这个文件,那么只能重建
    else
    {
        struct stat mmapfile_stat;
        ret = zce::stat(bus_mmap_name, &mmapfile_stat);
        //不存在，恢复个毛线
        if (ret != 0)
        {
            if_restore = false;
        }
    }

    size_t sz_malloc = 0;
    sz_malloc += sizeof(bus_head_);

    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        size_t sz_room = bus_pipe::alloc_size(bus_head_.size_of_pipe_[i]);
        bus_head_.size_of_room_[i] = sz_room;
        sz_malloc += sz_room;
    }

    //处理共享内存的操作方式

    //MAP一个文件
    ret = mmap_file_.open(bus_mmap_name,
                          sz_malloc,
                          if_restore);

    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] MMAP map a file (%s) to share memory fail,ret =%d, last error=%d|%s.",
                bus_mmap_name,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    if (if_restore)
    {
        auto pipe_head = static_cast<BUS_PIPE_HEAD<MAX_PIPE> *>(mmap_file_.addr());

        //对于各种长度进行检查
        if (pipe_head->size_of_sizet_ != bus_head_.size_of_sizet_
            || pipe_head->number_of_pipe_ != bus_head_.number_of_pipe_)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] mmap_buspipe::initialize pipe fail. BUS_PIPE_HEAD "
                    "old size_t_len[%u] numpipe[%u],new size_t_len[%u],numpipe[%u] ",
                    pipe_head->size_of_sizet_,
                    pipe_head->number_of_pipe_,
                    bus_head_.size_of_sizet_,
                    bus_head_.number_of_pipe_);
            return -1;
        }

        for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
        {
            if (pipe_head->size_of_pipe_[i] != bus_head_.size_of_pipe_[i]
                || pipe_head->size_of_room_[i] != bus_head_.size_of_room_[i])
            {
                ZCE_LOG(RS_ERROR, "[zcelib] mmap_buspipe::initialize pipe fail. BUS_PIPE_HEAD "
                        "<%u> old size_t_len[%u] numpipe[%u],new size_t_len[%u],numpipe[%u] .",
                        i,
                        pipe_head->size_of_pipe_[i],
                        pipe_head->size_of_room_[i],
                        bus_head_.size_of_pipe_[i],
                        bus_head_.size_of_room_[i]);
                return -1;
            }
        }
    }

    //把头部放入映射文件的头部
    memcpy(mmap_file_.addr(), &bus_head_, sizeof(bus_head_));

    //初始化所有的管道
    ret = init_all_pipe(max_frame_len, if_restore);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//初始化，只根据文件进行初始化，用于某些工具对MMAP文件进行处理的时候
//size_t max_frame_len参数有点讨厌，但如果不用这个参数，底层很多代码要改，
//而且对于一个项目，这个值应该应该是一个常量
template<size_t MAX_PIPE>
int mmap_buspipe<MAX_PIPE>::initialize(const char* bus_mmap_name,
                                       size_t max_frame_len)
{
    int ret = 0;

    struct stat mmapfile_stat;
    ret = zce::stat(bus_mmap_name, &mmapfile_stat);

    if (ret != 0)
    {
        return -1;
    }

    if ((size_t)mmapfile_stat.st_size <= sizeof(bus_head_))
    {
        return -1;
    }

    //MAP一个文件
    ret = mmap_file_.open(bus_mmap_name,
                          static_cast<size_t>(mmapfile_stat.st_size),
                          true);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] MMAP map a file (%s) to share memory fail,ret =%d, last error=%d|%s.",
                bus_mmap_name,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    auto pipe_head = static_cast<mmap_buspipe::BUS_PIPE_HEAD<MAX_PIPE>*>(mmap_file_.addr());
    bus_head_ = *pipe_head;

    //初始化所有的管道
    ret = init_all_pipe(max_frame_len, true);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//初始化所有的数据管道
template<size_t MAX_PIPE>
int mmap_buspipe<MAX_PIPE>::init_all_pipe(size_t max_frame_len,
                                          bool if_restore)
{
    size_t file_offset = 0;
    //偏移一个头部
    file_offset = sizeof(mmap_buspipe::BUS_PIPE_HEAD<MAX_PIPE>);

    //循环初始化每个PIPE
    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        char* pt_pipe = static_cast<char*>(mmap_file_.addr()) + file_offset;

        //初始化内存
        auto succ = bus_pipe_pointer_[i].initialize(bus_head_.size_of_pipe_[i],
                                                    max_frame_len,
                                                    pt_pipe,
                                                    if_restore);

        //管道创建自己也会检查是否能恢复
        if (succ)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] mmap_buspipe::initialize pipe[%u] size[%u] room[%u] fail.",
                    i,
                    bus_head_.size_of_pipe_[i],
                    bus_head_.size_of_room_[i]);
            return -1;
        }

        size_t sz_room = bus_pipe::alloc_size(bus_head_.size_of_pipe_[i]);
        file_offset += sz_room;
    }

    return 0;
}

//MMAP隐射文件名称
template<size_t MAX_PIPE>
const char* mmap_buspipe<MAX_PIPE>::mmap_file_name()
{
    return mmap_file_.file_name();
}
}
