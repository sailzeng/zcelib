/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_time_progress_counter.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��10��1��
* @brief      ��ʱ���ķ�װ�����ڼ�¼һЩ��ʱ�ȡ�Ҳ�������ڲ���
*             ���ܵȡ�
*
* @details    ZCE_Progress_Timer  ��ͨ��ʱ��
*             ʹ��std::clock ��ʱ������һ�㣬���Լ�¼�ϳ���ʱ�䣬
*             sleep�������ȵ���CPU�л��󣬲���Լ�ʱ����Ӱ�죬
*             ����ʱ������ȱ�ݣ�����ֻ��10ms����
*             ��һЩ�ϳ�ʱ��ļ�ʱ�У�ʹ�ýϺ�
*
*             ZCE_HR_Progress_Timer  �߾��ȵļ�ʱ��
*             ��Windows �£�ʹ��QueryPerformanceFrequency ��QueryPerformanceCounter
*             ��LINUX�£�ʹ��clock_getres��clock_gettime�õ���ȷʱ��
*             ���ȿ��Ե�ns����
*             ���ڷ���CPU�л���׼ȷ�Կ��ܳ������⣨��ʵ��һ���ı����������⣬��Զ��ԣ�
*             �ȽϺ�ʱ ���ر���Windows�汾��
*             ����һЩ���ݵģ���ȷ�ģ���Ƶ�����ܲ���
*
*             ZCE_TSC_Progress_Timer  ʹ��TSC�ļ�ʱ��
*             �ٶȷɿ�ļ�ʱ����ʹ�ü���û��ʲô���ģ����Զ��ʹ�ò�����ʲô������Ӱ��
*             ���Ⱥܸߵļ�ʱ������������CPU�л���Ƶ�ʱ仯���������ϱȽ����Σ�
*             ���ڸ�Ҫ�Եģ���ν��еģ�ÿ�β���ʱ�䲻���ļ�ʱ���ۼƣ���
*
*/
#ifndef ZCE_LIB_TIME_PROGRESS_COUNTER_H_
#define ZCE_LIB_TIME_PROGRESS_COUNTER_H_

#include "zce_boost_non_copyable.h"

/*!
* @brief      ��ʱ�� ע������һ����ʱ��������һ����ʱ�����������ڼ�¼ĳ���¼���
*             ��ʼ�ͽ���ʱ�䣬���õ�ʱ������
*             BOOST ��һ�����Ƶ���timer����progress_timer , �����ҵĴ���ֲ㣬
*             �Ҳ���װû�н������zce_boost_xxx�Ĵ�����,���Ƿŵ�������ط�
*             �ڽӿ��Ϻ�timer,progress_timer,����һЩ��ͬ��
*             ע��ײ��ʱ�õ���std::clock��(������ʵ��һЩ�鷳)��
*             ������clock_t����Windows����32λ�������Ǻ��룬�ⶫ����Windows��ֻ����49���
*             ��LINUX32�ϣ����ȿ�����΢�룬���Կ���ֻ����72���Ӷ�
*             ����������¼�������ʱ�����ֵ������ע�⣬������������������Ѿ��������˺ܶ��죬
*             ��ʱ���ʱ��ʵ����������¼�ʱ������Ȼ��������Ӧ�ú��ٳ��֣�����ȷ���������
*
* @note       �������⣬����std::clock��ʵ�֣���ʵ�����Ľ��ȿ�����10��ms��15-16����������
*             ��ֱ�����������ĳ�������ģ�һ�Σ���ʱ������׼ȷ�����ǳ���
*/
class ZCE_Progress_Timer
{

public:

    ///���캯��
    ZCE_Progress_Timer();
    ///��������
    ~ZCE_Progress_Timer();

    ///���¿�ʼ��ʱ
    void restart();
    ///������ʱ
    void end();
    ///�ۼƼ�ʱ��ʼ,���ڶ�μ�ʱ�Ĺ��̣�
    void addup_start();

    ///�������ĵ�ʱ��(s)
    double elapsed_sec() const;

public:

