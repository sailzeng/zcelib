#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_frame.h"


#if defined (ZCE_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

Zerg_App_Frame::Zerg_App_Frame() :
    frame_length_(CMD_INVALID_CMD),
    frame_option_(DESC_V1_VERSION),
    frame_command_(LEN_OF_APPFRAME_HEAD),
    frame_uid_(0),
    app_id_(0),
    send_service_(0, 0),
    recv_service_(0, 0),
    proxy_service_(0, 0),
    transaction_id_(0),
    backfill_trans_id_(0),
    send_ip_address_(0)
{
}

Zerg_App_Frame::~Zerg_App_Frame()
{
}


// Assign =�������
Zerg_App_Frame &Zerg_App_Frame::operator = (const Zerg_App_Frame &other)
{
    if (this != &other )
    {
        memcpy(this, &other, other.frame_length_);
    }

    return *this;
}

//����һ��Frame
Zerg_App_Frame *Zerg_App_Frame::new_frame(std::size_t frame_len)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (frame_len < sizeof(Zerg_App_Frame))
    {
        frame_len = sizeof(Zerg_App_Frame);
    }

    //�������ݼ��ܺ�ĳ���

    void *ptr = ::new unsigned char[frame_len];

#if defined(DEBUG) || defined(_DEBUG)
    //���֡���ĸ��ط��������⣬����������һ��
    memset(ptr, 0, frame_len);
#endif //DEBUG

    //û�б�Ҫ��������䣬��Ϊʵ�ʳ���Ҫ����ʹ�þ���
    //reinterpret_cast<Zerg_App_Frame*>(ptr)->frame_length_ = static_cast<uint32_t>(lenframe);
    return static_cast<Zerg_App_Frame *>(ptr);
}

//
void Zerg_App_Frame::delete_frame(Zerg_App_Frame *frame)
{
    char *ptr = reinterpret_cast<char *>(frame);
    delete[]ptr;
}

//���AppData���ݵ�APPFrame
int Zerg_App_Frame::fill_appdata(const size_t szdata, const char *vardata)
{
    // �жϳ����Ƿ��㹻
    if (szdata > MAX_LEN_OF_APPFRAME_DATA)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //��д�������ĳ���
    memcpy(frame_appdata_, vardata, szdata);
    frame_length_ = static_cast<uint32_t>( Zerg_App_Frame::LEN_OF_APPFRAME_HEAD + szdata);
    return 0;
}

//�����е�uint16_t,uint32_tת��Ϊ������
void Zerg_App_Frame::framehead_encode()
{
    frame_length_ = htonl(frame_length_);
    frame_option_ = htonl(frame_option_);
    frame_command_ = htonl(frame_command_);

    frame_uid_ = htonl(frame_uid_);

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
    app_id_ = htonl(app_id_);

    send_serial_number_ = htonl(send_serial_number_);

}

//�����е�uint16_t,uint32_tת��Ϊ������
void Zerg_App_Frame::framehead_decode()
{
    frame_length_ = ntohl(frame_length_);
    frame_option_ = ntohl(frame_option_);
    frame_command_ = ntohl(frame_command_);
    frame_uid_ = ntohl(frame_uid_);

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
    app_id_ = ntohl(app_id_);

    send_serial_number_ = ntohl(send_serial_number_);
}



//��д�����߷�����Ϣ
void Zerg_App_Frame::set_send_svcid(uint16_t svrtype, uint32_t svrid)
{
    send_service_.services_type_ = svrtype;
    send_service_.services_id_  = svrid;
}

void Zerg_App_Frame::set_recv_svcid(uint16_t svrtype, uint32_t svrid)
{
    recv_service_.services_type_ = svrtype;
    recv_service_.services_id_ = svrid;
}

void Zerg_App_Frame::set_proxy_svcid(uint16_t svrtype, uint32_t svrid)
{
    proxy_service_.services_type_ = svrtype;
    proxy_service_.services_id_ = svrid;
}

void Zerg_App_Frame::set_all_svcid(const SERVICES_ID &rcvinfo,
                                   const SERVICES_ID &sndinfo,
                                   const SERVICES_ID &proxyinfo)
{
    recv_service_ = rcvinfo;
    send_service_ = sndinfo;
    proxy_service_ = proxyinfo;
}


//���������ߺͷ�����,����Ҫ��������ʱ
void Zerg_App_Frame::exchange_rcvsnd_svcid(void )
{

    SERVICES_ID tmpsvrinfo = recv_service_;
    recv_service_ = send_service_;
    send_service_ = tmpsvrinfo;
}

