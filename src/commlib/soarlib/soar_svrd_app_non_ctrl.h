
#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_
#define SOARING_LIB_SVRD_APP_NONCTRL_H_

#include "soar_svrd_app_base.h"

class Zerg_App_Frame;
//单个函数处理APP FRAME
class Comm_SvrdApp_NonCtrl : public Comm_Svrd_Appliction
{
protected:

    //接受的数据区
    Zerg_App_Frame          *nonctrl_recv_buffer_;

protected:
    //
    Comm_SvrdApp_NonCtrl();
    virtual ~Comm_SvrdApp_NonCtrl();

public:

    //运行处理,
    virtual int run_instance();

protected:

    //处理接收到的Frame,
    virtual int popfront_recvpipe(size_t &procframe);
    //处理收到的APPFRAME，不使用const的原因是因为为了加快速度，很多地方是直接将recv_frame修改
    virtual int process_recv_appframe(Zerg_App_Frame *recv_frame) = 0;

    // 处理想要处理的数据
    virtual int proc(size_t &proc_data_num)
    {
        proc_data_num = 0;
        return 0;
    }

};

#endif //#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_

