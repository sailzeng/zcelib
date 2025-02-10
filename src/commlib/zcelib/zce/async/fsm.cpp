#include "zce/predefine.h"
#include "zce/async/fsm.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"

namespace zce::async
{
//=====================================================================================
//状态机的异步对象
fsm::fsm(zce::async::manager* async_mgr,
           uint32_t create_cmd) :
    actor(async_mgr, create_cmd),
    fsm_stage_(0)
{
}

fsm::~fsm()
{
}

//状态机运行的代码，这只是一个参考示例
void fsm::on_run(bool first_run, bool& running)
{
    enum
    {
        //开始
        STAGE_1 = 1,
        STAGE_2 = 2,
        STAGE_3 = 3,
        //结束
        STAGE_4 = 4,
    };
    if (first_run)
    {
        set_stage(STAGE_1);
    }
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
void fsm::on_timeout(const zce::time_value& now_time,
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
void fsm::set_stage(int stage)
{
    fsm_stage_ = stage;
}

//取得的状态机阶段
int fsm::get_stage() const
{
    return fsm_stage_;
}

//=====================================================================================

//状态机主控管理类
fsm_mgr::fsm_mgr() :
    zce::async::manager()
{
    pool_init_size_ = FSM_POOL_INIT_SIZE;
    pool_extend_size_ = FSM_POOL_INIT_SIZE;
}

fsm_mgr::~fsm_mgr()
{
}
} //namespace zce