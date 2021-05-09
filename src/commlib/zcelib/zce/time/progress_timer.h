/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_time_progress_counter.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年10月1日
* @brief      计时器的封装，用于记录一些耗时等。也可以用于测量
*             性能等。
*
* @details    Progress_Timer  普通计时器
*             使用std::clock 计时，性能一般，可以记录较长的时间，
*             sleep，阻塞等导致CPU切换后，不会对计时产生影响，
*             但计时精度有缺陷，精度只有10ms级别。
*             在一些较长时间的计时中，使用较好
*
*             ZCE_HR_Progress_Timer  高精度的计时器
*             在Windows 下，使用QueryPerformanceFrequency 和QueryPerformanceCounter
*             在LINUX下，使用clock_getres和clock_gettime得到精确时钟
*             精度可以到ns级别，
*             但在发生CPU切换后，准确性可能出现问题（其实有一定的保护），另外，相对而言，
*             比较耗时 （特别是Windows版本）
*             用于一些短暂的，精确的，不频繁性能测量
*
*             ZCE_TSC_Progress_Timer  使用TSC的计时器
*             速度飞快的计时器，使用几乎没有什么消耗，所以多次使用不会有什么心理阴影。
*             精度很高的计时器，但由于在CPU切换，频率变化这类问题上比较无奈，
*             用于概要性的，多次进行的，每次操作时间不长的计时（累计）。
*
*/
#pragma once

namespace zce
{
/*!
* @brief      计时器 注意这是一个计时器，不是一个定时器，就是用于记录某个事件的
*             开始和结束时间，所用的时长的类
*             BOOST 有一个类似的类timer，和progress_timer , 考虑我的代码分层，
*             我不安装没有将其放入zce_boost_xxx的代码中,而是放到了这个地方
*             在接口上和timer,progress_timer,都有一些不同，
*             注意底层计时用的是std::clock，(所以其实有一些麻烦)，
*             长度是clock_t，在Windows下是32位，精度是毫秒，这东西在Windows上只能跑49天多
*             在LINUX32上，精度可能是微秒，所以可能只能跑72分钟多
*             别用他来记录超过这个时间的数值，而且注意，你启动的这个服务器已经可能跑了很多天，
*             这时候计时其实可能溢出导致计时错误。虽然这种事情应该很少出现，但的确有这个可能
*
* @note       精度问题，由于std::clock的实现，其实真正的进度可能在10多ms（15-16）。所以如
*             果直接拿这个测量某个函数的（一次）耗时，不会准确，误差非常大。
*/
class Progress_Timer
{
public:

    ///构造函数
    Progress_Timer();
    ///析构函数
    ~Progress_Timer();

    ///从新开始计时
    void restart();
    ///结束计时
    void end();
    ///累计计时开始,用于多次计时的过程，
    void addup_start();

    ///计算消耗的时间(s)
    double elapsed_sec() const;

public:

    //返回最小的计时精度单位（s），各个平台不太一致，（作为Windows7以前，WIN是毫秒，Linux进入2.6以后可能是微秒）
    static double precision();

protected:

    ///开始时间
    ///注意CLOCKS_PER_SEC 并不是CPU Tick值（两个值可能不一样）。
    std::clock_t            start_time_;
    ///结束时间
    std::clock_t            end_time_;
    ///累计时间
    std::clock_t            addup_time_;
};

//====================================================================================================

/*!
* @brief      高精度的计时器，HR high-resolution.用于某些时候的快速的得到某个函数的耗时，
*             而不需要使用循环N次计算时间这种方法。
*             相对而言，会让使用更加简单，而且精确度有一定的提升，
*             在Windows 下，使用QueryPerformanceFrequency 和QueryPerformanceCounter
*             这两个函数也有一些说道，他可能就是用RDTSC，也可能是用主板的计时装置。
*             所以，QueryPerformanceFrequency的返回值很可能不是固定值。因为CPU频率可能是变频。
*             http://msdn.microsoft.com/en-us/library/windows/desktop/ee417693%28v=vs.85%29.aspx
*             在LINUX下，使用clock_getres和clock_gettime得到精确时钟,精度可以到ns级别。
*             做过测试。
*
* @note       精度较高，但也不是没有不足，第一，最好不要在多线程环境下使用，第二，最好不要中间有
*             sleep操作。（内部使用SetThreadAffinityMask帮助克服部分风险，但由于CPU的变
*             频机制，所以还是慎重为好） 第三，QueryPerformanceCounter比较耗时，远远不如
*             RDTSC。甚至和GetTickCount都有几十倍的差距。
*/
class HR_Progress_Timer
{
public:

    ///构造函数
    HR_Progress_Timer();
    ///析构函数
    ~HR_Progress_Timer();

