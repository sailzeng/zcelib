/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encode.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��2��16��
* @brief      �������ݸ�ʽ��ת���㷨��BASE16��BASE64�ȣ�
*
* @details
*
* http://news.cnblogs.com/n/171287/
*
* ������������ֵ������������ָ���������������һ����
*
* �ϵ��Ĺ���
* ��/��������
* �����ſڵ��ϵ� ���ڻԻ͵Ĵ�ǰ
* ������˫�����սȳ�˵���
* �����Ƕ����˽� �����ϵ������
* ��˵���ڵ������� �����ǱȲ��Զ
* �����ϵ������������
* �ϱ��Ѿ����˺ü���
* �ò��� �����������Ұѹ���˵��
* �ò��� �ò���
* �ſ�ɨ�ص��ϵ� ���ž�ƿ���ǰ
* ˵����Զ��ѧ���õ�̨�廰
* ��˵�ϵ���Ա���û û������������
* ������� �ۿ����溬ס�������˵
* ��ǰ���Ҵ�����Ӣ��
* ������Ĩ����Ӣ��
* �ò��� �����������Ұѹ���˵��
* �ò��� �ò��� Oh��
* �Ұ��ҵ����¶���������
* �ò��� �ò���
* �ò��� �����������Ұѹ���˵��
* �ò��� �ò��� Oh��
*
*/



#include "zce_bytes_base.h"

namespace zce
{

#ifndef ZCE_BASE64_ENCODE_BUFFER
#define ZCE_BASE64_ENCODE_BUFFER(in_len)  ( 4 * (((inlen) + 2) / 3) + 1 )
#endif

/*!
@brief      ��һ���ڴ�����BASE64���룬ĩβ���'/0'
@return        int     ����0��ʾ�ɹ��������ʶ����һ�����ԭ����out��buffer���Ȳ���
@param[in]     in      Ҫ���б�����ڴ�
@param[in]     inlen   Ҫ���б�����ڴ泤�ȣ�
@param[out]    out     ������������ڴ棬��֤���ȴ��� ( 4 * (((inlen) + 2) / 3) + 1 )
@param[in,out] out_len ���ر����ĳ��ȣ�
@note       ע�������Ҫ�ǰ���RFC 1421�� RFC 2045�� ���ڵ����ʼ��У�����RFC 822�涨��
            ÿ76���ַ�������Ҫ����һ���س����С�
            ���ôû�У�ע����û�У����յ����ʼ���RFC 822����Ҫ��������ӻس�������
*/
int base64_encode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

#ifndef ZCE_BASE64_DECODE_BUFFER
#define ZCE_BASE64_DECODE_BUFFER(in_len)  ( (in_len) / 4 *3 )
#endif

/*!
@brief      ��һ���ڴ�����base64�Ľ���
@return     int    ����0��ʾ�ɹ�������-1��ʾ�����������Ǳ������
@param[in]     in      Ҫ���н�����ڴ�
@param[in]     in_len  Ҫ���н�����ڴ泤�ȣ�
@param[out]    out     ��������������ڴ棬����֤����Ϊ����(inlen)/4*3����
@param[in,out] out_len ���ر����ĳ��ȣ�Ϊ in_len/4*3 - N (NΪ1����2)
*/
int base64_decode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

/*!
@brief      ��һ���ڴ�����BASE16���룬ĩβ���'/0'
@return        int     ����0��ʾ�ɹ��������ʶ����һ�����ԭ����out��buffer���Ȳ���
@param[in]     in      Ҫ���б�����ڴ�
@param[in]     in_len  Ҫ���б�����ڴ泤�ȣ�
@param[out]    out     ������������ڴ棬��֤���ȴ���in_len *2 +1
@param[in,out] out_len ���ر����ĳ��ȣ�Ϊ in_len *2
*/
int base16_encode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

/*!
@brief      ��һ���ڴ�����base16�Ľ���
@return        int    ����0��ʾ�ɹ�������-1��ʾ�����������Ǳ������,����out��buffer�ռ䲻��
@param[in]     in      Ҫ���н�����ڴ�
@param[in]     inlen   Ҫ���н�����ڴ泤�ȣ�
@param[out]    out     ��������������ڴ棬����֤���ȴ���(inlen) / 2��
@param[in,out] out_len ���ر����ĳ��ȣ�Ϊin_len/2
*/
int base16_decode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

};



