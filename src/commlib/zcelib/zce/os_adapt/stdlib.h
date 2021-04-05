/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adopt_stdlib.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��3��3��
* @brief      stdlib.h�ļ��м��һЩ�����Ŀ�ƽ̨ʵ��
*
* @details    ecvt_r �� fcvt_r �����Ǹ���ģ�2012����ǰ����Ҳ�벻�����
*             ���Ҫ��ม���������Щ�����Ǳ���ģ�
*             ��ʵ����һЩgcvt_r��
*             qfcvt_r��qecvt_r������long double)
*
* @note
*
*/

#ifndef ZCE_OS_ADOPT_STDLIB_H_
#define ZCE_OS_ADOPT_STDLIB_H_

namespace zce
{

/*!
* @brief      ecvt���̰߳�ȫ�汾�����ڽ�һ��doubleת����һ��10����ָ����ʽ���ַ�����
* @return     int     0 �ɹ�
* @param[in]  number  ������������д����double
* @param[in]  ndigits ΪҪ�󷵻����ֵĳ��ȣ����������֣�����С����������,���fcvt_r ������β����ʹ��0Ҳ���������������������
* @param[out] decpt   ����ָ����
* @param[out] sign    ���ط���
* @param[out] buf     ���ظ�����ֵ���ַ�����ʮ���ƣ�
* @param[in]  len     �ַ����ĳ���
* @note       ע������ַ�����һ������������ַ�����
*/
int ecvt_r(double number,
           int ndigits,
           int *decpt,
           int *sign,
           char *buf,
           size_t len);

/*!
* @brief      fcvt���̰߳�ȫ�汾�����ڽ�һ��doubleת����һ��10�������ֵ��ַ�����
* @return     int      0 �ɹ�
* @param[in]  number   ������������д����double
* @param[in]  ndigits  ΪҪ�󷵻�С�������ĳ��ȣ������ecvt_r������ע����С����С����ʹ��0Ҳ�����
* @param[out] decpt    ����ָ��
* @param[out] sign     ���ط���
* @param[out] buf      ���ظ�����ֵ���ַ�����ʮ���ƣ�
* @param[in]  len      �ַ����ĳ���
* @note       ע������ַ�����һ������������ַ�����
*/
int fcvt_r(double number,
           int ndigits,
           int *decpt,
           int *sign,
           char *buf,
           size_t len);
};

#endif //ZCE_OS_ADOPT_STDLIB_H_

