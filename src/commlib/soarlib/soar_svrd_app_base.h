
#ifndef SOARING_LIB_SERVER_APPLICATION_H_
#define SOARING_LIB_SERVER_APPLICATION_H_

#include "soar_services_info.h"
#include "soar_error_code.h"

class Server_Config_Base;
class Soar_MMAP_BusPipe;
class Server_Timer_Base;



/*!
* @brief      ���߳��Զ���ģ�͵ĳ����ܻ���
*
* @note
*/
class Soar_Svrd_Appliction : public ZCE_Server_Base
{
protected:

    ///���캯������������
    Soar_Svrd_Appliction();
    ///��������
    virtual ~Soar_Svrd_Appliction();

public:

    ///������־�����ȼ�
    void set_log_priority(ZCE_LOG_PRIORITY new_log_prio);

    ///�����־�����ȼ�
    ZCE_LOG_PRIORITY get_log_priority();

    ///�õ�APP�İ汾��Ϣ
    const char *get_app_version();


    /*!
    * @brief      ��ʼ��������һЩ�����ָ�룬
    * @return     int
    * @param      config_base ������ָ�룬ע��new�˷Ž���
    * @param      timer_base  ��ʱ�����������ָ�룬ע��new�˷Ž���
    */
    int initialize(Server_Config_Base *config_base,
                   Server_Timer_Base *timer_base);


    /*!
    * @brief      ����APP��������ָ��
    * @return     Server_Config_Base*�������ָ��
    */
    Server_Config_Base  *config_instance();

public:

    /// app�Ŀ�ʼ����
    virtual int app_start(int argc, const char *argv[]);

    /// app ����
    virtual int app_run() = 0;

    /// app���˳�
    virtual int app_exit();

protected:

    ///���¼�������
    virtual int reload_config();

    ///��ʼ����־ģ��
    int init_log();

public:

    ///ע��ʵ��ָ��
    static void set_instance(Soar_Svrd_Appliction *inst);

    ///�õ�ʵ��ָ��
    static Soar_Svrd_Appliction *instance();

    ///����ʵ��ʵ��ָ��
    static void clean_instance();

protected:

    //ʵ��ָ��
    static Soar_Svrd_Appliction *instance_;

protected:

    //ҵ��ID
    unsigned int         business_id_;
    //�Լ��ķ�����ID
    SERVICES_ID          self_svc_id_;

    //��windows�ķ���ʽ���У�
    bool                 run_as_win_serivces_;

    //�����Ϣ����
    size_t               max_msg_num_;

    ///��zerg�Ĺܵ�
    Soar_MMAP_BusPipe   *zerg_mmap_pipe_;


    ///��ܶ�ʱ��������
    Server_Timer_Base   *timer_base_;

    ///���õĴ���Ļ���
    Server_Config_Base  *config_base_;

};

#endif //SOARING_LIB_SERVER_APPLICATION_H_
