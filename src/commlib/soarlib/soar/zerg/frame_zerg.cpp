#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/frame_zerg.h"

namespace soar
{
#if defined (ZCE_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

//---------------------------------------------------------------------------------------------------------

//将所有的uint16_t,uint32_t转换为网络序
void Zerg_Head::hton()
{
    length_ = htonl(length_);
    u32_option_ = htonl(u32_option_);
    command_ = htonl(command_);
    user_id_ = htonl(user_id_);

    fsm_id_ = htonl(fsm_id_);
    backfill_fsm_id_ = htonl(backfill_fsm_id_);

    serial_number_ = htonl(serial_number_);
    business_id_ = htons(business_id_);
    //
    recv_service_.services_type_ = htons(recv_service_.services_type_);
    recv_service_.services_id_ = htonl(recv_service_.services_id_);

    send_service_.services_type_ = htons(send_service_.services_type_);
    send_service_.services_id_ = htonl(send_service_.services_id_);
    //
    proxy_service_.services_type_ = htons(proxy_service_.services_type_);
    proxy_service_.services_id_ = htonl(proxy_service_.services_id_);
}

//将所有的uint16_t,uint32_t转换为本地序
void Zerg_Head::ntoh()
{
    length_ = ntohl(length_);
    u32_option_ = ntohl(u32_option_);
    command_ = ntohl(command_);
    user_id_ = ntohl(user_id_);

    fsm_id_ = ntohl(fsm_id_);
    backfill_fsm_id_ = ntohl(backfill_fsm_id_);

    serial_number_ = ntohl(serial_number_);
    business_id_ = ntohs(business_id_);
    //
    recv_service_.services_type_ = ntohs(recv_service_.services_type_);
    recv_service_.services_id_ = ntohl(recv_service_.services_id_);

    send_service_.services_type_ = ntohs(send_service_.services_type_);
    send_service_.services_id_ = ntohl(send_service_.services_id_);
    //
    proxy_service_.services_type_ = ntohs(proxy_service_.services_type_);
    proxy_service_.services_id_ = ntohl(proxy_service_.services_id_);
}

//---------------------------------------------------------------------------------------------------------

//创建一个Frame
Zerg_Frame *Zerg_Frame::new_frame(std::size_t frame_len)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (frame_len < sizeof(Zerg_Frame))
    {
        frame_len = sizeof(Zerg_Frame);
    }

    //计算数据加密后的长度

    void *ptr = ::new unsigned char[frame_len];

#if defined(DEBUG) || defined(_DEBUG)
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr,0,frame_len);
#endif //DEBUG

    //没有必要加下面这句，因为实际长度要根据使用决定
    //reinterpret_cast<Zerg_Frame*>(ptr)->frame_length_ = static_cast<uint32_t>(lenframe);
    return static_cast<Zerg_Frame *>(ptr);
}

//
void Zerg_Frame::delete_frame(Zerg_Frame *frame)
{
    char *ptr = reinterpret_cast<char *>(frame);
    delete[]ptr;
}

//填充AppData数据到APPFrame
int Zerg_Frame::fill_appdata(const size_t szdata,const char *vardata)
{
    // 判断长度是否足够
    if (szdata > MAX_LEN_OF_APPFRAME_DATA)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //填写数据区的长度
    memcpy(frame_appdata_,vardata,szdata);
    length_ = static_cast<uint32_t>(Zerg_Frame::LEN_OF_APPFRAME_HEAD + szdata);
    return 0;
}

//交换接受者和发送者,由于要回送数据时
void Zerg_Frame::exchange_rcvsnd_svcid(void)
{
    soar::SERVICES_ID tmpsvrinfo = recv_service_;
    recv_service_ = send_service_;
    send_service_ = tmpsvrinfo;
}

void Zerg_Frame::exchange_rcvsnd_svcid(Zerg_Frame &exframe)
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    fsm_id_ = exframe.fsm_id_;
    backfill_fsm_id_ = exframe.backfill_fsm_id_;
    user_id_ = exframe.user_id_;
}

void Zerg_Frame::fillback_appframe_head(Zerg_Frame &exframe)
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    fsm_id_ = exframe.backfill_fsm_id_;
    //回去的事务id应当是请求的事务id
    backfill_fsm_id_ = exframe.fsm_id_;
    user_id_ = exframe.user_id_;
}

