/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/string/from_str.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年5月2日
* @brief      C++ 的标准总是落后我的代码。C++ 17 新增了函数from_chars
*
* @details    对于转换函数，有一组benchmark的数据供参考，
*             https://zhuanlan.zhihu.com/p/618928075
*
*
*/
#pragma once

namespace zce
{
//----------------------------------------------------------------------------------------------------------

/*!
* @brief      将c str 转换为很多数值类型，作为返回值返回
* @tparam     T 转换的值类型
* @return     int
* @param[in]  str  c string 字符串参数
* @param[out] to_val
*/
template<typename T>
int from_str(const char* str, T& to_val);

template<typename T>
T from_str(const char* str)
{
    T to_val;
    from_str(str, to_val);
    return to_val;
}
/*!
* @brief      从std string字符串转换得到数据类型
* @tparam     ret_type  返回的类型
* @return     ret_type  返回的数据类型
* @param[in]  str       字符串
*/
template<typename T>
int from_str(const std::string& stdstr, T& to_val)
{
    return from_str<T>(stdstr.c_str(), to_val);
}
}
