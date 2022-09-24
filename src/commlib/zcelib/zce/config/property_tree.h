/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/config/property_tree.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月5日
* @brief      一个保存配置信息的数，通过path访问node上的信息，
*
*
* @details    这个类的最终实现目标是实现类似BOOST ，Property Tree 和 ACE_Configuration_Heap
*             类的目标，将配置信息组织成一棵树存放
*             思路还是和BOOST Property Tree有一些区别，我们内部直接放入一个map存放。属性数据，
*             XML文件的name->value里面的value也放入这个地方。
*             因为胃觉得BOOST 的Property Tree太慢了，虽然其可能是为了写回配置文件（顺序）有考虑，
*             但总体看上去有太多的节点。
* @note       今天IPhone 5没有发布，而是发布了一款Iphone 4S,广大果粉有点失望
*
*
*/

#pragma once

#include "zce/string/from_string.h"
#include "zce/string/to_string.h"

namespace zce
{
/*!
* @brief      配置文件读取后存放的树
*             将配置文件读取工具读取数据内容后都生成放入ZCE_Conf_PropertyTree中，
*             内部有2棵树，
*             一棵用于存放子树，存放name=>sub tree，
*             一棵用于存放叶子节点。存放属性key=>value,XML文件的name=>value也存放
*             在这个地方，
*
* @note       因为有2棵树，所以内部也有两个迭代器，
*
*/
class PropertyTree
{
protected:

    ///叶子节点,以及相应的迭代器
    typedef std::multimap<std::string, std::string> LEAF_NOTE_TYPE;
    typedef LEAF_NOTE_TYPE::iterator leaf_iterator;
    typedef LEAF_NOTE_TYPE::const_iterator const_leaf_iterator;

    ///子树的节点的类型,这儿不是map，所以不是高效实现，但为啥不用map呢，我估计是
    ///因为其实map本事并不了顺序，所以在还原的时候，会完全混乱原来的数据，（虽然
    ///并不错），所以
    typedef std::multimap<std::string, zce::PropertyTree > CHILDREN_NOTE_TYPE;
    typedef CHILDREN_NOTE_TYPE::iterator child_iterator;
    typedef CHILDREN_NOTE_TYPE::const_iterator const_child_iterator;
    //
public:

    //!构造函数
    PropertyTree() = default;
    ~PropertyTree() = default;

    /*!
    * @brief      根据路径得到一个CHILD 子树节点的迭代器
    * @return     int == 0 表示成功，
    * @param      path_str   访问的路径. 比如A1.B2.C3.D4，每个点标示一层
    * @param      child_iter 返回的迭代器，注意内部在没有找到的情况下，没有将其置为end，用
    *                        return 的返回值判断是否成功，不要用这个参数。
    */
    int path_get_childiter(const std::string& path_str,
                           child_iterator& child_iter);
    ///同上，只是const的
    int path_get_childiter(const std::string& path_str,
                           const_child_iterator& child_iter) const;

    /*!
    * @brief      取得叶子节点的迭代器
    * @return     int
    * @param      path_str 访问的路径. 比如A1.B2.C3.D4，每个点标示层
    * @param      key_str  访问val的key
    * @param      str_ptr  得到val的指针
    */
    int path_get_leafiter(const std::string& path_str,
                          const std::string& key_str,
                          leaf_iterator& leaf_iter);
    ///同上，只是const的
    int path_get_leafiter(const std::string& path_str,
                          const std::string& key_str,
                          const_leaf_iterator& leaf_iter) const;

    ///得到path对应的那个child note的指针
    int path_get_childptr(const std::string& path_str,
                          zce::PropertyTree*& child_ptr);
    ///同上，只是const的
    int path_get_childptr(const std::string& path_str,
                          const zce::PropertyTree*& child_ptr) const;

    ///得到（当前node）叶子节点的begin 位置的迭代器
    leaf_iterator leaf_begin();
    const_leaf_iterator leaf_cbegin() const;
    ///得到（当前node）叶子节点的end 位置的迭代器
    leaf_iterator leaf_end();
    const_leaf_iterator leaf_cend() const;

    ///得到（当前node）子树节点的begin 位置的迭代器
    child_iterator child_begin();
    const_child_iterator child_cbegin() const;
    ///得到（当前node）子树节点的begin 位置的迭代器
    child_iterator child_end();
    const_child_iterator child_cend() const;

    /*!
    * @brief      还是用了特化的模板高点这一组函数,模板函数,依靠特化实现,
    * @tparam     val_type 被特化成 zce::Sockaddr_In，Sockaddr_In6，zce::Time_Value
    *             int32_t ,int64_t,std::string, 等。
    * @return     int      是否正常的读取倒了配置
    * @param      path_str 读取的路径，比如A1.B2.C3.D4，每个点标示一层
    * @param      key_str  参考上面函数
    * @param      val      读取返回的值
    */
    template<typename val_type>
    int path_get_leaf(const std::string& path_str,
                      const std::string& key_str,
                      val_type& val) const
    {
        PropertyTree::const_leaf_iterator leaf_iter;
        int ret = path_get_leafiter(path_str, key_str, leaf_iter);
        if (0 != ret)
        {
            return ret;
        }
        std::string value_data;
        value_data = leaf_iter->second;
        if (value_data.empty())
        {
            ZCE_LOG(RS_INFO, "Value string is empty. path[%s] key [%s] ",
                    path_str.c_str(), key_str.c_str());
            return -2;
        }
        else
        {
            val = zce::from_string<val_type>(value_data);
            return 0;
        }
    }

    /*!
    * @brief      上面的函数的扩展函数，用于读取KEY1，KEY2，KEY3这种情况
    * @tparam     val_type 参考上面函数
    * @return     int
    * @param      path_str 参考上面函数
    * @param      key_str  参考上面函数
    * @param      key_sequence key的序列号,比如1,2,3……
    * @note
    */
    template<typename val_type>
    int pathseq_get_leaf(const std::string& path_str,
                         const std::string& key_str,
                         size_t key_sequence,
                         val_type& val) const
    {
        std::string seqkey_str = key_str + std::to_string(key_sequence);
        return path_get_leaf(path_str, seqkey_str, val);
    }

    /*!
    * @brief      还是用了特化的模板高点这一组函数,模板函数,只定义不实现
    * @tparam     val_type
    * @return     void
    * @param      key_str
    * @param      val
    */
    template<typename val_type>
    void set_leaf(const std::string& key_str,
                  const val_type& val)
    {
        std::string val_str;
        zce::to_string(val_str, val);
        this->leaf_node_.insert(std::make_pair(key_str, val_str));
    }

    ///增加一个新的CHILD,当然里面全部数据为NULL,并且返回新增的节点
    void add_child(const std::string& key_str,
                   zce::PropertyTree*& new_child_note);

    ///清理
    void clear();

public:
    ///设置分割符号,允许你更换这个
    static void set_separator(char separator_char);

protected:

    ///默认使用.作为风格符号，考虑到name里面可能会使用这个符号，
    ///所以这儿也可以让你改,没有用const ，而且提供了修改函数
    static char  SEPARATOR_STRING[2];

protected:

    //!叶子节点的MAP
    LEAF_NOTE_TYPE   leaf_node_;

    //!子树节点的MAP
    CHILDREN_NOTE_TYPE     child_node_;
};
}
