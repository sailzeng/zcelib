/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��11��27��
* @brief      Э�̵�OS����㣬
*
* @details    һ���򵥵�Э�̵ķ�װ
*             �ʼ�ο��������������
*             http://www.codeproject.com/Articles/4225/Unix-ucontext_t-Operations-on-Windows-Platforms
*             ��������ʵ��������ȷ��
*             1.CONTEXT����CPU�ṹ����ͬ�ģ�
*             2.Windows �µ�API GetThreadContext,SetThreadContext ����64λ�Ļ���
*               ����û���õġ�����Windows ������Wow64GetThreadContext ��
*             3.GetThreadContext�Ե�ǰ�����߳�����Ч�ģ���Ϊ��ǰ�߳��������еġ�
*             ��������ᣬ��Ͳ����л������߳��ˣ�
*
*             �ڶ��Σ���ϣ����Windows ��Fibers��ģ��Linux�µ�getcontext�Ⱥ�����������
*             ��ʵFibers��Context���б��ʲ�ͬ�ģ�Fibers�����̵߳Ķ��󣨹��̣���Context
*             ����һ����ջ���㣩������
*             0.context APIʵ����������goto��Fibers��API�������̡߳�ע�⣺context��
*             swapcontext�������������ʵswapcontext���ȱ��浱ǰ��context����һ��������
*             1.Fibers���޷�ʵ������SwitchToFiber( GetCurrentFiber() );�ĵ��ã���
*               ֻ����������һ��Fibers��
*             2.Fibers���������׶κ�SwitchToFiber �㣬������һ������getcontext�ĵ���
*               �л���ȥ����getcontext��GetCurrentFiber����һ��������
*             3.�޷���ȫ�ںϵĲ���ģ�����CreateFiber ���Լ������ջ�ģ���makecontext
*               ��������Щ���飬Ҫ���Լ��ڲ������������
*             ���ԡ���
*
*             ����ֻ��ʵ��һ��COROUTINE�ķ�װ���ҵ����ϣ����������
*             ��main���棬��������make_coroutine����Э�̣�
*             ��main���棬switch_to_coroutine�л���Э�̣�
*             ��coroutine���棬ʹ��switch_to_main�л�����Э�̣�
*
*             ���ڵȴ�C++ 20��Э�̣�����Ҹɵ���Щ���롣
*
* @note       ����Fibers������˵������ο������ĵ�������д�ķǳ������
*             ConvertFiberToThread
*             http://www.cnblogs.com/wz19860913/archive/2008/08/26/1276816.html
*
*/

#ifndef ZCE_LIB_OS_ADAPT_CORROUTINE_H_
#define ZCE_LIB_OS_ADAPT_CORROUTINE_H_

#include "zce_os_adapt_predefine.h"

#if defined ZCE_OS_WINDOWS

///coroutine_t �ڲ�����Э�̾�������ݣ�����MAIN��COROUTINE������
struct  coroutine_t
{
    coroutine_t()
    {
        main_ = NULL;
        coroutine_ = NULL;
    }

    void          *main_;
    void          *coroutine_;
};

#elif defined ZCE_OS_LINUX

struct  coroutine_t
{
    ucontext_t         main_;
    ucontext_t         coroutine_;
};

#endif

//Ϊʲô���ѡ��3�������ĺ�����Ϊ֧�ֵ����ͣ��������Ϊά�������ӣ����ұ���һֱ��Ϊ2�������㹻�ˣ�
typedef   void(*ZCE_COROUTINE_3PARA) (void *para1,
                                      void *para2,
                                      void *para3);


namespace zce
{


/*!
* @brief      �Ǳ�׼����������Э�̾����
*             ����LINUX�µ�makecontext��Windows�µ�CreateFiberExʵ
*             �֣���ע�������������context,����Э�̣�ע��
*             1.����ָ���ʶ�ĺ���ֻ����ʹ��2�����������ܺ�LINUXһ��ʹ�ñ��
*             2.LINUXԭ����::makecontext��ucontext_t����Ҫ����::getcontext��
*             ��ȡ�ģ�  ��ʹ�ô˺������ã��ڲ������ˣ�
*             3.�ڶ��������Ƕ�ջ��С���һ�������ָ������ѿռ䣬��Ϊ���COROUTINE
*             ��ջ����LINUXԭ����makecontext ��ucontext_t����ʹ��ǰҪ��ʼ����ָ
*             ��ͳ��ȣ�
*             4.�����󣬱������delete_coroutine����coroutine_t��
*             delete_coroutine��WINDOWS�µ���DeleteFiber��������Linux������
*             ��ucontext_t�����ջָ�루�Ӷ��Ϸ���Ŀռ䣩
*             5.LINUXԭ����::makecontextû�з���ֵ�����˷�װ�У���ΪWindows����
*             ������CreateFiberEx��Linux�������ڲ�������::getcontext����Щ����
*             ������ʧ��
*             6.LINUX��::makecontext����ʹ�ñ�Σ��������û�п��ǲ�������Ȼ�����
*             Ҳ���з��գ�����ʵʹ�ò���������ֲ���յģ�������������ĵ���˵����
*             http://en.wikipedia.org/wiki/Setcontext
*             http://pubs.opengroup.org/onlinepubs/009695399/functions/makecontext.html
*             ���⣬Ϊ�˰�װ�������ݣ���Windows�£����������newһ���ṹ
* @return     int ����0��ʶ�ɹ���
* @param      coroutine_hdl ucontext_t�����ɵ�CONTEXT�����
* @param      stack_size    ջ��С
* @param      back_main     Я������Ƿ񷵻�main����
* @param      fun_ptr       ����ָ�룬����3��ָ�����
* @param      para1         ָ�����1
* @param      para2         ָ�����2
* @param      para3         ָ�����3
*/
int make_coroutine(coroutine_t *coroutine_hdl,
                   size_t stack_size,
                   bool back_main,
                   ZCE_COROUTINE_3PARA fun_ptr,
                   void *para1,
                   void *para2,
                   void *para3);


/*!
* @brief      �Ǳ�׼������LINUX�µĻ����ԵĿռ�
* @return     void
* @param      coroutine_hdl  Ҫ�����ͷ�coroutine�ľ����
* @note       LINUX���ͷ����Զ������ucct�Ŀռ䣬Windows�µ��õ���DeleteFiber
*/
void delete_coroutine(coroutine_t *coroutine_hdl);


/*!
* @brief      ��Main�л���Э�̣�
* @return     int ����0��ʶ�ɹ�
* @param      coroutine_hdl
*/
int yeild_coroutine(coroutine_t *coroutine_hdl);

/*!
* @brief      ��coroutine�л���Main
* @return     int ����0��ʶ�ɹ�
* @param      coroutine_hdl
*/
int yeild_main(coroutine_t *coroutine_hdl);

/*!
* @brief      ��һ��coroutine�л�������һ��coroutine
* @return     int ����0��ʶ�ɹ�
* @param      save_hdl
* @param      goto_hdl
*/
int exchage_coroutine(coroutine_t *save_hdl,
                      coroutine_t *goto_hdl);
};

#endif //ZCE_LIB_OS_ADAPT_CORROUTINE_H_

