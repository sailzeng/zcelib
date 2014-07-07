
#ifndef ZCE_LIB_CONFIG_INI_IMPLEMENTION_H_
#define ZCE_LIB_CONFIG_INI_IMPLEMENTION_H_

#include "zce_boost_non_copyable.h"
#include "zce_config_property_tree.h"

/******************************************************************************************
class ZCE_INI_Implemention INI文件的配置读取，写入实现器
******************************************************************************************/
class ZCE_INI_Implemention : public ZCE_NON_Copyable
{
public:
    ZCE_INI_Implemention();
    ~ZCE_INI_Implemention();

    //读取，将结果保存在ZCE_INI_PropertyTree数据结构里面
    int read(const char *file_name, ZCE_Conf_PropertyTree &propertytree);

    //写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
    int write(const char *file_name, ZCE_Conf_PropertyTree &propertytree);

protected:
    //每行的最大长度
    static const size_t LINE_BUFFER_LEN = 8192;

};

/******************************************************************************************
class ZCE_INI_PropertyTree INI文件的配置读取后存放的属性树，这个扩展主要是给一些于语法糖给大家
******************************************************************************************/

//根据INI文件的特殊性，为ZCE_Conf_PropertyTree 增加了几个函数
//增加了几个函数，方便我们进行操作
class ZCE_INI_PropertyTree : public ZCE_Conf_PropertyTree
{
public:
    //读取配置1
    int get_string_value(const std::string &sectionname,
                         const std::string &keyname,
                         std::string &value) const;

    //读取配置，
    int get_string_value(const std::string &sectionname,
                         const std::string &keyname,
                         char *buf,
                         size_t size_buf) const;
    //
    int get_int32_value(const std::string &sectionname,
                        const std::string &keyname,
                        int32_t &value) const;

    int get_uint32_value(const std::string &sectionname,
                         const std::string &keyname,
                         uint32_t &value) const;

    //
    int get_bool_value(const std::string &sectionname,
                       const std::string &keyname,
                       bool &value) const;
};

#endif //ZCE_LIB_CONFIG_INI_IMPLEMENTION_H_

