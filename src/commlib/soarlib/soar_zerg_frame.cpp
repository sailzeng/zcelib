#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_frame.h"

//ZERG服务器间传递消息的通用帧头
Zerg_App_Frame::Zerg_App_Frame(uint32_t cmd,
                               uint32_t lenframe,
                               uint32_t frameoption):
    frame_length_(lenframe),
    frame_option_(frameoption),
    frame_command_(cmd),
    frame_uid_(0),
    send_service_(0, 0),
    recv_service_(0, 0),
    proxy_service_(0, 0),
    transaction_id_(0),
    backfill_trans_id_(0),
    send_ip_address_(0)
{
}

//构造函数,用于,客户端初始化,
Zerg_App_Frame::Zerg_App_Frame(uint32_t cmd,
                               uint32_t lenframe,
                               uint32_t uin,
                               uint16_t sndsvrtype,
                               uint16_t rcvsvctype,
                               uint32_t frameoption):
    frame_length_(lenframe),
    frame_option_(frameoption),
    frame_command_(cmd),
    frame_uid_(uin),
    send_service_(sndsvrtype, uin),
    recv_service_(rcvsvctype, 0),
    proxy_service_(0, 0),
    transaction_id_(0),
    backfill_trans_id_(0),
    app_id_(0),
    send_ip_address_(0)
{
}
//构造函数,用于发送给一个非代理服务器
Zerg_App_Frame::Zerg_App_Frame(uint32_t cmd,
                               uint32_t lenframe,
                               uint32_t uin,
                               const SERVICES_ID &sndsvr,
                               const SERVICES_ID &rcvsvr,
                               uint32_t frameoption):
    frame_length_(lenframe),
    frame_option_(frameoption),
    frame_command_(cmd),
    frame_uid_(uin),
    send_service_(sndsvr),
    recv_service_(rcvsvr),
    proxy_service_(0, 0),
    transaction_id_(0),
    backfill_trans_id_(0),
    app_id_(0),
    send_ip_address_(0)
{
}

//构造函数,用于发送给一个代理服务器
Zerg_App_Frame::Zerg_App_Frame(uint32_t cmd,
                               uint32_t lenframe,
                               uint32_t uin,
                               const SERVICES_ID &sndsvr,
                               const SERVICES_ID &proxysvr,
                               uint16_t rcvsvrtype,
                               uint32_t frameoption):
    frame_length_(lenframe),
    frame_option_(frameoption),
    frame_command_(cmd),
    frame_uid_(uin),
    send_service_(sndsvr),
    recv_service_(rcvsvrtype, 0),
    proxy_service_(proxysvr),
    transaction_id_(0),
    backfill_trans_id_(0),
    app_id_(0),
    send_ip_address_(0)
{
}

Zerg_App_Frame::~Zerg_App_Frame()
{
}

// Assign =运算符号
Zerg_App_Frame &Zerg_App_Frame::operator = (const Zerg_App_Frame &other)
{
    if (this != &other )
    {
        memcpy(this, &other, other.frame_length_);
    }

    return *this;
}

//重载New函数
void   *Zerg_App_Frame::operator new (size_t , size_t lenframe)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (lenframe < sizeof(Zerg_App_Frame))
    {
        lenframe = sizeof(Zerg_App_Frame);
    }

    //计算数据加密后的长度

    void *ptr = ::new unsigned char[lenframe];

#if defined(DEBUG) || defined(_DEBUG)
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr, 0, lenframe);
#endif //DEBUG

    //reinterpret_cast<Zerg_App_Frame*>(ptr)->frame_length_ = static_cast<uint32_t>(lenframe);
    return ptr;
}

//养成好习惯,写new,就写delete.
//其实不写也不会有内存泄露,但是为了不得罪讨厌的编译器.
#if defined ZCE_OS_WINDOWS
void Zerg_App_Frame::operator delete(void *ptrframe, size_t)
#elif defined ZCE_OS_LINUX
void Zerg_App_Frame::operator delete(void *ptrframe)
#endif
{
    char *ptr = reinterpret_cast<char *>(ptrframe) ;
    delete []ptr;
}



//填充AppData数据到APPFrame
int Zerg_App_Frame::fill_appdata(const size_t szdata, const char *vardata)
{
    // 判断长度是否足够
    if (szdata > MAX_LEN_OF_APPFRAME_DATA)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //填写数据区的长度
    memcpy(frame_appdata_, vardata, szdata);
    frame_length_ = static_cast<uint32_t>( Zerg_App_Frame::LEN_OF_APPFRAME_HEAD + szdata);
    return 0;
}

//将所有的uint16_t,uint32_t转换为网络序
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

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年11月3日
Function        : Zerg_App_Frame::framehead_decode
Return          : void
Parameter List  : NULL
Description     : 将所有的uint16_t,uint32_t转换为本地序
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
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

