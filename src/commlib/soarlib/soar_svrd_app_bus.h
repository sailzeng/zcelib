
#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_
#define SOARING_LIB_SVRD_APP_NONCTRL_H_

#include "soar_svrd_app_base.h"

class ZERG_FRAME_HEAD;

//单个函数处理APP FRAME
class Soar_SvrdApp_ZergBus : public Soar_Svrd_Appliction
{
protected:

    //接受的数据区
    ZERG_FRAME_HEAD          *nonctrl_recv_buffer_;

protected:
    //
    Soar_SvrdApp_ZergBus();
    virtual ~Soar_SvrdApp_ZergBus();

public:

    //运行处理,
    virtual int app_run();

protected:

    //处理接收到的Frame,
    virtual int popfront_recvpipe(size_t max_prc, size_t &proc_frame);

    //处理收到的APPFRAME，不使用const的原因是因为为了加快速度，很多地方是直接将recv_frame修改
    virtual int process_recv_frame(ZERG_FRAME_HEAD *recv_frame) = 0;


};

#endif //#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_

