#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/ogre/ogre_frame.h"

/******************************************************************************************
struct  Ogre4a_App_Frame OGRE 服务器的内部命令帧头
******************************************************************************************/
//最大的DATA数据区长度,可以配置
size_t Ogre4a_App_Frame::MAX_OF_OGRE_DATA_LEN = 64 * 1024;
//最大的FRAME的长度,为MAX_OF_OGRE_DATA_LEN ＋ LEN_OF_OGRE_FRAME_HEAD
size_t Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN = Ogre4a_App_Frame::MAX_OF_OGRE_DATA_LEN + Ogre4a_App_Frame::LEN_OF_OGRE_FRAME_HEAD;

Ogre4a_App_Frame::Ogre4a_App_Frame():
    ogre_frame_len_(LEN_OF_OGRE_FRAME_HEAD),
    snd_peer_info_(0, 0),
    ogre_frame_option_(0),
    ogre_send_errnum_(0)
{
}
//
Ogre4a_App_Frame::~Ogre4a_App_Frame()
{
}



Ogre4a_App_Frame *Ogre4a_App_Frame::new_ogre(size_t lenframe)
{
    //assert( FrameLength <= MAX_FRAME_SIZE );
    if (lenframe < sizeof(Ogre4a_App_Frame))
    {
        ZCE_ASSERT(false);
        lenframe = sizeof(Ogre4a_App_Frame);
    }

    void *ptr = ::new unsigned char[lenframe];

#if defined(DEBUG) || defined(_DEBUG)
    //检查帧的哪个地方出现问题，还是这样好一点
    memset(ptr, 0, lenframe);
#endif //DEBUG

    //reinterpret_cast<Zerg_App_Frame*>(ptr)->m_Length = static_cast<unsigned int>(lenframe);
    return static_cast<Ogre4a_App_Frame *>(ptr);
}



void Ogre4a_App_Frame::delete_ogre(Ogre4a_App_Frame *ptrframe) noexcept
{
    char *ptr = reinterpret_cast<char *>(ptrframe) ;
    delete []ptr;
}




//增加数据
int Ogre4a_App_Frame::add_data(unsigned int add_size, char *add_data)
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
void Ogre4a_App_Frame::set_max_framedata_len(unsigned int  max_framedata)
{
    MAX_OF_OGRE_DATA_LEN = max_framedata;
    //最大的FRAME的长度,为MAX_OF_OGRE_DATA_LEN ＋ LEN_OF_OGRE_FRAME_HEAD
    MAX_OF_OGRE_FRAME_LEN = MAX_OF_OGRE_DATA_LEN + LEN_OF_OGRE_FRAME_HEAD;
}

//交换自己Rcv ,Snd Peer Info
void Ogre4a_App_Frame::exchange_rcvsnd_peerInfo()
{
    OGRE_PEER_ID sock_peer = snd_peer_info_;
    snd_peer_info_  = rcv_peer_info_;
    rcv_peer_info_ = sock_peer;
}

//和其他人交换Rcv ,Snd Peer Info,
void Ogre4a_App_Frame::exchange_rcvsnd_peerInfo(const Ogre4a_App_Frame *exframe )
{
    ogre_frame_option_ = exframe->ogre_frame_option_;
    rcv_peer_info_ = exframe->snd_peer_info_ ;
    snd_peer_info_ = exframe->rcv_peer_info_ ;
}

//输出APPFRAME的全部部信息
void Ogre4a_App_Frame::dump_ogre_framehead(const Ogre4a_App_Frame *proc_frame,
                                           const char *outstr,
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
void Ogre4a_App_Frame::dump_ogre_framehead(const char *outstr,
                                           zce::LOG_PRIORITY log_priority) const
{
    this->dump_ogre_framehead(this, outstr, log_priority);
}

