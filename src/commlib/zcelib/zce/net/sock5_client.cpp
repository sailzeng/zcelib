#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/net/sock5_client.h"

namespace zce::socks5
{
const char SOCKS5_VER = 0x5;
const char SOCKS5_SUCCESS = 0x0;
const char SOCKS5_METHODS_NOAUTH = 0x0;
const char SOCKS5_METHODS_AUTH_GSSAPI = 0x1;   //不支持
const char SOCKS5_METHODS_AUTH_PASSWORD = 0x2;

const char SOCKS5_CMD_CONNECT = 0x1;
const char SOCKS5_CMD_BIND = 0x2;              //不支持
const char SOCKS5_CMD_UDP = 0x3;

const char SOCKS5_ATYP_IPV4 = 0x1;
const char SOCKS5_ATYP_HOSTNAME = 0x3;
const char SOCKS5_ATYP_IPV6 = 0x4;

const size_t MAX_HOSTNAME_LEN = 255;
//==============================================================================================
//!
int base::hostaddr_to_buf(const char* host_name,
                          uint16_t host_port,
                          const sockaddr* host_addr,
                          socklen_t addr_len,
                          char *buf,
                          size_t buf_len,
                          size_t &write_len)
{
    if (buf_len < 259)
    {
        errno = EINVAL;
        return -1;
    }
    write_len = 0;
    if (host_name)
    {
        size_t host_len = strlen(host_name);
        if (host_len > MAX_HOSTNAME_LEN)
        {
            errno = EINVAL;
            return -1;
        }
        buf[write_len] = SOCKS5_ATYP_HOSTNAME;
        write_len = +1;
        buf[write_len] = static_cast<unsigned char>(host_len);
        write_len += 1;
        memcpy(buf + write_len, host_name, host_len);
        write_len += host_len;
        uint16_t n_port = htons(host_port);
        memcpy(buf + write_len, &n_port, 2);
        write_len += 2;
    }
    else if (host_addr)
    {
        if (sizeof(sockaddr_in) == addr_len)
        {
            buf[write_len] = SOCKS5_ATYP_IPV4;
            write_len += 1;
            const sockaddr_in* addr_in = reinterpret_cast<const sockaddr_in*>(host_addr);
            memcpy(buf + write_len, &(addr_in->sin_addr), 4);
            write_len += 4;
            memcpy(buf + write_len, &(addr_in->sin_port), 2);
            write_len += 2;
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            buf[write_len] = SOCKS5_ATYP_IPV6;
            write_len += 1;
            const sockaddr_in6* addr_in6 = reinterpret_cast<const sockaddr_in6*>(host_addr);
            memcpy(buf + write_len, &(addr_in6->sin6_addr), 16);
            write_len += 16;
            memcpy(buf + write_len, &(addr_in6->sin6_port), 2);
            write_len += 2;
        }
        else
        {
            errno = EINVAL;
            return -1;
        }
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int base::buf_to_hostaddr(const char *buf,
                          size_t buf_len,
                          size_t &read_len,
                          char* host_name,
                          uint16_t *host_port,
                          sockaddr* host_addr,
                          socklen_t addr_len)
{
    read_len = 0;
    if (buf_len <= 5)
    {
        errno = EINVAL;
        return -1;
    }
    if (SOCKS5_ATYP_IPV4 == buf[0])
    {
        read_len += 1;
        //牛头不对马嘴，收到的数据长度和期待的不一样
        if (sizeof(sockaddr_in) == addr_len || host_addr == nullptr || buf_len < 7)
        {
            errno = EINVAL;
            return -1;
        }
        sockaddr_in* addr_in = (sockaddr_in*)(host_addr);
        memcpy(&(addr_in->sin_addr), buf + read_len, 4);
        read_len += 4;
        memcpy(&(addr_in->sin_port), buf + read_len, 2); //不转码，两边都要网络序
        read_len += 2;
    }
    else if (SOCKS5_ATYP_IPV6 == buf[0])
    {
        read_len += 1;
        if (sizeof(sockaddr_in6) == addr_len || host_addr == nullptr || buf_len < 19)
        {
            errno = EINVAL;
            return -1;
        }
        sockaddr_in6* addr_in6 = (sockaddr_in6*)(host_addr);
        memcpy(&(addr_in6->sin6_addr), buf + read_len, 16);
        read_len += 16;
        memcpy(&(addr_in6->sin6_port), buf + read_len, 2);
        read_len += 2;
    }
    else if (SOCKS5_ATYP_HOSTNAME == buf[0])
    {
        size_t name_len = static_cast<unsigned char>(buf[1]); //注意符号
        if (host_name == nullptr || host_port == nullptr || buf_len < 4 + name_len)
        {
            errno = EINVAL;
            return -1;
        }
        read_len += 2;
        memcpy(host_name, buf + read_len, name_len);
        read_len += name_len;
        uint16_t n_port = 0;
        memcpy(&n_port, buf + read_len, 2);
        *host_port = htons(n_port);
        read_len += 2;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
    buf_len = read_len;
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
    char *buffer = cmd_buffer_.get();
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
    recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv, 0, false);
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
        recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv, 0, false);
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
int client::socks5_cmd(char cmd,
                       const char* host_name,
                       uint16_t host_port,
                       const sockaddr* host_addr,
                       socklen_t addr_len,
                       sockaddr* bind_addr,
                       zce::time_value& timeout_tv)
{
    ZCE_ASSERT(host_name || host_addr);
    ZCE_ASSERT(cmd == SOCKS5_CMD_CONNECT || cmd == SOCKS5_CMD_UDP);

    int ret = 0;
    char *buffer = cmd_buffer_.get();
    ssize_t send_len = 0, recv_len = 0;
    buffer[0] = SOCKS5_VER;
    buffer[1] = cmd;  //命令
    buffer[2] = 0x00; //保留
    send_len += 3;

    size_t remain_len = CMD_BUF_LEN - send_len, write_len = 0;
    ret = hostaddr_to_buf(host_name,
                          host_port,
                          host_addr,
                          addr_len,
                          buffer + send_len,
                          remain_len,
                          write_len);
    if (ret != 0)
    {
        return ret;
    }
    send_len += write_len;
    ssize_t snd_ret = zce::sendn_timeout(socks5_hdl_, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to client proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }

    recv_len = zce::recvn_timeout(socks5_hdl_, buffer, 2, timeout_tv, 0, false);  //收取一个包足够了
    if (recv_len < 3)
    {
        errno = EPROTO;
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to client proxy fail, recv_len =%zd!", recv_len);
        return -1;
    }
    size_t use_len = 0;
    //验证结果
    if (buffer[0] != SOCKS5_VER || buffer[1] != SOCKS5_SUCCESS)
    {
        errno = EPROTO;
        ZCE_LOG(RS_ERROR, "Socks 5 proxy connect host fail, reponse code [%d] !", buffer[1]);
        return -1;
    }
    use_len += 3;
    size_t read_len = 0;
    ret = buf_to_hostaddr(buffer + use_len,
                          recv_len - use_len,
                          read_len,
                          nullptr,
                          nullptr,
                          bind_addr,
                          addr_len);
    if (ret != 0)
    {
        return ret;
    }
    ZCE_LOG(RS_DEBUG, "Socks 5 cmd %u!", cmd);
    return 0;
}

//socks5代理，UDP穿透
int client::init_udp_associate(udp_associate &ua,
                               zce::time_value& timeout_tv)
{
    int ret = 0;
    //UDP传统发送的是本地地址 0
    ret = socks5_cmd(SOCKS5_CMD_UDP,
                     nullptr,
                     0,
                     (sockaddr *)ua.local_addr_,
                     ua.family_ == AF_INET ? sizeof(::sockaddr_in) : sizeof(::sockaddr_in6),
                     (sockaddr *)ua.bind_addr_,
                     timeout_tv);
    if (ret != 0)
    {
        return ret;
    }
    ua.associate_ = true;
    return 0;
}

//=======================================================================
//
int udp_associate::open(int family)
{
    int ret = 0;
    family_ = family;
    ret = zce::open_socket(&associate_hdl_,
                           SOCK_DGRAM,
                           family);
    if (ret != 0)
    {
        return -1;
    }
    socklen_t addr_len = 0;
    ret = zce::getpeername(associate_hdl_,
                           (sockaddr*)&local_addr_,
                           &addr_len);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

//!
int udp_associate::send(const char* host_name,
                        uint16_t host_port,
                        const sockaddr* host_addr,
                        socklen_t addr_len,
                        const char *snd_buf,
                        size_t buf_len,
                        bool fragments)
{
    if (!associate_)
    {
        return -1;
    }
    if (buf_len > UDP_MAX_DGRAM)
    {
        return -1;
    }
    int ret = 0;
    char *buffer = snd_rcv_buf_.get();
    size_t use_len = 0;
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    use_len += 3;
    size_t write_len = 0;
    ret = hostaddr_to_buf(host_name,
                          host_port,
                          host_addr,
                          addr_len,
                          buffer + use_len,
                          SR_BUF_LEN - use_len,
                          write_len);
    use_len += write_len;
    size_t wand_len = buf_len;
    size_t send_len = 0, once_send = 0;
    size_t capacity = SR_BUF_LEN - use_len;
    char flag = 0;
    do
    {
        if (wand_len > capacity)
        {
            if (fragments == false)
            {
                errno = EINVAL;
                return -1;
            }

            flag = (flag == 0) ? 1 : flag + 1;
            once_send = capacity;
        }
        else
        {
            if (flag > 0)
            {
                flag = (flag + 1) | 0x80; //0x80最高位表示最后一个
            }
            once_send = wand_len;
        }
        buffer[2] = flag;
        memcpy(buffer + use_len, snd_buf + send_len, once_send);
        wand_len -= once_send;
        send_len += once_send;
        ssize_t sz = zce::sendto_timeout(associate_hdl_,
                                         buffer,
                                         send_len,
                                         (sockaddr *)bind_addr_,
                                         bind_addr_.socklen());
        if (sz <= 0)
        {
            return -1;
        }
    } while (wand_len > capacity);

    return 0;
}

//!Socks5，的UDP穿透的接收数据
int udp_associate::recv(sockaddr* dst_addr,
                        socklen_t addr_len,
                        char *rcv_buf,
                        size_t buf_len,
                        zce::time_value& timeout_tv,
                        bool fragments)
{
    if (!associate_)
    {
        return -1;
    }
    int ret = 0;
    char *buffer = snd_rcv_buf_.get();
    size_t use_len = 0, read_len = 0, rcv_len = 0;
    char flag = 0;
    do
    {
        zce::sockaddr_any from;
        socklen_t from_len = 0;
        ssize_t sz = zce::recvfrom_timeout(associate_hdl_,
                                           buffer,
                                           SR_BUF_LEN,
                                           (sockaddr *)from,
                                           &from_len,
                                           timeout_tv);
        if (sz <= 0)
        {
            return -1;
        }
        //如果不是socks5代理发过来的数据
        if (from != bind_addr_)
        {
            return -1;
        }
        size_t one_recv = sz;
        if (buffer[0] != 0 || buffer[1] != 0)
        {
            errno = EPROTO;
            return -1;
        }
        //数据有分片，但我不打算处理分片，
        if (fragments == false && buffer[2] != 0)
        {
            errno = EINVAL;
            return -1;
        }
        if (buffer[2] != 0)
        {
            flag = (flag == 0) ? 1 : flag + 1;

            //!UDP有倒霉的后发先到的情况，不特殊处理了。
            if ((buffer[2] & 0x7F) != flag)
            {
                errno = EPROTO;
                return -1;
            }
        }
        use_len += 3;
        ret = buf_to_hostaddr(buffer + use_len,
                              one_recv - use_len,
                              read_len,
                              nullptr,
                              nullptr,
                              dst_addr,
                              addr_len);

        use_len += read_len;
        if (rcv_len + (one_recv - use_len) > buf_len)
        {
            return -1;
        }
        memcpy(rcv_buf + rcv_len,
               buffer + use_len,
               one_recv - use_len);
        rcv_len += (one_recv - use_len);
    } while (buffer[2] != 0 || (buffer[2] & 0x80) != 0); //flag的高位为1表示

    return 0;
}
}