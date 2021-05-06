#include "zce/predefine.h"
#include "zce/lockfree/kfifo.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/error.h"
#include "zce/shared_mem/mmap.h"
#include "zce/logger/logging.h"
#include "zce/bus/mmap_pipe.h"

namespace zce
{

/****************************************************************************************************
class  MMAP_BusPipe::BUS_PIPE_HEAD
****************************************************************************************************/
//PIPE HEAD的构造函数
MMAP_BusPipe::BUS_PIPE_HEAD::BUS_PIPE_HEAD():
    size_of_sizet_(sizeof(size_t)),
    number_of_pipe_(0)
{
    memset(size_of_pipe_,0,sizeof(size_of_pipe_));
    memset(size_of_room_,0,sizeof(size_of_room_));
}

MMAP_BusPipe::BUS_PIPE_HEAD::~BUS_PIPE_HEAD()
{
}

/****************************************************************************************************
class  MMAP_BusPipe
****************************************************************************************************/

//构造函数
MMAP_BusPipe::MMAP_BusPipe()
{
    memset(bus_pipe_pointer_,0,sizeof(bus_pipe_pointer_));
}

MMAP_BusPipe::~MMAP_BusPipe()
{
    mmap_file_.flush();

    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        delete bus_pipe_pointer_[i];
        bus_pipe_pointer_[i] = NULL;

        bus_head_.size_of_pipe_[i] = 0;
        bus_head_.size_of_room_[i] = 0;
    }
}

//初始化
int MMAP_BusPipe::initialize(const char *bus_mmap_name,
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
        zce_os_stat mmapfile_stat;
        ret = zce::stat(bus_mmap_name,&mmapfile_stat);
        //不存在，恢复个毛线
        if (ret != 0)
        {
            if_restore = false;
        }
    }

    size_t sz_malloc = 0;
    sz_malloc += sizeof(BUS_PIPE_HEAD);

    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        size_t sz_room = zce::lockfree::shm_dequechunk::getallocsize(bus_head_.size_of_pipe_[i]);
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
        ZCE_LOG(RS_ERROR,"[zcelib] MMAP map a file (%s) to share memory fail,ret =%d, last error=%d|%s.",
                bus_mmap_name,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    if (if_restore)
    {
        BUS_PIPE_HEAD *pipe_head = static_cast<BUS_PIPE_HEAD *>(mmap_file_.addr());

        //对于各种长度进行检查
        if (pipe_head->size_of_sizet_ != bus_head_.size_of_sizet_
            || pipe_head->number_of_pipe_ != bus_head_.number_of_pipe_)
        {
            ZCE_LOG(RS_ERROR,"[zcelib] MMAP_BusPipe::initialize pipe fail. BUS_PIPE_HEAD old size_t_len[%u] numpipe[%u],new size_t_len[%u],numpipe[%u] ",
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
                ZCE_LOG(RS_ERROR,"[zcelib] MMAP_BusPipe::initialize pipe fail. BUS_PIPE_HEAD <%u> old size_t_len[%u] numpipe[%u],new size_t_len[%u],numpipe[%u] .",
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
    memcpy(mmap_file_.addr(),&bus_head_,sizeof(BUS_PIPE_HEAD));

    //初始化所有的管道
    ret = init_all_pipe(max_frame_len,if_restore);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//初始化，只根据文件进行初始化，用于某些工具对MMAP文件进行处理的时候
//size_t max_frame_len参数有点讨厌，但如果不用这个参数，底层很多代码要改，
//而且对于一个项目，这个值应该应该是一个常量
int MMAP_BusPipe::initialize(const char *bus_mmap_name,
                             size_t max_frame_len)
{
    int ret = 0;

    zce_os_stat mmapfile_stat;
    ret = zce::stat(bus_mmap_name,&mmapfile_stat);

    if (ret != 0)
    {
        return -1;
    }

    if ((size_t)mmapfile_stat.st_size <= sizeof(BUS_PIPE_HEAD))
    {
        return -1;
    }

    //MAP一个文件
    ret = mmap_file_.open(bus_mmap_name,
                          static_cast<size_t>(mmapfile_stat.st_size),
                          true);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR,"[zcelib] MMAP map a file (%s) to share memory fail,ret =%d, last error=%d|%s.",
                bus_mmap_name,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    BUS_PIPE_HEAD *pipe_head = static_cast<BUS_PIPE_HEAD *>(mmap_file_.addr());
    bus_head_ = *pipe_head;

    //初始化所有的管道
    ret = init_all_pipe(max_frame_len,true);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//初始化所有的数据管道
int MMAP_BusPipe::init_all_pipe(size_t max_frame_len,
                                bool if_restore)
{
    size_t file_offset = 0;
    //偏移一个头部
    file_offset = sizeof(BUS_PIPE_HEAD);

    //循环初始化每个PIPE
    for (size_t i = 0; i < bus_head_.number_of_pipe_; ++i)
    {
        char *pt_pipe = static_cast<char *>(mmap_file_.addr()) + file_offset;

        //初始化内存
        bus_pipe_pointer_[i] = zce::lockfree::shm_dequechunk::initialize(bus_head_.size_of_pipe_[i],
                                                                         max_frame_len,
                                                                         pt_pipe,
                                                                         if_restore
        );

        //管道创建自己也会检查是否能恢复
        if (bus_pipe_pointer_[i] == NULL)
        {
            ZCE_LOG(RS_ERROR,"[zcelib] MMAP_BusPipe::initialize pipe[%u] size[%u] room[%u] fail.",
                    i,
                    bus_head_.size_of_pipe_[i],
                    bus_head_.size_of_room_[i]);
            return -1;
        }

        ZCE_ASSERT(bus_pipe_pointer_[i] != NULL);

        size_t sz_room = zce::lockfree::shm_dequechunk::getallocsize(bus_head_.size_of_pipe_[i]);
        file_offset += sz_room;
    }

    return 0;
}

//MMAP隐射文件名称
const char *MMAP_BusPipe::mmap_file_name()
{
    return mmap_file_.file_name();
}

}