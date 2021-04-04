/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_fsm_trans_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��3��29��
* @brief
*
*
* @details
*
*
*
* @note       Ϊ����TRANSACTION�Զ�ע��,�Զ�����,��TRANSACTION������MANAGER��ָ��
*             �����Ǿ�̬��,����뷨��ʵ�����˲�����Ϊ.�ò���?
*             2010��3��22��
*             �����Ҳд��4����,�ܼ��4�꣬�����ף����ҽ�����Google�뿪�й��ĵ�3�죬���Σ�������
*/

#ifndef SOARING_LIB_TRANSACTION_BASE_H_
#define SOARING_LIB_TRANSACTION_BASE_H_

#include "soar_zerg_frame.h"
#include "soar_fsm_trans_mgr.h"
#include "soar_error_code.h"

class Zerg_App_Frame;
class Soar_MMAP_BusPipe;

//���������װ������̬����ΪSTATIC��,�������������ʵ����������

class SOARING_EXPORT Transaction_Base : public ZCE_Async_FSM
{
    friend class Transaction_Manager;
protected:


    enum WAIT_TIMEOUT_VALUE
    {
        TOO_SHORT_SOARING_LIB_TIME_OUT         = 5,
        SHORT_SOARING_LIB_TIME_OUT             = 10,
        NORMAL_SOARING_LIB_TIME_OUT            = 15,
        MIDDING_SOARING_LIB_TIME_OUT           = 30,
        LONG_SOARING_LIB_TIME_OUT              = 60,
        LONG_LONG_SOARING_LIB_TIME_OUT         = 120,
        TOO_LONG_SOARING_LIB_TIME_OUT          = 240,
        TOO_TOO_LONG_TIME_OUT                  = 480,
    };

protected:

    //˽�еĹ��캯��,�����ʹ��NEW�õ�
    virtual ~Transaction_Base();

public:
    //���캯��
    Transaction_Base(Transaction_Manager *pmngr,
                     unsigned int create_cmd,
                     bool trans_locker = false);



    //�õ�������
    inline  Transaction_Manager *get_trans_mgr() const;


    ///ÿ�����½��г�ʼ��ʱ������飬�ȵȣ�������֤���������������ȵ��ã����ƹ��캯��
    ///����ط����ڻָ��ܶ��ʼֵ
    virtual void on_start();

    ///�����on_run���������ص�FSM�ģ��ѵ���ת��trans_run
    virtual void on_run(void *outer_data, bool &continue_run);

    ///״̬������ʱ�������ص�FSM�ģ��첽����ģ����ѵ���ת��trans_timeout
    virtual void on_timeout(const ZCE_Time_Value &now_time,
                            bool &continue_run);

    /*!
    * @brief      �������У��������صĺ���
    * @return     virtual void
    * @param      recv_frame
    * @param      continue_run
    */
    virtual void trans_run(Zerg_App_Frame *recv_frame,
                           bool &continue_run) = 0;

    /*!
    * @brief      ��Ĭ��continue_run����false����ϵͳ���գ�
    * @param[in]  now_time  ������ʱ��ʱ�䣬
    * @param[out] continue_run �첽�����Ƿ��������,
    */
    virtual void trans_timeout(const ZCE_Time_Value &now_time,
                               bool &continue_run);

protected:


    /*!
    * @brief      ����Frame��ʼ���õ��Է����͵���Ϣ
    * @param      recv_frame ��ʼ�����յ�FRAME����,
    */
    void create_init(Zerg_App_Frame *recv_frame);





    //lock��ʵ��������������Ŀ���Ǳ�֤��ͬһʱ�̣�ֻ����һ���û���һ������
    //�Ե�ǰ�û��ģ���ǰ���������ֽ��м���
    int lock_qquin_cmd();
    //�Ե�ǰ�û��ģ���ǰ���������ֽ��н���
    void unlock_qquin_cmd();

    //�Ե�ǰ�û���һ����ID���м���
    int lock_qquin_key(unsigned int trnas_lock_id);
    //�Ե�ǰ�û���һ����ID���н���
    void unlock_qquin_key(unsigned int trnas_lock_id);

