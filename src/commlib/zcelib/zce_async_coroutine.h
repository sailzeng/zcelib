

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_
#define ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_


#include "zce_async_base.h"

//====================================================================================


/*!
* @brief      Э�̶���
*             ע�⣬Ϊ�˱���һЩ������ı�©�������ѡ��ļ̳з�ʽ��private
*/
class ZCE_Async_Coroutine : public ZCE_Async_Object
{
    friend class ZCE_Async_CoroutineMgr;

    //
    enum COROUTINE_STATE
    {
        //
        COROUTINE_INVALID = 0x0,
        //Я�̴��ݸ���������״ֵ̬��
        COROUTINE_CONTINUE = 0x10001,
        COROUTINE_EXIT = 0x10002,

        //��ʱ�󣬹�����֪ͨЯ�̵�״ֵ̬
        COROUTINE_TIMEOUT = 0x20002,
    };


public:
    /*!
    * @brief      ���캯����
    * @param      async_mgr ,Э���첽��������ָ��
    */
    ZCE_Async_Coroutine(ZCE_Async_ObjectMgr *async_mgr, unsigned int reg_cmd);
protected:
    /*!
    * @brief      ��������
    */
    ~ZCE_Async_Coroutine();

public:

    /*!
    * @brief      ��ʼ���������ڹ��캯������ã��ڷ������ǰִ��һ�Σ�
    * @return     int 0��ʶ�ɹ�
    */
    virtual int initialize();

    /*!
    * @brief      �������ٺ�����������ǰ�ĵ���
    * @return     int
    */
    virtual void finish();

protected:


    ///Э������,��Ҫ���صĺ���
    virtual void coroutine_run() = 0;

    ///�л���Main��Э�̻����������
    void yeild_main_continue();


    ///Э�̶�������к���
    void coroutine_do();

    ///�л���Main,Э���˳�
    void yeild_main_exit();

    ///�л���Э�̣�Ҳ�����л������Լ�����
    void yeild_coroutine();


    /*!
    * @brief      �ȴ�time_out ʱ���ʱ�����ö�ʱ�����л�Э�̵�main
    * @return     int
    * @param      time_out
    */
    int waitfor_timeout(const ZCE_Time_Value &time_out);


    /*!
    * @brief      �̳�ZCE_Async_Object�ĺ�����
    * @param[out] continue_run ���ز��������ص�ǰ��Э���Ƿ�Ҫ����������ȥ
    */
    virtual void on_run(const void *outer_data, bool &continue_run) override;

    /*!
    * @brief      �첽����ʱ����
    * @param[in]  now_time  ������ʱ��ʱ�䣬
    * @param[out] continue_run �첽�����Ƿ��������,
    */
    virtual void on_timeout(const ZCE_Time_Value &now_time,
                            bool &continue_run) override;


protected:

    ///��С�Ķ�ջ
    static const size_t MIN_STACK_SIZE = 16 * 1024;
    ///Ĭ�϶�ջ
    static const size_t DEF_STACK_SIZE = 64 * 1024;
    ///���Ķ�ջ
    static const size_t MAX_STACK_SIZE = 256 * 1024;

public:

    ///static ����������Э�����к���������Э�̶�������к���
    static void static_do(void *coroutine,void *,void *);

protected:

    ///Э�̶���
    coroutine_t   handle_;

    ///Э�̵Ķ�ջ��С��
    size_t           stack_size_ = DEF_STACK_SIZE;

    ///Э�̵�״̬
    COROUTINE_STATE  coroutine_state_ = COROUTINE_INVALID;


};

//====================================================================================

/*!
* @brief      Э�̶������ع�����
*
*/
class ZCE_Async_CoroutineMgr : public ZCE_Async_ObjectMgr
{
public:

    //
    ZCE_Async_CoroutineMgr();
    virtual ~ZCE_Async_CoroutineMgr();

protected:

    ///Ĭ���첽������ӵĳ�ʼ��������
    static const size_t COROUTINE_POOL_INIT_SIZE = 1;
    ///Ĭ�ϳ�����չ��ʱ����չ���첽���������
    static const size_t COROUTINE_POOL_EXTEND_SIZE = 16;

};


#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_COROUTINE_

