
#ifndef SOARING_LIB_SERVER_CONFIG_TRANS_H_
#define SOARING_LIB_SERVER_CONFIG_TRANS_H_

#include "soar_services_info.h"
#include "soar_svrd_cfg_base.h"



struct FRAMEWORK_CONFIG
{


    struct TaskInfo
    {
    public: /* members */
        uint32_t task_thread_num_; // task�߳�����
        uint32_t task_thread_stack_size_; // task�̶߳�ջ��С
        uint32_t enqueue_timeout_sec_; // push���ݵ�task����ʱ�ĳ�ʱʱ����
        uint32_t enqueue_timeout_usec_; // push���ݵ�task����ʱ�ĳ�ʱʱ��΢��
    };

    struct TransInfo
    {
    public: /* members */
        uint32_t trans_num_; // ���������
        uint32_t trans_cmd_num_; // ���������ֵĸ���
        uint32_t func_cmd_num_; // ���������ֵĸ���
    };

    struct MonitorInfo
    {
    public: /* members */
        uint32_t filter_statics_id_cnt_; // Ҫ�ų���ͳ��ID����
        uint32_t filter_statics_id_list_[100]; // Ҫ�ų���ͳ��ID�б�
    };

    TransInfo trans_info_; // ���������
    TaskInfo task_info_; // Task�����ã�ʹ��notifytransʱ��Ч

    MonitorInfo monitor_info_; // ���������Ϣ
};

struct SVCID_CONFIG
{

};


/*!
* @brief ���ҵ�������������
*
* @note
*/
class Server_Config_FSM : public Server_Config_Base
{

protected:
    //���캯��
    Server_Config_FSM();
    virtual ~Server_Config_FSM();

public:

    /// ����zerg framwork app������
    virtual int read_cfgfile();

protected:


    /// ʹ�ð���
    virtual int usage(const char *program_name);

public:

    ///
    FRAMEWORK_CONFIG framework_config_;

    ///
    ZCE_Conf_PropertyTree framework_ptree_;

};

#endif //SOARING_LIB_SERVER_CONFIG_TRANS_H_

