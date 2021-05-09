#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_
#define SOARING_LIB_SVRD_APP_NONCTRL_H_

#include "soar/svrd/app_base.h"

class soar::Zerg_Frame;

//单个函数处理APP FRAME
class SvrdApp_Plain: public soar::Svrd_Appliction
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
    virtual int process_recv_frame(soar::Zerg_Frame* recv_frame) = 0;

protected:

    //接受的数据区
    soar::Zerg_Frame* nonctrl_recv_buffer_;
};

#endif //#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_
