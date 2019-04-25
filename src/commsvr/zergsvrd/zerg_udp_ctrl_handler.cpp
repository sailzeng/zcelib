
#include "zerg_predefine.h"
#include "zerg_buf_storage.h"
#include "zerg_application.h"
#include "zerg_ip_restrict.h"
#include "zerg_comm_manager.h"
#include "zerg_stat_define.h"
#include "zerg_udp_ctrl_handler.h"

//
UDP_Svc_Handler::ARY_OF_UDPSVC_HANDLER UDP_Svc_Handler::ary_udpsvc_handler_;
//
Soar_Stat_Monitor  *UDP_Svc_Handler::server_status_ = NULL;
//通信管理器
Zerg_Comm_Manager *UDP_Svc_Handler::zerg_comm_mgr_ = NULL;

//自己是否是代理
bool           UDP_Svc_Handler::if_proxy_ = false;

//
UDP_Svc_Handler::UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                                 const ZCE_Sockaddr_In     &addr,
                                 bool sessionkey_verify) :
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    udp_bind_addr_(addr),
    my_svc_info_(my_svcinfo),
    sessionkey_verify_(sessionkey_verify),
    dgram_databuf_(NULL),
    ip_restrict_(Zerg_IPRestrict_Mgr::instance())
{
    //
    ary_udpsvc_handler_.push_back(this);
}

//这就要求UDP_Svc_Handler比InstOfZBufferStorage先释放
UDP_Svc_Handler::~UDP_Svc_Handler()
{
    if (dgram_databuf_ != NULL)
    {
        ZBuffer_Storage::instance()->free_byte_buffer(dgram_databuf_);
    }
}

//
/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年11月17日
Function        : UDP_Svc_Handler::init_udp_services
Return          : int
Parameter List  : NULL
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int UDP_Svc_Handler::init_udp_services()
{

    dgram_databuf_ = ZBuffer_Storage::instance()->allocate_buffer();

    int ret = 0;
    ret = dgram_peer_.open(&udp_bind_addr_);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] fail.",
                udp_bind_addr_.get_host_addr(),
                udp_bind_addr_.get_port_number());
        handle_close();
        return SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
    }

    ////调整Socket 为ACE_NONBLOCK
    ret = dgram_peer_.sock_enable(O_NONBLOCK);

    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    dgram_peer_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    dgram_peer_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);



    ret = reactor()->register_handler(this, ZCE_Event_Handler::READ_MASK);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] fail.",
                udp_bind_addr_.get_host_addr(),
                udp_bind_addr_.get_port_number());
        handle_close();
        return SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
    }

    ZCE_LOG(RS_INFO, "[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] success.",
            udp_bind_addr_.get_host_addr(),
            udp_bind_addr_.get_port_number());
    return 0;
}

//取得句柄
ZCE_HANDLE UDP_Svc_Handler::get_handle(void) const
{
    return (ZCE_HANDLE)dgram_peer_.get_handle();
}



int UDP_Svc_Handler::handle_input()
{

    int ret = 0;

    //多次读取UDP的数据，保证UDP的响应也比较及时。
    for (size_t i = 0; i < ONCE_MAX_READ_UDP_NUMBER; ++i)
    {
        size_t szrevc = 0;

        //读取数据
        ret = read_data_from_udp(szrevc);

		ZCE_LOGMSG_DEBUG(RS_DEBUG, "UPD Handle input event triggered. ret:%d,szrecv:%u.", ret, szrevc);

        if (ret != 0)
        {
			ZCE_LOGMSG_DEBUG(RS_ERROR, "UPD Handle input event triggered error. ret:%d,szrecv:%u,ZCE_LIB::last_error()=%d|%s",
							 ret,
							 szrevc,
							 ZCE_LIB::last_error(),
							 strerror(ZCE_LIB::last_error()));
            //return -1吗，但是我真不知道如何处理
            break;
        }

        //如果出来成功
        if (szrevc > 0)
        {
            zerg_comm_mgr_->pushback_recvpipe(reinterpret_cast<Zerg_App_Frame *>(dgram_databuf_->buffer_data_));
        }
    }

    return 0;
}


//
int UDP_Svc_Handler::handle_close()
{

    //
    if (dgram_peer_.get_handle() != ZCE_INVALID_SOCKET)
    {
        //内部会进行remove_handler
        ZCE_Event_Handler::handle_close();
        dgram_peer_.close();
    }

    //删除自己
    delete this;

    return 0;
}