    //�ر�����ĵ�Service
    int close_request_service() const;

    ///����û�������֡ͷ����UIN���������UIN�Ƿ�һ��,�����û�����
    int check_req_userid(unsigned int user_id) const;
    ///���ڼ�������IP��ַ�Ƿ����ڲ�IP��ַ
    int check_request_internal() const;
    ///�����ܵ���FRAME�����ݺ�����
    int check_receive_frame(const Zerg_App_Frame *recv_frame,
                            unsigned int wait_cmd);



    //DUMP���е��������Ϣ
    void dump_transa_info(std::ostringstream &strstream) const;
    //DUMP��������������Ϣ
    void output_trans_info(const char *outstr = "Output Transaction Info") const;

protected:

    //--------------------------------------------------------------------
    //�������麯����ز�Ҫ���ʹ��,���;������ڻ���,���͸���������������send_msg_to,request_send_to_proxy

    //���������ݵ�SERVICES_ID������(������Proxy),ʹ�������UIN��
    template< class T>
    int  request_send_to_peer(unsigned int cmd,
                              const SERVICES_ID &rcv_svc,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0);

    //���������ݵ�SERVICES_ID������(������Proxy),ָ���ض�UIN,������Щ����ʹ��
    template< class T>
    int request_send_to_peer(unsigned int cmd,
                             unsigned int qquin,
                             const SERVICES_ID &rcv_svc,
                             const T &info,
                             unsigned int app_id = 0,
                             unsigned int option = 0 );

    //���������ݵ�SERVICES_ID������,����ָ��UIN�ͣ����������ID
    template <class T>
    int request_send_to_peer(unsigned int cmd,
                             unsigned int qquin,
                             unsigned int backfill_trans_id,
                             const SERVICES_ID &rcv_svc,
                             const T &info,
                             unsigned int app_id = 0,
                             unsigned int option = 0
                            );

    // ���������ݵ�SERVICES_ID������(������Proxy),ָ���ض�UIN,������Щ����ʹ��
    int request_send_buf_to_peer(unsigned int cmd,
                                 unsigned int qquin,
                                 const SERVICES_ID &rcv_svc,
                                 const unsigned char *buf,
                                 unsigned int buf_len,
                                 unsigned int app_id = 0,
                                 unsigned int option = 0
                                );

    // ���������ݵ�proxy������, ָ���ض�UIN,������
    int request_send_buf_to_proxy(unsigned int cmd,
                                  unsigned int qquin,
                                  const SERVICES_ID &proxy_svc,
                                  const SERVICES_ID &recv_svc,
                                  const unsigned char *buf,
                                  unsigned int buf_len,
                                  unsigned int app_id = 0,
                                  unsigned int option = 0
                                 );

    //--------------------------------------------------------------------------------------
    //���������ݵ����������,�������USER ID����
    template< class T>
    int request_send_to_proxy(unsigned int cmd,
                              const SERVICES_ID &proxy_svc,
                              unsigned short rcvtype,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0 );

    //���������ݵ����������,��ָ��UIN����
    template< class T>
    int request_send_to_proxy(unsigned int cmd,
                              unsigned int qquin,
                              const SERVICES_ID &proxy_svc,
                              unsigned short rcvtype,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0);

    //���������ݵ����������,,ʹ�������USER ID,RCV SERVICESID(�����ô�����ת��ָ��������),
    template< class T>
    int request_send_to_proxy(unsigned int cmd,
                              unsigned int qquin,
                              const SERVICES_ID &proxy_svc,
                              const SERVICES_ID &recvsvc,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0);

    //���������ݵ����������,,ʹ�������USER ID,,RCV SERVICESID(�����ô�����ת��ָ��������),���������ID
    template< class T>
    int request_send_to_proxy(unsigned int cmd,
                              unsigned int qquin,
                              unsigned int backfill_trans_id,
                              const SERVICES_ID &proxy_svc,
                              const SERVICES_ID &recvsvc,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0);

