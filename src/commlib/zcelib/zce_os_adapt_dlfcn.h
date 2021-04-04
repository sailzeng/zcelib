/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_dlfcn.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��7��9��
* @brief      DLL������ļ�����ƽ̨����
*
* @details
*
*/

#ifndef ZCE_LIB_OS_ADAPT_DLFCN_H_
#define ZCE_LIB_OS_ADAPT_DLFCN_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

//��̬��ʹ�õ��ĸ�����

/*!
* @brief      �򿪶�̬��
* @return     ZCE_SHLIB_HANDLE ��̬��ľ�����ڲ�ͬ��ƽ̨�в�ͬ��typdef��==NULL��ʶʧ��
* @param      fname DLL���ļ�����
* @param      mode �򿪵�ģʽ
*/
ZCE_SHLIB_HANDLE dlopen (const char *fname, int mode = ZCE_DEFAULT_SHLIB_MODE);

/*!
* @brief      �رն�̬��
* @return     int = 0 ��ʶ�ɹ�����WINDOWS��ʵ�������û�з���ֵ����ʵ�ͷ���Դ��Զ��Ӧʧ�ܣ�
* @param      handle  �򿪶�̬��ʱ�õ��ľ��
*/
int dlclose (ZCE_SHLIB_HANDLE handle);

/*!
* @brief      �˶�̬����������
* @return     void* ����ָ��
* @param      handle �򿪶�̬��ʱ�õ��ľ��
* @param      symbolname �������ŵ�����
* @note
*/
void *dlsym (ZCE_SHLIB_HANDLE handle, const char *symbolname);

/*!
* @brief      ���ض�̬�⺯���Ĵ�������
* @return     const char* ��������
*/
const char *dlerror (void);

};

#endif //ZCE_LIB_OS_ADAPT_DLFCN_H_

