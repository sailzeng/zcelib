#include "zce_predefine.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_trace_log_debug.h"
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

//得到子树节点，
int ZCE_Conf_PropertyTree::path_get_childiter(const std::string &path_str,
                                              ZCE_Conf_PropertyTree::child_iterator &child_iter)
{

    //找到
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::iterator iter_tmp = child_node_.find(start_str);
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
            __ZCE_FUNC__,
            path_str.c_str());
        return -1;
    }

    ZCE_Conf_PropertyTree *child_tree = &(iter_tmp->second);

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
int ZCE_Conf_PropertyTree::path_get_childiter(const std::string &path_str,
                                              ZCE_Conf_PropertyTree::const_child_iterator &child_iter) const
{

    //找到
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::const_iterator iter_tmp = child_node_.find(start_str);
    //如果没有找到
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
            __ZCE_FUNC__,
            path_str.c_str());
        return -1;
    }

    const ZCE_Conf_PropertyTree *child_tree = &(iter_tmp->second);

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
int ZCE_Conf_PropertyTree::path_get_leafiter(const std::string &path_str,
                                             const std::string &key_str,
                                             ZCE_Conf_PropertyTree::leaf_iterator &leaf_iter)
{
    int ret = 0;

    ZCE_Conf_PropertyTree::child_iterator child_iter;
    ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
            __ZCE_FUNC__,
            path_str.c_str(),
            key_str.c_str());
        return ret;
    }

    ZCE_Conf_PropertyTree *child_note = &(child_iter->second);
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
        ZCE_LOGMSG(RS_ERROR, "Read config key fail, path[%s] key[%s]. ", 
            path_str.c_str(),
            key_str.c_str());
        return -1;
    }

    return 0;
}

//同上，只是const的
int ZCE_Conf_PropertyTree::path_get_leafiter(const std::string &path_str,
                                             const std::string &key_str,
                                             ZCE_Conf_PropertyTree::const_leaf_iterator &leaf_iter) const
{
    int ret = 0;

    ZCE_Conf_PropertyTree::const_child_iterator child_iter;
    ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }

    const ZCE_Conf_PropertyTree *child_note = &(child_iter->second);
    if (key_str.length() != 0)
    {
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
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
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]Read config key fail, path[%s] key[%s]. ",
            __ZCE_FUNC__,
            path_str.c_str(),
            key_str.c_str());
        return -1;
    }

    return 0;
}


//得到child node的指针
int ZCE_Conf_PropertyTree::path_get_childptr(const std::string &path_str,
                                             ZCE_Conf_PropertyTree *&child_ptr)
{
    ZCE_Conf_PropertyTree::child_iterator child_iter;
    int ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }
    child_ptr = &(child_iter->second);
    return 0;
}

//同上，只是const的
int ZCE_Conf_PropertyTree::path_get_childptr(const std::string &path_str,
                                             const ZCE_Conf_PropertyTree *&child_ptr) const
{
    ZCE_Conf_PropertyTree::const_child_iterator child_iter;
    int ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        return ret;
    }
    child_ptr = &(child_iter->second);
    return 0;
}


///得到（当前node）叶子节点的begin 位置的迭代器
ZCE_Conf_PropertyTree::leaf_iterator ZCE_Conf_PropertyTree::leaf_begin()
{
    return leaf_node_.begin();
}
///得到（当前node）叶子节点的end 位置的迭代器
ZCE_Conf_PropertyTree::leaf_iterator ZCE_Conf_PropertyTree::leaf_end()
{
    return leaf_node_.end();
}

//得到（当前node）子树节点的begin 位置的迭代器
ZCE_Conf_PropertyTree::child_iterator ZCE_Conf_PropertyTree::child_begin()
{
    return child_node_.begin();
}
//得到（当前node）子树节点的begin 位置的迭代器
ZCE_Conf_PropertyTree::child_iterator ZCE_Conf_PropertyTree::child_end()
{
    return child_node_.end();
}


//最后还是用模板函数特化了，也许能有什么幸福生活等着我呢？

/*!
* @brief      取得一个叶子节点的数据,取回数据是srting
* @return     int == 0 表示成功
* @param      path_str 路径，用.表示一段子树
* @param      val      返回的数值
*/
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         std::string &val) const
{
    ZCE_Conf_PropertyTree::const_leaf_iterator leaf_iter;

    int ret = path_get_leafiter(path_str, key_str, leaf_iter);

    if (0 != ret)
    {
        return ret;
    }

    val = leaf_iter->second;
    return 0;
}


