/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_async_fw_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Saturday, March 01, 2014
* @brief      һ�׺ܼ򵥵��첽��ܵĻ��࣬�Ѿ���չ����FSM��Э�̵��������࣬
*             Lua��Э����������װӦ��Ҳ���ѣ�Lua���벿�����Ѿ�û���ϰ���
*
* @details
*
*
*
* @note
*
*/


#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_
#define ZCE_LIB_ASYNC_FRAMEWORK_BASE_


#include "zce_boost_lord_rings.h"
#include "zce_os_adapt_coroutine.h"
#include "zce_timer_handler_base.h"
#include "zce_log_logging.h"
#include "zce_timer_queue_base.h"

class ZCE_Async_ObjectMgr;

/*!
* @brief      �첽����Ļ���
*
*/
class ZCE_Async_Object
{
    friend class ZCE_Async_ObjectMgr;

public:
    ZCE_Async_Object(ZCE_Async_ObjectMgr *async_mgr,
                     unsigned int create_cmd);
protected:
    virtual ~ZCE_Async_Object();

public:


    /*!
    * @brief      ��ʼ���������ڹ��캯������ã��ڷ������ǰִ��һ�Σ�
    *             Ϊʲô���������캯������Ϊ������Щ��ʼ�����в��ɹ��������
    * @return     int 0��ʶ�ɹ�
    */
    virtual int initialize();


    /*!
    * @brief      �������ٺ�����������ǰ�ĵ��ã���ʵ���Ƕ�Ӧ����
    * @return     int
    */
    virtual void finish();

    /*!
    * @brief      ��¡�Լ�
    * @return     ZCE_Async_Object*
    * @param      async_mgr
    */
    virtual ZCE_Async_Object *clone(ZCE_Async_ObjectMgr *async_mgr,
                                    unsigned int reg_cmd) = 0;

    /*!
    * @brief      �첽����ʼ,����������ÿ�����½��г�ʼ��ʱ�������
    */
    virtual void on_start();

    /*!
    * @brief      �첽��������
    * @param[out] continue_run �첽�����Ƿ�������У����������(����false)��
    */
    virtual void on_run(const void *outer_data, bool &continue_run) = 0;


    /*!
    * @brief      �첽����ʱ����
    * @param[in]  now_time  ������ʱ��ʱ�䣬
    * @param[out] continue_run �첽�����Ƿ��������,
    */
    virtual void on_timeout(const ZCE_Time_Value &now_time,
                            bool &continue_run) = 0;

    /*!
    * @brief      �첽�������н��������������ͷ���Դ������
    *             Ŀǰ���������������Ҫ������ʱ��
    */
    virtual void on_end();


    /*!
    * @brief      ��¼������̷����Ĵ���
    * @param      error_no
    */
    void set_errorno(int error_no);

    /*!
    * @brief      ������յ��ⲿ���ݵ�ָ�룬
    */
    inline void receive_data(const void *recv_data)
    {
        receive_data_ = recv_data;
    }

protected:

    /*!
    * @brief                   ���ó�ʱ��ʱ��,����������ص�����
    * @param  time_out         ��ʱ��ʱ�䣬���ʱ�䣬
    */
    int set_timeout(const ZCE_Time_Value &time_out);


    /*!
    * @brief      ȡ����ʱ��ʱ��
    */
    void cancel_timeout();



protected:

    ///TIME ID
    static const int ASYNCOBJ_ACTION_ID[2];

protected:

    ///�첽����ID
    unsigned int asyncobj_id_ = 0;

    ///������
    ZCE_Async_ObjectMgr *async_mgr_ = NULL;

    ///��Ӧ����Ĵ��������
    unsigned int  create_cmd_ = 0;

    ///��ʱ�Ķ�ʱ��ID
    int timeout_id_ = ZCE_Timer_Queue_Base::INVALID_TIMER_ID;

    /// �첽������Ĵ����룬ͳ����������ʱ��ʹ��
    int running_errno_ = 0;

    ///��־���ٵ����ȼ�
    ZCE_LOG_PRIORITY trace_log_pri_ = RS_INFO;

    ///���ݵ��ⲿ���ݣ�
    const void *receive_data_ = NULL;
};


//=======================================================================================

class ZCE_Timer_Queue_Base;

/*!
* @brief      �첽����Ĺ���������
*
*/
class ZCE_Async_ObjectMgr : public ZCE_Timer_Handler
{

protected:

    ///�첽������ӣ�
    typedef zce::lordrings<ZCE_Async_Object *>  ASYNC_OBJECT_POOL;

    ///�첽�����¼
    struct ASYNC_OBJECT_RECORD
    {
        ASYNC_OBJECT_RECORD();
        ~ASYNC_OBJECT_RECORD();

        //�첽������ӣ�
        ASYNC_OBJECT_POOL aysncobj_pool_;


        //������ͳ����Ϣ

        //���������������
        uint64_t create_num_;