    //������С�ļ�ʱ���ȵ�λ��s��������ƽ̨��̫һ�£�����ΪWindows7��ǰ��WIN�Ǻ��룬Linux����2.6�Ժ������΢�룩
    static double precision();

protected:

    ///��ʼʱ��
    ///ע��CLOCKS_PER_SEC ������CPU Tickֵ������ֵ���ܲ�һ������
    std::clock_t            start_time_;
    ///����ʱ��
    std::clock_t            end_time_;
    ///�ۼ�ʱ��
    std::clock_t            addup_time_;

};


//====================================================================================================

/*!
* @brief      �߾��ȵļ�ʱ����HR high-resolution.����ĳЩʱ��Ŀ��ٵĵõ�ĳ�������ĺ�ʱ��
*             ������Ҫʹ��ѭ��N�μ���ʱ�����ַ�����
*             ��Զ��ԣ�����ʹ�ø��Ӽ򵥣����Ҿ�ȷ����һ����������
*             ��Windows �£�ʹ��QueryPerformanceFrequency ��QueryPerformanceCounter
*             ����������Ҳ��һЩ˵���������ܾ�����RDTSC��Ҳ������������ļ�ʱװ�á�
*             ���ԣ�QueryPerformanceFrequency�ķ���ֵ�ܿ��ܲ��ǹ̶�ֵ����ΪCPUƵ�ʿ����Ǳ�Ƶ��
*             http://msdn.microsoft.com/en-us/library/windows/desktop/ee417693%28v=vs.85%29.aspx
*             ��LINUX�£�ʹ��clock_getres��clock_gettime�õ���ȷʱ��,���ȿ��Ե�ns����
*             �������ԡ�
*
* @note       ���Ƚϸߣ���Ҳ����û�в��㣬��һ����ò�Ҫ�ڶ��̻߳�����ʹ�ã��ڶ�����ò�Ҫ�м���
*             sleep���������ڲ�ʹ��SetThreadAffinityMask�����˷����ַ��գ�������CPU�ı�
*             Ƶ���ƣ����Ի�������Ϊ�ã� ������QueryPerformanceCounter�ȽϺ�ʱ��ԶԶ����
*             RDTSC��������GetTickCount���м�ʮ���Ĳ�ࡣ
*/
class ZCE_HR_Progress_Timer
{

public:

    ///���캯��
    ZCE_HR_Progress_Timer();
    ///��������
    ~ZCE_HR_Progress_Timer();

    ///���¿�ʼ��ʱ
    void restart();
    ///������ʱ
    void end();
    ///�ۼƼ�ʱ��ʼ,���ڶ�μ�ʱ�Ĺ��̣�
    void addup_start();

    ///�������ĵ�ʱ��(us,΢�� -6)
    double elapsed_usec() const;

    ///�õ���ʱ���ľ��ȣ�us΢��-6����Ϊʲô�ⲻ��һ��static��������Ϊǰ���Ѿ�˵����
    ///WIN��QueryPerformanceFrequency���ܷ�����CPUƵ�ʣ���CPUƵ�ʿ��ܻ�仯
    double precision_usec();

protected:

#if defined ZCE_OS_WINDOWS

    ///��ʼʱ��
    LARGE_INTEGER      start_time_;
    ///����ʱ��
    LARGE_INTEGER      end_time_;
    ///�ۼ�ʱ��
    LARGE_INTEGER      addup_time_;
    ///Ƶ�ȣ�
    LARGE_INTEGER      frequency_;
    ///�ɵ��̵߳İ󶨴�������MASKֵ�����ں��滹ԭ
    DWORD_PTR          old_affinity_mask_;

    ///�����̰߳󶨵�CPU��MASK����Ϊ���ܲ��ԣ�Ĭ�ϰ�1��
    static const DWORD_PTR ONLY_YOU_PROCESSOR = 0x1;

#elif defined ZCE_OS_LINUX

    ///��ʼʱ��
    ::timespec         start_time_;
    ///����ʱ��
    ::timespec         end_time_;
    ///�ۼ�ʱ��
    uint64_t           addup_time_;
    //ʱ�ӵľ���
    ::timespec         precision_;

#endif
};

//====================================================================================================

