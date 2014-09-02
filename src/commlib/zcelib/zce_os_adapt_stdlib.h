/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adopt_stdlib.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年3月3日
* @brief      stdlib.h文件中间的一些函数的跨平台实现
*
* @details    ecvt_r 和 fcvt_r 函数是干嘛的，2012年以前，我也想不明白喔
*             如果要搞掂浮点输出，这些方法是必须的，
*             其实还有一些gcvt_r，
*             qfcvt_r，qecvt_r（用于long double)
*
* @note
*
*/

#ifndef ZCE_OS_ADOPT_STDLIB_H_
#define ZCE_OS_ADOPT_STDLIB_H_

namespace ZCE_LIB
{

/*!
* @brief      ecvt的线程安全版本，用于将一个double转换成一个10进制指数方式的字符串，
* @return     int     0 成功
* @param[in]  number  输入参数，进行处理的double
* @param[in]  ndigits 为要求返回数字的长度，是所有数字，不是小数点后的数字,这和fcvt_r 有区别，尾数即使是0也会输出，精度是四舍五入
* @param[out] decpt   返回指数，
* @param[out] sign    返回符号
* @param[out] buf     返回浮点数值的字符串，十进制，
* @param[in]  len     字符串的长度
* @note       注意这个字符串是一个纯粹的数字字符串，
*/
int ecvt_r(double number,
           int ndigits,
           int *decpt,
           int *sign,
           char *buf,
           size_t len);

/*!
* @brief      fcvt的线程安全版本，用于将一个double转换成一个10进制数字的字符串，
* @return     int      0 成功
* @param[in]  number   输入参数，进行处理的double
* @param[in]  ndigits  为要求返回小数个数的长度，这个和ecvt_r有区别，注意是小数，小数即使是0也会输出
* @param[out] decpt    返回指数
* @param[out] sign     返回符号
* @param[out] buf      返回浮点数值的字符串，十进制，
* @param[in]  len      字符串的长度
* @note       注意这个字符串是一个纯粹的数字字符串，
*/
int fcvt_r(double number,
           int ndigits,
           int *decpt,
           int *sign,
           char *buf,
           size_t len);
};

#endif //ZCE_OS_ADOPT_STDLIB_H_

