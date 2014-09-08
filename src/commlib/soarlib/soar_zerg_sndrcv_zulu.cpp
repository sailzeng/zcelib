#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_svc_info.h"
#include "soar_frame_command.h"
#include "soar_zerg_frame.h"
#include "soar_zerg_sndrcv_zulu.h"

/******************************************************************************************
class Zulu_SendRecv_Package
******************************************************************************************/
Zulu_SendRecv_Package::Zulu_SendRecv_Package():
    zulu_connected_(false),
    zulu_stream_()
{
    zulu_svc_ip_.set("127.0.0.1", 8080);
}

Zulu_SendRecv_Package::~Zulu_SendRecv_Package()
{
    if ( true == zulu_connected_ )
    {
        zulu_stream_.close();
    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年4月25日
Function        : Zulu_SendRecv_Package::set_zulu_svcinfo
Return          : int
Parameter List  :
  Param1: const char* svc_ip
  Param2: unsigned short svc_port
  Param3: const SERVICES_ID& recv_service
  Param4: const SERVICES_ID& send_service
  Param5: const SERVICES_ID& proxy_service
  Param6: size_t frame_len
Description     : //设置相应的SVC INFO,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Zulu_SendRecv_Package::set_zulu_svcinfo(const char *svc_ip,
                                            unsigned short svc_port,
                                            const SERVICES_ID &recv_service,
                                            const SERVICES_ID &send_service,
                                            const SERVICES_ID &proxy_service,
                                            size_t frame_len)
{
    int ret = 0;

    set_services_id(recv_service, send_service, proxy_service, frame_len);

    zulu_svc_ip_.set(svc_ip, svc_port);

    if ( ret != 0)
    {
        return SOAR_RET::ERROR_ZULU_INPUT_IPADDRESS_ERROR;
    }

    return 0;
}

//链接服务器
int Zulu_SendRecv_Package::connect_zulu_server(ZCE_Time_Value *time_wait)
{
    int ret = 0;

    //GCS服务器的链接问题。
    if ( false == zulu_connected_ )
    {
        ZCE_Socket_Connector tmp_connector;
        //自己定义超时时间10s,如果不能链接认为失败，毕竟都是内网,
        // 默认超时是10s, 如果指定了time_wait则使用time_wait
        // 否则用默认的值
        ZCE_Time_Value real_time_wait(10, 0);

        if (time_wait != NULL)
        {
            real_time_wait = *time_wait;
        }

        //连接配置中心
        ret = tmp_connector.connect(zulu_stream_,
                                    &zulu_svc_ip_,
                                    real_time_wait);

        if (ret != 0 )
        {
            return SOAR_RET::ERROR_ZULU_CONNECT_SERVER_FAIL;
        }
    }

    zulu_connected_ = true;

    return 0;
}

void Zulu_SendRecv_Package::close()
{
    if (zulu_connected_)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
    }
}

