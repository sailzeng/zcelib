

#ifndef ZCE_LIB_ASYNC_FRAMEWORK_FSM_
#define ZCE_LIB_ASYNC_FRAMEWORK_FSM_


#include "zce_async_base.h"

//====================================================================================

/*!
* @brief      ״̬������
*/
class ZCE_Async_FSM : public ZCE_Async_Object
{
    friend class ZCE_Async_FSMMgr;


public:
    /*!
    * @brief      ���캯����
    * @param      async_mgr ,Э���첽��������ָ��
    */
    ZCE_Async_FSM(ZCE_Async_ObjectMgr *async_mgr, unsigned int create_cmd);
protected:
    /*!
    * @brief      ��������
    */
    ~ZCE_Async_FSM();


    /*!
    * @brief      ״̬�����д����̳�ZCE_Async_Object�ĺ������˺�������������Ϊ���ӽ��вο���
    * @param[in]  outer_data ����������ⲿ��������
    * @param[out] continue_run ���ز��������ص�ǰ��Э���Ƿ�Ҫ����������ȥ
    */
    virtual void on_run(const void *outer_data,
                        bool &continue_run) = 0;

    /*!
    * @brief      ״̬������ʱ����Ĭ��continue_run����false����ϵͳ���գ�
    * @param[in]  now_time  ������ʱ��ʱ�䣬
    * @param[out] continue_run �첽�����Ƿ��������,
    */
    virtual void on_timeout(const ZCE_Time_Value &now_time,
                            bool &continue_run) override;

    /*!
    * @brief      ���õ�״̬���׶Σ�
    * @param      stage
    */
    void set_stage(int stage);

    /*!
    * @brief      ȡ�õ�״̬���׶�
    * @return     int
    */
    int get_stage() const;

protected:

    ///״̬���Ľ׶�
    int               fsm_stage_;
};


//====================================================================================

/*!
* @brief      ״̬�����ع�����
*
*/
class ZCE_Async_FSMMgr : public ZCE_Async_ObjectMgr
{
public:

    //
    ZCE_Async_FSMMgr();
    virtual ~ZCE_Async_FSMMgr();

protected:


    ///Ĭ��״̬��������ӵĳ�ʼ��������,״̬����̫�����ڴ棬����׷����õ�����
    static const size_t FSM_POOL_INIT_SIZE = 4;
    ///Ĭ��״̬����չ��ʱ����չ���첽���������
    static const size_t FSM_POOL_EXTEND_SIZE = 256;

};



#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_FSM_

