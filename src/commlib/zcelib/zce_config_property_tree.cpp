#include "zce_predefine.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_config_property_tree.h"

//分割符,
char ZCE_Conf_PropertyTree::SEPARATOR_STRING[2] = ".";

//构造函数,析构函数
ZCE_Conf_PropertyTree::ZCE_Conf_PropertyTree()
{
}
ZCE_Conf_PropertyTree::~ZCE_Conf_PropertyTree()
{
}

//得到子树节点，const
int ZCE_Conf_PropertyTree::get_child(const std::string &path_str,
                                     const PROPERTY_TREE_NODE *& child_data) const
{
    child_data = NULL;

    //就是找自己
    if (0 == path_str.length())
    {
        child_data = this;
        return 0;
    }

    //找到以.开始的分割符号
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::const_iterator iter_tmp = child_node_.begin();
    for (; iter_tmp != child_node_.end(); ++iter_tmp)
    {
        if (iter_tmp->first == start_str)
        {
            break;
        }
    }
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        return -1;
    }

    const PROPERTY_TREE_NODE *child_tree = &(iter_tmp->second);

    //还有路径，进行递归查询
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->get_child(remain_str, child_data);
    }
    else
    {
        //这儿无非是提前返回了,
        child_data = child_tree;
        return 0;
    }
}

//得到子树节点，
int ZCE_Conf_PropertyTree::get_child(const std::string &path_str,
                                     PROPERTY_TREE_NODE *& child_data)
{
    child_data = NULL;

    //就是找自己
    if ( 0 == path_str.length() )
    {
        child_data = this;
        return 0;
    }

    //找到
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::iterator iter_tmp = child_node_.begin();
    for (; iter_tmp != child_node_.end(); ++iter_tmp)
    {
        if (iter_tmp->first == start_str)
        {
            break;
        }
    }
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        return -1;
    }

    PROPERTY_TREE_NODE *child_tree = &(iter_tmp->second);

    //还有路径，进行递归查询
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->get_child(remain_str, child_data);
    }
    else
    {
        //这儿无非是提前返回了,
        child_data = child_tree;
        return 0;
    }
}

//
int ZCE_Conf_PropertyTree::get_leafptr(const std::string &path_str,
                                       const std::string *& leaf_str_ptr) const
{
    int ret = 0;

    const PROPERTY_TREE_NODE *child_note = NULL;
    ret = get_child(path_str, child_note);
    if (0 != ret)
    {
        return ret;
    }
    

    leaf_str_ptr = &child_note->leaf_node_;

    return 0;
}

int ZCE_Conf_PropertyTree::get_leafptr(const std::string &path_str,
                                       std::string *& leaf_str_ptr)
{
    int ret = 0;

    PROPERTY_TREE_NODE *child_note = NULL;
    ret = get_child(path_str, child_note);
    if (0 != ret)
    {
        return ret;
    }


    leaf_str_ptr = &child_note->leaf_node_;

    return 0;
}

//最后还是用模板函数特化了，也许能有什么幸福生活等着我呢？

template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    std::string &val) const
{

    const std::string *leaf_str_ptr = nullptr;
    int ret = get_leafptr(path_str, leaf_str_ptr);

    if (0 != ret)
    {
        return ret;
    }

    val = *leaf_str_ptr;
    return 0;
}

//取得一个叶子节点的数据，返回值是char *
template<>
int  ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                     std::pair<char *, size_t > &str_data) const
{
    std::string value_data;
    int ret = get_leaf<std::string>(path_str, value_data);

    if (0 != ret)
    {
        return ret;
    }

    strncpy(str_data.first, value_data.c_str(), str_data.second);
    return 0;
}

//取得一个叶子节点的数据，取回数据是int,
template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    int32_t &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoi(value_str);
    return 0;
}

template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    uint32_t &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = static_cast<uint32_t>(std::stoul(value_str));
    return 0;
}



template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    int64_t &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoll(value_str);
    return 0;
}

template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    uint64_t &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoull(value_str);
    return 0;
}

//取得一个叶子节点的数据，取回数据是bool
template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    bool &val) const
{
    val = false;

    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    if (0 == strcasecmp("TRUE", value_str.c_str()))
    {
        val = true;
    }
    else if (1 == atoi(value_str.c_str()))
    {
        val = true;
    }

    return ret;
}


///取得IPV6的地址
template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    ZCE_Sockaddr_In &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.set(value_str.c_str());
    return 0;
}

///IPV6的地址
template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    ZCE_Sockaddr_In6 &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.set(value_str.c_str());
    return 0;
}


//取得时间
template<>
int ZCE_Conf_PropertyTree::get_leaf(const std::string &path_str,
                                    ZCE_Time_Value &val) const
{
    std::string value_str;
    int ret = get_leaf<std::string>(path_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.from_string(value_str.c_str(), false, ZCE_OS::TIME_STRFMT_US_SEC);
    return 0;
}

//增加一个新的CHILD,当然里面全部数据为NULL
int ZCE_Conf_PropertyTree::put_child(const std::string &path_str,
    const std::string &new_child_name,
    PROPERTY_TREE_NODE *&new_child_note)
{
    PROPERTY_TREE_NODE *child_note = NULL;
    int ret = get_child(path_str, child_note);

    if (0 != ret)
    {
        return ret;
    }
    PROPERTY_TREE_NODE null_node;
    child_note->child_node_.push_back(
        std::make_pair(new_child_name,
        null_node));

    new_child_note = &( (child_note->child_node_.rbegin())->second );

    return 0;
}




//放入一个叶子节点，
template<>
int ZCE_Conf_PropertyTree::put_leaf(const std::string &path_str,
                                    const std::string &key_str,
                                    const std::string &value_data)
{
    PROPERTY_TREE_NODE *tree_node = NULL;
    put_child(path_str, key_str, tree_node);

    tree_node->leaf_node_ = value_data;

    return 0;
}

//
template<>
int ZCE_Conf_PropertyTree::put_leaf(const std::string &path_str,
                                    const std::string &key_str,
                                    int value_int)
{
    const size_t BUF_LEN = 24;
    char str_int[BUF_LEN + 1];
    str_int[BUF_LEN] = '\0';
    snprintf(str_int, BUF_LEN, "%d", value_int);
    return put_leaf<std::string>(path_str, key_str, str_int);
}

//
template<>
int ZCE_Conf_PropertyTree::put_leaf(const std::string &path_str,
                                    const std::string &key_str,
                                    bool value_bool)
{
    if (value_bool)
    {
        return put_leaf<std::string>(path_str, key_str, "TRUE");
    }
    else
    {
        return put_leaf<std::string>(path_str, key_str, "FALSE");
    }

}

//设置分割符号
void ZCE_Conf_PropertyTree::set_separator(char separator_char)
{
    SEPARATOR_STRING[0] = separator_char;
}