//初始化包头的数据
void Zerg_App_Frame::init_framehead(uint32_t lenframe, uint32_t option, uint32_t cmd)
{
    frame_length_ = static_cast<uint32_t>(lenframe);
    frame_command_ = cmd;
    frame_option_ = option ;
    inner_option_.frame_version_ = TSS_APPFRAME_V1;

    frame_uid_ = 0;

    send_service_.set_svcid(0, 0);
    recv_service_.set_svcid(0, 0);
    proxy_service_.set_svcid(0, 0);
    send_serial_number_ = 0;
    transaction_id_ = 0;
    backfill_trans_id_ = 0;
    app_id_ = 0;
}

//填写发送者服务信息
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

void Zerg_App_Frame::set_all_svcid(const SERVICES_ID &rcvinfo, const SERVICES_ID &sndinfo, const SERVICES_ID &proxyinfo)
{
    recv_service_ = rcvinfo;
    send_service_ = sndinfo;
    proxy_service_ = proxyinfo;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年11月29日
Function        : Zerg_App_Frame::exchange_rcvsnd_svcid
Return          : void
Parameter List  :
  Param1: void
Description     : 交换接受者和发送者,由于要回送数据时
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
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
    //回去的事务id应当是请求的事务id
    backfill_trans_id_ = exframe.transaction_id_;
    app_id_ = exframe.app_id_;
    frame_uid_ = exframe.frame_uid_;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年11月29日
Function        : Zerg_App_Frame::dump_appframe_info
Return          : void
Parameter List  :
  Param1: std::ostringstream& strstream 调试的输出结果,
Description     : 调试AppFrame,转化所有的数据信息为一个你可以读懂的状态
Calls           :
Called By       :
Other           : 这个调试是一个非常,非常消耗时的操作,除非,发布版本千万不要使用
Modify Record   :
******************************************************************************************/
void Zerg_App_Frame::dump_appframe_info(std::ostringstream &strstream) const
{
    dump_appframe_head(strstream);
    dump_appframe_data(strstream);
    return;
}

//Dump所有的数据信息,一个字节字节的告诉你,
void Zerg_App_Frame::dump_appframe_data(std::ostringstream &strstream) const
{
    //
    char tmpstr[MAX_LEN_OF_APPFRAME * 2 + 1];
    size_t datalen = frame_length_ - LEN_OF_APPFRAME_HEAD;
    size_t out_len = MAX_LEN_OF_APPFRAME * 2 + 1;

    ZCE_LIB::base16_encode((unsigned char *)frame_appdata_, datalen, (unsigned char *)tmpstr, &out_len);

    strstream << " data:" <<  tmpstr << std::endl;

}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年12月1日
Function        : dump_appframe_head
Return          : void
Parameter List  :
  Param1: std::ostringstream& strstream 输出的Str String
Description     : 输出包头信息
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
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

//输出APPFRAME的头部信息
void Zerg_App_Frame::dumpoutput_framehead(const char *outstr, ZCE_LOG_PRIORITY log_priority) const
{
    std::ostringstream strstream;
    dump_appframe_head(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s" , outstr, strstream.str().c_str());
}
//输出APPFRAME的全部部信息
void Zerg_App_Frame::dumpoutput_frameinfo(const char *outstr, ZCE_LOG_PRIORITY log_priority)  const
{

    std::ostringstream strstream;
    dump_appframe_info(strstream);
    ZCE_LOG(log_priority, "[framework] [%s]%s" , outstr, strstream.str().c_str());
}

//输出APPFRAME的头部信息
void Zerg_App_Frame::dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const Zerg_App_Frame *proc_frame )
{
    proc_frame->dumpoutput_framehead(outstr, log_priority);
}
//输出APPFRAME的全部部信息
void Zerg_App_Frame::dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                          const char *outstr,
                                          const Zerg_App_Frame *proc_frame)
{
    proc_frame->dumpoutput_frameinfo(outstr, log_priority);
}

//Clone一个APP FRAME
Zerg_App_Frame *Zerg_App_Frame::clone() const
{
    Zerg_App_Frame *proc_frame = new(frame_length_) Zerg_App_Frame();
    memcpy(proc_frame, this, frame_length_);
    return proc_frame;
}

//Clone一个APP FRAME
void Zerg_App_Frame::clone(Zerg_App_Frame *clone_frame) const
{
    memcpy(clone_frame, this, frame_length_);
    return ;
}

//Clone一个APP FRAME 的头部
void Zerg_App_Frame::clone_head(Zerg_App_Frame *clone_frame) const
{
    memcpy(clone_frame, this, LEN_OF_APPFRAME_HEAD);
    return ;
}

