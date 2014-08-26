#ifndef ZERG_INNER_CONNECT_HANDLER_H_
#define ZERG_INNER_CONNECT_HANDLER_H_

#include "zerg_tcp_handler_impl.h"

class InnerConnectHandler: public TcpHandlerImpl
{
private:
    InnerConnectHandler();
    static InnerConnectHandler *inst_;

public:
    ~InnerConnectHandler();
    static InnerConnectHandler * instance();
public:
    virtual int get_pkg_len(unsigned int &whole_frame_len, ZByteBuffer* buf);
    virtual int preprocess_recvframe(Comm_App_Frame *proc_frame);
    virtual bool check_sender();

private:
    // InnerConnectHandler(const InnerConnectHandler&);
    // InnerConnectHandler& operator=(const InnerConnectHandler&);
};

#endif
