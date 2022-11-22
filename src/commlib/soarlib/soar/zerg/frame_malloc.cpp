#include "soar/predefine.h"
#include "soar/zerg/frame_malloc.h"

namespace soar
{
//放入CPP?还是?
zergframe_mallocor* zergframe_mallocor::instance_ = nullptr;

//构造函数
zergframe_mallocor::zergframe_mallocor()
{
    memset(size_appframe_, 0, sizeof(size_appframe_));
}

//析构函数
zergframe_mallocor::~zergframe_mallocor()
{
    //
    ZCE_LOG(RS_INFO, "[framework] zergframe_mallocor::~zergframe_mallocor.");

    //最后应该size == capacity , free==0
    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //如果内存全部归还
        if (frame_pool_[i].free() == 0)
        {
            //
            ZCE_LOG(RS_INFO, "[framework] List %u(frame size:%u):,free node:%u,"
                    "capacity node:%u,list node:%u.Ok.",
                    i,
                    size_appframe_[i],
                    frame_pool_[i].free(),
                    frame_pool_[i].capacity(),
                    frame_pool_[i].size());
        }
        //如果他在内存
        else
        {
            //
            ZCE_LOG(RS_ERROR, "[framework] List %u(frame size:%u):,free node:%u,capacity node:%u,"
                    "list node:%u.Have memory leak.Please check your code.",
                    i,
                    size_appframe_[i],
                    frame_pool_[i].free(),
                    frame_pool_[i].capacity(),
                    frame_pool_[i].size());
        }

        //释放掉分配的空间
        size_t frame_pool_len = frame_pool_[i].size();
        for (size_t j = 0; j < frame_pool_len; ++j)
        {
            soar::zerg_frame* proc_frame = nullptr;
            frame_pool_[i].pop_front(proc_frame);
            soar::zerg_frame::delete_frame(proc_frame);
        }
    }
}

//初始化
void zergframe_mallocor::initialize(bool multi_thread,
                                    size_t init_num,
                                    size_t max_frame_len)
{
    ZCE_ASSERT(max_frame_len > 2048 && init_num > 8);

    ZCE_LOG(RS_INFO, "[framework] zergframe_mallocor::initialize init num=%u,max_frame_len=%u.",
            init_num,
            max_frame_len);
    if (multi_thread)
    {
        my_lock_ = new std::mutex();
    }
    //得到分配
    size_t sz_frame = max_frame_len;

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        size_appframe_[NUM_OF_FRAMELIST - i - 1] = sz_frame;
        sz_frame = sz_frame / 2;
    }

    frame_pool_.resize(NUM_OF_FRAMELIST);

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        extend_list_capacity(i, init_num);
    }
}

//最大可以分配的FRAME的长度
size_t zergframe_mallocor::get_max_framelen()
{
    return size_appframe_[NUM_OF_FRAMELIST - 1];
}

//根据要求的的FRAME尺寸大小，分配一个FRAME
inline size_t zergframe_mallocor::get_roundup(size_t sz_frame)
{
    //也许循环找还快
    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        if (sz_frame <= size_appframe_[i])
        {
            return i;
        }
    }

    //死了好
    ZCE_ASSERT(false);
    return static_cast<size_t>(-1);
}

//根据需要长度，从池子分配一个APPFRAME
soar::zerg_frame* zergframe_mallocor::alloc_appframe(size_t frame_len)
{
    zce::lock_ptr_guard guard(my_lock_);
    size_t hk = get_roundup(frame_len);

    //
    if (frame_pool_[hk].size() <= 0)
    {
        extend_list_capacity(hk, NUM_OF_ONCE_INIT_FRAME);
    }

    //
    soar::zerg_frame* new_frame = nullptr;
    frame_pool_[hk].pop_front(new_frame);
    new_frame->init_head(static_cast<unsigned int>(frame_len));

    return new_frame;
}

//克隆一个APPFAME
//这个函数没有加锁，因为感觉不必要，alloc_appframe里面有锁，否则会造成重复加锁
void zergframe_mallocor::clone_appframe(const soar::zerg_frame* model_freame,
                                        soar::zerg_frame*& cloned_frame)
{
    //
    size_t frame_len = model_freame->length_;
    cloned_frame = alloc_appframe(frame_len);
    model_freame->clone(cloned_frame);
}

//释放一个APPFRAME到池子
void zergframe_mallocor::free_appframe(soar::zerg_frame* proc_frame)
{
    ZCE_ASSERT(proc_frame);
    zce::lock_ptr_guard guard(my_lock_);
    size_t hk = get_roundup(proc_frame->length_);
    frame_pool_[hk].push_back(proc_frame);
}

//调整池子的容量
void zergframe_mallocor::adjust_pool_capacity()
{
    zce::lock_ptr_guard guard(my_lock_);

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //如果剩余的容量
        if (frame_pool_[i].size() > 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME)
        {
            size_t free_sz = frame_pool_[i].size() - 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME;

            for (size_t j = 0; j < free_sz; ++j)
            {
                soar::zerg_frame* new_frame = nullptr;
                frame_pool_[i].pop_front(new_frame);
                soar::zerg_frame::delete_frame(new_frame);
            }
        }
    }
}

//扩展LIST的容量
void zergframe_mallocor::extend_list_capacity(size_t list_no, size_t extend_num)
{
    size_t old_capacity = frame_pool_[list_no].capacity();
    frame_pool_[list_no].reserve(old_capacity + extend_num);

    for (size_t j = 0; j < extend_num; ++j)
    {
        soar::zerg_frame* proc_frame = soar::zerg_frame::new_frame(size_appframe_[list_no] + 1);
        frame_pool_[list_no].push_back(proc_frame);
    }
}

//得到SINGLETON的实例

zergframe_mallocor* zergframe_mallocor::instance()
{
    if (instance_ == nullptr)
    {
        instance_ = new zergframe_mallocor();
    }

    return instance_;
}

//清理SINGLETON的实例
void zergframe_mallocor::clear_inst()
{
    if (instance_)
    {
        delete instance_;
        instance_ = nullptr;
    }
}
}