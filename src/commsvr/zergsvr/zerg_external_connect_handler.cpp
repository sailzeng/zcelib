#include "zerg_predefine.h"
#include "zerg_external_connect_handler.h"
#include "zerg_buf_storage.h"

ExternalConnectHandler::ExternalConnectHandler(size_t len_offset,
                                                 size_t len_bytes,
                                                 unsigned int cmd):
    len_offset_(len_offset),
    len_bytes_(len_bytes),
    cmd_(cmd)
{
}

ExternalConnectHandler::~ExternalConnectHandler()
{
}

void ExternalConnectHandler::init_buf(ZByteBuffer* buf)
{
    buf->size_of_buffer_ = Comm_App_Frame::LEN_OF_APPFRAME_HEAD;
    buf->size_of_use_ = Comm_App_Frame::LEN_OF_APPFRAME_HEAD;
}

int ExternalConnectHandler::get_pkg_len(unsigned int &whole_frame_len, ZByteBuffer* buf)
{
    if (buf->size_of_buffer_ - buf->size_of_use_ >= len_offset_ + len_bytes_ )
    {
        uint8_t* len  = reinterpret_cast<uint8_t*>(buf->buffer_data_ + buf->size_of_use_ + len_offset_);
        for (size_t i = 0; i < len_bytes_; ++i)
        {
            // 最初没加（），逻辑错误!!!
            whole_frame_len = (whole_frame_len << 8) + len[i];
        }

        if (whole_frame_len > Comm_App_Frame::MAX_LEN_OF_APPFRAME)
        {
            return SOAR_RET::ERR_ZERG_GREATER_MAX_LEN_FRAME;
        }
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

void ExternalConnectHandler::adjust_buf(ZByteBuffer* buf)
{
    if (buf->size_of_use_ > Comm_App_Frame::LEN_OF_APPFRAME_HEAD)
    {
        memmove(buf->buffer_data_ + Comm_App_Frame::LEN_OF_APPFRAME_HEAD,
                buf->buffer_data_ + buf->size_of_use_,
                buf->size_of_buffer_ - buf->size_of_use_);

        //改变buffer长度
        buf->size_of_buffer_ = buf->size_of_buffer_ - buf->size_of_use_
            + Comm_App_Frame::LEN_OF_APPFRAME_HEAD;
        buf->size_of_use_ = Comm_App_Frame::LEN_OF_APPFRAME_HEAD;
    }
}

Comm_App_Frame* ExternalConnectHandler::get_recvframe(ZByteBuffer* buf, unsigned int len)
{
    Comm_App_Frame* frame = reinterpret_cast<Comm_App_Frame *>(buf->buffer_data_
                                                               + buf->size_of_use_
                                                               - Comm_App_Frame::LEN_OF_APPFRAME_HEAD);
    frame->init_framehead(len + Comm_App_Frame::LEN_OF_APPFRAME_HEAD, 0, cmd_);
    return frame;
}

bool ExternalConnectHandler::check_sender()
{
    return false;
}

bool ExternalConnectHandler::need_register()
{
    return false;
}

