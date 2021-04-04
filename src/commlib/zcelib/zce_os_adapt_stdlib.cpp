#include "zce_predefine.h"
#include "zce_os_adapt_stdlib.h"

//ecvt���̰߳�ȫ�汾�����ڽ�һ��doubleת����һ��10����ָ����ʽ���ַ�����
//ע������ַ�����һ������������ַ�����
//ndigitsΪҪ�󷵻����ֵĳ��ȣ����������֣��������fcvt_r������β����ʹ��0Ҳ�����
//���ز�����decpt����ָ����sign���ط���
int zce::ecvt_r(double number,
                int ndigits,
                int *decpt,
                int *sign,
                char *buf,
                size_t len)
{
#ifdef ZCE_OS_WINDOWS
    return ::_ecvt_s(buf,
                     len,
                     number,
                     ndigits,
                     decpt,
                     sign);
#endif

#ifdef ZCE_OS_LINUX
    return ::ecvt_r(number,
                    ndigits,
                    decpt,
                    sign,
                    buf,
                    len);
#endif
}

//fcvt���̰߳�ȫ�汾�����ڽ�һ��doubleת����һ��10�������ֵ��ַ�����
//ע������ַ�����һ������������ַ�����
//ndigitsΪҪ�󷵻�С�������ĳ��ȣ������ecvt_r������ע����С����С����ʹ��0Ҳ�����
//���ز�����decpt����ָ����sign���ط���
int zce::fcvt_r(double number,
                int ndigits,
                int *decpt,
                int *sign,
                char *buf,
                size_t len)
{
#ifdef ZCE_OS_WINDOWS
    return _fcvt_s(buf,
                   len,
                   number,
                   ndigits,
                   decpt,
                   sign);
#endif

#ifdef ZCE_OS_LINUX
    return ::fcvt_r(number,
                    ndigits,
                    decpt,
                    sign,
                    buf,
                    len);
#endif

}

