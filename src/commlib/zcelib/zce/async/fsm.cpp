#include "zce/predefine.h"
#include "zce/async/fsm.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"

namespace zce
{
//=====================================================================================
//状态机的异步对象
Async_FSM::Async_FSM(zce::Async_Obj_Mgr* async_mgr, unsigned int create_cmd) :
    Async_Object(async_mgr, create_cmd),
    fsm_stage_(0)
{
}

Async_FSM::~Async_FSM()
{
}

//状态机运行的代码，这只是一个参考示例
void Async_FSM::on_run(const void* outer_data,
                       size_t /*data_len*/,
                       bool& running)
{
    ZCE_UNUSED_ARG(outer_data);
    enum
    {
        //开始
        STAGE_1 = 1,
        STAGE_2 = 2,
        STAGE_3 = 3,
        //结束
        STAGE_4 = 4,
    };
    switch (get_stage())
    {
    case STAGE_1:
        //Do stage 1 something.init.
        running = true;
        set_stage(STAGE_2);
        break;
    case STAGE_2:
        //Do stage 2 something.
        running = true;
        set_stage(STAGE_3);
        break;
    case STAGE_3:
        //Do stage 3 something.
        running = true;
        set_stage(STAGE_4);
        break;
    case STAGE_4:
        //Do stage 4 something. end.
        running = false;
        break;
    default:
        //一个无法识别的状态
        ZCE_ASSERT(false);
        break;
    }
    return;
}

//超时处理
void Async_FSM::on_timeout(const zce::Time_Value& now_time,
                           bool& continue_run)
{
    char time_string[64 + 1];
    ZCE_LOG(RS_INFO, "Time out event ,fun[%s] ,now time[%s].",
            __ZCE_FUNC__,
            now_time.timestamp(time_string, 64));
    continue_run = false;
    return;
}

//设置的状态机阶段
void Async_FSM::set_stage(int stage)
{
    fsm_stage_ = stage;
}

//取得的状态机阶段
int Async_FSM::get_stage() const
{
    return fsm_stage_;
}

//=====================================================================================

//状态机主控管理类
Async_FSMMgr::Async_FSMMgr() :
    zce::Async_Obj_Mgr()
{
    pool_init_size_ = FSM_POOL_INIT_SIZE;
    pool_extend_size_ = FSM_POOL_INIT_SIZE;
}

Async_FSMMgr::~Async_FSMMgr()
{
}
} //namespace zce