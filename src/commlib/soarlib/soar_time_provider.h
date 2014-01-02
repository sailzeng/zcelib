/// @file       soar_time_provider.h
/// @date       2012/08/13 11:58
///
/// @author     yunfeiyang
///
/// @brief      时间提供组件
///
#ifndef SOARING_LIB_TIME_PROVIDER_H_
#define SOARING_LIB_TIME_PROVIDER_H_

// 时间提供者, 需要框架驱动更新，性能好，精度不高，主要是为了
// 满足对时间精度要求不高但又需要频繁取时间的需求
class Comm_Time_Provider
{
public:
    // 单例
    static Comm_Time_Provider *instance();
    static void clean_instance();

    // 获取当前时间
    inline time_t time() const
    {
        return cur_time_.sec();
    }

    inline const char *get_str_time() const
    {
        return str_time_[str_time_idx_];
    }

    // 获取当前时间
    inline timeval gettimeofday() const
    {
        return cur_time_;
    }

    // 框架更新时间
    void update(const ZCE_Time_Value &now);

private:
    Comm_Time_Provider();
    ~Comm_Time_Provider();
    // 当前时间
    ZCE_Time_Value cur_time_;

    static const unsigned int STR_TIME_LEN = 32;

    char str_time_[2][STR_TIME_LEN];
    // 加volatile是为了防止寄存器缓存
    volatile unsigned int str_time_idx_;

    // 单例
    static Comm_Time_Provider *instance_;
};

#endif // SOARING_LIB_TIME_PROVIDER_H_

