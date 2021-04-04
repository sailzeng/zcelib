/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_server_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��3��7��
* @brief      ��������С���߼���,��Ϊ����������ʹ��
*             ���ܰ�����
*             1.���PID
              2.��ط��������ڴ棬CPUռ���ʵ�
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_SERVER_TOOLKIT_H_
#define ZCE_LIB_SERVER_TOOLKIT_H_

#include "zce_os_adapt_process.h"
#include "zce_os_adapt_sysinfo.h"
#include "zce_os_adapt_flock.h"
#include "zce_boost_non_copyable.h"


/*********************************************************************************
class ZCE_Server_Toolkit
*********************************************************************************/
class ZCE_Server_Base : public ZCE_NON_Copyable
{
protected:
    //���캯��,˽��,ʹ�õ������ʵ��,
    ZCE_Server_Base();
    ~ZCE_Server_Base();

public:

    //��ʼ��socket��Windows�µ��鷳
    int socket_init();

    /*!
    * @brief      ��ӡ���PID File
    * @return     int
    * @param      pragramname ���̵����֣����Խ�argv[0]���ݸ���,�ڲ��ᴦ��,
    * @param      lock_pid    �Ƿ��PID�ļ����м�������
    * @note
    */
    int out_pid_file(const char *pragramname);

    ///��ؽ��̵�����״̬
    int watch_dog_status(bool first_record);

    ///�����źŵƣ�����ƽ̨��һЩ���⴦��
    int process_signal(void);

    ///���黯��
    int daemon_init();


    ///���ý����Ƿ����еı�־
    void set_run_sign(bool app_run);

    /// ����reload��־
    void set_reload_sign(bool app_reload);

    ///ͨ����������0,�õ�app_base_name_��app_run_name_
    int create_app_name(const char *argv_0);


    ///�õ�������Ϣ�����ܰ���·����Ϣ
    const char *get_app_runname();

    ///�õ�����������ƣ�WINDOWS��ȥ���˺�׺
    const char *get_app_basename();


    /*!
    * @brief      windows�����÷�����Ϣ
    * @param      svc_name ��������
    * @param      svc_desc ��������
    */
    void set_service_info(const char *svc_name, const char *svc_desc);

    //�źŴ�����룬
#ifdef ZCE_OS_WINDOWS

    ///WIN ������˳�����
    static BOOL exit_signal(DWORD);

#else

    ///�˳��ź�
    static void exit_signal(int);

    ///���¼�������
    static void reload_cfg_signal(int);
#endif

    //WIN ����Ĵ��룬���ڷ�������ע��ע����
#ifdef ZCE_OS_WINDOWS
    ///���з���
    int win_services_run();

    ///��װ����
    int win_services_install();
    ///ж�ط���
    int win_services_uninstall();
    ///�������Ƿ�װ
    bool win_services_isinstalled();

    ///�������к���
    static void WINAPI win_service_main();
    ///�������̨����Ҫ�Ŀ��ƺ���
    static void WINAPI win_services_ctrl(DWORD op_code);

#endif

protected:

    //PID�ļ����ȣ���һ����󳤶ȣ�����һЩ���ȱ仯���鷳��
    static const size_t   PID_FILE_LEN = 16;

    //���ļ�¼�ڴ�й©�Ĵ���
    static const int      MAX_RECORD_MEMLEAK_NUMBER = 5;

    //�ڴ�й©����ֵ
    static const size_t   MEMORY_LEAK_THRESHOLD  = 128 * 1024 * 1024;

    //����CPU���ʹ���,��ʵ����һ��ĵ��߳�CPU�ﲻ�����ֵ
    static const size_t   PROCESS_CPU_RATIO_THRESHOLD = 600;
    //ϵͳCPU���ʹ���
    static const size_t   SYSTEM_CPU_RATIO_THRESHOLD = 750;


protected:

    //����ʵ��
    static ZCE_Server_Base *base_instance_;

protected:

    ///PID �ļ����
    ZCE_HANDLE            pid_handle_;
    ///
    zce_flock_t           pidfile_lock_;

    ///self��PID
    pid_t                 self_pid_;


    ///����״̬,�Ƿ��������
    bool                  app_run_;

    ///�Ƿ�����¼�������
    bool                  app_reload_;

    ///�������֣��׿�����Ŀ¼���ļ���׺�����֣�
    std::string           app_base_name_;
    ///������������,�������·�����У�����·����Ϣ
    std::string           app_run_name_;

    ///��������
    std::string           app_author_;

    ///��������
    std::string           service_name_;

    ///��������
    std::string           service_desc_;

public:

    ///�Ѿ���鵽���ڴ�й©�Ĵ�����ֻ��¼5��
    int                    check_leak_times_;


    ///��ʼ��ʱ�򣨻��߼����ʱ���ڴ�ĳߴ�
    size_t                 mem_checkpoint_size_;
    ///��ǰ�ڴ�ʹ��
    size_t                 cur_mem_usesize_;

    ///���̵�CPU������,ǧ����
    uint32_t               process_cpu_ratio_;
    ///ϵͳ��CPU������,ǧ����
    uint32_t               system_cpu_ratio_;

    ///�����������õ��ڴ���ڴ��С
    uint64_t               can_use_size_;
    ///ϵͳ���ڴ�ʹ����,ǧ����
    uint32_t               mem_use_ratio_;

    ///��һ�εĽ�����������
    ZCE_PROCESS_PERFORM    last_process_perf_;
    ///��ǰ��һ�εĽ�����������
    ZCE_PROCESS_PERFORM    now_process_perf_;

    ///��һ�ε�ϵͳ����������
    ZCE_SYSTEM_PERFORMANCE last_system_perf_;
    ///��ǰ��һ�ε�ϵͳ��������
    ZCE_SYSTEM_PERFORMANCE now_system_perf_;


};

#endif //_ZCE_LIB_SERVER_TOOLKIT_H_


