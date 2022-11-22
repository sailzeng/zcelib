/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_frame_malloc.cpp
* @author     Sailzeng <sailzeng.cn@gmail.com>,Scottxu,
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

#include "soar/zerg/frame_zerg.h"

namespace soar
{
class zergframe_mallocor
{
public:

    //构造函数
    zergframe_mallocor();
    //析构函数
    ~zergframe_mallocor();

public:

    /*!
    * @brief      初始化
    * @param      init_num   每个POOL内部的FRAME数量
    * @param      max_frame_len 最大的FRAME的长度，跟进这个分配后面的
    * @note
    */
    void initialize(bool multi_thread,
                    size_t init_num = NUM_OF_ONCE_INIT_FRAME,
                    size_t max_frame_len = soar::zerg_frame::MAX_LEN_OF_FRAME);

    /*!
    * @brief      根据要求的的FRAME尺寸大小，分配一个FRAME
    * @return     soar::zerg_frame*
    * @param      frame_len
    */
    soar::zerg_frame* alloc_appframe(size_t frame_len);

    /*!
    * @brief      释放一个APPFRAME到池子
    * @param      proc_frame 处理的frame
    */
    void free_appframe(soar::zerg_frame* proc_frame);

    /*!
    * @brief      复制一个APPFRAME
    * @param      model_freame  模板FRAME
    * @param      cloned_frame  被克隆的FRAME
    * @note
    */
    void clone_appframe(const soar::zerg_frame* model_freame,
                        soar::zerg_frame*& cloned_frame);

    /*!
    * @brief      返回最大可以分配的FRAME的长度
    * @return     size_t
    */
    size_t get_max_framelen();

    //调整池子的容量,会适当裁剪回收
    void adjust_pool_capacity();

protected:

    /*!
    * @brief      扩展POOL中某个LIST的容量
    * @param      list_no  LiST的下标
    * @param      extend_num 扩展的数量
    */
    void extend_list_capacity(size_t list_no,
                              size_t extend_num);

    /*!
    * @brief
    * @return     size_t   返回相关的LiST的下标编号
    * @param      sz_frame 需要分配的frame的长度
    */
    inline size_t get_roundup(size_t sz_frame);

public:

    //得到SINGLETON的实例
    static zergframe_mallocor* instance();
    //清理SINGLETON的实例
    static void clear_inst();

protected:
    //桶队列的个数
    static const size_t  NUM_OF_FRAMELIST = 10;

    //每个桶初始化的FRAME的个数
    static const size_t NUM_OF_ONCE_INIT_FRAME = 512;

    //允许空闲的NUM_OF_ALLOW_LIST_IDLE_FRAME
    static const size_t NUM_OF_ALLOW_LIST_IDLE_FRAME = 1024;

    //
    typedef zce::lord_rings <soar::zerg_frame*>     LIST_OF_APPFRAME;
    //
    typedef std::vector< LIST_OF_APPFRAME > APPFRAME_POOL;
protected:
    //单子实例
    static zergframe_mallocor* instance_;

protected:

    //GCC的版本如果小于4，会不支持模板中的static数组成员的长度用const static成员定义。
    //理论上可以用#if (__GNUC__ < 4)屏蔽，但是实在太忙法。下面的数组长度NUM_OF_FRAMELIST
    //如果你更改是要定义。
    size_t           size_appframe_[NUM_OF_FRAMELIST] = { 0 };

    //FRAME的内存池子
    APPFRAME_POOL    frame_pool_;

    //池子的锁
    std::mutex      *my_lock_ = nullptr;
};
}

//APPFRAME的分配器
typedef soar::zergframe_mallocor   APPFRAME_MALLOCOR;

////
//typedef zergframe_mallocor<zce::null_lock> NULLMUTEX_APPFRAME_MALLOCOR;
//typedef zergframe_mallocor<zce::thread_mutex> THREADMUTEX_APPFRAME_MALLOCOR;

#endif //#ifndef SOARING_LIB_APPFRAME_MALLOCOR_H_
