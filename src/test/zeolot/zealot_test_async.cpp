#include "zealot_predefine.h"
#include "zealot_test_function.h"


class FSM_1 :public ZCE_Async_FSM
{

public:
    FSM_1(ZCE_Async_ObjectMgr *async_mgr) :
        ZCE_Async_FSM(async_mgr)
    {
    }
protected:
    virtual ~FSM_1()
    {
    }

public:
    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr)
    {
        return dynamic_cast<ZCE_Async_Object * >(new FSM_1(async_mgr));
    }

    void on_run(bool &continue_run)
    {
        enum
        {
            //开始
            FMS1_STAGE_1 = 1,
            FMS1_STAGE_2 = 2,
            FSM1_STAGE_3 = 3,
            //结束
            FSM1_STAGE_4 = 4,
        };
        switch (get_stage())
        {
        case FMS1_STAGE_1:
            std::cout << "FSM1 stage" << get_stage() << "start."<< std::endl;
            continue_run = true;
            set_stage(FMS1_STAGE_2);
            break;
        case FMS1_STAGE_2:
            std::cout << "FSM1 stage" << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM1_STAGE_3);
            break;
        case FSM1_STAGE_3:
            std::cout << "FSM1 stage" << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM1_STAGE_4);
            break;
        case FSM1_STAGE_4:
            std::cout << "FSM1 stage" << get_stage() << "end."<<std::endl;
            continue_run = false;
            break;
        default:
            //一个无法识别的状态
            ZCE_ASSERT(false);
            break;
        }
        return;
    }

};


class FSM_2 :public ZCE_Async_FSM
{


public:
    FSM_2(ZCE_Async_ObjectMgr *async_mgr) :
        ZCE_Async_FSM(async_mgr)
    {
    }
protected:
    virtual ~FSM_2()
    {
    }
public:
    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr)
    {
        return dynamic_cast<ZCE_Async_Object *>(new FSM_2(async_mgr));
    }

    void on_run(bool &continue_run)
    {
        enum
        {
            //开始
            FMS2_STAGE_1 = 1,
            FMS2_STAGE_2 = 2,
            FSM2_STAGE_3 = 3,
            //结束
            FSM2_STAGE_4 = 4,
        };
        switch (get_stage())
        {
        case FMS2_STAGE_1:
            //Do stage 1 something.init.
            continue_run = true;
            set_stage(FMS2_STAGE_2);
            break;
        case FMS2_STAGE_2:
            //Do stage 2 something.
            continue_run = true;
            set_stage(FSM2_STAGE_3);
            break;
        case FSM2_STAGE_3:
            //Do stage 3 something.
            continue_run = true;
            set_stage(FSM2_STAGE_4);
            break;
        case FSM2_STAGE_4:
            //Do stage 4 something. end.
            continue_run = false;
            break;
        default:
            //一个无法识别的状态
            ZCE_ASSERT(false);
            break;
        }
        return;
    }
};

//
int test_async_fsm(int  /*argc*/, char * /*argv*/[])
{
    int ret = 0;
    const unsigned int CMD_1 = 10001;
    const unsigned int CMD_2 = 10002;
    const unsigned int CMD_3 = 10003;

    ZCE_Timer_Queue *time_queue = new ZCE_Timer_Wheel();
    ZCE_Async_FSMMgr *mgr = new ZCE_Async_FSMMgr(time_queue);
    mgr->register_asyncobj(CMD_1, new FSM_1(mgr));
    mgr->register_asyncobj(CMD_2, new FSM_2(mgr));
    
    unsigned int fsm1_async_id1;
    ret = mgr->create_asyncobj(CMD_1,&fsm1_async_id1);
    unsigned int fsm1_async_id2;
    ret = mgr->create_asyncobj(CMD_1, &fsm1_async_id2);

    unsigned int fsm2_async_id1;
    ret = mgr->create_asyncobj(CMD_2, &fsm2_async_id1);


    unsigned int nouse_fsm3_id;
    ret = mgr->create_asyncobj(CMD_3, &nouse_fsm3_id);
    ZCE_ASSERT(ret != 0);

    ret = mgr->active_asyncobj(fsm1_async_id1);
    ret = mgr->active_asyncobj(fsm1_async_id1);
    ret = mgr->active_asyncobj(fsm1_async_id1);
    ret = mgr->active_asyncobj(fsm1_async_id1);


    return 0;
}

//
int test_async_coroutine(int  /*argc*/, char * /*argv*/[])
{
    return 0;
}
