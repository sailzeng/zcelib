#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_frame.h"


#if defined (ZCE_OS_LINUX)
#if ( _GCC_VER >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
#endif

ZERG_FRAME_HEAD::ZERG_FRAME_HEAD() :
    frame_length_(CMD_INVALID_CMD),
    frame_option_(DESC_V1_VERSION),
    frame_command_(LEN_OF_APPFRAME_HEAD),
    frame_userid_(0),
    business_id_(0),
    send_service_(0, 0),
    recv_service_(0, 0),
    proxy_service_(0, 0),
    transaction_id_(0),
    backfill_trans_id_(0),
    send_ip_address_(0)
{
}

ZERG_FRAME_HEAD::~ZERG_FRAME_HEAD()
{
}


// Assign =运算符号
ZERG_FRAME_HEAD &ZERG_FRAME_HEAD::operator = (const ZERG_FRAME_HEAD &other)
{
    if (this != &other )
    {
        memcpy(this, &other, other.frame_length_);
    }

    return *this;
}

//创建一个Frame
ZERG_FRAME_HEAD *ZERG_FRAME_HEAD::new_frame(std::size_t frame_len)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (frame_len < sizeof(ZERG_FRAME_HEAD))
    {
        frame_len = sizeof(ZERG_FRAME_HEAD);
    }

    //计算数据加密后的长度

    void *ptr = ::new unsigned char[frame_len];

#if defined(DEBUG) || defined(_DEBUG)
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr, 0, frame_len);
#endif //DEBUG

    //没有必要加下面这句，因为实际长度要根据使用决定
    //reinterpret_cast<Zerg_App_Frame*>(ptr)->frame_length_ = static_cast<uint32_t>(lenframe);
    return static_cast<ZERG_FRAME_HEAD *>(ptr);
}

//
void ZERG_FRAME_HEAD::delete_frame(ZERG_FRAME_HEAD *frame)
{
    char *ptr = reinterpret_cast<char *>(frame);
    delete[]ptr;
}

//填充AppData数据到APPFrame
int ZERG_FRAME_HEAD::fill_appdata(const size_t szdata, const char *vardata)
{
    // 判断长度是否足够
    if (szdata > MAX_LEN_OF_APPFRAME_DATA)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //填写数据区的长度
    memcpy(frame_appdata_, vardata, szdata);
    frame_length_ = static_cast<uint32_t>( ZERG_FRAME_HEAD::LEN_OF_APPFRAME_HEAD + szdata);
    return 0;
}

//将所有的uint16_t,uint32_t转换为网络序
void ZERG_FRAME_HEAD::framehead_encode()
{
    frame_length_ = htonl(frame_length_);
    frame_option_ = htonl(frame_option_);
    frame_command_ = htonl(frame_command_);

    frame_userid_ = htonl(frame_userid_);

    //
    recv_service_.services_type_ = htons(recv_service_.services_type_);
    recv_service_.services_id_ = htonl(recv_service_.services_id_);

    send_service_.services_type_ = htons(send_service_.services_type_);
    send_service_.services_id_ = htonl(send_service_.services_id_);
    //
    proxy_service_.services_type_ = htons(proxy_service_.services_type_);
    proxy_service_.services_id_ = htonl(proxy_service_.services_id_);

    transaction_id_ = htonl(transaction_id_);
    backfill_trans_id_ = htonl(backfill_trans_id_);
    business_id_ = htonl(business_id_);

    send_serial_number_ = htonl(send_serial_number_);

}

//将所有的uint16_t,uint32_t转换为本地序
void ZERG_FRAME_HEAD::framehead_decode()
{
    frame_length_ = ntohl(frame_length_);
    frame_option_ = ntohl(frame_option_);
    frame_command_ = ntohl(frame_command_);
    frame_userid_ = ntohl(frame_userid_);

    //
    recv_service_.services_type_ = ntohs(recv_service_.services_type_);
    recv_service_.services_id_ = ntohl(recv_service_.services_id_);

    send_service_.services_type_ = ntohs(send_service_.services_type_);
    send_service_.services_id_ = ntohl(send_service_.services_id_);
    //
    proxy_service_.services_type_ = ntohs(proxy_service_.services_type_);
    proxy_service_.services_id_ = ntohl(proxy_service_.services_id_);

    transaction_id_ = ntohl(transaction_id_);
    backfill_trans_id_ = ntohl(backfill_trans_id_);
    business_id_ = ntohl(business_id_);

    send_serial_number_ = ntohl(send_serial_number_);
}



//填写发送者服务信息
void ZERG_FRAME_HEAD::set_send_svcid(uint16_t svrtype, uint32_t svrid)
{
    send_service_.services_type_ = svrtype;
    send_service_.services_id_  = svrid;
}

void ZERG_FRAME_HEAD::set_recv_svcid(uint16_t svrtype, uint32_t svrid)
{
    recv_service_.services_type_ = svrtype;
    recv_service_.services_id_ = svrid;
}

void ZERG_FRAME_HEAD::set_proxy_svcid(uint16_t svrtype, uint32_t svrid)
{
    proxy_service_.services_type_ = svrtype;
    proxy_service_.services_id_ = svrid;
}

void ZERG_FRAME_HEAD::set_all_svcid(const SERVICES_ID &rcvinfo,
                                   const SERVICES_ID &sndinfo,
                                   const SERVICES_ID &proxyinfo)
{
    recv_service_ = rcvinfo;
    send_service_ = sndinfo;
    proxy_service_ = proxyinfo;
}


