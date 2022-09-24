#pragma once

#include "zce/util/non_copyable.h"
#include "zce/config/property_tree.h"

namespace zce::cfg
{
//! @brief 读取，将结果保存在PropertyTree数据结构里面
//! @param file_name
//! @param propertytree
//! @return
int read_ini(const char* file_name,
             zce::PropertyTree* propertytree);

//! 写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
int write_ini(const char* file_name,
              const zce::PropertyTree* propertytree);

/*!
* @brief      XML文件的配置读取，写入实现器
* @note
*/

//! 读取，将结果保存在ZCE_INI_PropertyTree数据结构里面
int read_xml(const char* file_name,
             zce::PropertyTree* propertytree);

//! 写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
int write_xml(const char* file_name,
              const zce::PropertyTree* propertytree);

//! @brief
//! @param file_name
//! @param data
//! @param read_line_len
//! @param word_delim
//! @return
int read_cvs(const char* file_name,
             std::vector<std::vector<std::string> > *data,
             size_t read_line_len = 64 * 1024,
             unsigned char word_delim = ',');
}
