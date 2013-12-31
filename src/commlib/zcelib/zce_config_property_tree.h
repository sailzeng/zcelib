/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_config_property_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       2011年10月5日
* @brief      今天IPhone 5没有发布，而是发布了一款Iphone 4S,广大果粉有点失望
*             
*             
* @details    
*             
*             
*             
* @note       
*             
*/


#ifndef ZCE_LIB_CONFIG_PROPERTY_TREE_H_
#define ZCE_LIB_CONFIG_PROPERTY_TREE_H_

//这个类的最终实现目标是实现类似BOOST ，Property Tree 和 ACE_Configuration_Heap 类的目标，
//将配置信息组织成一棵树存放
//其他读取工具读取数据内容后都生成放入ZenConfigPropertyTree中，
//思路还是和Property Tree有一定差别
//自己本来就是一个节点
class ZCE_Conf_PropertyTree;
//self
typedef ZCE_Conf_PropertyTree                        PROPERTY_TREE_NODE;

/******************************************************************************************
class ZCE_INI_Implemention 配置文件读取后存放的地方
******************************************************************************************/
class ZCE_Conf_PropertyTree
{
protected:

    //
    typedef std::multimap<std::string, std::string>           KEY_VALUE_MAP;

    //
    typedef std::multimap<std::string, PROPERTY_TREE_NODE>    PROPERTY_TREE_MAP;

protected:

    //叶子节点的
    KEY_VALUE_MAP         leaf_node_map_;

    //子树节点的
    PROPERTY_TREE_MAP     child_node_map_;

    //
public:

    //构造函数
    ZCE_Conf_PropertyTree();
    ~ZCE_Conf_PropertyTree();

    //根据路径得到一个CHILD
    int get_child(const std::string &path_str,
                  const PROPERTY_TREE_NODE *& const_child) const;

    //
    int get_child(const std::string &path_str,
                  PROPERTY_TREE_NODE *& child_data);

    //放入一个CHILD
    int put_child(const std::string &path_str,
                  const std::string &new_child_name,
                  const PROPERTY_TREE_NODE &value_data);

    //增加一个新的CHILD,当然里面全部数据为NULL
    int new_child(const std::string &path_str,
                  const std::string &new_child_name);

    //本来打算用模版函数,后来发现我还没有BOOST那样蛋疼。

    //取得一个叶子节点的数据,取回数据是srting
    int get_leaf_str(const std::string &path_str,
                     const std::string &key_data,
                     std::string &value_data) const;

    //取得一个叶子节点的数据，取回数据是char *
    int  get_leaf_str(const std::string &path_str,
                      const std::string &key_data,
                      char *&str_data,
                      size_t max_str_len) const;

    //取得一个叶子节点的数据，取回数据是int,
    int get_leaf_int(const std::string &path_str,
                     const std::string &key_data,
                     int32_t &value_int) const;

    int get_leaf_uint(const std::string &path_str,
                      const std::string &key_data,
                      uint32_t &value_uint) const;

    //取得一个叶子节点的数据，取回数据是bool
    int get_leaf_bool(const std::string &path_str,
                      const std::string &key_data,
                      bool &value_bool) const;

    //放入一个叶子节点，string
    int put_leaf_str(const std::string &path_str,
                     const std::string &key_data,
                     const std::string &value_data);

    //放入一个叶子节点，int
    int put_leaf_int(const std::string &path_str,
                     const std::string &key_data,
                     int value_int);

    //放入一个叶子节点，bool
    int put_leaf_bool(const std::string &path_str,
                      const std::string &key_data,
                      bool value_bool);

    ////取得一个叶子节点的数据，如果没有数据，返回默认值
    //int get_leaf(const std::string &path_str,
    //    std::string &value_data,
    //    const std::string &defualt_value);

    //取得一个叶子节点的数据，取回数据是int,如果没有数据，返回默认值
    //int get_leaf(const std::string &path_str,
    //    int &value_int,
    //    const int defualt_int);

    //取得一个叶子节点的数据，取回数据是char *,如果没有数据，返回默认值
    //int get_leaf(const std::string &path_str,
    //    char *&str_data,
    //    size_t str_len,
    //    const char *defualt_str);
};

#endif //ZCE_LIB_CONFIG_PROPERTY_H_

