#ifndef SOARING_LIB_ZERG_MMAP_PIPE_H_
#define SOARING_LIB_ZERG_MMAP_PIPE_H_

#include "soar/zerg/frame_zerg.h"
#include "soar/enum/error_code.h"
#include "soar/stat/monitor.h"
#include "soar/stat/define.h"
#include "soar/svrd/cfg_fsm.h"

class soar::Zerg_Frame;

namespace soar
{
class Svrd_BusPipe : protected zce::TwoWay_BusPipe
{
public:
    //构造函数,
    Svrd_BusPipe();
    //析购函数
    ~Svrd_BusPipe();

public:

    //初始化部分参数,
    int initialize(soar::SERVICES_INFO& svr_info,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);

    //生成MMAP的配置文件名称
    void get_mmapfile_name(char* mmapfile, size_t buflen);

    //-----------------------------------------------------------------
    //从RECV管道读取帧
    int pop_front_recvbus(soar::Zerg_Frame* proc_frame);

    //从SEND管道读取帧
    int pop_front_sendbus(soar::Zerg_Frame* proc_frame);

    //向SEND管道写入帧
    int push_back_sendbus(const soar::Zerg_Frame* proc_frame);

    //向RECV管道写入帧
    int push_back_recvbus(const soar::Zerg_Frame* proc_frame);

    //发送bus是否是空的
    inline bool is_empty_sendbus()
    {
        return is_empty_bus(SEND_PIPE_ID);
    }
    //接收bus是否是空的
    inline bool is_empty_recvbus()
    {
        return is_empty_bus(RECV_PIPE_ID);
    }
    //发送bus是否是满的
    inline bool is_full_sendbus()
    {
        return is_full_bus(SEND_PIPE_ID);
    }
    //接收bus是否是满的
    inline bool is_full_recvbus()
    {
        return is_full_bus(RECV_PIPE_ID);
    }

public:

    //为了SingleTon类准备
    //实例的赋值
    static void instance(Svrd_BusPipe*);
    //实例的获得
    static Svrd_BusPipe* instance();
    //清除实例
    static void clean_instance();

protected:
    //instance函数使用的东西
    static Svrd_BusPipe* zerg_bus_instance_;

protected:

    ///这个服务器的配置信息.
    soar::SERVICES_INFO  zerg_svr_info_;
    ///发送的缓冲区
    static char          send_buffer_[soar::Zerg_Frame::MAX_LEN_OF_FRAME];

    ///监控对象
    soar::Stat_Monitor* monitor_ = nullptr;
};
}

#endif //SOARING_LIB_ZERG_MMAP_PIPE_H_
