#include "zce_predefine.h"
#include "zce_log_basic.h"
#include "zce_log_plus.h"

//����ָ��
ZCE_LogTrace_Plus *ZCE_LogTrace_Plus::lpp_instance_ = NULL;

//���캯��
ZCE_LogTrace_Plus::ZCE_LogTrace_Plus()
{

}
//��������
ZCE_LogTrace_Plus::~ZCE_LogTrace_Plus()
{

}

//�õ�Ψһ�ĵ���ʵ��
ZCE_LogTrace_Plus *ZCE_LogTrace_Plus::instance()
{
    if (lpp_instance_ == NULL)
    {
        lpp_instance_ = new ZCE_LogTrace_Plus();
    }

    return lpp_instance_;
}

//��ֵΨһ�ĵ���ʵ��
void ZCE_LogTrace_Plus::instance(ZCE_LogTrace_Plus *instatnce)
{
    clean_instance();
    lpp_instance_ = instatnce;
    return;
}

//�������ʵ��
void ZCE_LogTrace_Plus::clean_instance()
{
    if (lpp_instance_)
    {
        delete lpp_instance_;
    }

    lpp_instance_ = NULL;
    return;
}

