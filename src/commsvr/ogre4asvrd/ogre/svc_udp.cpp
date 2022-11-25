#include "ogre/predefine.h"
#include "ogre/buf_storage.h"
#include "ogre/configure.h"
#include "ogre/ip_restrict.h"
#include "ogre/svc_udp.h"

namespace ogre
{
//所有UPD端口的句柄
std::vector<svc_udp*> svc_udp::ary_upd_peer_;

//构造函数
svc_udp::svc_udp(const zce::skt::addr_in& upd_addr, zce::reactor* reactor) :
    zce::event_handler(reactor),
    udp_bind_addr_(upd_addr),
    peer_svc_info_(upd_addr.get_ip_address(), upd_addr.get_port()),
    dgram_databuf_(nullptr),
    ip_restrict_(ip_restrict::instance())
{
    //保存到PEER数组
    ary_upd_peer_.push_back(this);
}

//析构函数
svc_udp::~svc_udp()
{
    if (dgram_databuf_ != nullptr)
    {
        buffer_storage::instance()->free_byte_buffer(dgram_databuf_);
    }

    //删除掉保存的PEER数组
    std::vector<svc_udp*>::iterator iter_tmp = ary_upd_peer_.begin();
    std::vector<svc_udp*>::iterator iter_end = ary_upd_peer_.begin();

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
int svc_udp::init_udp_peer()
{
    dgram_databuf_ = buffer_storage::instance()->allocate_byte_buffer();

    int ret = 0;
    ret = dgram_peer_.open(&udp_bind_addr_);
    if (ret != 0)
    {
        close_handle();
        return -1;
    }

    ret = reactor()->register_handler(this, zce::READ_MASK);

    if (ret != 0)
    {
        return -2;
    }

    return 0;
}

//取得句柄
ZCE_HANDLE svc_udp::get_handle(void) const
{
    return (ZCE_HANDLE)dgram_peer_.get_handle();
}

void svc_udp::read_event()
{
    size_t szrevc = 0;
    int ret = 0;
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    zce::skt::addr_in remote_addr;
    //读取数据
    ret = read_data_fromudp(szrevc, remote_addr);

    ZCE_LOG_DEBUG(RS_DEBUG, "UDP Read Event[%s].UPD Handle input event triggered. ret:%d,szrecv:%u.\n",
                  remote_addr.to_str(ip_addr_str, IP_ADDR_LEN, use_len),
                  ret,
                  szrevc);

    if (ret != 0)
    {
        //close_handle 吗，但是我真不知道如何处理
        //return close_handle();
    }

    //如果出来成功
    if (szrevc > 0)
    {
        pushdata_to_recvpipe();
    }

    return;
}

//
void svc_udp::close_handle()
{
    //
    if (dgram_peer_.get_handle() != ZCE_INVALID_SOCKET)
    {
        reactor()->remove_handler(this, false);
        dgram_peer_.close();
    }

    //删除自己
    delete this;

    return;
}

//读取UDP数据
int svc_udp::read_data_fromudp(size_t& szrevc, zce::skt::addr_in& remote_addr)
{
    int ret = 0;
    szrevc = 0;
    ssize_t recvret = 0;
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    recvret = dgram_peer_.recvfrom(dgram_databuf_->frame_data_,
                                   soar::ogre4a_frame::MAX_OF_OGRE_DATA_LEN,
                                   0,
                                   &remote_addr);

    if (recvret < 0)
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            //遇到中断,等待重入
            if (zce::last_error() == EINVAL)
            {
                return 0;
            }

            //记录错误,返回错误
            ZCE_LOG(RS_ERROR, "UDP Read error [%s],receive data error peer:%u zce::last_error()=%d|%s.\n",
                    remote_addr.to_str(ip_addr_str, IP_ADDR_LEN, use_len),
                    dgram_peer_.get_handle(),
                    zce::last_error(),
                    strerror(zce::last_error()));
            return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }
        else
        {
            return 0;
        }
    }