/*!
* @brief      取得一个叶子节点的数据，取回数据是char *
* @return     int == 0 表示成功
* @param      path_str
* @param      val      是一个pair，输入的时候first 字符串指针，second是字符串的空间长度
*/
template<>
int  ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                          const std::string &key_str,
                                          std::pair<char *, size_t > &str_data) const
{
    std::string value_data;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_data);

    if (0 != ret)
    {
        return ret;
    }

    strncpy(str_data.first, value_data.c_str(), str_data.second);
    return 0;
}


/*!
* @brief      取得一个叶子节点的数据，取回数据是int32_t,支持16进制,8进制写法
* @return     int == 0 表示成功
* @param      path_str
* @param      val      返回的数值
* @note       如果是8进制的写法，请以0开头，如果是16进制写法，请以0x开头
*/
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         int32_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoi(value_str);
    return 0;
}

///同上，区别是得到一个有符号16位整数整数，
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
    const std::string &key_str,
    int16_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = static_cast<int16_t>(std::stoi(value_str));
    return 0;
}


///同上，区别是得到一个有符号16位整数整数，
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
    const std::string &key_str,
    uint16_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = static_cast<uint16_t>( std::stoul(value_str));
    return 0;
}

///同上，区别是得到一个无符号32位整数整数，
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         uint32_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = static_cast<uint32_t>(std::stoul(value_str));
    return 0;
}

///同上，区别是得到一个有符号64位整数整数，
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         int64_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoll(value_str);
    return 0;
}

///同上，区别是得到一个无符号64位整数整数，
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         uint64_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }

    val = std::stoull(value_str);
    return 0;
}

///取得一个叶子节点的数据，取回数据是bool
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         bool &val) const
{
    val = false;

    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

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

/*!
* @brief      取得IPV4的地址
* @return     int      == 0 表示成功
* @param      path_str 路径，用|表示一段子树
* @param      val      得到的IP地址，如果字符串里面有#，后面加上端口号，也会进行转换
*/
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         ZCE_Sockaddr_In &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.set(value_str.c_str());
    return 0;
}

///IPV6的地址
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         ZCE_Sockaddr_In6 &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.set(value_str.c_str());
    return 0;
}

///取得时间
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         ZCE_Time_Value &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret)
    {
        return ret;
    }
    val.from_string(value_str.c_str(), false, ZCE_LIB::TIME_STRFMT_US_SEC);
    return 0;
}

//增加一个新的CHILD,当然里面全部数据为NULL
void ZCE_Conf_PropertyTree::add_child(const std::string &key_str,
                                      ZCE_Conf_PropertyTree *&new_child_note)
{
    ZCE_Conf_PropertyTree null_node;
    CHILDREN_NOTE_TYPE::iterator iter =
        this->child_node_.insert(std::make_pair(key_str, null_node));

    new_child_note = &(iter->second);
    return;
}

//
template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     const std::string &val_str)
{
    this->leaf_node_.insert(std::make_pair(key_str, val_str));
}

template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     std::string &val_str)
{
    this->leaf_node_.insert(std::make_pair(key_str, val_str));
}

template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     const char *val_str)
{
    this->leaf_node_.insert(std::make_pair(key_str, std::string(val_str)));
}

template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     char *val_str)
{
    this->leaf_node_.insert(std::make_pair(key_str, std::string(val_str)));
}

//
template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     int value_int)
{
    const size_t BUF_LEN = 24;
    char str_int[BUF_LEN + 1];
    str_int[BUF_LEN] = '\0';
    snprintf(str_int, BUF_LEN, "%d", value_int);
    return set_leaf(key_str, str_int);
}

//
template<>
void ZCE_Conf_PropertyTree::set_leaf(const std::string &key_str,
                                     bool value_bool)
{
    if (value_bool)
    {
        return set_leaf<const char *>(key_str, "TRUE");
    }
    else
    {
        return set_leaf<const char *>(key_str, "FALSE");
    }

}

//设置分割符号
void ZCE_Conf_PropertyTree::set_separator(char separator_char)
{
    SEPARATOR_STRING[0] = separator_char;
}



void ZCE_Conf_PropertyTree::clear()
{
    leaf_node_.clear();
    child_node_.clear();
}

