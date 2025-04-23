#include "predefine.h"

class FSM_1 : public zce::async::fsm
{
private:
    enum
    {
        //开始
        FMS1_STAGE_1 = 1,
        FMS1_STAGE_2 = 2,
        FSM1_STAGE_3 = 3,
        //结束
        FSM1_STAGE_4 = 4,
    };
public:

    FSM_1(zce::async::manager* async_mgr, unsigned int create_cmd) :
        fsm(async_mgr, create_cmd)
    {
    }

protected:
    virtual ~FSM_1()
    {
    }

public:
    zce::async::actor* clone(zce::async::manager* async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<zce::async::actor*>(new FSM_1(async_mgr, create_cmd));
    }

    virtual void on_run(bool first_run, bool& continue_run)
    {
        if (first_run)
        {
            set_stage(FMS1_STAGE_1);
        }
        switch (get_stage())
        {
        case FMS1_STAGE_1:
            std::cout << "FSM1 stage " << get_stage() << " start." << std::endl;
            continue_run = true;
            set_stage(FMS1_STAGE_2);
            break;
        case FMS1_STAGE_2:
            std::cout << "FSM1 stage " << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM1_STAGE_3);
            break;
        case FSM1_STAGE_3:
            std::cout << "FSM1 stage " << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM1_STAGE_4);
            break;
        case FSM1_STAGE_4:
            std::cout << "FSM1 stage " << get_stage() << " end." << std::endl;
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

class FSM_2 : public zce::async::fsm
{
private:
    enum
    {
        //开始
        FMS2_STAGE_1 = 1,
        FMS2_STAGE_2 = 2,
        FSM2_STAGE_3 = 3,
        //结束
        FSM2_STAGE_4 = 4,
    };

public:
    FSM_2(zce::async::manager* async_mgr, unsigned int create_cmd) :
        fsm(async_mgr, create_cmd)
    {
    }
protected:
    virtual ~FSM_2()
    {
    }
public:
    zce::async::actor* clone(zce::async::manager* async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<zce::async::actor*>(new FSM_2(async_mgr, create_cmd));
    }

