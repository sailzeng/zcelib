#ifndef ZERG_TCP_HANDLER_IMPL_H_
#define ZERG_TCP_HANDLER_IMPL_H_

class ZByteBuffer;

// 内外协议的收发包处理逻辑基类接口
class TcpHandlerImpl
{
public:
    virtual ~TcpHandlerImpl();
public:
    // 是否发送注册命令，该命令当期作用不明
    virtual bool need_register();
    // 操作接收缓冲的不同策略
    virtual void init_buf(ZByteBuffer* buf);
    virtual void adjust_buf(ZByteBuffer* buf);

    // 获取下一个包，包头已完成解码
    virtual Comm_App_Frame* get_recvframe(ZByteBuffer* buf, unsigned int len);

    // 获取下一个包的包长
    virtual int get_pkg_len(unsigned int &whole_frame_len, ZByteBuffer* buf) = 0;

    virtual bool check_sender() = 0;
};

#endif
