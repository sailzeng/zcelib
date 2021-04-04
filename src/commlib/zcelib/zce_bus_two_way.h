#ifndef ZCE_LIB_BUS_TWO_WAY_H_
#define ZCE_LIB_BUS_TWO_WAY_H_

#include "zce_bus_mmap_pipe.h"

namespace zce
{
class dequechunk_node;
class shm_dequechunk;
};

//˫�е�
class ZCE_BusPipe_TwoWay : public ZCE_Bus_MMAPPipe
{
public:

    //PIPE�ı��
    enum ZCE_BUS_PIPE_ID
    {
        RECV_PIPE_ID     = 0,
        SEND_PIPE_ID     = 1,
        NUM_OF_PIPE      = 2,      //���ȱ�ʾ,��Ҫ��������������,����������
    };


protected:

    //ZERG�ܵ�������,�����������Ϊ��Ҷ���
    static const char          BUS_PIPE_NAME[NUM_OF_PIPE][16];



protected:
    //instance����ʹ�õĶ���
    static ZCE_BusPipe_TwoWay *two_way_instance_;

public:

    //���캯��,
    ZCE_BusPipe_TwoWay();
    //��������
    ~ZCE_BusPipe_TwoWay();

public:

    //��ʼ�����ֲ���,
    int initialize(const char *bus_mmap_name,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);


    //-----------------------------------------------------------------
    //��RECV�ܵ���ȡ���ݣ�
    inline int pop_front_recvpipe(zce::lockfree::dequechunk_node *const node);
    //��SEND�ܵ���ȡ���ݣ�
    inline int pop_front_sendpipe(zce::lockfree::dequechunk_node *const node);
    //��SEND�ܵ�д������
    inline int push_back_sendpipe(const zce::lockfree::dequechunk_node *node);
    //��RECV�ܵ�д������
    inline int push_back_recvpipe(const zce::lockfree::dequechunk_node *node);


    //ȡRecv�ܵ�ͷ��֡��
    inline int get_frontsize_recvpipe(size_t &note_size);
    //ȡSend�ܵ�ͷ��֡��
    inline int get_frontsize_sendpipe(size_t &note_size);



public:

    //ʵ���ĸ�ֵ
    static void instance(ZCE_BusPipe_TwoWay *);
    //ʵ���Ļ��
    static ZCE_BusPipe_TwoWay *instance();
    //���ʵ��
    static void clean_instance();

};


//ȡRecv�ܵ�ͷ��֡��
inline int ZCE_BusPipe_TwoWay::get_frontsize_recvpipe(size_t &note_size)
{
    return get_front_nodesize(RECV_PIPE_ID, note_size);
}

//ȡSend�ܵ�ͷ��֡��
inline int ZCE_BusPipe_TwoWay::get_frontsize_sendpipe(size_t &note_size)
{
    return get_front_nodesize(SEND_PIPE_ID, note_size);
}



//��RECV�ܵ���ȡ֡��
inline int ZCE_BusPipe_TwoWay::pop_front_recvpipe(zce::lockfree::dequechunk_node *const node)
{
    return pop_front_bus(RECV_PIPE_ID, node);
}


//��SEND�ܵ�д��֡��
inline int ZCE_BusPipe_TwoWay::push_back_sendpipe(const zce::lockfree::dequechunk_node *node)
{
    return push_back_bus(SEND_PIPE_ID, node);
}

//��SEND�ܵ���ȡ֡��
inline int ZCE_BusPipe_TwoWay::pop_front_sendpipe(zce::lockfree::dequechunk_node *const node)
{
    return pop_front_bus(SEND_PIPE_ID, node);
}


//��RECV�ܵ�д��֡��
inline int ZCE_BusPipe_TwoWay::push_back_recvpipe(const zce::lockfree::dequechunk_node *node)
{
    return push_back_bus(RECV_PIPE_ID, node);
}


#endif //ZCE_LIB_BUS_TWO_WAY_H_

