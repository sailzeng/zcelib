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

//
//
class ZCE_Sockaddr_In;
class ZCE_Sockaddr_In6;
class ZCE_Time_Value;

//自己本来就是一个节点
class ZCE_Conf_PropertyTree;
//self
typedef ZCE_Conf_PropertyTree                        PROPERTY_TREE_NODE;



/*!
* @brief      配置文件读取后存放的树
*             这个类的最终实现目标是实现类似BOOST ，Property Tree 和 ACE_Configuration_Heap
*             类的目标，将配置信息组织成一棵树存放
*             其他读取工具读取数据内容后都生成放入ZCE_Conf_PropertyTree中，
*             思路还是和Property Tree有一定差别
* @note
*/
class ZCE_Conf_PropertyTree
{
protected:

    //叶子节点的MAP类型
    typedef std::multimap<std::string, std::string>           KEY_VALUE_MAP;

    //子树的MAP类型
    typedef std::multimap<std::string, PROPERTY_TREE_NODE>    PROPERTY_TREE_MAP;

    //
public:

    ///构造函数
    ZCE_Conf_PropertyTree();
    ~ZCE_Conf_PropertyTree();

    ///根据路径得到一个CHILD
    int get_child(const std::string &path_str,
                  const PROPERTY_TREE_NODE *& const_child) const;

    //
    int get_child(const std::string &path_str,
                  PROPERTY_TREE_NODE *& child_data);

    ///放入一个CHILD
    int put_child(const std::string &path_str,
                  const std::string &new_child_name,
                  const PROPERTY_TREE_NODE &value_data);

    ///增加一个新的CHILD,当然里面全部数据为NULL
    int new_child(const std::string &path_str,
                  const std::string &new_child_name);

    ///还是用了特化的模板高点这一组函数,模板函数,只定义不实现
    template<typename val_type>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 val_type &val) const;

    /*!
    * @brief      取得一个叶子节点的数据,取回数据是srting
    * @return     int == 0 表示成功
    * @param      path_str 路径，用|表示一段子树
    * @param      key_data 叶子节点的key
    * @param      val      返回的数值
    */
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 std::string &val) const;

    /*!
    * @brief      取得一个叶子节点的数据，取回数据是char *
    * @return     int == 0 表示成功
    * @param      path_str
    * @param      key_data
    * @param      val      是一个pair，输入的时候first 字符串指针，second是字符串的空间长度
    */
    template<>
    int  get_leaf(const std::string &path_str,
                  const std::string &key_data,
                  std::pair<char *, size_t > &val) const;

    /*!
    * @brief      取得一个叶子节点的数据，取回数据是int32_t,支持16进制,8进制写法
    * @return     int == 0 表示成功
    * @param      path_str
    * @param      key_data
    * @param      val      返回的数值
    * @note       如果是8进制的写法，请以0开头，如果是16进制写法，请以0x开头
    */
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 int32_t &val) const;

    ///同上，区别是得到一个无符号32位整数整数，
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 uint32_t &val) const;

    ///同上，区别是得到一个有符号64位整数整数，
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 int64_t &val) const;

    ///同上，区别是得到一个无符号64位整数整数，
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 uint64_t &val) const;

    ///取得一个叶子节点的数据，取回数据是bool
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 bool &val) const;

    /*!
    * @brief      取得IPV6的地址
    * @return     int      == 0 表示成功
    * @param      path_str 路径，用|表示一段子树
    * @param      key_data
    * @param      val      得到的IP地址，如果字符串里面有#，后面加上端口号，也会进行转换
    */
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 ZCE_Sockaddr_In &val) const;

    ///取得IPV6的地址，
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 ZCE_Sockaddr_In6 &val) const;

    ///时间戳字符串，使用ISO的格式
    template<>
    int get_leaf(const std::string &path_str,
                 const std::string &key_data,
                 ZCE_Time_Value &val) const;


    ///还是用了特化的模板高点这一组函数,模板函数,只定义不实现
    template<typename val_type>
    int put_leaf(const std::string &path_str,
                 const std::string &key_data,
                 val_type val);


    //放入一个叶子节点，string
    template<>
    int put_leaf(const std::string &path_str,
                 const std::string &key_data,
                 const std::string &value_data);

    //放入一个叶子节点，int
    template<>
    int put_leaf(const std::string &path_str,
                 const std::string &key_data,
                 int value_int);

    //放入一个叶子节点，bool
    template<>
    int put_leaf(const std::string &path_str,
                 const std::string &key_data,
                 bool value_bool);

protected:

    ///叶子节点的MAP
    KEY_VALUE_MAP         leaf_node_map_;

    ///子树节点的MAP
    PROPERTY_TREE_MAP     child_node_map_;

};

#endif //ZCE_LIB_CONFIG_PROPERTY_H_