//输出APPFRAME的头部信息
void Zerg_Frame::dump_frame_head(zce::LOG_PRIORITY log_priority,
                                 const char *outer_str,
                                 const Zerg_Frame *proc_frame)
{
    const size_t MAX_LEN = 1024;
    char head_str[MAX_LEN + 1];
    head_str[MAX_LEN] = '\0';
    snprintf(head_str,
             MAX_LEN,
             "zerg fame head:length_=%u,option=%u,command_=%u,user_id_=%u"
             "serial_number_=%u,transaction_id_=%u,backfill_trans_id_=%u"
             "send_service_=%lu|%lu,recv_service_=%lu|%lu,proxy_service_=%lu|%lu",
             proc_frame->length_,
             proc_frame->u32_option_,
             proc_frame->command_,
             proc_frame->user_id_,
             proc_frame->serial_number_,
             proc_frame->fsm_id_,
             proc_frame->backfill_fsm_id_,
             proc_frame->send_service_.services_type_,
             proc_frame->send_service_.services_id_,
             proc_frame->recv_service_.services_type_,
             proc_frame->recv_service_.services_id_,
             proc_frame->proxy_service_.services_type_,
             proc_frame->proxy_service_.services_id_
    );
    ZCE_LOG(log_priority,"[framework] [%s]:%s",outer_str,head_str);
}

//输出APPFRAME的全部部信息
void Zerg_Frame::dump_frame_all(zce::LOG_PRIORITY log_priority,
                                const char *outer_str,
                                const Zerg_Frame *proc_frame)
{
    dump_frame_head(log_priority,outer_str,proc_frame);
    char data_str[MAX_LEN_OF_APPFRAME * 2 + 1];
    size_t datalen = proc_frame->length_ - LEN_OF_APPFRAME_HEAD;
    size_t out_len = MAX_LEN_OF_APPFRAME * 2 + 1;

    zce::base16_encode(proc_frame->frame_appdata_,datalen,data_str,&out_len);
    ZCE_LOG(log_priority,"[framework] [%s] zerg fame data:%s",outer_str,data_str);
}

//Clone一个APP FRAME
void Zerg_Frame::clone(Zerg_Frame *dst_frame) const
{
    memcpy(dst_frame,this,length_);
    return;
}

//Clone一个APP FRAME 的头部
void Zerg_Frame::clone_head(Zerg_Frame *clone_frame) const
{
    memcpy(clone_frame,this,LEN_OF_APPFRAME_HEAD);
    return;
}

//取得一个头部信息
void Zerg_Frame::get_head(Zerg_Head &frame_head) const
{
    memcpy(&frame_head,this,LEN_OF_APPFRAME_HEAD);
    frame_head.length_ = LEN_OF_APPFRAME_HEAD;
    return;
}

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

///将一个结构进行编码
int Zerg_Frame::protobuf_encode(size_t szframe_appdata,
                                const google::protobuf::MessageLite *msg,
                                size_t data_start,
                                size_t *sz_code)
{
    if (!msg->IsInitialized())
    {
        ZCE_LOG(RS_ERROR,"");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    size_t need_size = msg->ByteSize();
    if (data_start + need_size > szframe_appdata)
    {
        ZCE_LOG(RS_ERROR,"");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    bool bret = msg->SerializePartialToArray(frame_appdata_ + data_start,
                                             static_cast<int>(szframe_appdata - data_start));
    if (bret == false)
    {
        ZCE_LOG(RS_ERROR,"Portobuf encode SerializePartialToArray fail.");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    if (sz_code)
    {
        *sz_code = need_size;
    }
    return 0;
}

///将一个结构进行解码
int Zerg_Frame::protobuf_decode(google::protobuf::MessageLite *msg,
                                size_t data_start,
                                size_t *sz_code)
{
    size_t szframe_appdata = length_;
    bool bret = msg->ParseFromArray(frame_appdata_ + data_start,
                                    static_cast<int>(szframe_appdata - data_start));
    if (bret == false)
    {
        ZCE_LOG(RS_ERROR,"Portobuf decode ParseFromArray fail.");
        return SOAR_RET::ERROR_DR_DECODE_FAIL;
    }
    if (sz_code)
    {
        *sz_code = msg->ByteSize();
    }
    return 0;
}

#endif

#if defined (ZCE_OS_LINUX)
#pragma GCC diagnostic pop
#endif
};