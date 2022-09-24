#include "zce/predefine.h"
#include "zce/socket/addr_in.h"
#include "zce/socket/addr_in6.h"
#include "zce/logger/logging.h"
#include "zce/config/property_tree.h"

namespace zce
{
//分割符,
char propertytree::SEPARATOR_STRING[2] = ".";

//得到子树节点，
int propertytree::path_get_childiter(const std::string& path_str,
                                     propertytree::child_iterator& child_iter)
{
    //找到
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::iterator iter_tmp = child_node_.find(start_str);
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
                __ZCE_FUNC__,
                path_str.c_str());
        return -1;
    }

    propertytree* child_tree = &(iter_tmp->second);

    //还有路径，进行递归查询
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->path_get_childiter(remain_str, child_iter);
    }
    else
    {
        //这儿无非是提前返回了,
        child_iter = iter_tmp;
        return 0;
    }
}

//得到子树节点，const
int propertytree::path_get_childiter(const std::string& path_str,
                                     propertytree::const_child_iterator& child_iter) const
{
    //找到
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);
    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::const_iterator iter_tmp = child_node_.find(start_str);
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] .",
                __ZCE_FUNC__,
                path_str.c_str());
        return -1;
    }

    const propertytree* child_tree = &(iter_tmp->second);

    //还有路径，进行递归查询
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->path_get_childiter(remain_str, child_iter);
    }
    else
    {
        //这儿无非是提前返回了,
        child_iter = iter_tmp;
        return 0;
    }
}

//取得叶子节点的迭代器
int propertytree::path_get_leafiter(const std::string& path_str,
                                    const std::string& key_str,
                                    propertytree::leaf_iterator& leaf_iter)
{
    int ret = 0;

    propertytree::child_iterator child_iter;
    ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
                __ZCE_FUNC__,
                path_str.c_str(),
                key_str.c_str());
        return ret;
    }

    propertytree* child_note = &(child_iter->second);
    if (key_str.length() != 0)
    {
        leaf_iter = child_note->leaf_node_.find(key_str);
    }
    else
    {
        //对XML,XML的node也是有value，
        leaf_iter = child_note->leaf_node_.find("<self_note>");
    }
    if (child_note->leaf_node_.end() == leaf_iter)
    {
        ZCE_LOG(RS_ERROR, "Read config key fail, path[%s] key[%s]. ",
                path_str.c_str(),
                key_str.c_str());
        return -1;
    }

    return 0;
}

//同上，只是const的
int propertytree::path_get_leafiter(const std::string& path_str,
                                    const std::string& key_str,
                                    propertytree::const_leaf_iterator& leaf_iter) const
{
    int ret = 0;

    propertytree::const_child_iterator child_iter;
    ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }

    const propertytree* child_note = &(child_iter->second);
    if (key_str.length() != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
                __ZCE_FUNC__,
                path_str.c_str(),
                key_str.c_str());
        leaf_iter = child_note->leaf_node_.find(key_str);
    }
    else
    {
        //对XML,XML的node也是有value，
        leaf_iter = child_note->leaf_node_.find("<self_note>");
    }
    if (child_note->leaf_node_.end() == leaf_iter)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config key fail, path[%s] key[%s]. ",
                __ZCE_FUNC__,
                path_str.c_str(),
                key_str.c_str());
        return -1;
    }

    return 0;
}

//得到child node的指针
int propertytree::path_get_childptr(const std::string& path_str,
                                    propertytree*& child_ptr)
{
    propertytree::child_iterator child_iter;
    int ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }
    child_ptr = &(child_iter->second);
    return 0;
}

//同上，只是const的
int propertytree::path_get_childptr(const std::string& path_str,
                                    const propertytree*& child_ptr) const
{
    propertytree::const_child_iterator child_iter;
    int ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }
    child_ptr = &(child_iter->second);
    return 0;
}

///得到（当前node）叶子节点的begin 位置的迭代器
propertytree::leaf_iterator propertytree::leaf_begin()
{
    return leaf_node_.begin();
}
propertytree::const_leaf_iterator propertytree::leaf_cbegin() const
{
    return leaf_node_.cbegin();
}
///得到（当前node）叶子节点的end 位置的迭代器
propertytree::leaf_iterator propertytree::leaf_end()
{
    return leaf_node_.end();
}
propertytree::const_leaf_iterator propertytree::leaf_cend() const
{
    return leaf_node_.cend();
}

//得到（当前node）子树节点的begin 位置的迭代器
propertytree::child_iterator propertytree::child_begin()
{
    return child_node_.begin();
}
propertytree::const_child_iterator propertytree::child_cbegin() const
{
    return child_node_.cbegin();
}
//得到（当前node）子树节点的begin 位置的迭代器
propertytree::child_iterator propertytree::child_end()
{
    return child_node_.end();
}
propertytree::const_child_iterator propertytree::child_cend() const
{
    return child_node_.cend();
}

//增加一个新的CHILD,当然里面全部数据为NULL
void propertytree::add_child(const std::string& key_str,
                             propertytree*& new_child_note)
{
    propertytree null_node;
    CHILDREN_NOTE_TYPE::iterator iter =
        this->child_node_.insert(std::make_pair(key_str, null_node));

    new_child_note = &(iter->second);
    return;
}

//设置分割符号
void propertytree::set_separator(char separator_char)
{
    SEPARATOR_STRING[0] = separator_char;
}

void propertytree::clear()
{
    leaf_node_.clear();
    child_node_.clear();
}
}