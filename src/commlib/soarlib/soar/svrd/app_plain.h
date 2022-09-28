#pragma once

#include "soar/svrd/app_bus.h"

class soar::zerg_frame;

namespace soar
{
//单个函数处理APP FRAME
class SvrdApp_Plain : public soar::App_BusPipe
{
protected:
    //
    SvrdApp_Plain();
    virtual ~SvrdApp_Plain();

public:

    //运行处理,
    virtual int app_run();

protected:

    //处理接收到的Frame,
    virtual int popfront_recvpipe(size_t max_prc, size_t& proc_frame);

    //处理收到的APPFRAME，不使用const的原因是因为为了加快速度，很多地方是直接将recv_frame修改
    virtual int process_recv_frame(soar::zerg_frame* recv_frame) = 0;

protected:

    //接受的数据区
    soar::zerg_frame* nonctrl_recv_buffer_;
};
}