//交换接受者和发送者,由于要回送数据时
void ZERG_FRAME_HEAD::exchange_rcvsnd_svcid(void )
{

    SERVICES_ID tmpsvrinfo = recv_service_;
    recv_service_ = send_service_;
    send_service_ = tmpsvrinfo;
}

void ZERG_FRAME_HEAD::exchange_rcvsnd_svcid(ZERG_FRAME_HEAD &exframe )
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    transaction_id_ = exframe.transaction_id_;
    backfill_trans_id_ = exframe.backfill_trans_id_;
    business_id_ = exframe.business_id_;
    frame_userid_ = exframe.frame_userid_;
}

void ZERG_FRAME_HEAD::fillback_appframe_head(ZERG_FRAME_HEAD &exframe )
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    transaction_id_ = exframe.backfill_trans_id_;
    //回去的事务id应当是请求的事务id
    backfill_trans_id_ = exframe.transaction_id_;
    business_id_ = exframe.business_id_;
    frame_userid_ = exframe.frame_userid_;
}


//调试AppFrame, 转化所有的数据信息为一个你可以读懂的状态
//这个调试是一个非常,非常消耗时的操作,除非,发布版本千万不要使用
void ZERG_FRAME_HEAD::dump_appframe_info(std::ostringstream &strstream) const
{
    dump_appframe_head(strstream);
    dump_appframe_data(strstream);
    return;
}

//Dump所有的数据信息,一个字节字节的告诉你,
void ZERG_FRAME_HEAD::dump_appframe_data(std::ostringstream &strstream) const
{
    //
    char tmpstr[MAX_LEN_OF_APPFRAME * 2 + 1];
    size_t datalen = frame_length_ - LEN_OF_APPFRAME_HEAD;
    size_t out_len = MAX_LEN_OF_APPFRAME * 2 + 1;

    zce::base16_encode((unsigned char *)frame_appdata_, datalen, (unsigned char *)tmpstr, &out_len);

    strstream << " data:" <<  tmpstr << std::endl;

}


//输出包头信息
void ZERG_FRAME_HEAD::dump_appframe_head(std::ostringstream &strstream) const
{
    strstream << "Len:" << frame_length_
              << " Framedesc:0x " << frame_option_
              << " Command:" << frame_command_
              << " Uin:" << frame_userid_
              << " TransactionID:" << transaction_id_
              << " BackfillTransID:" << backfill_trans_id_
              << " ProcessHandler:" << business_id_
              << " Sendip:" << send_ip_address_;
    strstream << "Rcvsvr:" << recv_service_.services_type_
              << "|" << recv_service_.services_id_
              << " Sndsvr:" << send_service_.services_type_
              << "|" << send_service_.services_id_
              << " Proxysvr:" << proxy_service_.services_type_
              << "|" << proxy_service_.services_id_;
    return;
}

//输出APPFRAME的头部信息
void ZERG_FRAME_HEAD::dumpoutput_framehead(const char *outstr, ZCE_LOG_PRIORITY log_priority) const
{
    std::ostringstream strstream;
    dump_appframe_head(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s", outstr, strstream.str().c_str());
}
//输出APPFRAME的全部部信息
void ZERG_FRAME_HEAD::dumpoutput_frameinfo(const char *outstr, ZCE_LOG_PRIORITY log_priority)  const
{

    std::ostringstream strstream;
    dump_appframe_info(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s", outstr, strstream.str().c_str());
}

//输出APPFRAME的头部信息
void ZERG_FRAME_HEAD::dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const ZERG_FRAME_HEAD *proc_frame )
{
    proc_frame->dumpoutput_framehead(outstr, log_priority);
}
//输出APPFRAME的全部部信息
void ZERG_FRAME_HEAD::dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const ZERG_FRAME_HEAD *proc_frame)
{
    proc_frame->dumpoutput_frameinfo(outstr, log_priority);
}


//Clone一个APP FRAME
void ZERG_FRAME_HEAD::clone(ZERG_FRAME_HEAD *dst_frame) const
{
    memcpy(dst_frame, this, frame_length_);
    return ;
}

//Clone一个APP FRAME 的头部
void ZERG_FRAME_HEAD::clone_head(ZERG_FRAME_HEAD *clone_frame) const
{
    memcpy(clone_frame, this, LEN_OF_APPFRAME_HEAD);
    return ;
}



#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

///将一个结构进行编码
int ZERG_FRAME_HEAD::protobuf_encode(size_t szframe_appdata,
                                    const google::protobuf::MessageLite *msg,
                                    size_t data_start,
                                    size_t *sz_code)
{
    if (!msg->IsInitialized())
    {
        ZCE_LOG(RS_ERROR, "");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    size_t need_size = msg->ByteSize();
    if (data_start + need_size > szframe_appdata)
    {
        ZCE_LOG(RS_ERROR, "");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    bool bret = msg->SerializePartialToArray(frame_appdata_ + data_start,
                                             static_cast<int>(szframe_appdata - data_start));
    if (bret == false)
    {
        ZCE_LOG(RS_ERROR, "");
        return SOAR_RET::ERROR_DR_ENCODE_FAIL;
    }
    if (sz_code)
    {
        *sz_code = need_size;
    }
    return 0;
}

///将一个结构进行解码
int ZERG_FRAME_HEAD::protobuf_decode(google::protobuf::MessageLite *msg,
                                    size_t data_start,
                                    size_t *sz_code)
{
    size_t szframe_appdata = frame_length_;
    bool bret = msg->ParseFromArray(frame_appdata_ + data_start,
                                    static_cast<int>(szframe_appdata - data_start));
    if (bret == false)
    {
        ZCE_LOG(RS_ERROR, "");
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
