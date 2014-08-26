#include "zerg_predefine.h"
#include "zerg_inner_connect_handler.h"
#include "zerg_buf_storage.h"

InnerConnectHandler * InnerConnectHandler::inst_ = NULL;

InnerConnectHandler* InnerConnectHandler::instance()
{
    if (inst_ == NULL)
    {
        inst_ = new InnerConnectHandler;
    }
    return inst_;
}

InnerConnectHandler::~InnerConnectHandler()
{
}

InnerConnectHandler::InnerConnectHandler()
{

}

int InnerConnectHandler::preprocess_recvframe(Comm_App_Frame *proc_frame)
{
    zce_UNUSED_ARG(proc_frame);
    return SOAR_RET::SOAR_RET_SUCC;
}
int InnerConnectHandler::get_pkg_len(unsigned int &whole_frame_len, ZByteBuffer* buf)
{
    //如果连4个字节都没有收集齐,不用处理下面这段
    //注意这儿是在32位环境考虑
    if (buf->size_of_buffer_ - buf->size_of_use_ >= sizeof(unsigned int) )
    {
        //如果有4个字节,检查帧的长度
        whole_frame_len = *(reinterpret_cast<unsigned int *>(buf->buffer_data_ + buf->size_of_use_));
        whole_frame_len = ntohl(whole_frame_len);

        //如果包的长度大于定义的最大长度,小于最小长度,见鬼去,出现做个错误不是代码错误，就是被人整蛊
        if (whole_frame_len > Comm_App_Frame::MAX_LEN_OF_APPFRAME || whole_frame_len < Comm_App_Frame::LEN_OF_APPFRAME_HEAD)
        {
            return SOAR_RET::ERR_ZERG_GREATER_MAX_LEN_FRAME;
        }

    }
    return SOAR_RET::SOAR_RET_SUCC;
}

bool InnerConnectHandler::check_sender()
{
    return true;
}

