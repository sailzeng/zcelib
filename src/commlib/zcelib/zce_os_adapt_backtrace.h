#ifndef ZCE_LIB_OS_ADAPT_BACKTRACE_H_
#define ZCE_LIB_OS_ADAPT_BACKTRACE_H_

namespace zce
{

/*!
* @brief
* @return     int  ==0 ��ʾִ�гɹ�
* @param      str_ary
* @note
*/
int backtrace_stack(std::vector<std::string> &str_ary);

int backtrace_stack(FILE *stream);

int backtrace_stack(ZCE_LOG_PRIORITY dbg_lvl,
                    const char *dbg_info);

};

#endif //