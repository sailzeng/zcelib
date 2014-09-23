#include "ogre_predefine.h"
#include "ogre_udp_ctrl_handler.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_ip_restrict.h"

//所有UPD端口的句柄
std::vector<OgreUDPSvcHandler *> OgreUDPSvcHandler::ary_upd_peer_;

//构造函数
OgreUDPSvcHandler::OgreUDPSvcHandler(const ZEN_Sockaddr_In &upd_addr, ZEN_Reactor *reactor):
    ZEN_Event_Handler(reactor),
    udp_bind_addr_(upd_addr),
    peer_svc_info_(upd_addr.get_ip_address(), upd_addr.get_port_number()),
    dgram_databuf_(NULL),
    ip_restrict_(Ogre4aIPRestrictMgr::instance())
{
    //保存到PEER数组
    ary_upd_peer_.push_back(this);
}

//析构函数
OgreUDPSvcHandler::~OgreUDPSvcHandler()
{
    if (dgram_databuf_ != NULL)
    {
        Ogre_Buffer_Storage::instance()->free_byte_buffer(dgram_databuf_);
    }

    //删除掉保存的PEER数组
    std::vector<OgreUDPSvcHandler *>::iterator iter_tmp = ary_upd_peer_.begin();
    std::vector<OgreUDPSvcHandler *>::iterator iter_end = ary_upd_peer_.begin();

    for (; iter_tmp != iter_end; ++iter_tmp)
    {
        if (*iter_tmp == this)
        {
            ary_upd_peer_.erase(iter_tmp);
            break;
        }
    }
}

