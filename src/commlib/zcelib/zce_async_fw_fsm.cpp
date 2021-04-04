
#include "zce_predefine.h"
#include "zce_async_fsm.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"

//=====================================================================================
//״̬�����첽����
ZCE_Async_FSM::ZCE_Async_FSM(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd) :
    ZCE_Async_Object(async_mgr, create_cmd),
    fsm_stage_(0)
{
}

ZCE_Async_FSM::~ZCE_Async_FSM()
{
}

//״̬�����еĴ��룬��ֻ��һ���ο�ʾ��
void ZCE_Async_FSM::on_run(const void *outer_data, bool &continue_run)
{
    ZCE_UNUSED_ARG(outer_data);
    enum
    {
        //��ʼ
        STAGE_1 = 1,
        STAGE_2 = 2,
        STAGE_3 = 3,
        //����
        STAGE_4 = 4,
    };
    switch (get_stage())
    {
        case STAGE_1:
            //Do stage 1 something.init.
            continue_run = true;
            set_stage(STAGE_2);
            break;
        case STAGE_2:
            //Do stage 2 something.
            continue_run = true;
            set_stage(STAGE_3);
            break;
        case STAGE_3:
            //Do stage 3 something.
            continue_run = true;
            set_stage(STAGE_4);
            break;
        case STAGE_4:
            //Do stage 4 something. end.
            continue_run = false;
            break;
        default:
            //һ���޷�ʶ���״̬
            ZCE_ASSERT(false);
            break;
    }
    return;
}

//��ʱ����
void ZCE_Async_FSM::on_timeout(const ZCE_Time_Value &now_time,
                               bool &continue_run)
{
    char time_string[64 + 1];
    ZCE_LOG(RS_INFO, "Time out event ,fun[%s] ,now time[%s].",
            __ZCE_FUNC__,
            now_time.timestamp(time_string, 64));
    continue_run = false;
    return;
}

//���õ�״̬���׶�
void ZCE_Async_FSM::set_stage(int stage)
{
    fsm_stage_ = stage;
}


//ȡ�õ�״̬���׶�
int ZCE_Async_FSM::get_stage() const
{
    return fsm_stage_;
}

//=====================================================================================

//״̬�����ع�����
ZCE_Async_FSMMgr::ZCE_Async_FSMMgr() :
    ZCE_Async_ObjectMgr()
{
    pool_init_size_ = FSM_POOL_INIT_SIZE;
    pool_extend_size_ = FSM_POOL_INIT_SIZE;
}

ZCE_Async_FSMMgr::~ZCE_Async_FSMMgr()
{
}





