/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encode.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年2月16日
* @brief      几种数据格式的转换算法，BASE16，BASE64等，
*
* @details
*
* http://news.cnblogs.com/n/171287/
*
* 配的上人生歌手的这个名号人屈指可数，陈升算的上一个。
*
* 老爹的故事
* 词/曲：陈升
* 电梯门口的老爹 活在辉煌的从前
* 扶着那双徐蚌会战瘸了的腿
* 孩子们都不了解 讨厌老爹真多嘴
* 他说现在的年轻人 跟他们比差得远
* 我在老弟你这样的年纪
* 南北已经走了好几回
* 好不好 哪天你来听我把故事说完
* 好不好 好不好
* 门口扫地的老爹 杵着酒瓶想从前
* 说着永远都学不好的台湾话
* 他说老弟你吃饱了没 没事陪我聊聊天
* 张着嘴巴 眼眶里面含住眼泪对我说
* 以前的我带兵最英勇
* 别当我是抹布的英雄
* 好不好 哪天你来听我把故事说完
* 好不好 好不好 Oh　
* 我把我的岁月都虚掷磋跎
* 好不好 好不好
* 好不好 哪天你来听我把故事说完
* 好不好 好不好 Oh　
*
*/

#include "zce/bytes/bytes_common.h"

namespace zce
{
#ifndef ZCE_BASE64_ENCODE_BUFFER
#define ZCE_BASE64_ENCODE_BUFFER(in_len)  ( 4 * (((inlen) + 2) / 3) + 1 )
#endif

/*!
@brief      对一个内存块进行BASE64编码，末尾添加'/0'
@return        int     返回0表示成功，否则标识出错，一般出错原因是out的buffer长度不够
@param[in]     in      要进行编码的内存
@param[in]     inlen   要进行编码的内存长度，
@param[out]    out     编码后的输出的内存，保证长度大于 ( 4 * (((inlen) + 2) / 3) + 1 )
@param[in,out] out_len 返回编码后的长度，
@note       注意这儿主要是按照RFC 1421和 RFC 2045。 而在电子邮件中，根据RFC 822规定，
            每76个字符，还需要加上一个回车换行。
            这儿么没有（注意是没有）按照电子邮件（RFC 822）的要求进行增加回车操作。
*/
int base64_encode(const char* in,
                  size_t in_len,
                  char* out,
                  size_t* out_len);

#ifndef ZCE_BASE64_DECODE_BUFFER
#define ZCE_BASE64_DECODE_BUFFER(in_len)  ( (in_len) / 4 *3 )
#endif

/*!
@brief      对一个内存块进行base64的解码
@return     int    返回0表示成功，返回-1表示错误，最大可能是编码错误
@param[in]     in      要进行解码的内存
@param[in]     in_len  要进行解码的内存长度，
@param[out]    out     解码码后的输出的内存，（保证长度为大于(inlen)/4*3），
@param[in,out] out_len 返回编码后的长度，为 in_len/4*3 - N (N为1或者2)
*/
int base64_decode(const char* in,
                  size_t in_len,
                  char* out,
                  size_t* out_len);

/*!
@brief      对一个内存块进行BASE16编码，末尾添加'/0'
@return        int     返回0表示成功，否则标识出错，一般出错原因是out的buffer长度不够
@param[in]     in      要进行编码的内存
@param[in]     in_len  要进行编码的内存长度，
@param[out]    out     编码后的输出的内存，保证长度大于in_len *2 +1
@param[in,out] out_len 返回编码后的长度，为 in_len *2
*/
int base16_encode(const char* in,
                  size_t in_len,
                  char* out,
                  size_t* out_len);

/*!
@brief      对一个内存块进行base16的解码
@return        int    返回0表示成功，返回-1表示错误，最大可能是编码错误,或者out的buffer空间不够
@param[in]     in      要进行解码的内存
@param[in]     inlen   要进行解码的内存长度，
@param[out]    out     解码码后的输出的内存，（保证长度大于(inlen) / 2）
@param[in,out] out_len 返回编码后的长度，为in_len/2
*/
int base16_decode(const char* in,
                  size_t in_len,
                  char* out,
                  size_t* out_len);
};
