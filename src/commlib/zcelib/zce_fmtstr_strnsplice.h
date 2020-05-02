/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_fmtstr_strnsplice.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月6日
* @brief      字符串拼接函数
*
* @details
*
* @note
*
*/

#ifndef ZCE_FMTSTR_STRN_SPLICE_H_
#define ZCE_FMTSTR_STRN_SPLICE_H_

#include "zce_fmtstr_helper.h"

namespace ZCE_LIB
{


#ifndef FOO_FMT_STRING_USE_VARIADIC
#define FOO_FMT_STRING_USE_VARIADIC  1
#endif


//没有C++ 11的特性，没有任何用处
#if ZCE_SUPPORT_CPP11 == 0
#define FOO_FMT_STRING_USE_VARIADIC 0
#endif


#if FOO_FMT_STRING_USE_VARIADIC == 1




#else



#endif

};

#endif //ZCE_FMTSTR_STRNCPY_H_

