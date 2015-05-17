#include "zealot_predefine.h"
#include "zealot_test_function.h"


class FSM_1 :public ZCE_Async_FSM
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

    FSM_1(ZCE_Async_ObjectMgr *async_mgr,unsigned int create_cmd) :
        ZCE_Async_FSM(async_mgr, create_cmd)
    {
        set_stage(FMS1_STAGE_1);
    }


protected:
    virtual ~FSM_1()
    {
    }

public:
    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<ZCE_Async_Object * >(new FSM_1(async_mgr, create_cmd));
    }

    virtual void on_run(void *outer_data, bool &continue_run)
    {
        ZCE_UNUSED_ARG(outer_data);
        switch (get_stage())
        {
        case FMS1_STAGE_1:
            std::cout << "FSM1 stage " << get_stage() << " start."<< std::endl;
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
            std::cout << "FSM1 stage " << get_stage() << " end."<<std::endl;
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
    FSM_2(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd) :
        ZCE_Async_FSM(async_mgr, create_cmd)
    {
        set_stage(FMS2_STAGE_1);
    }
protected:
    virtual ~FSM_2()
    {
    }
public:
    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<ZCE_Async_Object *>(new FSM_2(async_mgr, create_cmd));
    }

    virtual void on_run(void *outer_data, bool &continue_run)
    {
        ZCE_UNUSED_ARG(outer_data);
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
int test_async_fsm(int  /*argc*/, char * /*argv*/[])
{
    int ret = 0;
    const unsigned int CMD_1 = 10001;
    const unsigned int CMD_2 = 10002;
    const unsigned int CMD_3 = 10003;

    ZCE_Timer_Queue *time_queue = new ZCE_Timer_Wheel();
    ZCE_Async_FSMMgr *mgr = new ZCE_Async_FSMMgr();
    mgr->initialize(time_queue,100,200);
    mgr->register_asyncobj(CMD_1, new FSM_1(mgr, CMD_1));
    mgr->register_asyncobj(CMD_2, new FSM_2(mgr, CMD_2));
    
    unsigned int fsm1_async_id1;
    ret = mgr->create_asyncobj(CMD_1,NULL,&fsm1_async_id1);
    unsigned int fsm1_async_id2;
    ret = mgr->create_asyncobj(CMD_1,NULL,&fsm1_async_id2);

    unsigned int fsm2_async_id1;
    ret = mgr->create_asyncobj(CMD_2, NULL,&fsm2_async_id1);


    unsigned int nouse_fsm3_id;
    ret = mgr->create_asyncobj(CMD_3, NULL, &nouse_fsm3_id);
    ZCE_ASSERT(ret != 0);

    ret = mgr->active_asyncobj(fsm1_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm2_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm1_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm2_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm1_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm2_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm1_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm2_async_id1, NULL);


    return 0;
}

class Coroutine_1 :public ZCE_Async_Coroutine
{
public:
    Coroutine_1(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd) :
        ZCE_Async_Coroutine(async_mgr, create_cmd)
    {
    }

    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<ZCE_Async_Object *>(new Coroutine_1(async_mgr, create_cmd));
    }

    ///协程运行,你要重载的函数
    virtual void coroutine_run()
    {
        std::cout << "Coroutine_1 start." << std::endl;
        yeild_main_continue();

        std::cout << "Coroutine_1 continue." << std::endl;
        yeild_main_continue();

        std::cout << "Coroutine_1 end." << std::endl;
    }
    
};

class Coroutine_2 :public ZCE_Async_Coroutine
{
public:
    Coroutine_2(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd) :
        ZCE_Async_Coroutine(async_mgr, create_cmd)
    {
    }

    ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd)
    {
        return dynamic_cast<ZCE_Async_Object *>(new Coroutine_2(async_mgr, create_cmd));
    }

    virtual void coroutine_run()
    {
        std::cout << "Coroutine_2 start." << std::endl;
        yeild_main_continue();

        std::cout << "Coroutine_2 continue." << std::endl;
        yeild_main_continue();

        std::cout << "Coroutine_2 end." << std::endl;
    }

};

