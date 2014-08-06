#include "zce_predefine.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_file.h"
#include "zce_trace_log_debug.h"
#include "zce_config_file_implement.h"

/******************************************************************************************
class ZCE_INI_Implement INI文件的配置读取，写入实现器
******************************************************************************************/
ZCE_INI_Implement::ZCE_INI_Implement()
{
}

ZCE_INI_Implement::~ZCE_INI_Implement()
{
}

//
int ZCE_INI_Implement::read(const char *file_name, ZCE_Conf_PropertyTree *propertytree)
{
    //1行的最大值
    char one_line[LINE_BUFFER_LEN + 1], str_key[LINE_BUFFER_LEN + 1], str_value[LINE_BUFFER_LEN + 1];

    one_line[LINE_BUFFER_LEN] = '\0';
    str_key[LINE_BUFFER_LEN] = '\0';
    str_value[LINE_BUFFER_LEN] = '\0';

    ZCE_Conf_PropertyTree *cur_node = NULL;

    std::ifstream cfgfile(file_name);

    //文件打不开，返回默认值
    if (!cfgfile)
    {
        return -1;
    }

    while (cfgfile)
    {

        cfgfile.getline(one_line, LINE_BUFFER_LEN);
        //整理
        ZCE_OS::strtrim(one_line);

        //注释行
        if (one_line[0] == ';' || one_line[0] == '#')
        {
            continue;
        }

        //找到一个section
        
        if (one_line[0] == '[' && one_line[strlen(one_line) - 1] == ']')
        {
            //已经找到下一个Section,没有发现相关的Key，返回默认值

            //去掉'[',']'
            memmove(one_line, one_line + 1, strlen(one_line) - 1);
            one_line[strlen(one_line) - 2] = '\0';

            //消灭空格
            ZCE_OS::strtrim(one_line);
            
            ZCE_Conf_PropertyTree *tree_node = NULL;
            propertytree->add_child(one_line, tree_node);
            cur_node = tree_node;

            continue;
        }

        char *str = strstr(one_line, "=");
        if (str != NULL && cur_node)
        {
            char *snext = str + 1;
            *str = '\0';
            strncpy(str_key, one_line, LINE_BUFFER_LEN);
            strncpy(str_value, snext, LINE_BUFFER_LEN);
            ////
            ZCE_OS::strtrim(str_key);
            ZCE_OS::strtrim(str_value);

            //找到返回。
            std::string val(str_value);
            cur_node->add_child_leaf<std::string&>(str_key, val);
        }
    }

    return 0;

}

#if defined ZCE_USE_RAPIDXML && ZCE_USE_RAPIDXML == 1

/******************************************************************************************
class ZCE_XML_Implement INI文件的配置读取，写入实现器
******************************************************************************************/
ZCE_XML_Implement::ZCE_XML_Implement()
{
}

ZCE_XML_Implement::~ZCE_XML_Implement()
{
}

int ZCE_XML_Implement::read(const char *file_name, ZCE_Conf_PropertyTree *propertytree)
{
    int ret = 0;
    size_t file_size = 0;
    ret = ZCE_OS::filelen(file_name, &file_size);
    if (0 != ret)
    {
        return 0;
    }
    size_t buf_len = file_size+16,read_len = 0;
    //只有unique_ptr 才能默认直接使用数组，
    std::unique_ptr<char[]> file_data(new char[buf_len]);
    ret = ZCE_OS::read_file_data(file_name, file_data.get(), buf_len, &read_len);
    if (0 != ret)
    {
        return 0;
    }
    try
    {
        // character type defaults to char  
        rapidxml::xml_document<char> doc;   
        //parse_non_destructive
        doc.parse<rapidxml::parse_default>(file_data.get());

        const rapidxml::xml_node<char> *root = doc.first_node();
        //广度遍历dom tree
        read_bfs(root, propertytree);
    }
    catch (rapidxml::parse_error &e)
    {
        ZCE_LOGMSG(RS_ERROR, "[ZCELIB]file [%s] don't parse error what[%s] where[%s].",
            e.what(),
            e.where<char>());
        return -1;
    }
    
    
    return 0;
}


//
void ZCE_XML_Implement::read_bfs(const rapidxml::xml_node<char> *node,
    ZCE_Conf_PropertyTree *propertytree)
{
    
    if (NULL == node->value() && NULL == node->first_attribute()  && NULL == node->first_node() )
    {
        return;
    }
    ZCE_Conf_PropertyTree *pt_note = NULL;
    propertytree->add_child(node->name(), pt_note);

    if (node->value())
    {
        pt_note->set_leaf(node->value());
    }
    if (node->first_attribute())
    {
        ZCE_Conf_PropertyTree *pt_attr = NULL;
        pt_note->add_child("<xmlattr>", pt_attr);
        rapidxml::xml_attribute<char> *node_attr = node->first_attribute();
        do 
        {
            pt_attr->add_child_leaf(node_attr->name(), node_attr->value());
            node_attr = node_attr->next_attribute();
        } while (node_attr);
    }
    //
    if (node->first_node())
    {
        rapidxml::xml_node<char> *node_child = node->first_node();
        do
        {
            ZCE_Conf_PropertyTree *pt_child = NULL;
            pt_note->add_child(node_child->name(), pt_child);
            read_bfs(node_child, pt_child);
            node_child = node_child->next_sibling();
        } while (node_child);
    }
}

#endif



