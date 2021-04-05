
/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_bus_pipe.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年9月18日晚上，中国人民抗日81周年纪念日，81年
* @brief
*
*
* @details
*
*
*
* @note
*
* 喔　你我霎眼抗战二十年　世界怎变　我答应你那一点　不会变
*
* 当天空手空臂我们就上街　没什么声势浩大
* 但被不安养大　不足养大　哪里怕表态
* 当中一起经过了时代瓦解　十大执位再十大
* 路上风急雨大　一起吓大　听惯了警戒
*
* 应该珍惜的　即使牺牲了　激起的火花　仍然照耀

* 几响枪火敲破了沉默领土　剩下烧焦了味道
* 现在少点愤怒　多些厚道　偶尔也很躁
* 不管这种争拗有型或老套　未做好的继续做
* 活着必须革命　心高气傲　哪里去不到

* 他虽走得早　他青春不老　灰色的轨迹　磨成血路

* 喔　你我霎眼抗战二十年　世界怎变　永远企你这一边
* 喔　哪怕再去抗战二十年　去到多远　我也铭记我起点　不会变
*/

#ifndef ZCE_LIB_THREAD_BUS_PIPE_H_
#define ZCE_LIB_THREAD_BUS_PIPE_H_



#include "zce/util/non_copyable.h"
#include "zce/lockfree/kfifo.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/error.h"
#include "zce/shared_mem/mmap.h"
#include "zce/log/logging.h"
#include "zce/lock/null_lock.h"
#include "zce/lock/thread_mutex.h"

namespace zce::lockfree
{
class dequechunk_node;
class shm_dequechunk;
};

//线程使用的双向BUS管道，
//不要直接使用这个类，使用下面的两个typedef
template <typename zce_lock>
class ZCE_Thread_Bus_Pipe : public zce::NON_Copyable
{

protected:

    //内部的枚举，PIPE的编号，外部不用了解
    enum ZCE_BUS_PIPE_ID
    {
        THR_RECV_PIPE_ID     = 0,
        THR_SEND_PIPE_ID     = 1,

        //长度标示,不要用于做函数参数,否则会有溢出
        THR_NUM_OF_PIPE      = 2,
    };

protected:

    //管道用的内存空间地址
    char                      *pipe_buffer_;

    //管道配置长度,2个管道的配置长度,
    size_t                     size_pipe_[THR_NUM_OF_PIPE];
    //发送管道的实际空间长度
    size_t                     size_room_[THR_NUM_OF_PIPE];

    //N个管道,比如接收管道,发送管道……,最大MAX_NUMBER_OF_PIPE个
    zce::lockfree::shm_dequechunk  *bus_pipe_[THR_NUM_OF_PIPE];

    //锁
    zce_lock                   bus_lock_[THR_NUM_OF_PIPE];

public:
    //构造函数,允许你有多个实例的可能，不做保护
    ZCE_Thread_Bus_Pipe():
        pipe_buffer_(NULL)
    {
        for (size_t i = 0; i < THR_NUM_OF_PIPE; ++i )
        {
            size_pipe_[i] = 0;
            size_room_[i] = 0;
            bus_pipe_[i] = NULL;
        }

    }
    //析购函数
    ~ZCE_Thread_Bus_Pipe()
    {

    }

public:

    //-----------------------------------------------------------------
    //初始化部分参数,允许发送和接收的长度不一致
    int initialize(size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len)
    {
        ZCE_ASSERT(NULL == pipe_buffer_);


        size_pipe_[THR_RECV_PIPE_ID] = size_recv_pipe;
        size_pipe_[THR_SEND_PIPE_ID] = size_send_pipe;

        size_room_[THR_RECV_PIPE_ID] = zce::lockfree::shm_dequechunk::getallocsize(size_pipe_[THR_RECV_PIPE_ID]);
        size_room_[THR_SEND_PIPE_ID] = zce::lockfree::shm_dequechunk::getallocsize(size_pipe_[THR_SEND_PIPE_ID]);

        //
        const size_t FIXED_INTERVALS = 16;
        size_t sz_malloc = size_room_[THR_RECV_PIPE_ID] + size_room_[THR_SEND_PIPE_ID] + FIXED_INTERVALS * 2;

        pipe_buffer_ = new char [sz_malloc ];

        //初始化内存
        bus_pipe_[THR_RECV_PIPE_ID] = zce::lockfree::shm_dequechunk::initialize(size_pipe_[THR_RECV_PIPE_ID],
                                                                      max_frame_len,
                                                                      pipe_buffer_,
                                                                      false);

        bus_pipe_[THR_SEND_PIPE_ID] = zce::lockfree::shm_dequechunk::initialize(size_pipe_[THR_SEND_PIPE_ID],
                                                                      max_frame_len,
                                                                      pipe_buffer_ + size_room_[THR_RECV_PIPE_ID] + FIXED_INTERVALS,
                                                                      false);

        //管道创建自己也会检查是否能恢复
        if ( NULL == bus_pipe_[THR_RECV_PIPE_ID]  || NULL == bus_pipe_[THR_SEND_PIPE_ID])
        {
            ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Thread_Bus_Pipe::initialize pipe fail recv[%p]size[%u],send[%p],size[%u].",
                    bus_pipe_[THR_RECV_PIPE_ID],
                    size_pipe_[THR_RECV_PIPE_ID],
                    bus_pipe_[THR_SEND_PIPE_ID],
                    size_pipe_[THR_SEND_PIPE_ID]
                   );
            return -1;
        }

        return 0;
    }



    //-----------------------------------------------------------------
    //注意

    //从RECV管道读取数据，
    inline bool pop_front_recvpipe(zce::lockfree::dequechunk_node *&node)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);
        return bus_pipe_[THR_RECV_PIPE_ID]->pop_front(node);
    }

    //向RECV管道写入数据
    inline bool push_back_recvpipe(const zce::lockfree::dequechunk_node *node)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);
        return bus_pipe_[THR_RECV_PIPE_ID]->push_end(node);
    }


    //从SEND管道读取数据，
    inline bool pop_front_sendpipe(zce::lockfree::dequechunk_node *&node)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);
        return bus_pipe_[THR_SEND_PIPE_ID]->pop_front(node);
    }

    //向SEND管道写入数据
    inline bool push_back_sendpipe(const zce::lockfree::dequechunk_node *node)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);
        return bus_pipe_[THR_SEND_PIPE_ID]->push_end(node);
    }


    //取Recv管道头的帧长
    inline int get_frontsize_recvpipe(size_t &note_size)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);

        if (bus_pipe_[THR_RECV_PIPE_ID] ->empty())
        {
            return -1;
        }

        note_size = bus_pipe_[THR_RECV_PIPE_ID]->get_front_len();
        return 0;
    }

    //取Send管道头的帧长
    inline int get_frontsize_sendpipe(size_t &note_size)
    {
        ZCE_Lock_Guard<zce_lock> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);

        if (bus_pipe_[THR_SEND_PIPE_ID] ->empty())
        {
            return -1;
        }

        note_size =  bus_pipe_[THR_SEND_PIPE_ID]->get_front_len();
        return 0;
    }



};




//--------------------------------------------------------------------------------------
//用于一个线程读，一个线程写的BUS，虽然叫ST，但其实还是在两个线程间有效，
//用于一些对于速度有极致追求的地方
typedef ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>   ZCE_Thread_ST_Bus_Pipe;

//加锁的BUS,可以有多人竞争
typedef ZCE_Thread_Bus_Pipe<ZCE_Thread_Light_Mutex> ZCE_Thread_MT_Bus_Pipe;






#endif //ZCE_LIB_THREAD_BUS_PIPE_H_


