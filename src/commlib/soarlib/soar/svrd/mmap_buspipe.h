#ifndef SOARING_LIB_ZERG_MMAP_PIPE_H_
#define SOARING_LIB_ZERG_MMAP_PIPE_H_

#include "soar/zerg/frame_zerg.h"
#include "soar/enum/error_code.h"
#include "soar/stat/monitor.h"
#include "soar/stat/define.h"
#include "soar/svrd/cfg_fsm.h"

class soar::Zerg_Frame;

class  Soar_MMAP_BusPipe: public ZCE_BusPipe_TwoWay
{
public:
    //构造函数,
    Soar_MMAP_BusPipe();
    //析购函数
    ~Soar_MMAP_BusPipe();

public:

    //初始化部分参数,
    int initialize(soar::SERVICES_INFO &svr_info,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);

    //生成MMAP的配置文件名称
    void get_mmapfile_name(char *mmapfile,size_t buflen);

    //-----------------------------------------------------------------
    //从RECV管道读取帧
    inline int pop_front_recvpipe(soar::Zerg_Frame *&proc_frame);
    //向SEND管道写入帧
    inline int push_back_sendpipe(const soar::Zerg_Frame *proc_frame);

    //原来的名字都是send_msg_to，一开始认为这样挺好的，但是逐步逐步的感觉发现修改替换的时候痛苦不少。
    //不确认改为pipe_sendmsg_to_service是不是一个好主意，但是试验一下?

    //发送一个信息给服务，无需指定代理服务器那种方式
    template< class T>
    int pipe_sendmsg_to_service(uint32_t cmd,
                                uint32_t user_id,
                                unsigned int transaction_id,
                                uint32_t backfill_fsm_id,
                                const soar::SERVICES_ID &rcvsvc,
                                const soar::SERVICES_ID &sendsvc,
                                const T &msg,
                                unsigned int app_id = 0,
                                uint32_t option = 0);

    //发送数据到代理服务器,指定代理服务器，以及接受的服务器类型
    template< class T>
    int pipe_sendmsg_to_proxy(uint32_t cmd,
                              uint32_t user_id,
                              unsigned int transaction_id,
                              uint32_t backfill_fsm_id,
                              uint16_t rcv_type,
                              const soar::SERVICES_ID &proxysvc,
                              const soar::SERVICES_ID &sendsvc,
                              const T &msg,
                              unsigned int app_id = 0,
                              uint32_t option = 0);

    //发送数据到服务器,可以指定具体的代理服务器，以及接受的服务器ID
    template< class T>
    int pipe_sendmsg_to_service(uint32_t cmd,
                                uint32_t user_id,
                                unsigned int transaction_id,
                                uint32_t backfill_fsm_id,
                                const soar::SERVICES_ID &rcvsvc,
                                const soar::SERVICES_ID &proxysvc,
                                const soar::SERVICES_ID &sendsvc,
                                const T &msg,
                                unsigned int app_id = 0,
                                uint32_t option = 0);

    // 发送一段buf给指定的服务器
    int pipe_sendbuf_to_service(uint32_t cmd,
                                uint32_t user_id,
                                unsigned int transaction_id,
                                uint32_t backfill_fsm_id,
                                const soar::SERVICES_ID &rcvsvc,
                                const soar::SERVICES_ID &proxysvc,
                                const soar::SERVICES_ID &sendsvc,
                                const unsigned char *buf,
                                size_t buf_len,
                                uint32_t option = 0);

    //-----------------------------------------------------------------
public:

    //为了SingleTon类准备
    //实例的赋值
    static void instance(Soar_MMAP_BusPipe *);
    //实例的获得
    static Soar_MMAP_BusPipe *instance();
    //清除实例
    static void clean_instance();

protected:

    //ZERG管道的名称,放在这儿，因为大家都用
    static const char    ZERG_PIPENAME[NUM_OF_PIPE][16];

protected:
    //instance函数使用的东西
    static Soar_MMAP_BusPipe *zerg_bus_instance_;

protected:

    ///这个服务器的配置信息.
    soar::SERVICES_INFO  zerg_svr_info_;
    ///发送的缓冲区
    static char          send_buffer_[soar::Zerg_Frame::MAX_LEN_OF_APPFRAME];

    ///监控对象
    Soar_Stat_Monitor *monitor_ = nullptr;
};