    ///从新开始计时
    void restart();
    ///结束计时
    void end();
    ///累计计时开始,用于多次计时的过程，
    void addup_start();

    ///计算消耗的时间(us,微妙 -6)
    double elapsed_usec() const;

    ///得到计时器的精度（us微秒-6），为什么这不是一个static函数？因为前面已经说过，
    ///WIN下QueryPerformanceFrequency可能返回是CPU频率，而CPU频率可能会变化
    double precision_usec();

protected:

#if defined ZCE_OS_WINDOWS

    ///开始时间
    LARGE_INTEGER      start_time_;
    ///结束时间
    LARGE_INTEGER      end_time_;
    ///累计时间
    LARGE_INTEGER      addup_time_;
    ///频度，
    LARGE_INTEGER      frequency_;
    ///旧的线程的绑定处理器的MASK值，用于后面还原
    DWORD_PTR          old_affinity_mask_;

    ///设置线程绑定的CPU的MASK，作为性能测试，默认绑定1吧
    static const DWORD_PTR ONLY_YOU_PROCESSOR = 0x1;

#elif defined ZCE_OS_LINUX

    ///开始时间
    ::timespec         start_time_;
    ///结束时间
    ::timespec         end_time_;
    ///累计时间
    uint64_t           addup_time_;
    //时钟的精度
    ::timespec         precision_;

#endif
};

//====================================================================================================

/*!
* @brief      也许那天我会用RDTSC实现一个好玩。还真没有想过在自己的代码里面嵌入汇编。吼吼
*             TSC :Time Stamp Counter   RDTSC : Read Time Stamp Counter
*             TSC计时器的场景用于，需要高速的计时操作，不影响性能，每次计时的时间都很较短。
*             但精度上要求不是那么高的地方，
*             TSC计时器的不足，第一个如果计时的过程中发生了什么阻塞，休眠，交出CPU的事情，
*             计时可能不准确，这是因为TSC在多个核上不一定一样，而且由于新的CPU主频会变化，
*             记录参考意义也会打折扣，
*             但如果说到对性能不要有影响，而且尽量只是作为测量参考，这个计时器也是不错的选
*             择。
*/
class TSC_Progress_Timer
{
public:

    ///构造函数
    TSC_Progress_Timer();
    ///析构函数
    ~TSC_Progress_Timer();

    ///从新开始计时
    void restart();
    ///结束计时
    void end();
    ///累计计时开始,用于多次计时的过程，
    void addup_start();

    ///计算消耗的TICK（CPU周期）数量，注意这个值，只能在自己的机器上做对比才有意义，
    uint64_t elapsed_tick() const;
    ///计算消耗的时间(us),注意这个数值不会太准确
    double elapsed_usec() const;

protected:

    ///开始时间
    uint64_t            start_time_ = 0;
    ///结束时间
    uint64_t            end_time_ = 0;
    ///累计时间
    uint64_t            addup_time_ = 0;

protected:

    ///CPU的主频
    static  uint64_t    cpu_hz_;
};

//====================================================================================================

/*!
* @brief      利用CPP 11的std::chrono::high_resolution_clock高精度计时器做的计时器，
*
*/
class Chrono_HR_Timer
{
public:

    ///构造函数
    Chrono_HR_Timer();
    ///析构函数
    ~Chrono_HR_Timer() = default;

    ///从新开始计时
    void restart();
    ///结束计时
    void end();
    ///累计计时开始,用于多次计时的过程，
    void addup_start();

    ///计算消耗的时间(us,微妙 -6)
    double elapsed_usec() const;

    ///精度
    static double precision_usec();

protected:

    ///开始的时间
    std::chrono::high_resolution_clock::time_point start_time_;
    ///结束的时间
    std::chrono::high_resolution_clock::time_point end_time_;

    ///累计时间
    std::chrono::high_resolution_clock::duration addup_time_;
};

//=======================================================================================================
/*!
* @brief      利用析构自动停止的的计时器
*             我不觉得这玩意作用多大。不过用来测试上面几个类凑合
*/
template<typename PROGRESS_TIMER>
class Auto_Progress_Timer: public zce::NON_Copyable
{
public:
    ///构造函数，同时开始计时
    Auto_Progress_Timer()
    {
        progress_timer_.restart();
    };

    ///析构函数把耗时打印出来
    ~Auto_Progress_Timer()
    {
        progress_timer_.end();
        ZCE_LOG(RS_INFO, "This operation in function[%s] use time :%.6f microseconds(usec).",
                __ZCE_FUNC__,
                progress_timer_.elapsed_usec());
    };

protected:

    PROGRESS_TIMER progress_timer_;
};
}