        //����ʱ״̬�쳣����������
        uint64_t active_num_;

        //��������������
        uint64_t end_num_;

        ///ǿ�н���������
        uint64_t force_end_num_;

        //���й��̷�����ʱ������
        uint64_t timeout_num_;


        //����������ʱ��
        uint64_t run_consume_ms_;
    };

    //
    typedef std::unordered_map<unsigned int, ASYNC_OBJECT_RECORD> ID_TO_REGASYNC_POOL_MAP;
    //
    typedef std::unordered_map<unsigned int, ZCE_Async_Object * > RUNNING_ASYNOBJ_MAP;

public:

    ///�첽����������Ĺ��캯��
    ZCE_Async_ObjectMgr();
    virtual ~ZCE_Async_ObjectMgr();

    /*!
    * @brief      ��ʼ�������Ƹ��ֳ��ӣ������Ĵ�С
    * @return     int
    * @param      crtn_type_num
    * @param      running_number
    */
    int initialize(ZCE_Timer_Queue_Base *tq,
                   size_t crtn_type_num = DEFUALT_ASYNC_TYPE_NUM,
                   size_t running_number = DEFUALT_RUNNIG_ASYNC_SIZE);


    /*!
    * @brief      �������е�Э�̴��������ͷ���Դ��
    *             ��ӡͳ����Ϣ������Ƿ���й©��
    */
    void finish();


    /*!
    * @brief      ע��һ��Э�̣�����reg_cmd��Ӧ��
    * @return     int
    * @param      create_cmd
    * @param      async_base
    * @param      init_clone_num
    */
    int register_asyncobj(unsigned int create_cmd,
                          ZCE_Async_Object *async_base);

    /*!
    * @brief      �ж�ĳ�������Ƿ���ע�ᣨ�������첽��������
    * @return     bool
    * @param      cmd
    */
    bool is_register_cmd(unsigned int cmd);

    /*!
    * @brief      ����һ���첽����
    * @return     int
    * @param      cmd         ��������������ע������ᴴ��һ���첽������д���
    * @param      outer_data  �ⲿ���ݣ������첽���󣬸�������
    * @param      id          ���ز������ڲ������첽�����ID��
    */
    int create_asyncobj(unsigned int cmd, void *outer_data, unsigned int *id);


    /*!
    * @brief      ����ĳ���Ѿ����е��첽����,
    * @return     int
    * @param      outer_data
    * @param      id
    */
    int active_asyncobj(unsigned int id, void *outer_data );

    /*!
    * @brief      ��ӡ�������Ļ�����Ϣ������״̬
    * @param      log_priority   ��־�����ȼ����Դ����ȼ��������
    */
    void dump_info(ZCE_LOG_PRIORITY log_priority) const;

protected:

    /*!
    * @brief      �ӳ����������һ���첽����
    * @return     int
    * @param      cmd
    * @param      async_rec
    * @param      alloc_aysnc
    */
    int allocate_from_pool(unsigned int cmd,
                           ASYNC_OBJECT_RECORD *&async_rec,
                           ZCE_Async_Object *&alloc_aysnc);

    /*!
    * @brief      �ͷ�һ���첽���󵽳�������
    * @return     int
    * @param      free_async
    */
    int free_to_pool(ZCE_Async_Object *free_async);


    /*!
    * @brief      ͨ��ID��Ѱ��һ���������е��첽����
    * @return     int  ����0��ʾ�ɹ�
    * @param[in]  id   ���е��첽����ı�ʶID
    * @param[out] running_aysnc ��ѯ�����첽����
    */
    int find_running_asyncobj(unsigned int id, ZCE_Async_Object *&running_aysnc);



    /*!
    * @brief      ��ʱ�������Ĵ�����
    * @return     int
    * @param      now_time
    * @param      act
    */
    int timer_timeout(const ZCE_Time_Value &now_time,
                      const void *act);

protected:

    ///Ĭ�ϵ��첽������������
    static const size_t DEFUALT_ASYNC_TYPE_NUM = 2048;
    ///Ĭ��ͬʱ���е�һ���ֶ��������
    static const size_t DEFUALT_RUNNIG_ASYNC_SIZE = 2048;



public:
    ///��Ч������ID
    static const unsigned int INVALID_IDENTITY = 0;
    ///��Ч�ĵ�����
    static const unsigned int INVALID_COMMAND = 0;

protected:

    //����ID������
    unsigned int id_builder_;

    //Э�̵ĳ��ӣ�����ע�������
    ID_TO_REGASYNC_POOL_MAP regaysnc_pool_;

    ///�������е�Э��
    RUNNING_ASYNOBJ_MAP running_aysncobj_;

    ///�첽������ӵĳ�ʼ����С��
    size_t  pool_init_size_;

    ///�첽������ӵ�ÿ�����������
    size_t  pool_extend_size_;
};

#endif //#ifndef ZCE_LIB_ASYNC_FRAMEWORK_BASE_