    //--------------------------------------------------------------------------------------
    //Ӧ����ط�������,ֻ���ǳ��˽��������ʱ��ſ��Ե����������,����....
    template< class T>
    int response_sendback(unsigned int cmd,
                          const T &info,
                          unsigned int option = 0);

    //������Ϣ,Ӧ��һ������,���ǿ���ָ����Ӧ��USER ID,ֻ���ǳ��˽��������ʱ��ſ��Ե����������,����....
    //���緢���ʼ���FRAME�е��������ռ���,���Ƿ�������Ӧ���Ƿ�����
    //ΪɶҪ��2��Ϊ�������ƶ��������أ�����Կ���һ��,�Ҳ�����
    template< class T>
    int response_sendback2(unsigned int cmd,
                           unsigned int uid,
                           const T &info,
                           unsigned int option = 0);

    //Ӧ����ط�������,������һ��buf,���buf����һ�������Ŀ��Э���
    // ���������response_sendback2��������info���������buf,
    // һ��ֻ����Ҫ�ص����buf,���Լ��ⲻ����ʱ��ʹ�ã�����
    int response_buf_sendback(unsigned int cmd,
                              unsigned int uid,
                              const unsigned char *buf,
                              unsigned int buf_len,
                              unsigned int option = 0);

    //--------------------------------------------------------------------------------------
    //��Ȱ��Ҳ�Ҫ���������󼯺�,
    template< class T>
    int sendmsg_to_service(unsigned int cmd,
                           unsigned int uid,
                           unsigned int transaction_id,
                           unsigned int backfill_trans_id,
                           const SERVICES_ID &rcv_svc,
                           const SERVICES_ID &proxy_svc,
                           const SERVICES_ID &snd_svc,
                           const T &tmpvalue,
                           unsigned int app_id,
                           unsigned int option );

    // �������ݵ�SERVICES_ID������,��󼯺�, һ���÷�װ���ĺ���
    int sendbuf_to_service(unsigned int cmd,
                           unsigned int uid,
                           unsigned int transaction_id,
                           unsigned int backfill_trans_id,
                           const SERVICES_ID &rcv_svc,
                           const SERVICES_ID &proxy_svc,
                           const SERVICES_ID &snd_svc,
                           const unsigned char *buf,
                           size_t buf_len,
                           unsigned int app_id,
                           unsigned int option);



public:

    //����Է�װҪ����ϸ񣬵����Ҹо����Ⱪ¶�Ľӿ��㹻�ˡ�
protected:

    //�¼�������
    Transaction_Manager     *trans_manager_;

    //�Ƿ��������
    bool                    trans_locker_;

    //���ﱻ��������
    bool                    trans_create_;

    //���������UID
    unsigned int            req_user_id_;
    //��������������
    unsigned int            req_command_;


    //��������ķ�����
    SERVICES_ID             req_snd_service_;
    //��������Ĵ���
    SERVICES_ID             req_rcv_service_;
    //��������Ĵ���
    SERVICES_ID             req_proxy_service_;

    //���������ID
    unsigned int            req_trans_id_;
    //����ĵĻỰID:�����ǻ�������id����˼����,��ʵû��ôʹ��
    unsigned int            req_session_id_;
    //�����app_id
    unsigned int            req_game_app_id_;

    //�����IP��ַ��ʲô.
    unsigned int            req_ip_address_;
    //�����FRAME�ĵ�ѡ��
    unsigned int            req_frame_option_;

    //����ʱ�Ķ�ʱ��ID
    int                     trans_timeout_id_;
    //����active���Ƿ��Զ�ֹͣtime out��ʱ��
    bool                    active_auto_stop_;
    //����Ķ�ʱ��ID
    int                     trans_touchtimer_id_;

    //����Ĵ���ʱ��
    time_t                  trans_create_time_;

    ///��־���ٵ����ȼ�
    ZCE_LOG_PRIORITY        trace_log_pri_;

};







