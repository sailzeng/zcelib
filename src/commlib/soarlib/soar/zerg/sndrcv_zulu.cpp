#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/services_info.h"
#include "soar/zerg/frame_command.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/zerg/sndrcv_zulu.h"

/******************************************************************************************
class Zulu_SendRecv_Package
******************************************************************************************/
Zulu_SendRecv_Msg::Zulu_SendRecv_Msg() :
    zulu_connected_(false),
    zulu_stream_()
{
    zulu_svc_ip_.set("127.0.0.1", 8080);
}

Zulu_SendRecv_Msg::~Zulu_SendRecv_Msg()
{
    if (true == zulu_connected_)
    {
        zulu_stream_.close();
    }
}

int Zulu_SendRecv_Msg::set_zulu_svcinfo(const char* svc_ip,
                                        unsigned short svc_port,
                                        const soar::SERVICES_ID& recv_service,
                                        const soar::SERVICES_ID& send_service,
                                        const soar::SERVICES_ID& proxy_service,
                                        size_t frame_len)
{
    int ret = 0;

    set_services_id(recv_service, send_service, proxy_service, frame_len);

    zulu_svc_ip_.set(svc_ip, svc_port);

    if (ret != 0)
    {
        return SOAR_RET::ERROR_ZULU_INPUT_IPADDRESS_ERROR;
    }

    return 0;
}

//链接服务器
int Zulu_SendRecv_Msg::connect_zulu_server(zce::time_value* time_wait)
{
    int ret = 0;

    //GCS服务器的链接问题。
    if (false == zulu_connected_)
    {
        zce::skt::connector tmp_connector;
        //自己定义超时时间10s,如果不能链接认为失败，毕竟都是内网,
        // 默认超时是10s, 如果指定了time_wait则使用time_wait
        // 否则用默认的值
        zce::time_value real_time_wait(10, 0);

        if (time_wait != NULL)
        {
            real_time_wait = *time_wait;
        }

        //连接配置中心
        ret = tmp_connector.connect(zulu_stream_,
                                    &zulu_svc_ip_,
                                    real_time_wait);

        if (ret != 0)
        {
            return SOAR_RET::ERROR_ZULU_CONNECT_SERVER_FAIL;
        }
    }

    zulu_connected_ = true;

    return 0;
}

void Zulu_SendRecv_Msg::close()
{
    if (zulu_connected_)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
    }
}

