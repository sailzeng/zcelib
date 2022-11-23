#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/net/sock5_client.h"

namespace zce::socks5
{
const unsigned char SOCKS5_VER = 0x5;
const unsigned char SOCKS5_SUCCESS = 0x0;
const unsigned char SOCKS5_METHODS_NOAUTH = 0x0;
const unsigned char SOCKS5_METHODS_AUTH_GSSAPI = 0x1; //不支持
const unsigned char SOCKS5_METHODS_AUTH_PASSWORD = 0x2;
const unsigned char SOCKS5_CMD_CONNECT = 0x1;
const unsigned char SOCKS5_CMD_BIND = 0x2;
const unsigned char SOCKS5_CMD_UDP = 0x3;
const unsigned char SOCKS5_ATYP_IPV4 = 0x1;
const unsigned char SOCKS5_ATYP_HOSTNAME = 0x3;
const unsigned char SOCKS5_ATYP_IPV6 = 0x4;
const size_t MAX_HOSTNAME_LEN = 255;
//==============================================================================================
//!
int base::hostaddr_to_buf(const char* host_name,
                          const sockaddr* host_addr,
                          uint16_t host_port,
                          socklen_t addr_len,
                          char *buf,
                          size_t &buf_len)
{
    if (buf_len < 259)
    {
        return EINVAL;
    }
    size_t use_len = 0;
    if (host_name)
    {
        size_t host_len = strlen(host_name);
        if (host_len > MAX_HOSTNAME_LEN)
        {
            return EINVAL;
        }
        buf[use_len] = SOCKS5_ATYP_HOSTNAME;
        use_len = +1;
        buf[use_len] = static_cast<unsigned char>(host_len);
        use_len += 1;
        memcpy(buf + use_len, host_name, host_len);
        use_len += host_len;
        uint16_t n_port = htons(host_port);
        memcpy(buf + use_len, &n_port, 2);
        use_len += 2;
    }
    else if (host_addr)
    {
        if (sizeof(sockaddr_in) == addr_len)
        {
            buf[use_len] = SOCKS5_ATYP_IPV4;
            use_len += 1;
            const sockaddr_in* addr_in = reinterpret_cast<const sockaddr_in*>(host_addr);
            memcpy(buf + use_len, &(addr_in->sin_addr), 4);
            use_len += 4;
            memcpy(buf + use_len, &(addr_in->sin_port), 2);
            use_len += 2;
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            buf[use_len] = SOCKS5_ATYP_IPV6;
            use_len += 1;
            const sockaddr_in6* addr_in6 = reinterpret_cast<const sockaddr_in6*>(host_addr);
            memcpy(buf + use_len, &(addr_in6->sin6_addr), 16);
            use_len += 16;
            memcpy(buf + use_len, &(addr_in6->sin6_port), 2);
            use_len += 2;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }
    buf_len = use_len;
    return 0;
}

int base::buf_to_hostaddr(const char *buf,
                          size_t buf_len,
                          char* host_name,
                          uint16_t *host_port,
                          sockaddr* host_addr,
                          socklen_t addr_len)
{
    size_t use_len = 0;
    if (SOCKS5_ATYP_IPV4 == buf[0])
    {
        use_len += 1;
        //牛头不对马嘴，收到的数据长度和期待的不一样
        if (sizeof(sockaddr_in) == addr_len || buf_len < 7)
        {
            return -1;
        }
        sockaddr_in* addr_in = (sockaddr_in*)(host_addr);
        memcpy(&(addr_in->sin_addr), buf + use_len, 4);
        use_len += 4;
        //不转码，两边都要网络序
        memcpy(&(addr_in->sin_port), buf + use_len, 2);
        use_len += 2;
    }
    else if (SOCKS5_ATYP_IPV6 == buf[0])
    {
        use_len += 1;
        if (sizeof(sockaddr_in6) == addr_len || buf_len < 19)
        {
            return -1;
        }
        sockaddr_in6* addr_in6 = (sockaddr_in6*)(host_addr);
        memcpy(&(addr_in6->sin6_addr), buf + use_len, 16);
        use_len += 16;
        memcpy(&(addr_in6->sin6_port), buf + use_len, 2);
        use_len += 2;
    }
    else
    {
        return EINVAL;
    }

    return 0;
}

//==============================================================================================
//SOCKS5支持UDP穿透和TCP代理，比较全面

//socks5代理初始化，连接，认证，
int client::initialize(const sockaddr* socks5_addr,
                       socklen_t addr_len,
                       const char* username,
                       const char* password,
                       zce::time_value& timeout_tv)
{
    int ret = 0;
    int family = 0;
    if (sizeof(sockaddr_in) == addr_len)
    {
        family = AF_INET;
    }
    else if (sizeof(sockaddr_in6) == addr_len)
    {
        family = AF_INET6;
    }
    ret = zce::open_socket(&socks5_hdl_,
                           SOCK_STREAM,
                           family);
    if (ret != 0)
    {
        return -1;
    }
    //socks5_hdl_ 要先connect，可以使用connect_timeout
    ret = zce::connect_timeout(socks5_hdl_,
                               socks5_addr,
                               addr_len,
                               timeout_tv);
    if (ret != 0)
    {
        return -1;
    }
    ssize_t send_len = 0, recv_len = 0;
    ssize_t snd_ret = 0;
    unsigned char *buffer = cmd_buffer_.get();
    buffer[0] = SOCKS5_VER;
    //支持不验证和验证两种方式
    buffer[1] = 2;
    buffer[2] = 0x0; //不需要验证
    buffer[3] = 0x2; //需要验证
    send_len = 4;

    send_len = zce::sendn_timeout(socks5_hdl_, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        return -1;
    }
    //协议规定返回两个字节,第一个是版本号5,第二个是方式
    recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv);
    if (recv_len != 2)
    {
        return -1;
    }
    bool need_auth = false;
    if (SOCKS5_VER != buffer[0])
    {
        return -1;
    }

    //支持不验证和验证两种方式
    if (SOCKS5_METHODS_NOAUTH == buffer[1])
    {
        need_auth = false;
    }
    else if (SOCKS5_METHODS_AUTH_PASSWORD == buffer[1])
    {
        need_auth = true;
    }
    else
    {
        return -1;
    }

    //如果需要验证
    if (need_auth)
    {
        if (!username || !password)
        {
            return EINVAL;
        }
        size_t user_len = strlen(username);
        size_t pass_len = strlen(password);
        if (pass_len > MAX_HOSTNAME_LEN || user_len > MAX_HOSTNAME_LEN)
        {
            return EINVAL;
        }

        //组织验证协议
        buffer[0] = SOCKS5_VER;
        buffer[1] = static_cast<unsigned char>(user_len);
        memcpy(buffer + 2, username, user_len);
        buffer[2 + user_len] = static_cast<unsigned char>(pass_len);
        memcpy(buffer + 3 + user_len, password, pass_len);
        send_len = 3 + user_len + pass_len;
        snd_ret = zce::sendn_timeout(socks5_hdl_, buffer, send_len, timeout_tv);
        if (snd_ret <= 0)
        {
            return -1;
        }
        recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv);
        if (recv_len != 2)
        {
            return -1;
        }
        //验证失败
        if (buffer[0] != SOCKS5_VER || buffer[1] != SOCKS5_SUCCESS)
        {
            return -1;
        }
    }
    return 0;
}

