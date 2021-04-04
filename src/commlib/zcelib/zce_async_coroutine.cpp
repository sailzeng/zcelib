
#include "zce_predefine.h"
#include "zce_async_coroutine.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"



//========================================================================================

ZCE_Async_Coroutine::ZCE_Async_Coroutine(ZCE_Async_ObjectMgr *async_mgr, unsigned int reg_cmd) :
    ZCE_Async_Object(async_mgr, reg_cmd)
{
    //��ջ��СĬ��ѡ����С�ģ�
}

ZCE_Async_Coroutine::~ZCE_Async_Coroutine()
{
}


//��ʼ��Э�̵Ķ���
int ZCE_Async_Coroutine::initialize()
{
    ZCE_Async_Object::initialize();
    int ret = 0;
    ret = zce::make_coroutine(&handle_,
                              stack_size_,
                              true,
                              (ZCE_COROUTINE_3PARA)ZCE_Async_Coroutine::static_do,
                              (void *)this,
                              NULL,
                              NULL
                             );
    if (ret != 0)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::make_coroutine return fail.", ret);
        return ret;
    }
    return 0;
}

//����Э�̶���
void ZCE_Async_Coroutine::finish()
{
    ZCE_Async_Object::finish();
    zce::delete_coroutine(&handle_);
    return;
}

//����Э��
void ZCE_Async_Coroutine::on_run(const void *outer_data, bool &continue_run)
{
    receive_data(outer_data);
    continue_run = false;

    yeild_coroutine();

    receive_data_ = NULL;

    //���ݵ��÷��صĺ�����¼��״ֵ̬�õ���ǰ��״̬
    if (coroutine_state_ == COROUTINE_CONTINUE)
    {
        continue_run = true;
    }
    else if (coroutine_state_ == COROUTINE_EXIT)
    {
        continue_run = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}


//����Э��
void ZCE_Async_Coroutine::on_timeout(const ZCE_Time_Value & /*now_time*/,
                                     bool &continue_run)
{
    continue_run = false;
    coroutine_state_ = COROUTINE_TIMEOUT;
    yeild_coroutine();

    //���ݵ��÷��صĺ�����¼��״ֵ̬�õ���ǰ��״̬
    if (coroutine_state_ == COROUTINE_CONTINUE)
    {
        continue_run = true;
    }
    else if (coroutine_state_ == COROUTINE_EXIT)
    {
        continue_run = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}

//�л���Main��Э�̻����������
void ZCE_Async_Coroutine::yeild_main_continue()
{
    coroutine_state_ = COROUTINE_CONTINUE;
    zce::yeild_main(&handle_);
}

//�л���Main,Э���˳�
void ZCE_Async_Coroutine::yeild_main_exit()
{
    coroutine_state_ = COROUTINE_EXIT;
    zce::yeild_main(&handle_);
}


//�л���Э�̣�Ҳ�����л������Լ�����
void ZCE_Async_Coroutine::yeild_coroutine()
{
    zce::yeild_coroutine(&handle_);
}



//Э�̶�������к���
void ZCE_Async_Coroutine::coroutine_do()
{
    //�����ҪЭ��
    for (;;)
    {
        //
        coroutine_run();

        //������Ϻ󣬷���������
        yeild_main_exit();
    }
}

///static ����������Э�����к���������Э�̶�������к���
void ZCE_Async_Coroutine::static_do(void *coroutine,
                                    void*,
                                    void*)
{
    ((ZCE_Async_Coroutine*)coroutine)->coroutine_do();
}



//�ȴ�time_out ʱ���ʱ�����ö�ʱ�����л�Э�̵�main
int ZCE_Async_Coroutine::waitfor_timeout(const ZCE_Time_Value &time_out)
{
    int ret = 0;
    ret = set_timeout(time_out);
    if (0 != ret)
    {
        return ret;
    }
    yeild_main_continue();
    return 0;
}



//=====================================================================================

//Я�����ع�����
ZCE_Async_CoroutineMgr::ZCE_Async_CoroutineMgr() :
    ZCE_Async_ObjectMgr()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

ZCE_Async_CoroutineMgr::~ZCE_Async_CoroutineMgr()
{
}

