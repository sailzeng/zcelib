#include "zce_predefine.h"
#include "zce_config_property_tree.h"

//构造函数,析构函数
ZCE_Conf_PropertyTree::ZCE_Conf_PropertyTree()
{
}
ZCE_Conf_PropertyTree::~ZCE_Conf_PropertyTree()
{
}

//得到子树节点，const的，
int ZCE_Conf_PropertyTree::get_child(const std::string &path_str,
                                     const PROPERTY_TREE_NODE *& const_child_data) const
{

    const_child_data = NULL;

    //就是找自己
    if ( 0 == path_str.length() )
    {
        const_child_data = this;
        return 0;
    }

    //
    size_t str_pos =  path_str.find("|", 0);

    std::string start_str(path_str, 0, str_pos);

    PROPERTY_TREE_MAP::const_iterator iter_tmp = child_node_map_.find(start_str);

    if ( child_node_map_.end() == iter_tmp )
    {
        return -1;
    }

    const PROPERTY_TREE_NODE *child_tree = &(iter_tmp->second);

    //还有路径，进行递归查询
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->get_child(remain_str, const_child_data);
    }
    else
    {
        //这儿无非是提前返回了,
        const_child_data = child_tree;
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

    //
    size_t str_pos =  path_str.find("|", 0);

    std::string start_str(path_str, 0, str_pos);

    PROPERTY_TREE_MAP::iterator iter_tmp = child_node_map_.find(start_str);

    if ( child_node_map_.end() == iter_tmp )
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

//放入一个CHILD
int ZCE_Conf_PropertyTree::put_child(const std::string &path_str,
                                     const std::string &new_child_name,
                                     const PROPERTY_TREE_NODE &new_child_note)
{
    PROPERTY_TREE_NODE *child_note = NULL;
    int ret = get_child(path_str, child_note);

    if (0 != ret)
    {
        return ret;
    }

    PROPERTY_TREE_MAP::iterator iter = child_note->child_node_map_.insert(
                                           std::pair<std::string, PROPERTY_TREE_NODE>(new_child_name, new_child_note));

    //multimap理论上不会出现失败
    if ( child_note->child_node_map_.end() == iter )
    {
        return -1;
    }

    return 0;
}

//增加一个新的CHILD,当然里面全部数据为NULL
int ZCE_Conf_PropertyTree::new_child(const std::string &path_str,
                                     const std::string &new_child_name)
{
    PROPERTY_TREE_NODE null_node;
    return put_child(path_str, new_child_name, null_node);

}

//本来打算用模版函数,后来发现我还没有BOOST那样蛋疼。
int ZCE_Conf_PropertyTree::get_leaf_str(const std::string &path_str,
                                        const std::string &key_data,
                                        std::string &value_data) const
{

    const PROPERTY_TREE_NODE *child_note = NULL;
    int ret = get_child(path_str, child_note);

    if (0 != ret)
    {
        return ret;
    }

    KEY_VALUE_MAP::const_iterator iter = child_note->leaf_node_map_.find(key_data);

    //multimap理论上不会出现失败
    if ( child_note->leaf_node_map_.end() == iter )
    {
        return -1;
    }

    value_data = iter->second;
    return 0;
}

//取得一个叶子节点的数据，返回值是char *
int  ZCE_Conf_PropertyTree::get_leaf_str(const std::string &path_str,
                                         const std::string &key_data,
                                         char *&str_data,
                                         size_t max_str_len) const
{
    std::string value_data;
    int ret = get_leaf_str(path_str, key_data, value_data);

    if (0 != ret)
    {
        return ret;
    }

    strncpy(str_data, value_data.c_str(), max_str_len);
    return 0;
}

//取得一个叶子节点的数据，取回数据是int,
int ZCE_Conf_PropertyTree::get_leaf_int(const std::string &path_str,
                                        const std::string &key_data,
                                        int32_t &value_int) const
{
    std::string value_data;
    int ret = get_leaf_str(path_str, key_data, value_data);

    if (0 != ret)
    {
        return ret;
    }

    sscanf(value_data.c_str(), "%d", &value_int);
    return ret;
}

int ZCE_Conf_PropertyTree::get_leaf_uint(const std::string &path_str,
                                         const std::string &key_data,
                                         uint32_t &value_uint) const
{
    std::string value_data;
    int ret = get_leaf_str(path_str, key_data, value_data);

    if (0 != ret)
    {
        return ret;
    }

    sscanf(value_data.c_str(), "%u", &value_uint);
    return ret;
}

//取得一个叶子节点的数据，取回数据是bool
int ZCE_Conf_PropertyTree::get_leaf_bool(const std::string &path_str,
                                         const std::string &key_data,
                                         bool &value_bool) const
{
    value_bool = false;

    std::string value_data;
    int ret = get_leaf_str(path_str, key_data, value_data);

    if (0 != ret)
    {
        return ret;
    }

    if (0 == strcasecmp("TRUE", value_data.c_str()))
    {
        value_bool = true;
    }
    else if (1 == atoi(value_data.c_str()))
    {
        value_bool = true;
    }

    return ret;
}

//放入一个叶子节点，
int ZCE_Conf_PropertyTree::put_leaf_str(const std::string &path_str,
                                        const std::string &key_data,
                                        const std::string &value_data)
{
    PROPERTY_TREE_NODE *child_note = NULL;
    int ret = get_child(path_str, child_note);

    if (0 != ret)
    {
        return ret;
    }

    KEY_VALUE_MAP::iterator iter = child_note->leaf_node_map_.insert(
                                       std::pair<std::string, std::string>(key_data, value_data));

    //multimap理论上不会出现失败
    if ( child_note->leaf_node_map_.end() == iter )
    {
        return -1;
    }

    return 0;
}

//
int ZCE_Conf_PropertyTree::put_leaf_int(const std::string &path_str,
                                        const std::string &key_data,
                                        int value_int)
{
    const size_t BUF_LEN = 24;
    char str_int[BUF_LEN + 1];
    str_int[BUF_LEN] = '\0';
    snprintf(str_int, BUF_LEN, "%d", value_int);
    return put_leaf_str(path_str, key_data, str_int);
}

//
int ZCE_Conf_PropertyTree::put_leaf_bool(const std::string &path_str,
                                         const std::string &key_data,
                                         bool value_bool)
{
    if (value_bool)
    {
        return put_leaf_str(path_str, key_data, "TRUE");
    }
    else
    {
        return put_leaf_str(path_str, key_data, "FALSE");
    }

}

