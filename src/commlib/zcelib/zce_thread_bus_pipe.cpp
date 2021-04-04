#include "zce_predefine.h"

#include "zce_thread_bus_pipe.h"

/****************************************************************************************************
class  ZCE_Thread_Bus_Pipe
****************************************************************************************************/

//����CPP?����?
template <> ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex> *
ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::instance_ = NULL;

template <> ZCE_Thread_Bus_Pipe<ZCE_Thread_Light_Mutex> *
ZCE_Thread_Bus_Pipe<ZCE_Thread_Light_Mutex>::instance_ = NULL;

//�ػ�����

//��RECV�ܵ���ȡ����
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::pop_front_recvpipe(zce::lockfree::dequechunk_node *&node)
{
    return bus_pipe_[THR_RECV_PIPE_ID]->pop_front(node);
}

//��RECV�ܵ�д������
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::push_back_recvpipe(const zce::lockfree::dequechunk_node *node)
{
    return bus_pipe_[THR_RECV_PIPE_ID]->push_end(node);
}

//��SEND�ܵ���ȡ����
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::pop_front_sendpipe(zce::lockfree::dequechunk_node *&node)
{
    return bus_pipe_[THR_SEND_PIPE_ID]->pop_front(node);
}

//��SEND�ܵ�д������
template <>
inline bool ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>::push_back_sendpipe(const zce::lockfree::dequechunk_node *node)
{
    return bus_pipe_[THR_SEND_PIPE_ID]->push_end(node);
}

//ȡRecv�ܵ�ͷ��֡��
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

//ȡSend�ܵ�ͷ��֡����
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