//��������Ϣ��Service,
template <class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option)
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              req_user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//��������Ϣ��Service,
template <class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           unsigned int qquin,
                                           unsigned int backfill_trans_id,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option)
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              backfill_trans_id,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//��ĳ��Service��������,ͬʱʹ��UIN
template< class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           unsigned int qquin,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option )
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);

}

//--------------------------------------------------------------------------------------------------------------------------
//���������ݵ����������,�������USER ID����
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            const SERVICES_ID &proxy_svc,
                                            unsigned short rcvtype,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    SERVICES_ID rcv_svc(rcvtype, 0);
    return sendmsg_to_service(cmd,
                              this->req_user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//���������ݵ����������,�ò���ָ��USER ID����
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            const SERVICES_ID &proxy_svc,
                                            unsigned short rcvtype,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    SERVICES_ID rcv_svc(rcvtype, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//���������ݵ����������,,ʹ�������USER ID,RCV SERVICESID(�����ô�����ת��ָ��������),
//ע��RECV SVC ��PROXY SVC������˳��,��Ҫ�����
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            const SERVICES_ID &proxy_svc,
                                            const SERVICES_ID &recvsvc,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//���������ݵ����������,ʹ�������USER ID,,RCV SERVICESID(�����ô�����ת��ָ��������),���������ID
//ע��RECV SVC ��PROXY SVC������˳��
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            unsigned int backfill_trans_id,
                                            const SERVICES_ID &recvsvc,
                                            const SERVICES_ID &proxy_svc,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              backfill_trans_id,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//������Ϣ,Ӧ��һ������,ֻ���ǳ��˽��������ʱ��ſ��Ե����������,����....
template< class T>
int Transaction_Base::response_sendback(unsigned int cmd,
                                        const T &info,
                                        unsigned int option)
{

    //����UDP���صĴ��벿��
    if (req_frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
    {
        option |= Zerg_App_Frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              this->req_user_id_,
                              this->asyncobj_id_,
                              this->req_trans_id_,
                              this->req_snd_service_,
                              this->req_proxy_service_,
                              this->req_rcv_service_,
                              info,
                              this->req_game_app_id_,
                              option);
}

//������Ϣ,Ӧ��һ������,���ǿ���ָ����Ӧ��USER ID,ֻ���ǳ��˽��������ʱ��ſ��Ե����������,����....
//���緢���ʼ���FRAME�е��������ռ���,���Ƿ�������Ӧ���Ƿ�����
//ΪɶҪ��2��Ϊ�������ƶ��������أ�����Կ���һ��,�Ҳ�����
template< class T>
int Transaction_Base::response_sendback2(unsigned int cmd,
                                         unsigned int qquin,
                                         const T &info,
                                         unsigned int option)
{
    //����UDP���صĴ��벿��
    if (req_frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
    {
        option |= Zerg_App_Frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              this->req_trans_id_,
                              this->req_snd_service_,
                              this->req_proxy_service_,
                              this->req_rcv_service_,
                              info,
                              this->req_game_app_id_,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//����������д������Ϣ������
template< class T>
int Transaction_Base::sendmsg_to_service(unsigned int cmd,
                                         unsigned int qquin,
                                         unsigned int transaction_id,
                                         unsigned int backfill_trans_id,
                                         const SERVICES_ID &rcv_svc,
                                         const SERVICES_ID &proxy_svc,
                                         const SERVICES_ID &snd_svc,
                                         const T &info,
                                         unsigned int app_id,
                                         unsigned int option)
{
    //������������Ҫ��Ҫ��أ�����Ĵ�����м��
    if (req_frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        option |= Zerg_App_Frame::DESC_MONITOR_TRACK;
    }

    //������͵�
    if (app_id == 0)
    {
        app_id = req_game_app_id_;
    }

    //���ù������ķ��ͺ���
    return trans_manager_->mgr_sendmsg_to_service(cmd,
                                                  qquin,
                                                  transaction_id,
                                                  backfill_trans_id,
                                                  rcv_svc,
                                                  proxy_svc,
                                                  snd_svc,
                                                  info,
                                                  app_id,
                                                  option);
}

#endif //SOARING_LIB_TRANSACTION_MANAGER_H_

