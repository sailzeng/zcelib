#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/ogre/frame_ogre.h"

/******************************************************************************************
struct  ogre4a_frame OGRE 服务器的内部命令帧头
******************************************************************************************/
//最大的DATA数据区长度,可以配置
size_t ogre4a_frame::MAX_OF_OGRE_DATA_LEN = 64 * 1024;
//最大的FRAME的长度,为MAX_OF_OGRE_DATA_LEN ＋ LEN_OF_OGRE_FRAME_HEAD
size_t ogre4a_frame::MAX_OF_OGRE_FRAME_LEN = ogre4a_frame::MAX_OF_OGRE_DATA_LEN + ogre4a_frame::LEN_OF_OGRE_FRAME_HEAD;

ogre4a_frame::ogre4a_frame() :
    ogre_frame_len_(LEN_OF_OGRE_FRAME_HEAD),
    snd_peer_info_(0, 0),
    ogre_frame_option_(0),
    ogre_send_errnum_(0)
{
}
//
ogre4a_frame::~ogre4a_frame()
{
}

ogre4a_frame* ogre4a_frame::new_ogre(size_t lenframe)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (lenframe < sizeof(ogre4a_frame))
    {
        ZCE_ASSERT(false);
        lenframe = sizeof(ogre4a_frame);
    }

    void* ptr = ::new unsigned char[lenframe];

#if defined(DEBUG) || defined(_DEBUG)
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr, 0, lenframe);
#endif //DEBUG

    //reinterpret_cast<soar::zerg_frame*>(ptr)->m_Length = static_cast<unsigned int>(lenframe);
    return static_cast<ogre4a_frame*>(ptr);
}

void ogre4a_frame::delete_ogre(ogre4a_frame* ptrframe) noexcept
{
    char* ptr = reinterpret_cast<char*>(ptrframe);
    delete[]ptr;
}

//增加数据
int ogre4a_frame::add_data(unsigned int add_size, char* add_data)
{
    if (ogre_frame_len_ + add_size > MAX_OF_OGRE_DATA_LEN)
    {
        return SOAR_RET::ERROR_APPFRAME_ERROR;
    }
    else
    {
        memcpy(frame_data_ + ogre_frame_len_, add_data, add_size);
        ogre_frame_len_ += add_size;
        return 0;
    }
}

//配置最大的DATA数据区长度,
void ogre4a_frame::set_max_framedata_len(unsigned int  max_framedata)
{
    MAX_OF_OGRE_DATA_LEN = max_framedata;
    //最大的FRAME的长度,为MAX_OF_OGRE_DATA_LEN ＋ LEN_OF_OGRE_FRAME_HEAD
    MAX_OF_OGRE_FRAME_LEN = MAX_OF_OGRE_DATA_LEN + LEN_OF_OGRE_FRAME_HEAD;
}

//交换自己Rcv ,Snd Peer Info
void ogre4a_frame::exchange_rcvsnd_peerInfo()
{
    OGRE_PEER_ID sock_peer = snd_peer_info_;
    snd_peer_info_ = rcv_peer_info_;
    rcv_peer_info_ = sock_peer;
}

//和其他人交换Rcv ,Snd Peer Info,
void ogre4a_frame::exchange_rcvsnd_peerInfo(const ogre4a_frame* exframe)
{
    ogre_frame_option_ = exframe->ogre_frame_option_;
    rcv_peer_info_ = exframe->snd_peer_info_;
    snd_peer_info_ = exframe->rcv_peer_info_;
}

//输出APPFRAME的全部部信息
void ogre4a_frame::dump_ogre_framehead(const ogre4a_frame* proc_frame,
                                       const char* outstr,
                                       zce::LOG_PRIORITY log_priority)
{
    ZCE_LOG(log_priority, "[framework] [OGRE_FRAME_MONITOR]location:%s ,frame_len:%u,send_ip:%u,send_port:%u,rcv_ip:%u,rcv_port:%u,",
            outstr,
            proc_frame->ogre_frame_len_,
            proc_frame->snd_peer_info_.peer_ip_address_,
            proc_frame->snd_peer_info_.peer_port_,
            proc_frame->rcv_peer_info_.peer_ip_address_,
            proc_frame->rcv_peer_info_.peer_port_
    );

#ifdef ZCE_OS_WINDOWS
    ZCE_UNUSED_ARG(outstr);
    ZCE_UNUSED_ARG(log_priority);
#endif
}

//输出APPFRAME的全部部信息
void ogre4a_frame::dump_ogre_framehead(const char* outstr,
                                       zce::LOG_PRIORITY log_priority) const
{
    this->dump_ogre_framehead(this, outstr, log_priority);
}