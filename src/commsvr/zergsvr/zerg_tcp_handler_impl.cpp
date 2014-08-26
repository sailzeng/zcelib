#include "zerg_predefine.h"
#include "zerg_tcp_handler_impl.h"
#include "zerg_buf_storage.h"

void TcpHandlerImpl::init_buf(ZByteBuffer* buf)
{
    zce_UNUSED_ARG(buf);
}

TcpHandlerImpl::~TcpHandlerImpl()
{

}

void TcpHandlerImpl::adjust_buf(ZByteBuffer* buf)
{
    if (buf->size_of_use_ > 0)
    {
        //拷贝的内存可能交错,所以不用memcpy
        memmove(buf->buffer_data_,
                buf->buffer_data_ + buf->size_of_use_,
                buf->size_of_buffer_ - buf->size_of_use_);

        //改变buffer长度
        buf->size_of_buffer_ = buf->size_of_buffer_ - buf->size_of_use_;
        buf->size_of_use_ = 0;
    }
}

Comm_App_Frame* TcpHandlerImpl::get_recvframe(ZByteBuffer* buf, unsigned int)
{
    Comm_App_Frame* frame = reinterpret_cast<Comm_App_Frame *>( buf->buffer_data_ + buf->size_of_use_);
    frame->framehead_decode();
    frame->clear_inner_option();
    return frame;
}

bool TcpHandlerImpl::need_register()
{
    return true;
}