void Zerg_App_Frame::exchange_rcvsnd_svcid(Zerg_App_Frame &exframe )
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    transaction_id_ = exframe.transaction_id_;
    backfill_trans_id_ = exframe.backfill_trans_id_;
    app_id_ = exframe.app_id_;
    frame_uid_ = exframe.frame_uid_;
}

void Zerg_App_Frame::fillback_appframe_head(Zerg_App_Frame &exframe )
{
    recv_service_ = exframe.send_service_;
    send_service_ = exframe.recv_service_;
    proxy_service_ = exframe.proxy_service_;
    transaction_id_ = exframe.backfill_trans_id_;
    //��ȥ������idӦ�������������id
    backfill_trans_id_ = exframe.transaction_id_;
    app_id_ = exframe.app_id_;
    frame_uid_ = exframe.frame_uid_;
}


//����AppFrame, ת�����е�������ϢΪһ������Զ�����״̬
//���������һ���ǳ�,�ǳ�����ʱ�Ĳ���,����,�����汾ǧ��Ҫʹ��
void Zerg_App_Frame::dump_appframe_info(std::ostringstream &strstream) const
{
    dump_appframe_head(strstream);
    dump_appframe_data(strstream);
    return;
}

//Dump���е�������Ϣ,һ���ֽ��ֽڵĸ�����,
void Zerg_App_Frame::dump_appframe_data(std::ostringstream &strstream) const
{
    //
    char tmpstr[MAX_LEN_OF_APPFRAME * 2 + 1];
    size_t datalen = frame_length_ - LEN_OF_APPFRAME_HEAD;
    size_t out_len = MAX_LEN_OF_APPFRAME * 2 + 1;

    zce::base16_encode((unsigned char *)frame_appdata_, datalen, (unsigned char *)tmpstr, &out_len);

    strstream << " data:" <<  tmpstr << std::endl;

}


//�����ͷ��Ϣ
void Zerg_App_Frame::dump_appframe_head(std::ostringstream &strstream) const
{
    strstream << "Len:" << frame_length_
              << " Framedesc:0x " << frame_option_
              << " Command:" << frame_command_
              << " Uin:" << frame_uid_
              << " TransactionID:" << transaction_id_
              << " BackfillTransID:" << backfill_trans_id_
              << " ProcessHandler:" << app_id_
              << " Sendip:" << send_ip_address_;
    strstream << "Rcvsvr:" << recv_service_.services_type_
              << "|" << recv_service_.services_id_
              << " Sndsvr:" << send_service_.services_type_
              << "|" << send_service_.services_id_
              << " Proxysvr:" << proxy_service_.services_type_
              << "|" << proxy_service_.services_id_;
    return;
}

//���APPFRAME��ͷ����Ϣ
void Zerg_App_Frame::dumpoutput_framehead(const char *outstr, ZCE_LOG_PRIORITY log_priority) const
{
    std::ostringstream strstream;
    dump_appframe_head(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s", outstr, strstream.str().c_str());
}
//���APPFRAME��ȫ������Ϣ
void Zerg_App_Frame::dumpoutput_frameinfo(const char *outstr, ZCE_LOG_PRIORITY log_priority)  const
{

    std::ostringstream strstream;
    dump_appframe_info(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s", outstr, strstream.str().c_str());
}

//���APPFRAME��ͷ����Ϣ
void Zerg_App_Frame::dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const Zerg_App_Frame *proc_frame )
{
    proc_frame->dumpoutput_framehead(outstr, log_priority);
}
//���APPFRAME��ȫ������Ϣ
void Zerg_App_Frame::dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const Zerg_App_Frame *proc_frame)
{
    proc_frame->dumpoutput_frameinfo(outstr, log_priority);
}


//Cloneһ��APP FRAME
void Zerg_App_Frame::clone(Zerg_App_Frame *dst_frame) const
{
    memcpy(dst_frame, this, frame_length_);
    return ;
}

//Cloneһ��APP FRAME ��ͷ��
void Zerg_App_Frame::clone_head(Zerg_App_Frame *clone_frame) const
{
    memcpy(clone_frame, this, LEN_OF_APPFRAME_HEAD);
    return ;
}



#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

///��һ���ṹ���б���
int Zerg_App_Frame::protobuf_encode(size_t szframe_appdata,
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

///��һ���ṹ���н���
int Zerg_App_Frame::protobuf_decode(google::protobuf::MessageLite *msg,
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
