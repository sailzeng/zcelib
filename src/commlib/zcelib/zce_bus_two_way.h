#ifndef ZCE_LIB_BUS_TWO_WAY_H_
#define ZCE_LIB_BUS_TWO_WAY_H_

#include "zce_bus_mmap_pipe.h"

namespace ZCE_LIB
{
class dequechunk_node;
class shm_dequechunk;
};

//双行道
class ZCELIB_EXPORT ZCE_BusPipe_TwoWay : public ZCE_Bus_MMAPPipe
{
public:

    //PIPE的编号
    enum ZCE_BUS_PIPE_ID
    {
        RECV_PIPE_ID     = 0,
        SEND_PIPE_ID     = 1,
        NUM_OF_PIPE      = 2,      //长度标示,不要用于做函数参数,否则会有溢出
    };


protected:

    //ZERG管道的名称,放在这儿，因为大家都用
    static const char          BUS_PIPE_NAME[NUM_OF_PIPE][16];



protected:
    //instance函数使用的东西
    static ZCE_BusPipe_TwoWay *two_way_instance_;

public:

    //构造函数,
    ZCE_BusPipe_TwoWay();
    //析购函数
    ~ZCE_BusPipe_TwoWay();

public:

    //初始化部分参数,
    int initialize(const char *bus_mmap_name,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);


    //-----------------------------------------------------------------
    //从RECV管道读取数据，
    inline int pop_front_recvpipe(ZCE_LIB::dequechunk_node *const node);
    //从SEND管道读取数据，
    inline int pop_front_sendpipe(ZCE_LIB::dequechunk_node *const node);
    //向SEND管道写入数据
    inline int push_back_sendpipe(const ZCE_LIB::dequechunk_node *node);
    //向RECV管道写入数据
    inline int push_back_recvpipe(const ZCE_LIB::dequechunk_node *node);


    //取Recv管道头的帧长
    inline int get_frontsize_recvpipe(size_t &note_size);
    //取Send管道头的帧长
    inline int get_frontsize_sendpipe(size_t &note_size);



public:

    //实例的赋值
    static void instance(ZCE_BusPipe_TwoWay *);
    //实例的获得
    static ZCE_BusPipe_TwoWay *instance();
    //清除实例
    static void clean_instance();

};


//取Recv管道头的帧长
inline int ZCE_BusPipe_TwoWay::get_frontsize_recvpipe(size_t &note_size)
{
    return get_front_nodesize(RECV_PIPE_ID, note_size);
}

//取Send管道头的帧长
inline int ZCE_BusPipe_TwoWay::get_frontsize_sendpipe(size_t &note_size)
{
    return get_front_nodesize(SEND_PIPE_ID, note_size);
}



//从RECV管道读取帧，
inline int ZCE_BusPipe_TwoWay::pop_front_recvpipe(ZCE_LIB::dequechunk_node *const node)
{
    return pop_front_bus(RECV_PIPE_ID, node);
}


//向SEND管道写入帧，
inline int ZCE_BusPipe_TwoWay::push_back_sendpipe(const ZCE_LIB::dequechunk_node *node)
{
    return push_back_bus(SEND_PIPE_ID, node);
}

//从SEND管道读取帧，
inline int ZCE_BusPipe_TwoWay::pop_front_sendpipe(ZCE_LIB::dequechunk_node *const node)
{
    return pop_front_bus(SEND_PIPE_ID, node);
}


//向RECV管道写入帧，
inline int ZCE_BusPipe_TwoWay::push_back_recvpipe(const ZCE_LIB::dequechunk_node *node)
{
    return push_back_bus(RECV_PIPE_ID, node);
}






#endif //ZCE_LIB_BUS_TWO_WAY_H_

