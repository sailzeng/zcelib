
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_coroutine.h"


#if defined ZCE_OS_WINDOWS

///Windows��Fiberʵ��(CreateFiber)����ָ���Ӧ�Ĳ���ֻ��һ����������Ҫ�ĵĺ���ֽ����WINAPI�ģ�
///����__stdcall�ģ�����Fiberû�з��ص�contextָ��,������һ��ת����
struct _FIBERS_3PARAFUN_ADAPT
{
    ///
    coroutine_t          *handle_;
    ///�Ƿ����˳���ʱ�򷵻���Э�̣�
    bool                  exit_back_main_;
    ///����ָ��
    ZCE_COROUTINE_3PARA   fun_ptr_;

    //�����ĵ�1��������
    void                 *para1_;
    //�����ĵ�2������
    void                 *para2_;
    //�����ĵ�3������
    void                 *para3_;
};

//������ɺ�����������
VOID  WINAPI _fibers_adapt_fun (VOID *fun_para)
{



    _FIBERS_3PARAFUN_ADAPT *fun_adapt = (_FIBERS_3PARAFUN_ADAPT *)fun_para;

    coroutine_t *handle = fun_adapt->handle_;
    bool exit_back_main = fun_adapt->exit_back_main_;

    ZCE_COROUTINE_3PARA treepara_fun = fun_adapt->fun_ptr_;
    void *para1 = fun_adapt->para1_;
    void *para2 = fun_adapt->para2_;
    void *para3 = fun_adapt->para3_;

    //��������Ƕѷ���ģ�Ҫ������ͷ�
    delete fun_adapt;

    treepara_fun(para1, para2, para3);

    if (exit_back_main)
    {
        ::SwitchToFiber(handle->main_);
    }
}

#endif



//���ݷ�װ��makecontext���Ǳ�׼����������ʹ��2�������ĺ���ָ��
int zce::make_coroutine(coroutine_t *coroutine_hdl,
                        size_t stack_size,
                        bool exit_back_main,
                        ZCE_COROUTINE_3PARA fun_ptr,
                        void *para1,
                        void *para2,
                        void *para3)
{
#if defined ZCE_OS_WINDOWS

    coroutine_hdl->main_ = NULL;
    coroutine_hdl->coroutine_ = NULL;

    //�����ǰ�������˳̣�����ת����ͬʱҲ����ǰ���˳̱�ʶ
    if (FALSE == ::IsThreadAFiber())
    {
        //FIBER_FLAG_FLOAT_SWITCH XP��֧�֣����㻷���л�Ӧ�û��ʱ����һЩ��ȥ����
        coroutine_hdl->main_ = ::ConvertThreadToFiberEx(NULL,
                                                        FIBER_FLAG_FLOAT_SWITCH);
        if (NULL == coroutine_hdl->main_)
        {
            return -1;
        }
    }
    //����Ѿ����˳��ˣ��õ���ǰ�˳̵ı�ʶ
    else
    {
        coroutine_hdl->main_ = ::GetCurrentFiber();
        if (NULL == coroutine_hdl->main_)
        {
            return -1;
        }
    }


    //ʹ������ṹ��ɺ�������
    struct _FIBERS_3PARAFUN_ADAPT *fibers_adapt = new _FIBERS_3PARAFUN_ADAPT();
    fibers_adapt->exit_back_main_ = exit_back_main;
    fibers_adapt->fun_ptr_ = fun_ptr;
    fibers_adapt->para1_ = para1;
    fibers_adapt->para2_ = para2;
    fibers_adapt->para3_ = para3;


    //ע��FIBER_FLAG_FLOAT_SWITCH ��XP�ǲ���֧�ֵģ�
    coroutine_hdl->coroutine_ = ::CreateFiberEx(stack_size,
                                                stack_size,
                                                FIBER_FLAG_FLOAT_SWITCH,
                                                _fibers_adapt_fun,
                                                fibers_adapt);

    if (NULL == coroutine_hdl->coroutine_)
    {
        return -1;
    }

    fibers_adapt->handle_ = coroutine_hdl;

    return 0;
#elif defined ZCE_OS_LINUX

    //������getcontext����makecontext
    int ret = ::getcontext(&(coroutine_hdl->main_));
    if (0 != ret)
    {
        return ret;
    }
    ret = ::getcontext(&(coroutine_hdl->coroutine_));
    if (0 != ret)
    {
        return ret;
    }

    //ֻʹ��һ��������������ʹ�ñ�Σ�Windwos��֧��
    const int ONLY_3_ARG_COUNT = 3;
    if (exit_back_main)
    {
        coroutine_hdl->coroutine_.uc_link = &(coroutine_hdl->main_);
    }
    else
    {
        coroutine_hdl->coroutine_.uc_link = NULL;
    }
    coroutine_hdl->coroutine_.uc_stack.ss_sp = new char[stack_size];
    coroutine_hdl->coroutine_.uc_stack.ss_size = stack_size;

    ::makecontext(&coroutine_hdl->coroutine_,
                  (void( *)(void)) fun_ptr,
                  ONLY_3_ARG_COUNT,
                  para1,
                  para2,
                  para3
                 );
    return 0;
#endif
}

//�Ǳ�׼������
void zce::delete_coroutine(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    //
    if (coroutine_hdl->coroutine_)
    {
        ::DeleteFiber(coroutine_hdl->coroutine_);
    }
    return ;
#elif defined ZCE_OS_LINUX

    //�ͷ�zce::makecontext����Ŀռ�
    delete[](char *) coroutine_hdl->coroutine_.uc_stack.ss_sp;
    coroutine_hdl->coroutine_.uc_stack.ss_sp = NULL;
    coroutine_hdl->coroutine_.uc_stack.ss_size = 0;
#endif
}


//�л���Э��
int zce::yeild_coroutine(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == NULL) )
    {
        errno = EINVAL;
        return -1;
    }

    ::SwitchToFiber(coroutine_hdl->coroutine_);
    return ret;

#elif defined ZCE_OS_LINUX
    //ע��ѽ��swapcontext����һ������������㻳�ɣ�û��getcontext��ô��������
    //��ʵswapcontext�Ǳ����˲���1��context��Ȼ����������2��context�ģ�
    return ::swapcontext(&coroutine_hdl->main_,
                         &coroutine_hdl->coroutine_);
#endif
}


//�л���Main
int zce::yeild_main(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == NULL))
    {
        errno = EINVAL;
        return -1;
    }

    ::SwitchToFiber(coroutine_hdl->main_);
    return ret;

#elif defined ZCE_OS_LINUX

    return ::swapcontext(&coroutine_hdl->coroutine_,
                         &coroutine_hdl->main_);
#endif
}

int zce::exchage_coroutine(coroutine_t *save_hdl,
                           coroutine_t *goto_hdl)
{

#if defined ZCE_OS_WINDOWS
    //
    ZCE_UNUSED_ARG(save_hdl);
    ::SwitchToFiber(goto_hdl->coroutine_);
    return 0;

#elif defined ZCE_OS_LINUX
    return ::swapcontext(&save_hdl->coroutine_,
                         &goto_hdl->coroutine_);
#endif
}