//
int test_async_coroutine(int  /*argc*/, char * /*argv*/[])
{
    int ret = 0;
    const unsigned int CMD_1 = 10001;
    const unsigned int CMD_2 = 10002;
    //const unsigned int CMD_3 = 10003;

    ZCE_Timer_Queue *time_queue = new ZCE_Timer_Wheel();
    ZCE_Async_CoroutineMgr *mgr = new ZCE_Async_CoroutineMgr();
    mgr->initialize(time_queue, 100, 2000);
    mgr->register_asyncobj(CMD_1, new Coroutine_1(mgr, CMD_1));
    mgr->register_asyncobj(CMD_2, new Coroutine_2(mgr, CMD_2));

    unsigned int fsm1_async_id1;
    ret = mgr->create_asyncobj(CMD_1, NULL,&fsm1_async_id1);
    unsigned int fsm1_async_id2;
    ret = mgr->create_asyncobj(CMD_1, NULL, &fsm1_async_id2);

    ret = mgr->active_asyncobj(fsm1_async_id1,NULL);
    ret = mgr->active_asyncobj(fsm1_async_id2, NULL);
    ret = mgr->active_asyncobj(fsm1_async_id1, NULL);
    ret = mgr->active_asyncobj(fsm1_async_id2, NULL);

    ZCE_ASSERT(ret == 0);

    return 0;
}



/*
注意，Fiber是无法达到这个效果的
int test_coroutine1(int argc, char * argv[])
{
coroutine_t context;

ZCE_LIB::getcontext(&context);
puts("Hello world");
ZCE_LIB::sleep(1);
ZCE_LIB::setcontext(&context);
return 0;
}
*/


const int COROUTINE_LOOP_NUMBER = 3;

/* This is the iterator function. It is entered on the first call to
* swapcontext, and loops from 0 to 9. Each value is saved in i_from_iterator,
* and then swapcontext used to return to the main loop.  The main loop prints
* the value and calls swapcontext to swap back into the function. When the end
* of the loop is reached, the function exits, and execution switches to the
* context pointed to by main_context1. */
void loop(
    coroutine_t *loop_context,
    int *i_from_iterator)
{
    int i;

    for (i = 0; i < COROUTINE_LOOP_NUMBER; ++i)
    {
        /* Write the loop counter into the iterator return location. */
        *i_from_iterator = i;

        /* Save the loop context (this point in the code) into ''loop_context'',
        * and switch to other_context. */
        ZCE_LIB::yeild_main(loop_context);
    }

    /* The function falls through to the calling context with an implicit
    * ''setcontext(&loop_context->uc_link);'' */
}


int test_coroutine2(int /*argc*/, char * /*argv*/[])
{

    /* The three contexts:
    *    (1) main_context1 : The point in main to which loop will return.
    *    (2) main_context2 : The point in main to which control from loop will
    *                        flow by switching contexts.
    *    (3) loop_context  : The point in loop to which control from main will
    *                        flow by switching contexts. */
    coroutine_t loop_context;



    /* Flag indicating that the iterator has completed. */
    volatile int iterator_finished;

    /* The iterator return value. */
    volatile int i_from_iterator = 0;

    /* Fill in loop_context so that it makes swapcontext start loop. The
    * (void (*)(void)) typecast is to avoid a compiler warning but it is
    * not relevant to the behaviour of the function. */
    ZCE_LIB::make_coroutine(&loop_context,
        8192 * 100,
        true,
        (ZCE_COROUTINE_3PARA)loop,
        (void *)&loop_context,
        (void *)&i_from_iterator,
        NULL);

    /* Clear the finished flag. */
    iterator_finished = 0;

    /* Save the current context into main_context1. When loop is finished,
    * control flow will return to this point. */
    //ZCE_LIB::getcontext(&main_context1);

    if (!iterator_finished)
    {
        /* Set iterator_finished so that when the previous getcontext is
        * returned to via uc_link, the above if condition is false and the
        * iterator is not restarted. */
        iterator_finished = 1;

        while (COROUTINE_LOOP_NUMBER - 1 > i_from_iterator)
        {
            /* Save this point into main_context2 and switch into the iterator.
            * The first call will begin loop.  Subsequent calls will switch to
            * the swapcontext in loop. */
            ZCE_LIB::yeild_coroutine(&loop_context);
            printf("%d\n", i_from_iterator);
        }
    }

    return 0;

}