//通过socks5代理，TCP连接服务器
int client::tcp_connect(const char* host_name,
                        const sockaddr* host_addr,
                        uint16_t host_port,
                        socklen_t addr_len,
                        zce::time_value& timeout_tv)
{
    unsigned char *buffer = cmd_buffer_.get();
    ssize_t send_len = 0, recv_len = 0;

    ZCE_ASSERT(host_name || host_addr);

    buffer[0] = SOCKS5_VER;

    //命令
    buffer[1] = SOCKS5_CMD_CONNECT;
    //保留
    buffer[2] = 0x00;

    ssize_t snd_ret = zce::sendn_timeout(socks5_hdl_, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to client proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv);
    if (recv_len != 2)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to client proxy fail, recv_len =%zd!", recv_len);
        return -1;
    }
    //验证结果
    int reponse_code = buffer[1];
    if (buffer[0] != SOCKS5_VER || reponse_code != SOCKS5_SUCCESS)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy connect host fail, reponse code [%d] !", reponse_code);
        return reponse_code;
    }
    ZCE_LOG(RS_DEBUG, "Socks 5 Connected!");
    return 0;
}

//socks5代理，UDP穿透
int client::init_udp_associate(udp_associate &ua,
                               zce::time_value& timeout_tv)
{
    unsigned char *buffer = cmd_buffer_.get();
    ssize_t send_len = 0, recv_len = 0;

    buffer[0] = SOCKS5_VER;
    //命令
    buffer[1] = SOCKS5_CMD_UDP;
    //保留
    buffer[2] = 0x00;

    if (AF_INET == ua.family_)
    {
        buffer[3] = SOCKS5_ATYP_IPV4;
        const sockaddr_in* addr_in = (ua.local_addr_.get_in());
        //无效，填写0
        memset(buffer + 4, 0, 4);
        //不转码，两边都要网络序
        uint16_t n_port = addr_in->sin_port;
        memcpy(buffer + 8, &n_port, 2);
        send_len = 10;
    }
    else if (AF_INET6 == ua.family_)
    {
        buffer[3] = SOCKS5_ATYP_IPV6;
        const sockaddr_in6* addr_in6 = (ua.local_addr_.get_in6());
        memset(buffer + 4, 0, 16);
        uint16_t n_port = addr_in6->sin6_port;
        memcpy(buffer + 20, &n_port, 2);
        send_len = 22;
    }
    else
    {
        return EINVAL;
    }

    ssize_t snd_ret = zce::sendn_timeout(socks5_hdl_, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to client proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    //只收取一次数据，不多次收取
    recv_len = zce::recvn_timeout(socks5_hdl_, buffer, BUF_LEN, timeout_tv, 0, true);
    //至少会接受5个字节
    if (recv_len <= 4)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to client proxy fail, recv_len =%zd!", recv_len);
        return -1;
    }
    //验证结果
    int reponse_code = buffer[1];
    if (buffer[0] != SOCKS5_VER || reponse_code != SOCKS5_SUCCESS)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy connect host fail, reponse code [%d] !", reponse_code);
        return reponse_code;
    }

    if (SOCKS5_ATYP_IPV4 == buffer[3])
    {
        //牛头不对马嘴，收到的数据长度和期待的不一样
        if (AF_INET == ua.family_ || recv_len < 10)
        {
            return -1;
        }
        sockaddr_in* addr_in = (ua.socks_bind_.get_in());
        memcpy(&(addr_in->sin_addr), buffer + 4, 4);
        //不转码，两边都要网络序
        memcpy(&(addr_in->sin_port), buffer + 8, 2);
    }
    else if (SOCKS5_ATYP_IPV6 == buffer[3])
    {
        if (AF_INET6 == ua.family_ || recv_len < 22)
        {
            return -1;
        }
        sockaddr_in6* addr_in6 = (ua.socks_bind_.get_in6());
        memcpy(&(addr_in6->sin6_addr), buffer + 4, 16);
        memcpy(&(addr_in6->sin6_port), buffer + 20, 2);
    }
    else
    {
        return EINVAL;
    }

    return 0;
}

//=======================================================================
//
int udp_associate::open(const sockaddr* local_addr,
                        socklen_t addr_len)
{
    int ret = 0;
    if (sizeof(sockaddr_in) == addr_len)
    {
        family_ = AF_INET;
    }
    else if (sizeof(sockaddr_in6) == addr_len)
    {
        family_ = AF_INET6;
    }
    ret = zce::open_socket(&associate_hdl_,
                           SOCK_DGRAM,
                           local_addr,
                           addr_len);
    if (ret != 0)
    {
        return -1;
    }
    local_addr_ = local_addr;
    return 0;
}

//!
int udp_associate::send(const char* host_name,
                        const sockaddr* host_addr,
                        const char *snd_buf,
                        size_t buf_len)
{
    return 0;
}

//!
int udp_associate::recv(sockaddr* host_addr,
                        char *rcv_buf,
                        size_t buf_len)
{
    return 0;
}
}