/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_sysinfo.h
* @author     Pascalshen <pascalshen@tencent.com>  Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��11��1��
* @brief      �õ�����ϵͳ�ĸ�����Ϣ���������ڴ��С��CPU����
*             ��LINUX�£�ʹ��/proc�ļ�ϵͳ����sysconf��systinfo ��Щ������
*             ��WINDOWS��ʹ��GlobalMemoryStatusEx��GetSystemInfo��������
*             Ϊʲô��ֱ��ģ��sysconf ��sysinfo����,ΪɶҪ�����أ�Ҳ�ǵ�ù�ߵģ�
*             ��Ϊsysinfo�Ľṹ��ʵҲ��һ�������Ľ����
*             ԭ��2.3.23ǰ��İ汾��ǰ�İ汾�ڴ�ߴ粻֧��
*
* @details    ���ں����ӿڣ���������ԥ����ԥȥ��һ��ʼ����ģ��sysconf��systinfo��
*             �������sysinfo���ڴ洦���ϱȽϱ�̬�����������˵����ӿڣ����д��һ�ᣬ
*             ��������ı�̬������ԥ�Ƿ�д��sysconf��systinfo,����ϸ����systinfo�Ľṹ��TMD���ա�
*             ���ˣ����ˣ����Լ������ṹ�ɡ���TMD����ȥ����
*
* @note       ���⣺Windows��LINUX�ڻ�ȡϵͳ��Ϣ��������Ϣ��Щ�����ϻ�������һȺ���͵�����
*             ��ҶԵ���ʲô������ϵͳ�ģ�ʲô�����ǽ��̵Ķ��ȽϺ���
*             LINUX���к���getrlimit,(ulimit�Ѿ�������),sysconf��sysinfo���ȣ�
*             Windows���к���GlobalMemoryStatusEx��GetSystemInfo��
*             ����WINDOWS������������������ܶ�ʱ���ȡ���ǲ�̫���Ƶ�̬�ȣ�������û������߳�
*             �������ƣ�û�������̸������ƣ��ȵȣ�����Ŀ���������Ϊֹ
*
*             ����һ����Ȥ�ĸ裬��������ͲԾ��ա������ģ�Ī��������̣���
*/

#ifndef ZCE_LIB_OS_ADAPT_SYSINFO_H_
#define ZCE_LIB_OS_ADAPT_SYSINFO_H_

#include "zce_os_adapt_predefine.h"

//TODO:����������һЩIO����NET��ϵͳ�������ݡ���WINDOWS�µõ���Щ���ݾ��е��鷳�ˣ�Ҫ����ע���TNNND

/// @brief ϵͳ����Ϣ���ݣ�����CPU�������ڴ��С��
struct ZCE_SYSTEM_INFO
{
    ///CPU����
    int       nprocs_conf_;
    ///��Ч��CPU����
    int       nprocs_av_;

    ///����������Ƶ
    int64_t   cpu_hz_;

    ///�ܹ����ڴ��С����λ�ֽ�
    int64_t  totalram_size_;
    ///�����ڴ�Ĵ�С����λ�ֽ�,��ʵ��LINUXϵͳ�У�����ʹ�õ��ڴ���bufferram_size_ +  freeram_size_
    int64_t  freeram_size_;

    ///�����ڴ����Ĵ�С��WINDOWS�������ֵû����
    int64_t   shared_size_;
    ///����IO�����buffer�Ĵ�С��WINDOWS�������ֵû���ã��㶨Ϊ0
    int64_t   bufferram_size_;

    ///�ܼƵĽ���������С Total swap space size
    int64_t   totalswap_size_;
    ///��Ч�Ľ���������С swap space still available
    int64_t   freeswap_size_;

};

///@brief ϵͳ��������Ϣ���ݣ�����CPU�������ڴ��С��
struct ZCE_SYSTEM_PERFORMANCE
{

    ///�ܹ����ڴ��С����λ�ֽ�
    int64_t   totalram_size_;
    ///�����ڴ�Ĵ�С����λ�ֽ�,��ʵ��LINUXϵͳ�У�����ʹ�õ��ڴ���bufferram_size_ + cachedram_size_ + freeram_size_
    int64_t   freeram_size_;

    ///�����ڴ����Ĵ�С��WINDOWS�������ֵû����
    int64_t   shared_size_;
    ///����IO�����buffer�Ĵ�С(����д)��WINDOWS�������ֵû���ã��㶨Ϊ0
    int64_t   bufferram_size_;
    ///����IO�����cached�Ĵ�С(���ڶ�ȡ)��WINDOWS�������ֵû���ã��㶨Ϊ0
    int64_t   cachedram_size_;

    ///�ܼƵĽ���������С Total swap space size
    int64_t   totalswap_size_;
    ///��Ч�Ľ���������С swap space still available
    int64_t   freeswap_size_;
    ///���������ĳߴ�
    int64_t   swapcached_size_;

    ///����������ʱ��
    timeval   up_time_;

    ///�û�̬ʱ��
    timeval   user_time_;
    ///NICEΪ�����Ľ�����ʹ�õ�CPUʱ��,��ʵ�ⲻӦ�ý�NICE TIME��Ӧ����NO NICETime��WINDOWS�������ֵû���ã��㶨Ϊ0
    ///NICEΪ����ָֻ��root�����趨��һ�����ȼ���������LINUX������ʵʱ����ϵͳ��ĳЩ���ʣ�
    timeval   nice_time_;
    ///ϵͳ̬ʱ��
    timeval   system_time_;
    ///����ʱ��
    timeval   idle_time_;
    ///iowait��ʱ�䣬WINDOWS����Ч���㶨Ϊ0
    timeval   iowait_time_;
    ///�жϵ�ʱ�䣬WINDOWS����Ч���㶨Ϊ0
    timeval   hardirq_time_;
    ///���жϵ�ʱ�䣬WINDOWS����Ч���㶨Ϊ0
    timeval   softirq_time_;

    ///ϵͳ��1���ӣ�5����15���ӵ�ƽ�����أ�WINDOWS�������ֵû����,�㶨Ϊ0.0
    double    sys_loads_[3];

    ///��ǰ�Ľ�������Number of current processes
    int       processes_num_;
    ///�������еĽ�������������WINDOWS�������ֵû���ã��㶨Ϊ0
    int       running_num_;

};

namespace zce
{

//----------------------------------------------------------------------------------------------

/*!
* @brief      �õ��ڴ�ʹ�����,
* @return     int 0�ɹ���-1ʧ��
* @param      zce_system_info  ���ص�ϵͳ��Ϣ @ref ZCE_SYSTEM_INFO
*/
int get_system_info(ZCE_SYSTEM_INFO *zce_system_info);

//
/*!
* @brief      �õ�����ϵͳ��������Ϣ
* @return     int  0�ɹ���-1ʧ��
* @param      zce_system_perf  ���ص�ϵͳ������Ϣ  @ref ZCE_SYSTEM_PERFORMANCE
*/
int get_system_perf(ZCE_SYSTEM_PERFORMANCE *zce_system_perf);

#if defined ZCE_OS_LINUX

///LINUX�¶�ȡprocĿ¼�õ�ϵͳ��������Ϣ
int read_proc_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *info);

///ͨ��ϵͳ�����õ�ϵͳ��������Ϣ���ڲ�û��ʹ�����������д����Ҫ��Ϊ����Ϥ���֪ʶ��
int read_fun_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *info);

#endif //end of #if define ZCE_OS_LINUX

};

#endif //ZCE_LIB_OS_ADAPT_SYSCONF_H_

