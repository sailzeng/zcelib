/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/dlfcn.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年7月9日
* @brief      DLL库操作的几个跨平台函数
*
* @details
*
*/

#ifndef ZCE_LIB_OS_ADAPT_DLFCN_H_
#define ZCE_LIB_OS_ADAPT_DLFCN_H_

#include "zce/os_adapt/common.h"

namespace zce
{
//动态库使用的四个函数

/*!
* @brief      打开动态库
* @return     ZCE_SHLIB_HANDLE 动态库的句柄，在不同的平台有不同的typdef，==NULL标识失败
* @param      fname DLL的文件名称
* @param      mode 打开的模式
*/
ZCE_SHLIB_HANDLE dlopen(const char *fname,int mode = ZCE_DEFAULT_SHLIB_MODE);

/*!
* @brief      关闭动态库
* @return     int = 0 标识成功，（WINDOWS其实这个函数没有返回值，其实释放资源永远不应失败）
* @param      handle  打开动态库时得到的句柄
*/
int dlclose(ZCE_SHLIB_HANDLE handle);

/*!
* @brief      此动态库引出函数
* @return     void* 函数指针
* @param      handle 打开动态库时得到的句柄
* @param      symbolname 函数符号的名称
* @note
*/
void *dlsym(ZCE_SHLIB_HANDLE handle,const char *symbolname);

/*!
* @brief      返回动态库函数的错误描述
* @return     const char* 错误描述
*/
const char *dlerror(void);
};

#endif //ZCE_LIB_OS_ADAPT_DLFCN_H_
