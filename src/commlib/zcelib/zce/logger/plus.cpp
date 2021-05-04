#include "zce/predefine.h"
#include "zce/logger/log_basic.h"
#include "zce/logger/plus.h"

//单子指针
ZCE_LogTrace_Plus *ZCE_LogTrace_Plus::lpp_instance_ = NULL;

//构造函数
ZCE_LogTrace_Plus::ZCE_LogTrace_Plus()
{
}
//析构函数
ZCE_LogTrace_Plus::~ZCE_LogTrace_Plus()
{
}

//得到唯一的单子实例
ZCE_LogTrace_Plus *ZCE_LogTrace_Plus::instance()
{
    if (lpp_instance_ == NULL)
    {
        lpp_instance_ = new ZCE_LogTrace_Plus();
    }

    return lpp_instance_;
}

//赋值唯一的单子实例
void ZCE_LogTrace_Plus::instance(ZCE_LogTrace_Plus *instatnce)
{
    clean_instance();
    lpp_instance_ = instatnce;
    return;
}

//清除单子实例
void ZCE_LogTrace_Plus::clean_instance()
{
    if (lpp_instance_)
    {
        delete lpp_instance_;
    }

    lpp_instance_ = NULL;
    return;
}