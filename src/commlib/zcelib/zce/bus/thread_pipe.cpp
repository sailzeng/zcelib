#include "zce/predefine.h"

#include "zce/bus/thread_pipe.h"

/****************************************************************************************************
class  ZCE_Thread_Bus_Pipe
****************************************************************************************************/

//特化函数

//从RECV管道读取数据
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::pop_front_recvpipe(zce::lockfree::dequechunk_node *&node)
{
    return bus_pipe_[THR_RECV_PIPE_ID]->pop_front(node);
}

//向RECV管道写入数据
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::push_back_recvpipe(const zce::lockfree::dequechunk_node *node)
{
    return bus_pipe_[THR_RECV_PIPE_ID]->push_end(node);
}

//从SEND管道读取数据
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::pop_front_sendpipe(zce::lockfree::dequechunk_node *&node)
{
    return bus_pipe_[THR_SEND_PIPE_ID]->pop_front(node);
}

//向SEND管道写入数据
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::push_back_sendpipe(const zce::lockfree::dequechunk_node *node)
{
    return bus_pipe_[THR_SEND_PIPE_ID]->push_end(node);
}

//取Recv管道头的帧长
template <>
inline int ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::get_frontsize_recvpipe(size_t &note_size)
{
    if (bus_pipe_[THR_RECV_PIPE_ID] ->empty())
    {
        return -1;
    }

    note_size = bus_pipe_[THR_RECV_PIPE_ID]->get_front_len();
    return 0;
}

//取Send管道头的帧长、
template <>
inline int ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::get_frontsize_sendpipe(size_t &note_size)
{
    if (bus_pipe_[THR_SEND_PIPE_ID] ->empty())
    {
        return -1;
    }

    note_size =  bus_pipe_[THR_SEND_PIPE_ID]->get_front_len();
    return 0;
}