//接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
int Zulu_SendRecv_Package::receive_svc_package(ZCE_Time_Value *time_wait)
{

    ssize_t socket_ret  = 0;
    int data_len = 0;

    //
    for (;;)
    {
        //接收一个帧头,长度必然是LEN_OF_APPFRAME_HEAD,
        socket_ret  = zulu_stream_.recv_n((void *)(tibetan_recv_appframe_),
                                          Zerg_App_Frame::LEN_OF_APPFRAME_HEAD,
                                          time_wait);

        //ret == 0的情况下一般是链接被断开
        if (socket_ret == 0 )
        {
            ZLOG_INFO("[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                      socket_ret,
                      ZCE_LIB::last_error(),
                      strerror(ZCE_LIB::last_error()));
            return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
        }
        //小于0一般表示错误,对于阻塞，这儿也表示一个错误,我给了你等待时间作为选择
        else if (socket_ret < 0)
        {
            //如果错误是信号导致的重入
            int last_error =  ZCE_LIB::last_error();

            ZLOG_ERROR("[framework] RECV Zerg_App_Frame head error or time out. Ret:%d, error[%u|%s].",
                       socket_ret,
                       last_error,
                       strerror(last_error));

            //阻塞的代码，可能有这个问题
            if ( EINTR == last_error )
            {
                continue;
            }

            return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
        }

        tibetan_recv_appframe_->framehead_decode();

#if defined _DEBUG || defined DEBUG
        ZCE_ASSERT(test_frame_len_ > tibetan_recv_appframe_->frame_length_);
#endif //#if defined _DEBUG || defined DEBUG

        //保存接收到的事务ID
        recv_trans_id_ = tibetan_recv_appframe_->transaction_id_;
        backfill_trans_id_ = tibetan_recv_appframe_->backfill_trans_id_;

        //数据包的长度
        data_len = tibetan_recv_appframe_->frame_length_ - Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;

        if (data_len < 0)
        {
            ZLOG_ERROR("[framework] Receive Zerg_App_Frame head len error ,frame len:%d,error[%u|%s].",
                       tibetan_recv_appframe_->frame_length_,
                       ZCE_LIB::last_error(),
                       strerror(ZCE_LIB::last_error()) );
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
            socket_ret  = zulu_stream_.recv_n((void *)(&(tibetan_recv_appframe_->frame_appdata_)),
                                              data_len,
                                              time_wait);

            //对于阻塞，这儿也表示一个错误
            if (socket_ret < 0)
            {
                //如果错误是信号导致的重入
                int last_error = ZCE_LIB::last_error();
                ZLOG_ERROR("[framework] RECV Zerg_App_Frame body data error. Ret:%d, error[%u|%s].",
                           socket_ret,
                           ZCE_LIB::last_error(),
                           ::strerror(ZCE_LIB::last_error()));

                if ( EINTR == last_error )
                {
                    continue;
                }

                return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;

            }
            // ret == 0
            else if (socket_ret == 0 )
            {
                ZLOG_INFO("[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                          socket_ret,
                          ZCE_LIB::last_error(),
                          strerror(ZCE_LIB::last_error()));
                return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
            }

            //
            if (socket_ret != data_len)
            {
                ZLOG_ERROR("[framework] Receive Zerg_App_Frame body data error or time out ,ret:%d,error[%u|%s].",
                           socket_ret,
                           ZCE_LIB::last_error(),
                           strerror(ZCE_LIB::last_error()) );
                return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
            }

            //跳出循环
            break;
        }

        //跳出循环
        break;
    }

    ZLOG_DEBUG("[framework] Recv cmd [%u] bytes [%u] Frame From Svr Succ. ",
               tibetan_recv_appframe_->frame_command_,
               tibetan_recv_appframe_->frame_length_);

    //如果是动态分配的tibetan_send_service_，收到后重新填写一下，小虫会分配一个
    if ( SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID == tibetan_send_service_.services_id_ )
    {
        tibetan_send_service_.services_id_ = tibetan_recv_appframe_->recv_service_.services_id_;
    }

    //原来出现过不是自己的数据返回回来的事情，加个日志输出
    if (tibetan_send_service_ != tibetan_recv_appframe_->recv_service_)
    {
        ZLOG_ERROR("[framework] zulu recv a error or unexpect frame,cmd %u. snd svc id [%u|%u] recv svc id[%u|%u].",
                   tibetan_recv_appframe_->frame_command_,
                   tibetan_send_service_.services_type_,
                   tibetan_send_service_.services_id_,
                   tibetan_recv_appframe_->recv_service_.services_type_,
                   tibetan_recv_appframe_->recv_service_.services_id_
                  );
        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    return 0;
}

//发送tibetan_send_appframe_出去，
int Zulu_SendRecv_Package::send_svc_package(ZCE_Time_Value *time_wait)
{
    tibetan_send_appframe_->send_service_ = tibetan_send_service_;
    tibetan_send_appframe_->recv_service_ = tibetan_recv_service_;
    tibetan_send_appframe_->proxy_service_ = tibetan_proxy_service_;

    //保证事务ID不是0
    ++trans_id_builder_;

    if (++trans_id_builder_ == 0)
    {
        ++trans_id_builder_;
    }

    tibetan_send_appframe_->transaction_id_ = trans_id_builder_;

    int len = tibetan_send_appframe_->frame_length_;
    unsigned int cmd = tibetan_send_appframe_->frame_command_;

    //阻塞发送所有的数据
    tibetan_send_appframe_->framehead_encode();

    //
    ssize_t socket_ret = zulu_stream_.send_n((void *)(tibetan_send_appframe_),
                                             len,
                                             time_wait);

    if (socket_ret <= 0 )
    {
        ZLOG_ERROR("[framework] SEND cmd [%u] bytes[%u] frame To Svr fail ret =%d. ",
                   cmd,
                   len,
                   socket_ret);
        return SOAR_RET::ERROR_ZULU_SEND_PACKAGE_FAIL;
    }

    return 0;
}

//接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
int Zulu_SendRecv_Package::receive_svc_package(unsigned int &recv_cmd,
                                               ZCE_Time_Value *time_out)
{
    int ret =  0;

    ret = receive_svc_package(time_out);

    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    //接收的命令
    recv_cmd = tibetan_recv_appframe_->frame_command_;

    return 0;
}

//取得本地的地址信息
int Zulu_SendRecv_Package::getsockname (ZCE_Sockaddr *addr)  const
{
    return zulu_stream_.getsockname(addr);
}

