/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_frame_malloc.cpp
* @author     Sailzeng <sailerzeng@gmail.com>,Scottxu,
* @version
* @date       2008年2月20日
* @brief
*
*
* @details
*
*
*
* @note
*
*/

#ifndef SOARING_LIB_APPFRAME_MALLOCOR_H_
#define SOARING_LIB_APPFRAME_MALLOCOR_H_

#include "soar_zerg_frame.h"

template <typename ZCE_LOCK >
class AppFrame_Mallocor_Mgr
{
protected:
    //桶队列的个数
    static const size_t  NUM_OF_FRAMELIST = 10;

    //每个桶初始化的FRAME的个数
    static const size_t NUM_OF_ONCE_INIT_FRAME = 512;

    //允许空闲的NUM_OF_ALLOW_LIST_IDLE_FRAME
    static const size_t NUM_OF_ALLOW_LIST_IDLE_FRAME = 1024;

    //
    typedef ZCE_LIB::lordrings <Zerg_App_Frame *>     LIST_OF_APPFRAME;
    //
    typedef std::vector< LIST_OF_APPFRAME > APPFRAME_MEMORY_POOL;

protected:

    //GCC的版本如果小于4，会不支持模板中的static数组成员的长度用const static成员定义。
    //理论上可以用#if (__GNUC__ < 4)屏蔽，但是实在太忙法。下面的数组长度NUM_OF_FRAMELIST
    //如果你更改是要定义。
    size_t                  size_appframe_[NUM_OF_FRAMELIST];

    //FRAME的内存池子
    APPFRAME_MEMORY_POOL    frame_pool_;
    //池子的锁
    ZCE_LOCK                zce_lock_;

protected:

    //单子实例
    static AppFrame_Mallocor_Mgr    *instance_;

public:

    //构造函数
    AppFrame_Mallocor_Mgr();
    //析构函数
    ~AppFrame_Mallocor_Mgr();

public:
    //初始化
    void initialize(size_t init_num = NUM_OF_ONCE_INIT_FRAME,
                    size_t max_frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME);

    //从池子分配一个APPFRAME
    Zerg_App_Frame *alloc_appframe(size_t frame_len);
    //释放一个APPFRAME到池子
    void free_appframe(Zerg_App_Frame *proc_frame);
    //复制一个APPFRAME
    void clone_appframe(const Zerg_App_Frame *model_freame, Zerg_App_Frame *&cloned_frame);

    //最大可以分配的FRAME的长度
    size_t GetMaxFrameLength();

    //调整池子的容量
    void AdjustPoolListCapacity();

public:

    //得到SINGLETON的实例
    static AppFrame_Mallocor_Mgr *instance();
    //清理SINGLETON的实例
    static void clean_instance();

protected:

    //扩展LIST的容量
    void ExtendFrameListCapacity(size_t list_no, size_t extend_num);
    //
    inline size_t GetRoundUp(size_t sz_frame);

};

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2008年9月18日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::initialize
Return          : void
Parameter List  :
  Param1: size_t init_num      每个POOL内部的FRAME数量
  Param2: size_t max_frame_len 最大的FRAME的长度，跟进这个分配后面的
Description     : 初始化
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::initialize(size_t init_num,
                                                 size_t max_frame_len)
{
    ZCE_ASSERT(max_frame_len > 2048 && init_num > 8);

    ZLOG_INFO("[framework] AppFrame_Mallocor_Mgr::AppFrame_Mallocor_Mgr init num=%u,max_frame_len=%u.",
              init_num,
              max_frame_len);

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
        ExtendFrameListCapacity(i, init_num);
    }

}

