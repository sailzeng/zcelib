/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_process.h
* @author     Sailzeng <sailerzeng@gmail.com>  pascalshen <pascalshen@tencent.com>
* @version
* @date       2013��1��3��
* @brief      ������ص�
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_PROCESS_H_
#define ZCE_LIB_OS_ADAPT_PROCESS_H_

#include "zce_os_adapt_predefine.h"

///@brief      �����µ�����������Ϣ��
struct ZCE_PROCESS_PERFORM
{
    //����ID
    pid_t         process_id_;

    //�������е�ʱ��
    timeval       running_time_;
    // ��������ʱ��
    timeval       start_time_;
    //����ʱ��ϵͳʱ��Ҫ��
    timeval       run_stime_;
    //����ʱ����û�ʱ��
    timeval       run_utime_;

    //���ȼ�
    int           priority_;
    //NICE����
    int           nice_;

    //ռ�õ������ڴ��С��Ҳ�������ڴ��С
    size_t        vm_size_;
    //ռ�������ڴ�Ĵ�С��Resident Set Size: number of pages the process has in real    memory.,WINDOWS�����ֵ���ɿ��������˰���Ҳû���ҵ����ʵ�ֵ
    size_t        resident_set_;

    //������ڴ�Ĵ�С,WINDOWS��û�����ֵ��Ч
    size_t        shared_size_;
    //code size ����Ĵ�С,WINDOWS��û�����ֵ��Ч
    size_t        text_size_;
    //data + stack��data + ��ջ�Ĵ�С,WINDOWS��û�����ֵ��Ч
    size_t        data_size_;

};

namespace zce
{

///�õ���ǰ�Ľ���ID
pid_t getpid();

///�õ���ǰ�Ľ��̵ĸ�����ID
pid_t getppid (void);

///fork
pid_t fork (void);

///ȡ��ĳ����������
char *getenv(const char *name);

///
pid_t setsid (void);

///signal�źŴ����������������ʵ�е����࣬�Ǻ�
sighandler_t signal (int signum,
                     sighandler_t);

//--------------------------------------------------------
//�Ǳ�׼�������õ����̵ĸ�����������

///�õ����������������Ϣ������CPUʱ�䣬�ڴ�ʹ���������Ϣ
int get_self_perf(ZCE_PROCESS_PERFORM *prc_perf_info);

///ȡ��ı�����̵�������Ϣ
int get_process_perf(pid_t process_id, ZCE_PROCESS_PERFORM *prc_perf_info);

};

#endif //ZCE_LIB_OS_ADAPT_TIME_H_