/******************************************************************************************
Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2007年3月24日
Function        : PipeRequstMsgToService
Return          : int
Parameter List  :
  Param1: uint32_t cmd  发送的命令
  Param2: uint32_t user_id 发送的UIN
  Param3: unsigned int transaction_id,事务ID
  Param4: uint32_t backfill_fsm_id,回填的事务ID
  Param5: const soar::SERVICES_ID& rcvsvc  接受者是谁
  Param6: const soar::SERVICES_ID& sendsvc 发送者是谁
  Param7: const T& info 发送的数据
  Param8: uint32_t option=0 发送的选项
Description     : 制定一个接受服务器发送数据,不经过PROXY服务器
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_service(uint32_t cmd,
                                               uint32_t user_id,
                                               unsigned int transaction_id,
                                               uint32_t backfill_fsm_id,
                                               const soar::SERVICES_ID &rcvsvc,
                                               const soar::SERVICES_ID &sendsvc,
                                               const T &msg,
                                               unsigned int app_id,
                                               uint32_t option)
{
    const soar::SERVICES_ID proxysvc(0,0);
    return pipe_sendmsg_to_service(cmd,
                                   user_id,
                                   transaction_id,
                                   backfill_trans_id,
                                   rcvsvc,
                                   proxysvc,
                                   sendsvc,
                                   info,
                                   app_id,
                                   option);
}

/******************************************************************************************
Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2007年3月24日
Function        : pipe_sendmsg_to_proxy
Return          : template< class T> int
Parameter List  :
  Param1: uint32_t cmd   发送的命令
  Param2: uint32_t user_id 发送的UIN
  Param3: unsigned int transaction_id,事务ID
  Param4: uint32_t backfill_fsm_id,回填的事务ID
  Param5: uint16_t rcv_type 最终的接受者的类型是什么
  Param7: const soar::SERVICES_ID& proxysvc PROXY服务器是谁
  Param6: const soar::SERVICES_ID& sendsvc  发送者是谁
  Param8: const T& info 发送的数据
  Param9: uint32_t option=0 发送的选项
Description     : 制定一个PROXY服务器进行接收数据
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_proxy(uint32_t cmd,
                                             uint32_t user_id,
                                             unsigned int transaction_id,
                                             uint32_t backfill_fsm_id,
                                             uint16_t rcv_type,
                                             const soar::SERVICES_ID &proxysvc,
                                             const soar::SERVICES_ID &sendsvc,
                                             const T &msg,
                                             unsigned int app_id,
                                             uint32_t option)
{
    const soar::SERVICES_ID rcvsvc(rcvtype,0);
    return pipe_sendmsg_to_service(cmd,
                                   user_id,
                                   transaction_id,
                                   backfill_trans_id,
                                   rcvsvc,
                                   proxysvc,
                                   sendsvc,
                                   info,
                                   app_id,
                                   option);
}

/******************************************************************************************
Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2007年3月24日
Function        : pipe_sendmsg_to_service
Return          : template< class T>
Parameter List  :
  Param1: uint32_t cmd   发送的命令
  Param2: uint32_t user_id 发送的UIN
  Param3: unsigned int transaction_id,事务ID,没有要求对方返回就不用填写，填写0
  Param4: uint32_t backfill_fsm_id,回填的事务ID,没有要求对方返回就不用填写，填写0
  Param5: const soar::SERVICES_ID& rcvsvc 最终的接受者的类型是谁
  Param6: const soar::SERVICES_ID& proxysvc PROXY服务器是谁
  Param7: const soar::SERVICES_ID& sendsvc 发送者是谁
  Param8: const T& info 发送的数据
  Param9: uint32_t option=0 发送的选项
Description     : 最复杂参数的发送函数，
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_service(uint32_t cmd,
                                               uint32_t user_id,
                                               unsigned int transaction_id,
                                               uint32_t backfill_fsm_id,
                                               const soar::SERVICES_ID &rcvsvc,
                                               const soar::SERVICES_ID &proxysvc,
                                               const soar::SERVICES_ID &sendsvc,
                                               const T &msg,
                                               unsigned int app_id,
                                               uint32_t option)
{
    int ret = 0;

    soar::Zerg_Frame *send_frame = reinterpret_cast<soar::Zerg_Frame *>(send_buffer_);

    send_frame->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME,option,cmd);
    send_frame->user_id_ = user_id;
    send_frame->app_id_ = app_id;

    send_frame->send_service_ = sendsvc;
    send_frame->proxy_service_ = proxysvc;
    send_frame->recv_service_ = rcvsvc;

    //填写事务ID和回填事务ID
    send_frame->fsm_id_ = transaction_id;
    send_frame->backfill_fsm_id_ = backfill_trans_id;

    ret = send_frame->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME_DATA,info);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR,"[framework] [%s]TDR encode fail.ret =%d,Please check your code and buffer len.",
                __ZCE_FUNC__,
                ret);
        return ret;
    }

    //
    return push_back_sendpipe(send_frame);
}

#endif //SOARING_LIB_ZERG_MMAP_PIPE_H_
