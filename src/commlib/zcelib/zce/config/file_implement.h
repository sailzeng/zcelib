
#ifndef ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_
#define ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_

#include "zce/util/non_copyable.h"
#include "zce/config/property_tree.h"



/*!
* @brief      INI文件的配置读取，写入实现器
*
* @note
*/
class ZCE_INI_Implement : public zce::NON_Copyable
{
public:


    //读取，将结果保存在ZCE_INI_PropertyTree数据结构里面
    static int read(const char *file_name, ZCE_Conf_PropertyTree *propertytree);

    //写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
    static int write(const char *file_name, const ZCE_Conf_PropertyTree *propertytree);

protected:
    //每行的最大长度
    static const size_t LINE_BUFFER_LEN = 8191;

};




/*!
* @brief      XML文件的配置读取，写入实现器
*
* @note
*/
class ZCE_XML_Implement : public zce::NON_Copyable
{
public:
    ZCE_XML_Implement();
    ~ZCE_XML_Implement();

    //读取，将结果保存在ZCE_INI_PropertyTree数据结构里面
    static int read(const char *file_name, ZCE_Conf_PropertyTree *propertytree);

    //写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
    static int write(const char *file_name, const ZCE_Conf_PropertyTree *propertytree);

protected:
    //
    static void read_dfs(const rapidxml::xml_node<char> *note,
                         ZCE_Conf_PropertyTree *propertytree);

};


#endif //ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_

