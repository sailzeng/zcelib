
#ifndef ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_
#define ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_

#include "zce_boost_non_copyable.h"
#include "zce_config_property_tree.h"



/*!
* @brief      INI�ļ������ö�ȡ��д��ʵ����
*
* @note
*/
class ZCE_INI_Implement : public ZCE_NON_Copyable
{
public:


    //��ȡ�������������ZCE_INI_PropertyTree���ݽṹ����
    static int read(const char *file_name, ZCE_Conf_PropertyTree *propertytree);

    //д�룬��ʱû��ʵ�֣�ʵ����©��̫�࣬10.1�ڼ��е�̰�࣬
    static int write(const char *file_name, const ZCE_Conf_PropertyTree *propertytree);

protected:
    //ÿ�е���󳤶�
    static const size_t LINE_BUFFER_LEN = 8191;

};




/*!
* @brief      XML�ļ������ö�ȡ��д��ʵ����
*
* @note
*/
class ZCE_XML_Implement : public ZCE_NON_Copyable
{
public:
    ZCE_XML_Implement();
    ~ZCE_XML_Implement();

    //��ȡ�������������ZCE_INI_PropertyTree���ݽṹ����
    static int read(const char *file_name, ZCE_Conf_PropertyTree *propertytree);

    //д�룬��ʱû��ʵ�֣�ʵ����©��̫�࣬10.1�ڼ��е�̰�࣬
    static int write(const char *file_name, const ZCE_Conf_PropertyTree *propertytree);

protected:
    //
    static void read_dfs(const rapidxml::xml_node<char> *note,
                         ZCE_Conf_PropertyTree *propertytree);

};


#endif //ZCE_LIB_CONFIG_FILE_IMPLEMENTION_H_

