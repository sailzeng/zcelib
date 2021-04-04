#include "zce_predefine.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"
#include "zce_log_logging.h"
#include "zce_config_property_tree.h"

//�ָ��,
char ZCE_Conf_PropertyTree::SEPARATOR_STRING[2] = ".";

//���캯��,��������
ZCE_Conf_PropertyTree::ZCE_Conf_PropertyTree()
{
}
ZCE_Conf_PropertyTree::~ZCE_Conf_PropertyTree()
{
}

//�õ������ڵ㣬
int ZCE_Conf_PropertyTree::path_get_childiter(const std::string &path_str,
                                              ZCE_Conf_PropertyTree::child_iterator &child_iter)
{

    //�ҵ�
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::iterator iter_tmp = child_node_.find(start_str);
    //���û���ҵ�
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
                __ZCE_FUNC__,
                path_str.c_str());
        return -1;
    }

    ZCE_Conf_PropertyTree *child_tree = &(iter_tmp->second);

    //����·�������еݹ��ѯ
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->path_get_childiter(remain_str, child_iter);
    }
    else
    {
        //����޷�����ǰ������,
        child_iter = iter_tmp;
        return 0;
    }
}

//�õ������ڵ㣬const
int ZCE_Conf_PropertyTree::path_get_childiter(const std::string &path_str,
                                              ZCE_Conf_PropertyTree::const_child_iterator &child_iter) const
{

    //�ҵ�
    size_t str_pos = path_str.find(SEPARATOR_STRING, 0);

    std::string start_str(path_str, 0, str_pos);

    CHILDREN_NOTE_TYPE::const_iterator iter_tmp = child_node_.find(start_str);
    //���û���ҵ�
    if (child_node_.end() == iter_tmp)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] .",
                __ZCE_FUNC__,
                path_str.c_str());
        return -1;
    }

    const ZCE_Conf_PropertyTree *child_tree = &(iter_tmp->second);

    //����·�������еݹ��ѯ
    if (str_pos != std::string::npos)
    {
        std::string remain_str(path_str, str_pos + 1);
        return child_tree->path_get_childiter(remain_str, child_iter);
    }
    else
    {
        //����޷�����ǰ������,
        child_iter = iter_tmp;
        return 0;
    }
}


//ȡ��Ҷ�ӽڵ�ĵ�����
int ZCE_Conf_PropertyTree::path_get_leafiter(const std::string &path_str,
                                             const std::string &key_str,
                                             ZCE_Conf_PropertyTree::leaf_iterator &leaf_iter)
{
    int ret = 0;

    ZCE_Conf_PropertyTree::child_iterator child_iter;
    ret = path_get_childiter(path_str, child_iter);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
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
        //��XML,XML��nodeҲ����value��
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

//ͬ�ϣ�ֻ��const��
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
        ZCE_LOG(RS_ERROR, "[zcelib][%s]Read config path fail, path[%s] key[%s].",
                __ZCE_FUNC__,
                path_str.c_str(),
                key_str.c_str());
        leaf_iter = child_note->leaf_node_.find(key_str);
    }
    else
    {
        //��XML,XML��nodeҲ����value��
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


//�õ�child node��ָ��
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

//ͬ�ϣ�ֻ��const��
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


///�õ�����ǰnode��Ҷ�ӽڵ��begin λ�õĵ�����
ZCE_Conf_PropertyTree::leaf_iterator ZCE_Conf_PropertyTree::leaf_begin()
{
    return leaf_node_.begin();
}
///�õ�����ǰnode��Ҷ�ӽڵ��end λ�õĵ�����
ZCE_Conf_PropertyTree::leaf_iterator ZCE_Conf_PropertyTree::leaf_end()
{
    return leaf_node_.end();
}

//�õ�����ǰnode�������ڵ��begin λ�õĵ�����
ZCE_Conf_PropertyTree::child_iterator ZCE_Conf_PropertyTree::child_begin()
{
    return child_node_.begin();
}
//�õ�����ǰnode�������ڵ��begin λ�õĵ�����
ZCE_Conf_PropertyTree::child_iterator ZCE_Conf_PropertyTree::child_end()
{
    return child_node_.end();
}


//�������ģ�庯���ػ��ˣ�Ҳ������ʲô�Ҹ�����������أ�

/*!
* @brief      ȡ��һ��Ҷ�ӽڵ������,ȡ��������srting
* @return     int == 0 ��ʾ�ɹ�
* @param      path_str ·������.��ʾһ������
* @param      val      ���ص���ֵ
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
* @brief      ȡ��һ��Ҷ�ӽڵ�����ݣ�ȡ��������char *
* @return     int == 0 ��ʾ�ɹ�
* @param      path_str
* @param      val      ��һ��pair�������ʱ��first �ַ���ָ�룬second���ַ����Ŀռ䳤��
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
* @brief      ȡ��һ��Ҷ�ӽڵ�����ݣ�ȡ��������int32_t,֧��16����,8����д��
* @return     int == 0 ��ʾ�ɹ�
* @param      path_str
* @param      val      ���ص���ֵ
* @note       �����8���Ƶ�д��������0��ͷ�������16����д��������0x��ͷ
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
    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = std::stoi(value_str);
    }

    return 0;
}

///ͬ�ϣ������ǵõ�һ���з���16λ����������
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

    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = static_cast<int16_t>(std::stoi(value_str));
    }
    return 0;
}


///ͬ�ϣ������ǵõ�һ���з���16λ����������
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
    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = static_cast<uint16_t>(std::stoul(value_str));
    }
    return 0;
}

///ͬ�ϣ������ǵõ�һ���޷���32λ����������
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
    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = static_cast<uint32_t>(std::stoul(value_str));
    }

    return 0;
}

///ͬ�ϣ������ǵõ�һ���з���64λ����������
template<>
int ZCE_Conf_PropertyTree::path_get_leaf(const std::string &path_str,
                                         const std::string &key_str,
                                         int64_t &val) const
{
    std::string value_str;
    int ret = path_get_leaf<std::string>(path_str, key_str, value_str);

    if (0 != ret || true == value_str.empty())
    {
        return ret;
    }
    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = std::stoll(value_str);
    }
    return 0;
}

///ͬ�ϣ������ǵõ�һ���޷���64λ����������
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
    if (value_str.empty())
    {
        ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                path_str.c_str(), key_str.c_str());
        val = 0;
    }
    else
    {
        val = std::stoull(value_str);
    }
    return 0;
}

///ȡ��һ��Ҷ�ӽڵ�����ݣ�ȡ��������bool
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
* @brief      ȡ��IPV4�ĵ�ַ
* @return     int      == 0 ��ʾ�ɹ�
* @param      path_str ·������|��ʾһ������
* @param      val      �õ���IP��ַ������ַ���������#��������϶˿ںţ�Ҳ�����ת��
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

///IPV6�ĵ�ַ
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

///ȡ��ʱ��
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
    val.from_string(value_str.c_str(), false, zce::TIME_STRFMT_US_SEC);
    return 0;
}

//����һ���µ�CHILD,��Ȼ����ȫ������ΪNULL
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

//���÷ָ����
void ZCE_Conf_PropertyTree::set_separator(char separator_char)
{
    SEPARATOR_STRING[0] = separator_char;
}



void ZCE_Conf_PropertyTree::clear()
{
    leaf_node_.clear();
    child_node_.clear();
}

