#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_predefine.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_base.h"
#include "zce_socket_stream.h"
#include "zce_socket_acceptor.h"

/************************************************************************************************************
Class           : ZCE_Socket_Acceptor
************************************************************************************************************/

//���캯��
ZCE_Socket_Acceptor::ZCE_Socket_Acceptor():
    ZCE_Socket_Base()
{
}

ZCE_Socket_Acceptor::~ZCE_Socket_Acceptor()
{
    //Ϊʲô�����˲��ر�socket_handle_,�ǿ��ǵ���һҪ���и�����
    close();
}

//������ַ�����ȣ���һ��Accepet�Ķ˿� (Bind,���Ҽ���),
//��һ��������ַ��Ŀǰֻ֧��AF_INET,��AFINET6
int ZCE_Socket_Acceptor::open(const ZCE_Sockaddr *local_addr,
                              bool reuse_addr,
                              int protocol_family,
                              int backlog,
                              int protocol)
{

    int ret = 0;

    //���û�б�עЭ�����ͣ��õ�ַ
    if (protocol_family == AF_UNSPEC)
    {
        protocol_family = local_addr->sockaddr_ptr_->sa_family;
    }

    //
    ret = ZCE_Socket_Base::open(SOCK_STREAM,
                                protocol_family,
                                protocol,
                                reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    ret = ZCE_Socket_Base::bind(local_addr);

    if (ret != 0)
    {
        zce::closesocket(socket_handle_);
        return ret;
    }

    //��������
    ret = zce::listen (socket_handle_,
                       backlog);

    if (ret != 0)
    {
        zce::closesocket(socket_handle_);
        return ret;
    }

    return 0;
}

//�ǳ�ʱ�����accept,NONBLOCKģʽ�»�Ѹ���˳�������ģʽ�»�һ�µȴ�
int ZCE_Socket_Acceptor::accept (ZCE_Socket_Stream &new_stream,
                                 ZCE_Sockaddr *remote_addr) const
{
    ZCE_SOCKET sock_handle = zce::accept(socket_handle_,
                                         remote_addr->sockaddr_ptr_,
                                         &remote_addr->sockaddr_size_);

    if (sock_handle == ZCE_INVALID_SOCKET)
    {
        return -1;
    }

    new_stream.set_handle(sock_handle);
    return 0;
}

//
int ZCE_Socket_Acceptor::accept (ZCE_Socket_Stream &new_stream,
                                 ZCE_Time_Value &timeout,
                                 ZCE_Sockaddr *remote_addr) const
{
    int ret = 0;
    ret = zce::handle_ready(socket_handle_,
                            &timeout,
                            zce::HANDLE_READY_ACCEPT);

    const int HANDLE_READY_ONE = 1;

    if (ret != HANDLE_READY_ONE)
    {
        return -1;
    }

    //
    ZCE_SOCKET sock_handle = zce::accept(socket_handle_,
                                         remote_addr->sockaddr_ptr_,
                                         &remote_addr->sockaddr_size_);

    if (sock_handle == ZCE_INVALID_SOCKET)
    {
        return -1;
    }

    new_stream.set_handle(sock_handle);
    return 0;
}