//初始化一个UDP PEER
int OgreUDPSvcHandler::InitUDPServices()
{

    dgram_databuf_ = Ogre_Buffer_Storage::instance()->allocate_byte_buffer();

    int ret = 0;
    ret = dgram_peer_.open(&udp_bind_addr_);

    if (ret != 0)
    {
        handle_close();
        return -1;
    }

    ret = reactor()->register_handler (this, ZEN_Event_Handler::READ_MASK );

    if (ret != 0)
    {
        return -2;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//打开一个临时PEER用于发送UDP数据
int OgreUDPSvcHandler::OpenUDPSendPeer()
{
    int ret = 0;

    if (ret != 0)
    {
        return TSS_RET::ERR_OGRE_INIT_UDP_PEER;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//取得句柄
ZEN_SOCKET OgreUDPSvcHandler::get_handle(void) const
{
    return dgram_peer_.get_handle();
}

int OgreUDPSvcHandler::handle_input(ZEN_HANDLE)
{
    size_t szrevc = 0;
    int ret = 0;
    ZEN_Sockaddr_In remote_addr;
    //读取数据
    ret = ReadDataFromUDP(szrevc, remote_addr);

    ZEN_LOGMSG_DBG(RS_DEBUG, "UDP Read Event[%s|%u].UPD Handle input event triggered. ret:%d,szrecv:%u.\n",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number(),
                   ret,
                   szrevc);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        //return -1吗，但是我真不知道如何处理
        //return -1;
    }

    //如果出来成功
    if (szrevc > 0)
    {
        pushdata_to_recvpipe();
    }

    return 0;
}

//
int OgreUDPSvcHandler::handle_close ()
{
    //
    if (dgram_peer_.get_handle () != ZEN_INVALID_SOCKET)
    {
        reactor()->remove_handler (this, false);
        dgram_peer_.close ();
    }

    //删除自己
    delete this;

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年7月18日
Function        : OgreUDPSvcHandler::ReadDataFromUDP
Return          : int == 0表示成功
Parameter List  :
  Param1: size_t& szrevc 收到数据的长度
  Param2: ZEN_Sockaddr_In &remote_addr 远端的地址
Description     : 读取UDP数据
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int OgreUDPSvcHandler::ReadDataFromUDP(size_t &szrevc, ZEN_Sockaddr_In &remote_addr)
{
    int ret = 0;
    szrevc = 0;
    ssize_t recvret = 0;

    recvret = dgram_peer_.recvfrom(dgram_databuf_->frame_data_,
                                   Ogre4a_AppFrame::MAX_OF_OGRE_DATA_LEN,
                                   0,
                                   &remote_addr);

    if (recvret < 0)
    {

        if (ZEN_OS::last_error() != EWOULDBLOCK )
        {
            //遇到中断,等待重入
            if (ZEN_OS::last_error() == EINVAL)
            {
                return TSS_RET::TSS_RET_SUCC;
            }

            //记录错误,返回错误
            ZLOG_ERROR( "UDP Read error [%s|%u],receive data error peer:%u ZEN_OS::last_error()=%d|%s.\n",
                        remote_addr.get_host_addr(),
                        remote_addr.get_port_number(),
                        dgram_peer_.get_handle(),
                        ZEN_OS::last_error(),
                        strerror(ZEN_OS::last_error()));
            return TSS_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }
        else
        {
            return TSS_RET::TSS_RET_SUCC;
        }
    }

    //如果允许的连接的服务器地址中间没有.或者在拒绝的服务列表中... kill
    ret =  ip_restrict_->check_ip_restrict(remote_addr) ;

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    //Socket被关闭，也返回错误标示,但是我不知道会不会出现这个问题...
    if (recvret == 0)
    {
        ZLOG_ERROR( "UDP Read error [%s|%u].UDP Peer recv return 0, I don't know how to process.?\n",
                    remote_addr.get_host_addr(),
                    remote_addr.get_port_number());
        return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
    }

    dgram_databuf_->snd_peer_info_.SetSocketPeerInfo(remote_addr);
    dgram_databuf_->rcv_peer_info_ = this->peer_svc_info_;
    dgram_databuf_->ogre_frame_len_ = Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD + recvret;
    //避免发生其他人填写的情况
    dgram_databuf_->ogre_frame_option_ = 0;
    dgram_databuf_->ogre_frame_option_ |= Ogre4a_AppFrame::OGREDESC_PEER_UDP;

    szrevc = recvret;

    return TSS_RET::TSS_RET_SUCC;
}

int OgreUDPSvcHandler::pushdata_to_recvpipe()
{

    int ret = Zerg_MMAP_BusPipe::instance()->push_back_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID,
                                                           reinterpret_cast<ZEN_LIB::dequechunk_node *>(dgram_databuf_));

    //无论处理正确与否,都释放缓冲区的空间

    //日志在函数中有输出,这儿略.
    if (ret != TSS_RET::TSS_RET_SUCC )
    {
        return TSS_RET::ERR_OGRE_RECEIVE_PIPE_IS_FULL;
    }

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年7月18日
Function        : OgreUDPSvcHandler::SendAllDataToUDP
Return          : int
Parameter List  :
  Param1: Ogre4a_AppFrame* send_frame 发送的FRAME
Description     : 发送UDP数据。
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int OgreUDPSvcHandler::SendAllDataToUDP(Ogre4a_AppFrame *send_frame)
{
    ssize_t szsend = -1;

    const size_t BUFFER_LEN = 32;
    char buffer[BUFFER_LEN + 1];

    ZEN_Sockaddr_In remote_addr(send_frame->rcv_peer_info_.peer_ip_address_,
                                send_frame->rcv_peer_info_.peer_port_);

    //
    size_t i = 0;

    for (; i < ary_upd_peer_.size(); ++i)
    {
        //找到对应的端口，有多个UDP的端口，选择业务层标注的端口发送
        if (ary_upd_peer_[i]->peer_svc_info_ == send_frame->snd_peer_info_ )
        {
            szsend = ary_upd_peer_[i]->dgram_peer_.sendto(send_frame->frame_data_,
                                                          send_frame->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD,
                                                          0,
                                                          &remote_addr);
            break;
        }
    }

    //没有找到相应的端口，给你一点信息
    if (i == ary_upd_peer_.size())
    {
        ZLOG_ERROR( "Can't find send peer[%s|%u].Please check code.\n",
                    ZEN_OS::inet_ntoa(send_frame->snd_peer_info_.peer_ip_address_, buffer, BUFFER_LEN),
                    send_frame->snd_peer_info_.peer_port_);
        return TSS_RET::ERR_OGRE_SOCKET_OP_ERROR;
    }

    //发送失败
    if (szsend <= 0)
    {
        ZLOG_ERROR( "UDP send error[%s|%u]. Send data error peer:%u ZEN_OS::last_error()=%d|%s.\n",
                    remote_addr.get_host_addr(),
                    remote_addr.get_port_number(),
                    ary_upd_peer_[i]->get_handle(),
                    ZEN_OS::last_error(),
                    strerror(ZEN_OS::last_error()));
        return TSS_RET::ERR_OGRE_SOCKET_OP_ERROR;
    }

    ZEN_LOGMSG_DBG(RS_DEBUG, "UDP Send data to peer [%s|%u]  Socket %u bytes data Succ.\n",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number(),
                   szsend);
    return TSS_RET::TSS_RET_SUCC;
}
