/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/string/from_string.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年5月2日
* @brief
*
* @details
*
*
*/
#pragma once

namespace zce
{
//----------------------------------------------------------------------------------------------------------

/*!
* @brief      将c str 转换为很多数值类型，作为返回值返回
* @tparam     ret_type 返回值类型
* @return     ret_type
* @param[in]  str  c string 字符串参数
*/
template<typename ret_type>
ret_type from_str(const char* str);

/*!
* @brief      从std string字符串转换得到数据类型
* @tparam     ret_type  返回的类型
* @return     ret_type  返回的数据类型
* @param[in]  str       字符串
*/
template<typename ret_type>
ret_type from_string(const std::string& stdstr)
{
    return from_str<ret_type>(stdstr.c_str());
}
}
