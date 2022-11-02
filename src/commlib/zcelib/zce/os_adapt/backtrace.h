#pragma once

namespace zce
{
/*!
* @brief
* @return     int  ==0 表示执行成功
* @param      level_num 层数
* @param      str_ary   地址信息
* @note
*/
int backtrace_stack(size_t level_num,
                    std::vector<std::string>& str_ary);

int backtrace_stack(size_t level_num,
                    FILE* stream);

int backtrace_stack(zce::LOG_PRIORITY dbg_lvl,
                    size_t level_num,
                    const char* dbg_info);
};
