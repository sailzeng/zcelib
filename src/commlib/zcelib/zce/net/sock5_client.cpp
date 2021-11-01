#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/net/sock5_client.h"

//==============================================================================================
//SOCKS5支持UDP穿透和TCP代理，比较全面
namespace zce
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

//socks5代理初始化，handle 要先connect，可以使用connect_timeout
int socks5_initialize(ZCE_SOCKET handle,
                      const char* username,
                      const char* password,
                      zce::Time_Value& timeout_tv)
{
    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;
    ssize_t snd_ret = 0;

    buffer[0] = SOCKS5_VER;
    //支持不验证和验证两种方式
    buffer[1] = 2;
    buffer[2] = 0x0; //不需要验证
    buffer[3] = 0x2; //需要验证
    send_len = 4;

    send_len = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        return -1;
    }
    //协议规定返回两个字节,第一个是版本号5,第二个是方式
    recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
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
        const size_t MAX_STRING_LEN = 255;
        if (!username || !password)
        {
            return EINVAL;
        }
        size_t user_len = strlen(username);
        size_t pass_len = strlen(password);
        if (pass_len > MAX_STRING_LEN || user_len > MAX_STRING_LEN)
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
        snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
        if (snd_ret <= 0)
        {
            return -1;
        }
        recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
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
int socks5_connect_host(ZCE_SOCKET handle,
                        const char* host_name,
                        const sockaddr* host_addr,
                        int addr_len,
                        uint16_t host_port,
                        zce::Time_Value& timeout_tv)
{
    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;
    const size_t MAX_STRING_LEN = 255;

    ZCE_ASSERT(host_name || host_addr);

    buffer[0] = SOCKS5_VER;

    //命令
    buffer[1] = SOCKS5_CMD_CONNECT;
    //保留
    buffer[2] = 0x00;
    if (host_name)
    {
        size_t host_len = strlen(host_name);
        if (host_len > MAX_STRING_LEN)
        {
            return EINVAL;
        }
        buffer[3] = SOCKS5_ATYP_HOSTNAME;
        buffer[4] = static_cast<unsigned char>(host_len);
        memcpy(buffer + 5, host_name, host_len);
        uint16_t n_port = htons(host_port);
        memcpy(buffer + 5 + host_len, &n_port, 2);
        send_len = host_len + 6;
    }
    else if (host_addr)
    {
        if (sizeof(sockaddr_in) == addr_len)
        {
            buffer[3] = SOCKS5_ATYP_IPV4;
            const sockaddr_in* addr_in = reinterpret_cast<const sockaddr_in*>(host_addr);
            memcpy(buffer + 4, &(addr_in->sin_addr), 4);
            uint16_t n_port = addr_in->sin_port;
            memcpy(buffer + 8, &n_port, 2);
            send_len = 10;
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            buffer[3] = SOCKS5_ATYP_IPV6;
            const sockaddr_in6* addr_in6 = reinterpret_cast<const sockaddr_in6*>(host_addr);
            memcpy(buffer + 4, &(addr_in6->sin6_addr), 16);
            uint16_t n_port = addr_in6->sin6_port;
            memcpy(buffer + 20, &n_port, 2);
            send_len = 22;
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

    ssize_t snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to socks5 proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
    if (recv_len != 2)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to socks5 proxy fail, recv_len =%zd!", recv_len);
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
int socks5_udp_associate(ZCE_SOCKET handle,
                         const sockaddr* bind_addr,
                         int addr_len,
                         sockaddr* udp_addr,
                         zce::Time_Value& timeout_tv)
{
    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;

    ZCE_ASSERT(bind_addr && udp_addr);

    buffer[0] = SOCKS5_VER;

    //命令
    buffer[1] = SOCKS5_CMD_UDP;
    //保留
    buffer[2] = 0x00;

    if (sizeof(sockaddr_in) == addr_len)
    {
        buffer[3] = SOCKS5_ATYP_IPV4;
        const sockaddr_in* addr_in = reinterpret_cast<const sockaddr_in*>(bind_addr);
        //无效，填写0
        memset(buffer + 4, 0, 4);
        //不转码，两边都要网络序
        uint16_t n_port = addr_in->sin_port;
        memcpy(buffer + 8, &n_port, 2);
        send_len = 10;
    }
    else if (sizeof(sockaddr_in6) == addr_len)
    {
        buffer[3] = SOCKS5_ATYP_IPV6;
        const sockaddr_in6* addr_in6 = reinterpret_cast<const sockaddr_in6*>(bind_addr);
        memset(buffer + 4, 0, 16);
        uint16_t n_port = addr_in6->sin6_port;
        memcpy(buffer + 20, &n_port, 2);
        send_len = 22;
    }
    else
    {
        return EINVAL;
    }

    ssize_t snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to socks5 proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    //只收取一次数据，不多次收取
    recv_len = zce::recvn_timeout(handle, buffer, BUFFER_LEN, timeout_tv, 0, true);
    //至少会接受5个字节
    if (recv_len <= 4)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to socks5 proxy fail, recv_len =%zd!", recv_len);
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
        if (sizeof(sockaddr_in) != addr_len || recv_len < 10)
        {
            return -1;
        }
        sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(udp_addr);
        memcpy(&(addr_in->sin_addr), buffer + 4, 4);
        //不转码，两边都要网络序
        memcpy(&(addr_in->sin_port), buffer + 8, 2);
    }
    else if (SOCKS5_ATYP_IPV6 == buffer[3])
    {
        if (sizeof(sockaddr_in6) != addr_len || recv_len < 22)
        {
            return -1;
        }
        sockaddr_in6* addr_in6 = reinterpret_cast<sockaddr_in6*>(udp_addr);
        memcpy(&(addr_in6->sin6_addr), buffer + 4, 16);
        memcpy(&(addr_in6->sin6_port), buffer + 20, 2);
    }
    else
    {
        return EINVAL;
    }

    return 0;
}
}