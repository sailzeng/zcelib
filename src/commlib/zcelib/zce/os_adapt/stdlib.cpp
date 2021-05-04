#include "zce/predefine.h"
#include "zce/os_adapt/stdlib.h"

//ecvt的线程安全版本，用于将一个double转换成一个10进制指数方式的字符串，
//注意这个字符串是一个纯粹的数字字符串，
//ndigits为要求返回数字的长度，是所有数字，这个参数fcvt_r有区别，尾数即使是0也会输出
//返回参数，decpt返回指数，sign返回符号
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

//fcvt的线程安全版本，用于将一个double转换成一个10进制数字的字符串，
//注意这个字符串是一个纯粹的数字字符串，
//ndigits为要求返回小数个数的长度，这个和ecvt_r有区别，注意是小数，小数即使是0也会输出
//返回参数，decpt返回指数，sign返回符号
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