//最大可以分配的FRAME的长度
template <typename ZCE_LOCK >
size_t AppFrame_Mallocor_Mgr<ZCE_LOCK>::GetMaxFrameLength()
{
    return size_appframe_[NUM_OF_FRAMELIST - 1];
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::GetRoundUp
Return          : inline size_t
Parameter List  :
  Param1: size_t sz_frame
Description     : 根据要求的的FRAME尺寸大小，分配一个FRAME
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
inline size_t AppFrame_Mallocor_Mgr<ZCE_LOCK>::GetRoundUp(size_t sz_frame)
{
    //也许循环找还快
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

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::AppFrame_Mallocor_Mgr
Return          : template <typename ZCE_LOCK >
Parameter List  :
  Param1: size_t init_num
Description     : 构造函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK>::AppFrame_Mallocor_Mgr()
{
    memset(size_appframe_, 0, sizeof(size_appframe_));
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::~AppFrame_Mallocor_Mgr
Return          : NULL
Parameter List  : NULL
Description     : 析构函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK>::~AppFrame_Mallocor_Mgr()
{

    //
    ZLOG_INFO("[framework] AppFrame_Mallocor_Mgr::~AppFrame_Mallocor_Mgr.");

    //最后应该size == capacity , freesize==0
    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //如果内存全部归还
        if (frame_pool_[i].freesize() == 0)
        {
            //
            ZLOG_INFO("[framework] List %u(frame size:%u):,free node:%u,capacity node:%u,list node:%u.Ok.",
                      i,
                      size_appframe_[i],
                      frame_pool_[i].freesize(),
                      frame_pool_[i].capacity(),
                      frame_pool_[i].size());
        }
        //如果他在内存
        else
        {
            //
            ZLOG_ERROR("[framework] List %u(frame size:%u):,free node:%u,capacity node:%u,list node:%u.Have memory leak.Please check your code.",
                       i,
                       size_appframe_[i],
                       frame_pool_[i].freesize(),
                       frame_pool_[i].capacity(),
                       frame_pool_[i].size());
        }

        //释放掉分配的空间
        size_t frame_pool_len = frame_pool_[i].size();

        for (size_t j = 0; j < frame_pool_len; ++j)
        {
            Zerg_App_Frame *proc_frame = NULL;
            frame_pool_[i].pop_front(proc_frame);
            delete proc_frame;
        }
    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::alloc_appframe
Return          : template <typename ZCE_LOCK > Zerg_App_Frame*
Parameter List  :
  Param1: size_t frame_len FRAME的长度
Description     : 从池子分配一个APPFRAME
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
Zerg_App_Frame *AppFrame_Mallocor_Mgr<ZCE_LOCK>::alloc_appframe(size_t frame_len)
{
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);
    size_t hk = GetRoundUp(frame_len);

    //
    if (frame_pool_[hk].size() <= 0 )
    {
        ExtendFrameListCapacity(hk, NUM_OF_ONCE_INIT_FRAME);
    }

    //
    Zerg_App_Frame *new_frame = NULL;
    frame_pool_[hk].pop_front(new_frame);

    new_frame->init_framehead(static_cast<unsigned int>(frame_len));

    return new_frame;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::clone_appframe
Return          : template <typename ZCE_LOCK > void
Parameter List  :
  Param1: const Zerg_App_Frame* model_freame 模板FRAME
  Param2: Zerg_App_Frame*& cloned_frame      被克隆的FRAME
Description     : 克隆一个APPFAME
Calls           :
Called By       :
Other           : 这个函数没有加锁，因为感觉不必要，alloc_appframe里面有锁，否则会造成重复加锁
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::clone_appframe(const Zerg_App_Frame *model_freame, Zerg_App_Frame *&cloned_frame)
{
    //
    size_t frame_len = model_freame->frame_length_;
    cloned_frame = alloc_appframe(frame_len);
    memcpy(cloned_frame, model_freame, frame_len);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::free_appframe
Return          : template <typename ZCE_LOCK > void
Parameter List  :
  Param1: Zerg_App_Frame* proc_frame 释放的内存
Description     : 释放一个APPFRAME到池子
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::free_appframe(Zerg_App_Frame *proc_frame)
{
    ZCE_ASSERT(proc_frame);
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);
    size_t hk = GetRoundUp(proc_frame->frame_length_);
    frame_pool_[hk].push_back(proc_frame);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::AdjustPoolListCapacity
Return          : void
Parameter List  : NULL
Description     : 调整池子的容量
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::AdjustPoolListCapacity()
{
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //如果剩余的容量
        if (frame_pool_[i].size() > 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME)
        {
            size_t free_sz = frame_pool_[i].size() - 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME;

            for (size_t j = 0; j < free_sz; ++j)
            {
                Zerg_App_Frame *new_frame = NULL;
                frame_pool_[i].pop_front(new_frame);
                delete new_frame;
            }
        }
    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : AppFrame_Mallocor_Mgr<ZCE_LOCK>::ExtendFrameListCapacity
Return          : void
Parameter List  :
Param1: size_t list_no
Param2: size_t extend_num
Description     : 扩展LIST的容量
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::ExtendFrameListCapacity(size_t list_no, size_t extend_num)
{
    size_t old_capacity =  frame_pool_[list_no].capacity();
    frame_pool_[list_no].resize(old_capacity + extend_num);

    for (size_t j = 0; j < extend_num; ++j)
    {
        Zerg_App_Frame *proc_frame = new(size_appframe_[list_no] + 1) Zerg_App_Frame();
        frame_pool_[list_no].push_back(proc_frame);
    }
}

//得到SINGLETON的实例
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK> *AppFrame_Mallocor_Mgr<ZCE_LOCK>::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new AppFrame_Mallocor_Mgr();
    }

    return instance_;
}

//清理SINGLETON的实例
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

//
typedef AppFrame_Mallocor_Mgr<ZCE_Null_Mutex> NULLMUTEX_APPFRAME_MALLOCOR ;
typedef AppFrame_Mallocor_Mgr<ZCE_Thread_Light_Mutex> THREADMUTEX_APPFRAME_MALLOCOR ;

#endif //#ifndef SOARING_LIB_APPFRAME_MALLOCOR_H_