    //如果允许的连接的服务器地址中间没有.或者在拒绝的服务列表中... kill
    ret = ip_restrict_->check_ip_restrict(remote_addr);

    if (ret != 0)
    {
        return ret;
    }

    //Socket被关闭，也返回错误标示,但是我不知道会不会出现这个问题...
    if (recvret == 0)
    {
        ZCE_LOG(RS_ERROR, "UDP Read error [%s].UDP Peer recv return 0, I don't know how to process.?\n",
                remote_addr.to_str(ip_addr_str, IP_ADDR_LEN, use_len));
        return SOAR_RET::ERR_OGRE_SOCKET_CLOSE;
    }

    dgram_databuf_->snd_peer_info_.set(remote_addr);
    dgram_databuf_->rcv_peer_info_ = this->peer_svc_info_;
    dgram_databuf_->ogre_frame_len_ = static_cast<unsigned int>(soar::ogre4a_frame::LEN_OF_OGRE_FRAME_HEAD + recvret);
    //避免发生其他人填写的情况
    dgram_databuf_->ogre_frame_option_ = 0;
    dgram_databuf_->ogre_frame_option_ |= soar::ogre4a_frame::OGREDESC_PEER_UDP;

    szrevc = recvret;

    return 0;
}

int svc_udp::pushdata_to_recvpipe()
{
    int ret = soar::svrd_buspipe::instance()->push_back_recvbus(
        reinterpret_cast<soar::zerg_frame*>(dgram_databuf_));

    //无论处理正确与否,都释放缓冲区的空间

    //日志在函数中有输出,这儿略.
    if (ret != 0)
    {
        return SOAR_RET::ERR_OGRE_RECEIVE_PIPE_IS_FULL;
    }

    return 0;
}

//发送UDP数据。
int svc_udp::send_alldata_to_udp(soar::ogre4a_frame* send_frame)
{
    ssize_t szsend = -1;

    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    zce::skt::addr_in remote_addr(send_frame->rcv_peer_info_.peer_ip_address_,
                                  send_frame->rcv_peer_info_.peer_port_);

    //
    size_t i = 0;

    for (; i < ary_upd_peer_.size(); ++i)
    {
        //找到对应的端口，有多个UDP的端口，选择业务层标注的端口发送
        if (ary_upd_peer_[i]->peer_svc_info_ == send_frame->snd_peer_info_)
        {
            szsend = ary_upd_peer_[i]->dgram_peer_.sendto(send_frame->frame_data_,
                                                          send_frame->ogre_frame_len_ - soar::ogre4a_frame::LEN_OF_OGRE_FRAME_HEAD,
                                                          0,
                                                          &remote_addr);
            break;
        }
    }

    //没有找到相应的端口，给你一点信息
    if (i == ary_upd_peer_.size())
    {
        ZCE_LOG(RS_ERROR, "Can't find send peer[%s|%u].Please check code.\n",
                zce::inet_ntoa(send_frame->snd_peer_info_.peer_ip_address_, ip_addr_str, IP_ADDR_LEN),
                send_frame->snd_peer_info_.peer_port_);
        return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
    }

    //发送失败
    if (szsend <= 0)
    {
        ZCE_LOG(RS_ERROR, "UDP send error[%s]. Send data error peer:%u zce::last_error()=%d|%s.\n",
                remote_addr.to_str(ip_addr_str, IP_ADDR_LEN, use_len),
                ary_upd_peer_[i]->get_handle(),
                zce::last_error(),
                strerror(zce::last_error()));
        return SOAR_RET::ERR_OGRE_SOCKET_OP_ERROR;
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "UDP Send data to peer [%s]  Socket %u bytes data Succ.\n",
                  remote_addr.to_str(ip_addr_str, IP_ADDR_LEN, use_len),
                  szsend);
    return 0;
}
}//namespace ogre