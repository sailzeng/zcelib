#ifndef ZERG_EXTERNAL_CONNECT_HANDLER_H_
#define ZERG_EXTERNAL_CONNECT_HANDLER_H_

#include "zerg_tcp_handler_impl.h"

class ExternalConnectHandler: public TcpHandlerImpl
{
public:
    ExternalConnectHandler(size_t len_offset,
                           size_t len_bytes,
                           unsigned int cmd);
    ~ExternalConnectHandler();
public:
    virtual bool need_register();
    virtual void init_buf(ZByteBuffer* buf);
    virtual void adjust_buf(ZByteBuffer* buf);
    virtual int get_pkg_len(unsigned int &whole_frame_len, ZByteBuffer* buf);

    // 获取下一个包，包头已完成解码
    virtual Comm_App_Frame* get_recvframe(ZByteBuffer* buf, unsigned int len);
    virtual bool check_sender();

private:
    size_t len_offset_;
    size_t len_bytes_;
    unsigned int cmd_;

    // ExternalConnectHandler(const ExternalConnectHandler&);
    // ExternalConnectHandler& operator=(const ExternalConnectHandler&);
};

#endif