//接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
int Zulu_SendRecv_Msg::receive_svc_msg(zce::time_value* time_wait)
{
    ssize_t socket_ret = 0;
    int data_len = 0;

    //
    for (;;)
    {
        //接收一个帧头,长度必然是LEN_OF_HEAD,
        socket_ret = zulu_stream_.recv_n((void*)(msg_recv_frame_),
                                         soar::Zerg_Frame::LEN_OF_HEAD,
                                         time_wait);

        //ret == 0的情况下一般是链接被断开
        if (socket_ret == 0)
        {
            ZCE_LOG(RS_INFO, "[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                    socket_ret,
                    zce::last_error(),
                    strerror(zce::last_error()));
            return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
        }
        //小于0一般表示错误,对于阻塞，这儿也表示一个错误,我给了你等待时间作为选择
        else if (socket_ret < 0)
        {
            //如果错误是信号导致的重入
            int last_error = zce::last_error();

            ZCE_LOG(RS_ERROR, "[framework] RECV soar::Zerg_Frame head error or "
                    "time out. Ret:%d, error[%u|%s].",
                    socket_ret,
                    last_error,
                    strerror(last_error));

            //阻塞的代码，可能有这个问题
            if (EINTR == last_error)
            {
                continue;
            }

            return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
        }

        msg_recv_frame_->ntoh();

#if defined _DEBUG || defined DEBUG
        ZCE_ASSERT(test_frame_len_ > msg_recv_frame_->length_);
#endif //#if defined _DEBUG || defined DEBUG

        //保存接收到的事务ID
        recv_trans_id_ = msg_recv_frame_->fsm_id_;
        backfill_trans_id_ = msg_recv_frame_->backfill_fsm_id_;

        //数据包的长度
        data_len = msg_recv_frame_->length_ - soar::Zerg_Frame::LEN_OF_HEAD;

        if (data_len < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Receive soar::Zerg_Frame head len error ,frame len:%d,error[%u|%s].",
                    msg_recv_frame_->length_,
                    zce::last_error(),
                    strerror(zce::last_error()));
            return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
        }

        //只处理包头信息
        if (data_len == 0)
        {
            break;
        }

        //为了continue，这个代码是阻塞的
        for (;;)
        {
            //
            socket_ret = zulu_stream_.recv_n((void*)(&(msg_recv_frame_->frame_appdata_)),
                                             data_len,
                                             time_wait);

            //对于阻塞，这儿也表示一个错误
            if (socket_ret < 0)
            {
                //如果错误是信号导致的重入
                int last_error = zce::last_error();
                ZCE_LOG(RS_ERROR, "[framework] RECV soar::Zerg_Frame body data error. Ret:%d, error[%u|%s].",
                        socket_ret,
                        zce::last_error(),
                        ::strerror(zce::last_error()));

                if (EINTR == last_error)
                {
                    continue;
                }

                return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
            }
            // ret == 0
            else if (socket_ret == 0)
            {
                ZCE_LOG(RS_INFO, "[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                        socket_ret,
                        zce::last_error(),
                        strerror(zce::last_error()));
                return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
            }

            //
            if (socket_ret != data_len)
            {
                ZCE_LOG(RS_ERROR, "[framework] Receive soar::Zerg_Frame body data error or time out ,ret:%d,error[%u|%s].",
                        socket_ret,
                        zce::last_error(),
                        strerror(zce::last_error()));
                return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
            }

            //跳出循环
            break;
        }

        //跳出循环
        break;
    }

    ZCE_LOG(RS_DEBUG, "[framework] Recv cmd [%u] bytes [%u] Frame From Svr Succ. ",
            msg_recv_frame_->command_,
            msg_recv_frame_->length_);

    //如果是动态分配的tibetan_send_service_，收到后重新填写一下，小虫会分配一个
    if (soar::SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID == msg_send_service_.services_id_)
    {
        msg_send_service_.services_id_ = msg_recv_frame_->recv_service_.services_id_;
    }

    //原来出现过不是自己的数据返回回来的事情，加个日志输出
    if (msg_send_service_ != msg_recv_frame_->recv_service_)
    {
        ZCE_LOG(RS_ERROR, "[framework] zulu recv a error or unexpect frame,cmd %u. snd svc id [%u|%u] recv svc id[%u|%u].",
                msg_recv_frame_->command_,
                msg_send_service_.services_type_,
                msg_send_service_.services_id_,
                msg_recv_frame_->recv_service_.services_type_,
                msg_recv_frame_->recv_service_.services_id_
        );
        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    return 0;
}

//发送tibetan_send_appframe_出去，
int Zulu_SendRecv_Msg::send_svc_msg(zce::time_value* time_wait)
{
    msg_send_frame_->send_service_ = msg_send_service_;
    msg_send_frame_->recv_service_ = msg_recv_service_;
    msg_send_frame_->proxy_service_ = msg_proxy_service_;

    //保证事务ID不是0
    ++trans_id_builder_;

    if (++trans_id_builder_ == 0)
    {
        ++trans_id_builder_;
    }

    msg_send_frame_->fsm_id_ = trans_id_builder_;

    int len = msg_send_frame_->length_;
    uint32_t cmd = msg_send_frame_->command_;

    //阻塞发送所有的数据
    msg_send_frame_->hton();

    //
    ssize_t socket_ret = zulu_stream_.send_n((void*)(msg_send_frame_),
                                             len,
                                             time_wait);

    if (socket_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] SEND cmd [%u] bytes[%u] frame To Svr fail ret =%d. ",
                cmd,
                len,
                socket_ret);
        return SOAR_RET::ERROR_ZULU_SEND_PACKAGE_FAIL;
    }

    return 0;
}

//接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
int Zulu_SendRecv_Msg::receive_svc_msg(unsigned int& recv_cmd,
                                       zce::time_value* time_out)
{
    int ret = 0;

    ret = receive_svc_msg(time_out);

    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    //接收的命令
    recv_cmd = msg_recv_frame_->command_;

    return 0;
}

//取得本地的地址信息
int Zulu_SendRecv_Msg::getsockname(zce::skt::addr_base* addr)  const
{
    return zulu_stream_.getsockname(addr);
}