#include "zce/predefine.h"
#include "zce/os_adapt/string.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/config/file_implement.h"

namespace zce::cfg
{
/******************************************************************************************
class zce::cfg::read_ini INI文件的配置读取，写入实现器
******************************************************************************************/
//每行的最大长度
static const size_t LINE_BUFFER_LEN = 8191;

//读取INI文件
int read_ini(const char* file_name, zce::propertytree* propertytree)
{
    //1行的最大值
    std::unique_ptr<char[]> one_line(new char[LINE_BUFFER_LEN + 1]);
    std::unique_ptr<char[]> str_key(new char[LINE_BUFFER_LEN + 1]);
    std::unique_ptr<char[]> str_value(new char[LINE_BUFFER_LEN + 1]);

    one_line[LINE_BUFFER_LEN] = '\0';
    str_key[LINE_BUFFER_LEN] = '\0';
    str_value[LINE_BUFFER_LEN] = '\0';

    zce::propertytree* cur_node = NULL;

    std::ifstream cfgfile(file_name);

    //文件打不开，返回默认值
    if (!cfgfile)
    {
        ZCE_LOG(RS_ERROR, "[zcelib]: zce::cfg::read_ini config fail.path=[%s] ,"
                "last error [%d]",
                file_name,
                zce::last_error());
        return -1;
    }

    while (cfgfile)
    {
        cfgfile.getline(one_line.get(), LINE_BUFFER_LEN);
        //整理
        zce::strtrim(one_line.get());

        //注释行
        if (one_line[0] == ';' || one_line[0] == '#')
        {
            continue;
        }

        //找到一个section
        if (one_line[0] == '[' && one_line[strlen(one_line.get()) - 1] == ']')
        {
            //已经找到下一个Section,没有发现相关的Key，返回默认值

            //去掉'[',']'
            memmove(one_line.get(), one_line.get() + 1, strlen(one_line.get()) - 1);
            one_line[strlen(one_line.get()) - 2] = '\0';

            //消灭空格
            zce::strtrim(one_line.get());

            zce::propertytree* tree_node = NULL;
            propertytree->add_child(one_line.get(), tree_node);
            cur_node = tree_node;

            continue;
        }

        char* str = strstr(one_line.get(), "=");
        if (str != NULL && cur_node)
        {
            char* snext = str + 1;
            *str = '\0';
            strncpy(str_key.get(), one_line.get(), LINE_BUFFER_LEN);
            strncpy(str_value.get(), snext, LINE_BUFFER_LEN);
            ////
            zce::strtrim(str_key.get());
            zce::strtrim(str_value.get());

            //找到返回。
            std::string val(str_value.get());
            std::string key(str_key.get());
            cur_node->set_leaf<std::string&>(key, val);
        }
    }
    return 0;
}

//写入INI文件
int write_ini(const char* file_name,
              const zce::propertytree* propertytree)
{
    //1行的最大值
    std::unique_ptr<char[]> one_line(new char[LINE_BUFFER_LEN + 1]);
    one_line[LINE_BUFFER_LEN] = '\0';

    const zce::propertytree* cur_node = NULL;
    std::ofstream cfgfile(file_name);

    //文件打不开，返回默认值
    if (!cfgfile)
    {
        ZCE_LOG(RS_ERROR, "[zcelib]: zce::cfg::write_ini config fail."
                "path=[%s] ,last error [%d]",
                file_name,
                zce::last_error());
        return -1;
    }
    auto c_iter = propertytree->child_cbegin();
    while (c_iter != propertytree->child_cend())
    {
        int len = snprintf(one_line.get(),
                           LINE_BUFFER_LEN,
                           "[%s]\n",
                           c_iter->first.c_str());
        cfgfile.write(one_line.get(), len);
        cur_node = &c_iter->second;
        auto l_iter = cur_node->leaf_cbegin();
        len = snprintf(one_line.get(),
                       LINE_BUFFER_LEN,
                       "%s=%s\n",
                       l_iter->first.c_str(),
                       l_iter->second.c_str());
        cfgfile.write(one_line.get(), len);
        ++l_iter;
        ++c_iter;
    }
    return 0;
}

#if defined ZCE_USE_RAPIDXML && ZCE_USE_RAPIDXML == 1

/******************************************************************************************
class XML_Implement INI文件的配置读取，写入实现器
******************************************************************************************/

//深度优先读写
void read_xml_dfs(const rapidxml::xml_node<char>* node,
                  zce::propertytree* propertytree)
{
    if (NULL == node->value() && NULL == node->first_attribute() &&
        NULL == node->first_node())
    {
        return;
    }
    //一些node暂时不处理
    if (node->type() == rapidxml::node_comment)
    {
        return;
    }
    zce::propertytree* pt_note = NULL;
    propertytree->add_child(node->name(), pt_note);

    if (node->value())
    {
        pt_note->set_leaf("<self_note>", node->value());
    }
    if (node->first_attribute())
    {
        rapidxml::xml_attribute<char>* node_attr = node->first_attribute();
        do
        {
            pt_note->set_leaf(node_attr->name(), node_attr->value());
            node_attr = node_attr->next_attribute();
        } while (node_attr);
    }
    //还有子节点，深度递归
    if (node->first_node())
    {
        rapidxml::xml_node<char>* node_child = node->first_node();
        do
        {
            read_xml_dfs(node_child, pt_note);
            node_child = node_child->next_sibling();
        } while (node_child);
    }
}

int read_xml(const char* file_name, zce::propertytree* propertytree)
{
    size_t file_len = 0;
    auto pair = zce::read_file(file_name, &file_len);
    if (0 != pair.first)
    {
        ZCE_LOG(RS_ERROR, "[zcelib]: XML_Implement::read fail,zce::read_file."
                "path=[%s],last error [%d]",
                file_name,
                zce::last_error());
        return pair.first;
    }
    try
    {
        // character type defaults to char
        std::unique_ptr<rapidxml::xml_document<char> >
            doc(new rapidxml::xml_document<char>);
        //parse_non_destructive
        doc->parse<rapidxml::parse_default>(pair.second.get());

        const rapidxml::xml_node<char>* root = doc->first_node();
        //广度遍历dom tree
        read_xml_dfs(root, propertytree);
    }
    catch (rapidxml::parse_error& e)
    {
        ZCE_LOG(RS_ERROR, "[ZCELIB]file [%s] don't parse error what[%s] where[%s].",
                e.what(),
                e.where<char>());
        return -1;
    }
    return 0;
}

//! 写入，暂时没有实现，实在是漏的太多，10.1期间有点贪多，
int write_xml(const char* /*file_name*/,
              const zce::propertytree* /*propertytree*/)
{
    return 0;
}

#endif

int read_cvs(const char* file_name,
             std::vector<std::vector<std::string> > *data,
             size_t read_line_len,
             unsigned char word_delim)
{
    //1行的最大值
    std::string readfile_name = file_name;
    std::unique_ptr<char[]> one_line(new char[read_line_len + 1]);

    //
    std::ifstream read_file(readfile_name.c_str(), std::ios::in | std::ios::binary);

    //先读取文件，全部读取
    if (!read_file)
    {
        ZCE_LOG(RS_ERROR, "[framework] Batch process file read fail[%s].",
                readfile_name.c_str());
        return -1;
    }
    ZCE_LOG(RS_INFO, "[framework] Batch process file [%s] open success.file .",
            readfile_name.c_str());

    //如果文件状态正确,
    while (read_file)
    {
        //
        read_file.getline(one_line.get(), read_line_len);
        std::vector<std::string> line_data;
        auto start_p = one_line.get();
        auto end_p = ::strchr(start_p, word_delim);
        while (end_p)
        {
            line_data.push_back(std::string(start_p, end_p));
            start_p = end_p + 1;
            end_p = ::strchr(start_p, word_delim);
        }
        line_data.push_back(std::string(start_p, end_p));
        data->push_back(std::move(line_data));
    }
    return 0;
}
}