//
int UDP_Svc_Handler::read_data_from_udp(size_t &size_revc)
{
    int ret = 0;
    size_revc = 0;
    ssize_t recvret = 0;


    ZCE_Sockaddr_In     remote_addr;

    Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *> (dgram_databuf_->buffer_data_);

    recvret = dgram_peer_.recvfrom(dgram_databuf_->buffer_data_,
                                   Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA,
                                   0,
                                   &remote_addr);

    if (recvret < 0)
    {
        if (ZCE_LIB::last_error() != EWOULDBLOCK)
        {

            //遇到中断,等待重入
            if (ZCE_LIB::last_error() == EINVAL)
            {
                return 0;
            }

            //记录错误,返回错误
            ZCE_LOG(RS_ERROR, "[zergsvr] UDP receive data error IP[%s|%u] peer:%u ZCE_LIB::last_error()=%d|%s.",
                    remote_addr.get_host_addr(),
                    remote_addr.get_port_number(),
                    dgram_peer_.get_handle(),
                    ZCE_LIB::last_error(),
                    strerror(ZCE_LIB::last_error()));
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }
        else
        {
            return 0;
        }
    }

    ret = ip_restrict_->check_iprestrict(remote_addr);

    if (ret != 0)
    {
        return ret;
    }

    //Socket被关闭，也返回错误标示,但是我不知道会不会出现这个问题...
    if (recvret == 0)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] UDP Peer IP [%s|%u] recv return 0, I don't know how to process.?",
                remote_addr.get_host_addr(),
                remote_addr.get_port_number());
        return 0;
    }

    //这个函数放在这儿好不好，hoho，有点耗时喔，呵呵
    server_status_->increase_by_statid(ZERG_UDP_RECV_COUNTER, 0, 0, 1);
    server_status_->increase_by_statid(ZERG_UDP_RECV_BYTES_COUNTER, 0, 0 , recvret);


    proc_frame->framehead_decode();

    //如果收到的是APPFRAME的数据，检查对方帧是否填写正确
    if (proc_frame->frame_length_ != static_cast<unsigned int>(recvret))
    {
        return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
    }

    proc_frame->send_ip_address_ = remote_addr.get_ip_address();
    proc_frame->send_service_.services_type_ = remote_addr.get_port_number();
    proc_frame->send_service_.services_id_ = remote_addr.get_ip_address();

    //
    if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == true)
    {
        proc_frame->recv_service_ = my_svc_info_;
    }
    else
    {
        proc_frame->recv_service_ = my_svc_info_;
    }

    //避免发生其他人填写的情况
    proc_frame->clear_inner_option();
    proc_frame->frame_option_ |= Zerg_App_Frame::DESC_UDP_FRAME;

    size_revc = recvret;

    ZCE_LOG(RS_DEBUG, "[zergsvr] UDP recviese data success. peer IP [%s|%u] handle:%u .recv len :%u.",
            remote_addr.get_host_addr(),
            remote_addr.get_port_number(),
            dgram_peer_.get_handle(),
            size_revc);

    return 0;
}

//
int UDP_Svc_Handler::write_data_to_udp(Zerg_App_Frame *send_frame)
{
    ssize_t szsend = 0;

    //这里service_id_和services_type_保存的是对方的ip和port，而不是真正的type和id
    ZCE_Sockaddr_In remote_addr(send_frame->recv_service_.services_id_,
                                send_frame->recv_service_.services_type_);
    size_t send_len = send_frame->frame_length_;

    //
    send_frame->framehead_encode();


    //发送数据应该不阻塞
    szsend = dgram_peer_.sendto(send_frame,
                                send_len,
                                0,
                                &remote_addr);

    //
    if (szsend <= 0)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] UDP send data error. peer IP [%s|%u] handle:%u ZCE_LIB::last_error()=%d|%s.",
                remote_addr.get_host_addr(),
                remote_addr.get_port_number(),
                dgram_peer_.get_handle(),
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()));
        return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
    }

    ZCE_LOG(RS_DEBUG, "[zergsvr] UDP send data success. peer IP [%s|%u] handle:%u send len :%u.",
            remote_addr.get_host_addr(),
            remote_addr.get_port_number(),
            dgram_peer_.get_handle(),
            send_len);
    //
    server_status_->increase_by_statid(ZERG_UDP_SEND_COUNTER, 0, 0, 1);
    server_status_->increase_by_statid(ZERG_UDP_SEND_BYTES_COUNTER, 0, 0, szsend);

    return 0;
}






int UDP_Svc_Handler::send_all_to_udp(Zerg_App_Frame *send_frame)
{
    //找到原来的那个UDP端口，使用原来的端口发送，
    //这样可以保证防火墙的穿透问题
    for (size_t i = 0; i < ary_udpsvc_handler_.size(); ++i)
    {
        //
        if (ary_udpsvc_handler_[i]->my_svc_info_ ==
            send_frame->send_service_)
        {
            return ary_udpsvc_handler_[i]->write_data_to_udp(send_frame);
        }
    }

    //
    ZCE_LOG(RS_ERROR, "[zergsvr] Error UDP Send Svc Info %u|%u.",
            send_frame->send_service_.services_type_,
            send_frame->send_service_.services_id_
           );
    return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
}


//初始化静态参数
int UDP_Svc_Handler::init_all_static_data()
{

    //服务器的统计操作实例
    server_status_ = Soar_Stat_Monitor::instance();

    //通信管理器
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();

    return 0;
}

int UDP_Svc_Handler::get_config(const Zerg_Server_Config *config)
{
    if_proxy_ = config->zerg_cfg_data_.is_proxy_;
    return 0;
}



