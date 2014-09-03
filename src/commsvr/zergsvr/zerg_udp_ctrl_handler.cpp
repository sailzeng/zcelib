
#include "zerg_predefine.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_buf_storage.h"
#include "zerg_application.h"
#include "zerg_ip_restrict.h"
#include "zerg_comm_manager.h"
#include "zerg_stat_define.h"
#include "zerg_application.h"

//
UDP_Svc_Handler::ARY_OF_UDPSVC_HANDLER UDP_Svc_Handler::ary_udpsvc_handler_;

//
Comm_Stat_Monitor  *UDP_Svc_Handler::server_status_ = NULL;
//通信管理器
Zerg_Comm_Manager *UDP_Svc_Handler::zerg_comm_mgr_  = NULL;

//自己是否是代理
bool           UDP_Svc_Handler::if_proxy_ = false;

// game_id
unsigned int UDP_Svc_Handler::game_id_ = 0;

// 监听handler的初始化
UDP_Svc_Handler::UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                                 const ZCE_Sockaddr_In &addr,
                                 bool sessionkey_verify,
                                 bool is_external_pkg,
                                 uint32_t cmd_offset,
                                 uint32_t cmd_len):
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    self_udp_addr_(addr),
    peer_udp_addr_(ZCE_Sockaddr_In()),
    self_svc_info_(my_svcinfo),
    peer_svc_info_(SERVICES_ID(0, 0)),
    sessionkey_verify_(sessionkey_verify),
    is_external_pkg_(is_external_pkg),
    cmd_offset_(cmd_offset),
    cmd_len_(cmd_len),
    dgram_databuf_(NULL),
    ip_restrict_(Zerg_IPRestrict_Mgr::instance()),
    is_initiative_handler_(false)
{
    ary_udpsvc_handler_.push_back(this);
}

//主动发送handler的初始化
UDP_Svc_Handler::UDP_Svc_Handler(const SERVICES_ID &my_svcinfo,
                                 const SERVICES_ID &peer_svcinfo,
                                 const ZCE_Sockaddr_In &peer_addr,
                                 bool is_external_pkg,
                                 uint32_t cmd_offset,
                                 uint32_t cmd_len):
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    self_udp_addr_(ZCE_Sockaddr_In()),
    peer_udp_addr_(peer_addr),
    self_svc_info_(my_svcinfo),
    peer_svc_info_(peer_svcinfo),
    sessionkey_verify_(false),
    is_external_pkg_(is_external_pkg),
    cmd_offset_(cmd_offset),
    cmd_len_(cmd_len),
    dgram_databuf_(NULL),
    ip_restrict_(Zerg_IPRestrict_Mgr::instance()),
    is_initiative_handler_(true)
{
    // 如果是主动连接的外部包handler，需要配置主动连接的proto_cfg_index对应的external_proto_cfg
    // 如果is_external_pkg是true，可以从cmd_offset_直接获取到cmd
    // 如果is_external_pkg是false，cmd_offset_和cmd_len_没有意义
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

//主动发包的初始化
int UDP_Svc_Handler::init_udpsvr_handler()
{
    dgram_databuf_ = ZBuffer_Storage::instance()->allocate_buffer();

    int ret = 0;
    ret = dgram_peer_.open();

    if (ret != 0)
    {
        ZLOG_ERROR("[zergsvr] init_udpsvr_handler ,create udp socket fail.");
        handle_close();
        return SOAR_RET::FAIL_INTERNAL_ERORR;
    }

    ////调整Socket 为O_NONBLOCK
    ret = dgram_peer_.sock_enable(O_NONBLOCK);

    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    dgram_peer_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    dgram_peer_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);

    ret = reactor()->register_handler(this, ZCE_Event_Handler::READ_MASK);

    if (ret != 0)
    {
        ZLOG_ERROR("[zergsvr] init_udpsvr_handler ,create udp socket fail.");
        handle_close();
        return SOAR_RET::FAIL_INTERNAL_ERORR;
    }

    ZLOG_INFO("[zergsvr] init_udp_services ,create udp socket success.");
    return SOAR_RET::SOAR_RET_SUCC;
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
    ret = dgram_peer_.open(&self_udp_addr_);

    if (ret != 0)
    {
        ZLOG_ERROR("[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] fail.",
                   self_udp_addr_.get_host_addr(),
                   self_udp_addr_.get_port_number());
        handle_close();
        return SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
    }

    ////调整Socket 为O_NONBLOCK
    ret = dgram_peer_.sock_enable(O_NONBLOCK);

    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    dgram_peer_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    dgram_peer_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);

    ret = reactor()->register_handler (this, ZCE_Event_Handler::READ_MASK );

    if (ret != 0)
    {
        ZLOG_ERROR("[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] fail.",
                   self_udp_addr_.get_host_addr(),
                   self_udp_addr_.get_port_number());
        handle_close();
        return SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
    }

    ZLOG_INFO("[zergsvr] init_udp_services ,UDP bind ip address [%s|%u] success.",
              self_udp_addr_.get_host_addr(),
              self_udp_addr_.get_port_number());
    return SOAR_RET::SOAR_RET_SUCC;
}