/*!
* @brief      Ҳ�������һ���RDTSCʵ��һ�����档����û��������Լ��Ĵ�������Ƕ���ࡣ���
*             TSC :Time Stamp Counter   RDTSC : Read Time Stamp Counter
*             TSC��ʱ���ĳ������ڣ���Ҫ���ٵļ�ʱ��������Ӱ�����ܣ�ÿ�μ�ʱ��ʱ�䶼�ܽ϶̡�
*             ��������Ҫ������ô�ߵĵط���
*             TSC��ʱ���Ĳ��㣬��һ�������ʱ�Ĺ����з�����ʲô���������ߣ�����CPU�����飬
*             ��ʱ���ܲ�׼ȷ��������ΪTSC�ڶ�����ϲ�һ��һ�������������µ�CPU��Ƶ��仯��
*             ��¼�ο�����Ҳ����ۿۣ�
*             �����˵�������ܲ�Ҫ��Ӱ�죬���Ҿ���ֻ����Ϊ�����ο��������ʱ��Ҳ�ǲ����ѡ
*             ��
*/
class ZCE_TSC_Progress_Timer
{
public:

    ///���캯��
    ZCE_TSC_Progress_Timer();
    ///��������
    ~ZCE_TSC_Progress_Timer();

    ///���¿�ʼ��ʱ
    void restart();
    ///������ʱ
    void end();
    ///�ۼƼ�ʱ��ʼ,���ڶ�μ�ʱ�Ĺ��̣�
    void addup_start();

    ///�������ĵ�TICK��CPU���ڣ�������ע�����ֵ��ֻ�����Լ��Ļ��������ԱȲ������壬
    uint64_t elapsed_tick() const;
    ///�������ĵ�ʱ��(us),ע�������ֵ����̫׼ȷ
    double elapsed_usec() const;

protected:

    ///��ʼʱ��
    uint64_t            start_time_ = 0;
    ///����ʱ��
    uint64_t            end_time_ = 0;
    ///�ۼ�ʱ��
    uint64_t            addup_time_ = 0;

protected:

    ///CPU����Ƶ
    static  uint64_t    cpu_hz_;
};


//====================================================================================================

/*!
* @brief      ����CPP 11��std::chrono::high_resolution_clock�߾��ȼ�ʱ�����ļ�ʱ����
*
*/
class ZCE_Chrono_HR_Timer
{

public:

    ///���캯��
    ZCE_Chrono_HR_Timer();
    ///��������
    ~ZCE_Chrono_HR_Timer() = default;

    ///���¿�ʼ��ʱ
    void restart();
    ///������ʱ
    void end();
    ///�ۼƼ�ʱ��ʼ,���ڶ�μ�ʱ�Ĺ��̣�
    void addup_start();

    ///�������ĵ�ʱ��(us,΢�� -6)
    double elapsed_usec() const;

    ///����
    static double precision_usec();

protected:

    ///��ʼ��ʱ��
    std::chrono::high_resolution_clock::time_point start_time_;
    ///������ʱ��
    std::chrono::high_resolution_clock::time_point end_time_;

    ///�ۼ�ʱ��
    std::chrono::high_resolution_clock::duration addup_time_;
};

//=======================================================================================================
/*!
* @brief      ���������Զ�ֹͣ�ĵļ�ʱ��
*             �Ҳ��������������ö�󡣲��������������漸����պ�
*/
template<typename PROGRESS_TIMER>
class ZCE_Auto_Progress_Timer : public ZCE_NON_Copyable
{
public:
    ///���캯����ͬʱ��ʼ��ʱ
    ZCE_Auto_Progress_Timer()
    {
        progress_timer_.restart();
    };

    ///���������Ѻ�ʱ��ӡ����
    ~ZCE_Auto_Progress_Timer()
    {
        progress_timer_.end();
        ZCE_LOG(RS_INFO, "This operation in function[%s] use time :%.6f microseconds(usec).",
                __ZCE_FUNC__,
                progress_timer_.elapsed_usec());
    };

protected:

    PROGRESS_TIMER progress_timer_;
};

#endif //# ZCE_LIB_TIME_PROGRESS_COUNTER_H_

