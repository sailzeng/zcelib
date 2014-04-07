/// @file       soar_time_provider.cpp
/// @date       2012/08/13 11:58
///
/// @author     yunfeiyang
///
/// @brief      时间提供组件
///
#include "soar_predefine.h"
#include "soar_time_provider.h"

Comm_Time_Provider *Comm_Time_Provider::instance_ = NULL;

Comm_Time_Provider *Comm_Time_Provider::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Comm_Time_Provider;
    }

    return instance_;
}

void Comm_Time_Provider::clean_instance()
{
    delete instance_;
    instance_ = NULL;
}

Comm_Time_Provider::Comm_Time_Provider()
{
    str_time_idx_ = 0;
    memset(str_time_, 0, sizeof(str_time_));
    cur_time_.gettimeofday();
    update(cur_time_);
}

Comm_Time_Provider::~Comm_Time_Provider()
{
}

void 
Comm_Time_Provider::update(const ZCE_Time_Value &now)
{
    cur_time_ = now;
    // 使用两个时间字符串，是为了避免在更新时，其它线程访问时，出现错误的字符
    // 只有一个线程在更新，其它多个线程访问更新前的字符串，不会出现乱码的情况
    unsigned int cur_idx = (str_time_idx_+1)%2;
    cur_time_.timestamp(str_time_[cur_idx], STR_TIME_LEN, 
        ZCE_OS::TIME_STRFMT_LOCALTIME|ZCE_OS::TIME_STRFFMT_ISO|ZCE_OS::TIME_STRFFMT_PRECISION_SEC);
    str_time_idx_ = cur_idx;
}