    virtual void on_run(bool first_run, bool& continue_run)
    {
        if (first_run)
        {
            set_stage(FMS2_STAGE_1);
        }
        switch (get_stage())
        {
        case FMS2_STAGE_1:
            std::cout << "FSM2 stage " << get_stage() << " start." << std::endl;
            continue_run = true;
            set_stage(FMS2_STAGE_2);
            break;
        case FMS2_STAGE_2:
            std::cout << "FSM2 stage " << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM2_STAGE_3);
            break;
        case FSM2_STAGE_3:
            std::cout << "FSM2 stage " << get_stage() << std::endl;
            continue_run = true;
            set_stage(FSM2_STAGE_4);
            break;
        case FSM2_STAGE_4:
            std::cout << "FSM2 stage" << get_stage() << " end." << std::endl;
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
int test_async_fsm(int  /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    const unsigned int CMD_1 = 10001;
    const unsigned int CMD_2 = 10002;
    const unsigned int CMD_3 = 10003;

    zce::timer_queue* time_queue = new zce::timer_wheel();
    zce::async::fsm_mgr* mgr = new zce::async::fsm_mgr();
    mgr->initialize(time_queue, 100, 200);
    mgr->register_asyncobj(CMD_1, new FSM_1(mgr, CMD_1));
    mgr->register_asyncobj(CMD_2, new FSM_2(mgr, CMD_2));

    bool running;
    uint32_t fsm1_async_id1;
    ret = mgr->create_asyncobj(CMD_1, fsm1_async_id1, running);
    uint32_t fsm1_async_id2;
    ret = mgr->create_asyncobj(CMD_1, fsm1_async_id2, running);

    uint32_t fsm2_async_id1;
    ret = mgr->create_asyncobj(CMD_2, fsm2_async_id1, running);

    uint32_t nouse_fsm3_id;
    ret = mgr->create_asyncobj(CMD_3, nouse_fsm3_id, running);
    ZCE_ASSERT(ret != 0);

    ret = mgr->active_asyncobj(fsm1_async_id1, running);
    ret = mgr->active_asyncobj(fsm2_async_id1, running);
    ret = mgr->active_asyncobj(fsm1_async_id1, running);
    ret = mgr->active_asyncobj(fsm2_async_id1, running);
    ret = mgr->active_asyncobj(fsm1_async_id1, running);
    ret = mgr->active_asyncobj(fsm2_async_id1, running);
    ret = mgr->active_asyncobj(fsm1_async_id1, running);
    ret = mgr->active_asyncobj(fsm2_async_id1, running);

    return 0;
}

//class Coroutine_1 : public zce::coro
//{
//public:
//    Coroutine_1(zce::manager* async_mgr, unsigned int create_cmd) :
//        zce::coro(async_mgr, create_cmd)
//    {
//    }
//
//    zce::actor* clone(zce::manager* async_mgr, unsigned int create_cmd)
//    {
//        return dynamic_cast<zce::actor*>(new Coroutine_1(async_mgr, create_cmd));
//    }
//
//    ///协程运行,你要重载的函数
//    virtual void coroutine_run()
//    {
//        std::cout << "Coroutine_1 start." << std::endl;
//        yeild_main_continue();
//
//        std::cout << "Coroutine_1 continue." << std::endl;
//        yeild_main_continue();
//
//        std::cout << "Coroutine_1 end." << std::endl;
//    }
//};
//
//class Coroutine_2 : public zce::coro
//{
//public:
//    Coroutine_2(zce::manager* async_mgr, unsigned int create_cmd) :
//        zce::coro(async_mgr, create_cmd)
//    {
//    }
//
//    zce::actor* clone(zce::manager* async_mgr, unsigned int create_cmd)
//    {
//        return dynamic_cast<zce::actor*>(new Coroutine_2(async_mgr, create_cmd));
//    }
//
//    virtual void coroutine_run()
//    {
//        std::cout << "Coroutine_2 start." << std::endl;
//        yeild_main_continue();
//
//        std::cout << "Coroutine_2 continue." << std::endl;
//        yeild_main_continue();
//
//        std::cout << "Coroutine_2 end." << std::endl;
//    }
//};

////
//int test_async_coroutine(int  /*argc*/, char* /*argv*/[])
//{
//    int ret = 0;
//    const unsigned int CMD_1 = 10001;
//    const unsigned int CMD_2 = 10002;
//    //const unsigned int CMD_3 = 10003;
//
//    zce::Timer_Queue* time_queue = new zce::Timer_Wheel();
//    zce::coro_mgr* mgr = new zce::coro_mgr();
//    mgr->initialize(time_queue, 100, 2000);
//    mgr->register_asyncobj(CMD_1, new Coroutine_1(mgr, CMD_1));
//    mgr->register_asyncobj(CMD_2, new Coroutine_2(mgr, CMD_2));
//    bool running;
//    uint32_t fsm1_async_id1;
//    ret = mgr->create_asyncobj(CMD_1, nullptr, 0, fsm1_async_id1, running);
//    uint32_t fsm1_async_id2;
//    ret = mgr->create_asyncobj(CMD_1, nullptr, 0, fsm1_async_id2, running);
//
//    ret = mgr->active_asyncobj(fsm1_async_id1, nullptr, 0, running);
//    ret = mgr->active_asyncobj(fsm1_async_id2, nullptr, 0, running);
//    ret = mgr->active_asyncobj(fsm1_async_id1, nullptr, 0, running);
//    ret = mgr->active_asyncobj(fsm1_async_id2, nullptr, 0, running);
//
//    ZCE_ASSERT(ret == 0);
//
//    return 0;
//}

/*
注意，Fiber是无法达到这个效果的
int test_coroutine1(int argc, char * argv[])
{
coroutine_t context;

zce::getcontext(&context);
puts("Hello world");
zce::sleep(1);
zce::setcontext(&context);
return 0;
}
*/
