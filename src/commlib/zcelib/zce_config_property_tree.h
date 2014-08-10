/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_config_property_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年10月5日
* @brief      一个保存配置信息的数，通过path访问node上的信息，
*
*
* @details    这个类的最终实现目标是实现类似BOOST ，Property Tree 和 ACE_Configuration_Heap
*             类的目标，将配置信息组织成一棵树存放
*
*
*
* @note      今天IPhone 5没有发布，而是发布了一款Iphone 4S,广大果粉有点失望
*
*
*/


#ifndef ZCE_LIB_CONFIG_PROPERTY_TREE_H_
#define ZCE_LIB_CONFIG_PROPERTY_TREE_H_

//
//
class ZCE_Sockaddr_In;
class ZCE_Sockaddr_In6;
class ZCE_Time_Value;





/*!
* @brief      配置文件读取后存放的树
*             其他读取工具读取数据内容后都生成放入ZCE_Conf_PropertyTree中，
*             思路还是和Property Tree基本一致。
* @note       2013年回头仔细看了BOOST的说明，发现人家的实现比我考虑的还是完整不
*             少，最后还是向其靠拢了。
*/
class ZCE_Conf_PropertyTree
{
protected:

    ///叶子节点
    typedef std::multimap<std::string, std::string> LEAF_NOTE_TYPE;
    typedef LEAF_NOTE_TYPE::iterator leaf_iterator;
    typedef LEAF_NOTE_TYPE::const_iterator const_leaf_iterator;
    
    ///子树的节点的类型,这儿不是map，所以不是高效实现，但为啥不用map呢，我估计是
    ///因为其实map本事并不了顺序，所以在还原的时候，会完全混乱原来的数据，（虽然
    ///并不错），所以
    typedef std::multimap<std::string, ZCE_Conf_PropertyTree > CHILDREN_NOTE_TYPE;
    typedef CHILDREN_NOTE_TYPE::iterator child_iterator;
    typedef CHILDREN_NOTE_TYPE::const_iterator const_child_iterator;
    //
public:

    ///构造函数
    ZCE_Conf_PropertyTree();
    ~ZCE_Conf_PropertyTree();



    /*!
    * @brief      根据路径得到一个CHILD 子树节点的迭代器
    * @return     int == 0 表示成功，
    * @param      path_str   访问的路径
    * @param      child_iter 返回的迭代器，注意内部在没有找到的情况下，没有将其置为end，用
    *                        return 的返回值判断是否成功，不要用这个参数。
    */
    int path_get_childiter(const std::string &path_str,
                           child_iterator &child_iter);
    ///同上，只是const的
    int path_get_childiter(const std::string &path_str,
                           const_child_iterator &child_iter) const;

    /*!
    * @brief      取得叶子节点的迭代器
    * @return     int
    * @param      path_str 访问的路径
    * @param      key_str  访问val的key
    * @param      str_ptr  得到val的指针
    */
    int path_get_leafiter(const std::string &path_str,
                          const std::string &key_str,
                          leaf_iterator &leaf_iter);
    ///同上，只是const的
    int path_get_leafiter(const std::string &path_str,
                          const std::string &key_str,
                          const_leaf_iterator &leaf_iter) const;

    ///得到path对应的那个child note的指针
    int path_get_childptr(const std::string &path_str,
                          ZCE_Conf_PropertyTree *&child_ptr);
    ///同上，只是const的
    int path_get_childptr(const std::string &path_str,
                          const ZCE_Conf_PropertyTree *&child_ptr) const;


    ///得到（当前node）叶子节点的begin 位置的迭代器
    leaf_iterator leaf_begin();
    ///得到（当前node）叶子节点的end 位置的迭代器
    leaf_iterator leaf_end();

    ///得到（当前node）子树节点的begin 位置的迭代器
    child_iterator child_begin();
    ///得到（当前node）子树节点的begin 位置的迭代器
    child_iterator child_end();

    /*!
    * @brief      还是用了特化的模板高点这一组函数,模板函数,依靠特化实现,
    * @tparam     val_type 被特化成 ZCE_Sockaddr_In，ZCE_Sockaddr_In6，ZCE_Time_Value
    *             int32_t ,int64_t,std::string, 等。
    * @return     int      是否正常的读取倒了配置
    * @param      path_str 读取的路径
    * @param      val      读取返回的值
    */
    template<typename val_type>
    int path_get_leaf(const std::string &path_str,
                      const std::string &key_str,
                      val_type &val) const;

    /*!
    * @brief      还是用了特化的模板高点这一组函数,模板函数,只定义不实现
    * @tparam     val_type
    * @return     void
    * @param      key_str
    * @param      val
    */
    template<typename val_type>
    void set_leaf(const std::string &key_str,
                  val_type val);


    ///增加一个新的CHILD,当然里面全部数据为NULL,并且返回新增的节点
    void add_child(const std::string &key_str,
                   ZCE_Conf_PropertyTree *&new_child_note);


public:
    ///设置分割符号,允许你更换这个
    static void set_separator(char separator_char);

protected:

    ///默认使用.作为风格符号，考虑到name里面可能会使用这个符号，
    ///所以这儿也可以让你改,没有用const ，而且提供了修改函数
    static char  SEPARATOR_STRING[2];

protected:

    ///叶子节点的MAP
    LEAF_NOTE_TYPE   leaf_node_;

    ///子树节点的MAP
    CHILDREN_NOTE_TYPE     child_node_;

};

#endif //ZCE_LIB_CONFIG_PROPERTY_H_

