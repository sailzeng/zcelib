
#ifndef SOARING_LIB_SERVER_CONFIG_BASE_H_
#define SOARING_LIB_SERVER_CONFIG_BASE_H_

#include "soar_services_info.h"

//============================================================================================

/*!
* @brief      PIPE����������
*
* @note
*/
struct SOAR_PIPE_CFG_DATA
{

public:
    ///�Ƿ�ָ��ܵ�
    bool if_restore_pipe_ = true;

    /// ���չܵ�����, Ĭ��50M
    uint32_t recv_pipe_len_ = 50 * 1024 * 1024;
    /// ���͹ܵ�����, Ĭ��50M
    uint32_t send_pipe_len_ = 50 * 1024 * 1024;
};

//============================================================================================
/*!
* @brief      ��־����������
*
* @note
*/
struct SOAR_LOG_CFG_DATA
{
public:

    ///������־�ļ�������
    static const size_t DEF_RESERVE_FILE_NUM = 8;

public:

    ///app��־����: @ref ZCE_LOG_PRIORITY
    ZCE_LOG_PRIORITY log_level_ = RS_DEBUG;

    ///��־�����ʽ: @ref LOG_OUTPUT_WAY ,���������ʽ�������
    uint32_t log_output_ = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT;

    // ��־�ָʽ:101����С 201��Сʱ 205����
    ZCE_LOGFILE_NAME_DEVIDE log_div_type_ = NAME_TIME_DAY_DEVIDE_TIME;

    // ��־�ļ������������������־�ļ����ᱻɾ��
    uint32_t reserve_file_num_ = DEF_RESERVE_FILE_NUM;

    // ��־�ļ�����С,��log_div_type_ �� LOGDEVIDE_BY_SIZE ʱ��Ч��
    uint32_t max_log_file_size_ = 32 * 1024 * 1024;
};


///���ڶ�ȡ����ʧ�ܼ�¼����ʧ�ܵĵص�ĵط���
#ifndef SOAR_CFG_READ_FAIL
#define SOAR_CFG_READ_FAIL(x)        ZCE_LOG(x,"[CFG]Config file read fail. code line [%s|%d],function:%s.",\
                                             __FILE__,__LINE__,__ZCE_FUNC__)
#endif

//============================================================================================
/*!
* @brief ����
*
* @note
*/
class Server_Config_Base
{

public:
    //���캯��
    Server_Config_Base();
    virtual ~Server_Config_Base();

public:

    /*!
    * @brief      ���������в���
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int read_start_arg(int argc, const char *argv[]);

    /*!
    * @brief      ���������ļ�������
    * @return     int
    */
    virtual int read_cfgfile();

    /*!
    * @brief      ��ȡ������־�������һЩ������Ϣ���Ա���ٻ���
    * @param      out_lvl �������
    */
    virtual void dump_cfg_info(ZCE_LOG_PRIORITY out_lvl);

protected:

    /// ʹ�ð���
    virtual int usage(const char *program_name);

    //����

    ///�������ж�ȡ��ܻ���������,����self_svc_id_��
    int get_common_cfg(const ZCE_Conf_PropertyTree *conf_tree);

    ///�������ж�ȡ��־������
    int get_log_cfg(const ZCE_Conf_PropertyTree *conf_tree);


public:
    //
    static const size_t MAX_ALL_TIMER_NUMBER = 1024;

public:

    /// �Լ��ķ�����ID
    SERVICES_ID self_svc_id_;

    /// ������ʵ��id
    unsigned int instance_id_ = 1;

    ///�ܵ�������
    SOAR_PIPE_CFG_DATA pipe_cfg_;

    ///��־�ʵ�����������
    SOAR_LOG_CFG_DATA log_config_;

    ///�Ƿ��̨����, windows������������˴�ֵ�����Է���ķ�ʽ����
    bool app_run_daemon_ = false;

    /// Windows���Ƿ�װ����
    bool win_install_service_ = false;
    /// Windows���Ƿ�ж�ط���
    bool win_uninstall_service_ = false;


    ///�Ƿ�ʹ�����÷����������false����ʹ�ñ�������
    bool is_use_cfgsvr_ = false;
    ///���÷�������Ϣ
    ZCE_Sockaddr_In master_cfgsvr_ip_;

    ///���Ķ�ʱ������
    size_t max_timer_nuamber_ = 1024;
    ///���ķ�Ӧ���ľ������
    size_t max_reactor_hdl_num_ = 1024;

    ///��������Ŀ¼
    std::string app_run_dir_;

    ///��־·��
    std::string log_file_prefix_;

    ///ҵ���ͨ�Ž��̹��õĵ������ļ�
    std::string common_cfg_file_;

    ///�Լ��������ļ�
    std::string app_cfg_file_;

    ///svcid�������ļ�
    std::string svc_table_file_;



};

#endif //SOARING_LIB_SERVER_CONFIG_BASE_H_