//取得句柄
ZCE_HANDLE UDP_Svc_Handler::get_handle(void) const
{
    return(ZCE_HANDLE)dgram_peer_.get_handle();
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

        ZCE_LOGMSG_DBG(RS_DEBUG, "UPD Handle input event triggered. ret:%d,szrecv:%u.", ret, szrevc);

        if (ret != SOAR_RET::SOAR_RET_SUCC)
        {
            ZCE_LOGMSG_DBG(RS_ERROR, "UPD Handle input event triggered error. ret:%d,szrecv:%u,ZCE_LIB::last_error()=%d|%s",
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
        else
        {
            // 读到没有数据的时候就退出降低cpu占用
            break;
        }
    }

    return 0;
}

//
int UDP_Svc_Handler::handle_close ()
{

    //
    if (dgram_peer_.get_handle () != ZCE_INVALID_SOCKET)
    {
        //内部会进行remove_handler
        ZCE_Event_Handler::handle_close ();
        dgram_peer_.close ();
    }

    //删除自己
    delete this;

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年11月17日
Function        : UDP_Svc_Handler::read_data_from_udp
Return          : int
Parameter List  :
  Param1: size_t& szrevc
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int UDP_Svc_Handler::read_data_from_udp(size_t &size_revc)
{
    int ret = 0;
    size_revc = 0;
    ssize_t recvret = 0;

    ZCE_Sockaddr_In     remote_addr;

    Zerg_App_Frame *proc_frame;

    recvret = dgram_peer_.recvfrom(dgram_databuf_->buffer_data_ + Zerg_App_Frame::LEN_OF_APPFRAME_HEAD, // 后移动50个字节, 方便接收非框架包
                                   Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA - Zerg_App_Frame::LEN_OF_APPFRAME_HEAD,
                                   0,
                                   &remote_addr);

    if (recvret < 0)
    {
        if (ZCE_LIB::last_error() != EWOULDBLOCK )
        {

            //遇到中断,等待重入
            if (ZCE_LIB::last_error() == EINVAL)
            {
                return SOAR_RET::SOAR_RET_SUCC;
            }

            //记录错误,返回错误
            ZLOG_ERROR("[zergsvr] UDP receive data error IP[%s|%u] peer:%u ZCE_LIB::last_error()=%d|%s.",
                       remote_addr.get_host_addr(),
                       remote_addr.get_port_number(),
                       dgram_peer_.get_handle(),
                       ZCE_LIB::last_error(),
                       strerror(ZCE_LIB::last_error()));
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }
        else
        {
            return SOAR_RET::SOAR_RET_SUCC;
        }
    }

    ret = ip_restrict_->check_iprestrict(remote_addr);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    //Socket被关闭，也返回错误标示,但是我不知道会不会出现这个问题...
    if (recvret == 0)
    {
        ZLOG_ERROR("[zergsvr] UDP Peer IP [%s|%u] recv return 0, I don't know how to process.?",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number());
        return SOAR_RET::SOAR_RET_SUCC;
    }

    //这个函数放在这儿好不好，hoho，有点耗时喔，呵呵
    server_status_->increase_by_statid(ZERG_UDP_RECV_COUNTER, game_id_, 1);
    server_status_->increase_by_statid(ZERG_UDP_RECV_BYTES_COUNTER, game_id_, recvret);

    if (is_external_pkg_)
    {
        // 非框架包需要先转成框架包, 越过的50个字节为包头
        proc_frame = reinterpret_cast<Zerg_App_Frame *> (dgram_databuf_->buffer_data_);
        uint32_t tmpframe_length = recvret + Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;
        uint32_t tmp_cmd = 0;

        if ((cmd_offset_>>31) == 1)
        {
            // 如果最高位为1则表明是固定的命令字
            tmp_cmd = cmd_offset_ ^ 0x80000000;
        }
        else
        {
            int ret = get_external_pkg_cmd(tmp_cmd, (uint8_t*)proc_frame->frame_appdata_, recvret);
            if (ret != SOAR_RET::SOAR_RET_SUCC)
            {
                ZLOG_ERROR("[%s]get external udp pkg cmd failed, ret=%d, recvlen=%d, cmdoffset=%u, cmdlen=%u",
                    __ZCE_FUNCTION__,
                    ret,
                    recvret,
                    cmd_offset_,
                    cmd_len_);
                return ret;
            }
        }

        proc_frame->init_framehead(tmpframe_length, Zerg_App_Frame::DESC_UDP_FRAME, tmp_cmd);
    }
    else
    {
        // 收包处往后调整了50个字节
        proc_frame = reinterpret_cast<Zerg_App_Frame *> (dgram_databuf_->buffer_data_ + Zerg_App_Frame::LEN_OF_APPFRAME_HEAD);
        proc_frame->framehead_decode();

        //如果收到的是APPFRAME的数据，检查对方帧是否填写正确
        if (proc_frame->frame_length_ != static_cast<unsigned int>(recvret) )
        {
            return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
        }
    }

    proc_frame->send_ip_address_ = remote_addr.get_ip_address();

    if (is_initiative_handler_)
    {
        // 主动连接的方式
        // send_service_是对方的type和id
        // recv_service_是自己的type和id
        proc_frame->send_service_ = peer_svc_info_;
        proc_frame->recv_service_ = self_svc_info_;
    }
    else
    {
        // 监听的方式
        // send_service_保存对方的ip和port
        // recv_service_是自己的type和id
        proc_frame->send_service_.services_type_ = remote_addr.get_port_number();
        proc_frame->send_service_.services_id_ = remote_addr.get_ip_address();

        //
        if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == true )
        {
            proc_frame->recv_service_  = self_svc_info_;
        }
        else
        {
            proc_frame->recv_service_ = self_svc_info_;
        }
    }

    //避免发生其他人填写的情况
    proc_frame->clear_inner_option();
    proc_frame->frame_option_ |= Zerg_App_Frame::DESC_UDP_FRAME;

    size_revc = recvret;

    // 考虑到性能问题，这里的日志不使用ZLOG_DEBUG，而使用zce_LOGMSG_DBG-RS_DEBUG
    ZCE_LOGMSG_DBG(RS_DEBUG, "[zergsvr] UDP recviese data success. peer IP [%s|%u] handle:%u .recv len :%u.",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number(),
                   dgram_peer_.get_handle(),
                   size_revc);

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年11月17日
Function        : UDP_Svc_Handler::write_data_to_udp
Return          : int
Parameter List  :
  Param1: Zerg_App_Frame* send_frame
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int UDP_Svc_Handler::write_data_to_udp(Zerg_App_Frame *send_frame)
{
    ssize_t szsend = 0;

    ZCE_Sockaddr_In remote_addr;
    if (is_initiative_handler_)
    {
        // 主动连接的方式
        remote_addr = peer_udp_addr_;
    }
    else
    {
        // 监听的方式
        //这里send_frame的recv_service_保存的是对方的ip和port，而不是真正的type和id
        remote_addr.set(send_frame->recv_service_.services_id_,
                        send_frame->recv_service_.services_type_);
    }

    size_t send_len = send_frame->frame_length_;

    //
    send_frame->framehead_encode();

    //发送数据应该不阻塞
    if (is_external_pkg_)
    {
        // 如果是发给非框架服务器，是不需要发送包头的
        szsend = dgram_peer_.sendto(send_frame->frame_appdata_,
                                    send_len - Zerg_App_Frame::LEN_OF_APPFRAME_HEAD,
                                    0,
                                    &remote_addr);
    }
    else
    {
        szsend = dgram_peer_.sendto(send_frame,
                                    send_len,
                                    0,
                                    &remote_addr);
    }

    //
    if (szsend <= 0)
    {
        ZLOG_ERROR("[zergsvr] UDP send data error. peer IP [%s|%u] handle:%u ZCE_LIB::last_error()=%d|%s.",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number(),
                   dgram_peer_.get_handle(),
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
    }

    // 考虑到性能问题，这里的日志不使用ZLOG_DEBUG，而使用zce_LOGMSG_DBG-RS_DEBUG
    ZCE_LOGMSG_DBG(RS_DEBUG, "[zergsvr] UDP send data success. peer IP [%s|%u] handle:%u send len :%u.",
                   remote_addr.get_host_addr(),
                   remote_addr.get_port_number(),
                   dgram_peer_.get_handle(),
                   send_len);
    //
    server_status_->increase_by_statid(ZERG_UDP_SEND_COUNTER, game_id_, 1);
    server_status_->increase_by_statid(ZERG_UDP_SEND_BYTES_COUNTER, game_id_, szsend);

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年11月17日
Function        : UDP_Svc_Handler::send_all_to_udp
Return          : int
Parameter List  :
  Param1: Zerg_App_Frame* send_frame
Description     : 发送UDP数据
Calls           :
Called By       :
Other           : 要找到原有的
Modify Record   :
******************************************************************************************/
int UDP_Svc_Handler::send_all_to_udp(Zerg_App_Frame *send_frame)
{
    for (size_t i = 0; i < ary_udpsvc_handler_.size(); ++i)
    {
        if (ary_udpsvc_handler_[i]->is_initiative_handler_)
        {
            // 主动发送
            if (ary_udpsvc_handler_[i]->peer_svc_info_ ==
                send_frame->recv_service_)
            {
                return ary_udpsvc_handler_[i]->write_data_to_udp(send_frame);
            }
        }
        else
        {
            // 监听
            //找到原来的那个UDP端口，使用原来的端口发送，
            //这样可以保证防火墙的穿透问题
            if (ary_udpsvc_handler_[i]->self_svc_info_ ==
                send_frame->send_service_)
            {
                return ary_udpsvc_handler_[i]->write_data_to_udp(send_frame);
            }
        }
    }

    //
    ZLOG_ERROR("[zergsvr] Error UDP Send Svc Info send_service %u|%u, recv service %u|%u",
               send_frame->send_service_.services_type_,
               send_frame->send_service_.services_id_,
               send_frame->recv_service_.services_type_,
               send_frame->recv_service_.services_id_
              );
    return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年11月17日
Function        : UDP_Svc_Handler::init_all_static_data
Return          : int
Parameter List  : NULL
Description     : 初始化静态参数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int UDP_Svc_Handler::init_all_static_data()
{

    //服务器的统计操作实例
    server_status_ = Comm_Stat_Monitor::instance();

    //通信管理器
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();

    return SOAR_RET::SOAR_RET_SUCC;
}

int UDP_Svc_Handler::get_config(const conf_zerg::ZERG_CONFIG *config)
{
    // 是否proxy的配置从命令行读取，不再从配置中读取
    ZCE_UNUSED_ARG(config);
    if_proxy_ = Zerg_Service_App::instance()->if_proxy();
    return SOAR_RET::SOAR_RET_SUCC;
}

int UDP_Svc_Handler::find_services_peer(const SERVICES_ID& svr_info, UDP_Svc_Handler*& svchanle)
{
    for (size_t i = 0; i < ary_udpsvc_handler_.size(); ++i)
    {
        if (ary_udpsvc_handler_[i]->is_initiative_handler_)
        {
            // 查找vector，看是否已经创建了对应svr_info的handler
            if (ary_udpsvc_handler_[i]->peer_svc_info_ == svr_info)
            {
                svchanle = ary_udpsvc_handler_[i];
                return SOAR_RET::SOAR_RET_SUCC;
            }
        }
        else
        {
            // 查找vector，看是否已经创建了对应svr_info的handler
            if (ary_udpsvc_handler_[i]->self_svc_info_ == svr_info)
            {
                svchanle = ary_udpsvc_handler_[i];
                return SOAR_RET::SOAR_RET_SUCC;
            }
        }
    }

    return SOAR_RET::ERR_ZERG_NO_FIND_EVENT_HANDLE;
}

int UDP_Svc_Handler::get_external_pkg_cmd(uint32_t &cmd, const uint8_t *buff,
                                          const ssize_t buf_len)
{
    if ((uint32_t)buf_len < (cmd_offset_ + cmd_len_))
    {
        // cmd字段都没有收全！
        return -1;
    }

    uint32_t tmp = 0;
    const uint8_t *cmd_buf = buff + cmd_offset_;
    for (size_t i = 0; i < cmd_len_; ++ i)
    {
        tmp = (tmp << 8) + cmd_buf[i];
    }

    cmd = tmp;
    return SOAR_RET::SOAR_RET_SUCC;
}




