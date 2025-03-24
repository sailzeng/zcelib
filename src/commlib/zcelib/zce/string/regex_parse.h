#pragma once

/// 用正则表达式分析一些语句，

namespace zce
{
struct ztm
{
    int tz_ = 0;    // time zone
    int year_ = 0;  // years since 0
    int mon_ = 0;       // months since January - [1, 12]
    int day_ = 0;       // day of the month - [1, 31]
    int hour_ = 0;  // hours since midnight - [0, 23]
    int min_ = 0;   // minutes after the hour - [0, 59]
    int sec_ = 0;   // seconds after the minute - [0, 60] including leap second
    int usec_ = 0;  // microseconds after the second - [0, 999999]
};

/*!
* @brief      将参数strtm的字符串，转化为tm结构的 ptr_tm 参数输出
*             时间格式比较可以做一些自适应匹配，支持 ISO 格式，美国格式，欧洲时间格式
*             2024-08-12 13:35:33.680210 +0800  （支持-，:省略，- 改为/，2024缩写为24，时间省略，时区省略）
*             Thu, 26 Dec 2009 13:50:19.680210 +0800（支持星期:省略，26 Dec 换为 Dec 26，时间省略，时区省略）
* @return     int ==0 表示成功
* @param[in]  strtm   输入的时间字符串
* @param[out] pztm    输出的ztm结构体
*/
int parse_str_to_ztm(const char* strtm,
                     zce::ztm* pztm);

/*!
* @brief      将参数timeval的值作为的时间格式化后输出打印出来，可以控制各种格式输出
* @return     int
* @param[in]  strtm    打印的时间字符串
* @param[in]  uct_time 将timeval视为UCT/GMT时间还是本地时间Local Time，true表示视为UCT/GMT时间，false表示视为本地时间
* @param[out] tval     转化得到的timeval结构体
*/
int parse_str_to_timeval(const char* strtm,
                         bool uct_time,
                         timeval* tval);
}
