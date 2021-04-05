/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_config_property_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��10��5��
* @brief      һ������������Ϣ������ͨ��path����node�ϵ���Ϣ��
*
*
* @details    ����������ʵ��Ŀ����ʵ������BOOST ��Property Tree �� ACE_Configuration_Heap
*             ���Ŀ�꣬��������Ϣ��֯��һ�������
*             ˼·���Ǻ�BOOST Property Tree��һЩ���������ڲ�ֱ�ӷ���һ��map��š��������ݣ�
*             XML�ļ���name->value�����valueҲ��������ط���
*             ��Ϊθ����BOOST ��Property Tree̫���ˣ���Ȼ�������Ϊ��д�������ļ���˳���п��ǣ�
*             �����忴��ȥ��̫��Ľڵ㡣
* @note       ����IPhone 5û�з��������Ƿ�����һ��Iphone 4S,�������е�ʧ��
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
* @brief      �����ļ���ȡ���ŵ���
*             �������ļ���ȡ���߶�ȡ�������ݺ����ɷ���ZCE_Conf_PropertyTree�У�
*             �ڲ���2������
*             һ�����ڴ�����������name=>sub tree��
*             һ�����ڴ��Ҷ�ӽڵ㡣�������key=>value,XML�ļ���name=>valueҲ���
*             ������ط���
*
* @note       ��Ϊ��2�����������ڲ�Ҳ��������������
*
*/
class ZCE_Conf_PropertyTree
{
protected:

    ///Ҷ�ӽڵ�,�Լ���Ӧ�ĵ�����
    typedef std::multimap<std::string, std::string> LEAF_NOTE_TYPE;
    typedef LEAF_NOTE_TYPE::iterator leaf_iterator;
    typedef LEAF_NOTE_TYPE::const_iterator const_leaf_iterator;

    ///�����Ľڵ������,�������map�����Բ��Ǹ�Чʵ�֣���Ϊɶ����map�أ��ҹ�����
    ///��Ϊ��ʵmap���²�����˳�������ڻ�ԭ��ʱ�򣬻���ȫ����ԭ�������ݣ�����Ȼ
    ///������������
    typedef std::multimap<std::string, ZCE_Conf_PropertyTree > CHILDREN_NOTE_TYPE;
    typedef CHILDREN_NOTE_TYPE::iterator child_iterator;
    typedef CHILDREN_NOTE_TYPE::const_iterator const_child_iterator;
    //
public:

    ///���캯��
    ZCE_Conf_PropertyTree();
    ~ZCE_Conf_PropertyTree();


    /*!
    * @brief      ����·���õ�һ��CHILD �����ڵ�ĵ�����
    * @return     int == 0 ��ʾ�ɹ���
    * @param      path_str   ���ʵ�·��
    * @param      child_iter ���صĵ�������ע���ڲ���û���ҵ�������£�û�н�����Ϊend����
    *                        return �ķ���ֵ�ж��Ƿ�ɹ�����Ҫ�����������
    */
    int path_get_childiter(const std::string &path_str,
                           child_iterator &child_iter);
    ///ͬ�ϣ�ֻ��const��
    int path_get_childiter(const std::string &path_str,
                           const_child_iterator &child_iter) const;

    /*!
    * @brief      ȡ��Ҷ�ӽڵ�ĵ�����
    * @return     int
    * @param      path_str ���ʵ�·��
    * @param      key_str  ����val��key
    * @param      str_ptr  �õ�val��ָ��
    */
    int path_get_leafiter(const std::string &path_str,
                          const std::string &key_str,
                          leaf_iterator &leaf_iter);
    ///ͬ�ϣ�ֻ��const��
    int path_get_leafiter(const std::string &path_str,
                          const std::string &key_str,
                          const_leaf_iterator &leaf_iter) const;

    ///�õ�path��Ӧ���Ǹ�child note��ָ��
    int path_get_childptr(const std::string &path_str,
                          ZCE_Conf_PropertyTree *&child_ptr);
    ///ͬ�ϣ�ֻ��const��
    int path_get_childptr(const std::string &path_str,
                          const ZCE_Conf_PropertyTree *&child_ptr) const;


    ///�õ�����ǰnode��Ҷ�ӽڵ��begin λ�õĵ�����
    leaf_iterator leaf_begin();
    ///�õ�����ǰnode��Ҷ�ӽڵ��end λ�õĵ�����
    leaf_iterator leaf_end();

    ///�õ�����ǰnode�������ڵ��begin λ�õĵ�����
    child_iterator child_begin();
    ///�õ�����ǰnode�������ڵ��begin λ�õĵ�����
    child_iterator child_end();

    /*!
    * @brief      ���������ػ���ģ��ߵ���һ�麯��,ģ�庯��,�����ػ�ʵ��,
    * @tparam     val_type ���ػ��� ZCE_Sockaddr_In��ZCE_Sockaddr_In6��ZCE_Time_Value
    *             int32_t ,int64_t,std::string, �ȡ�
    * @return     int      �Ƿ������Ķ�ȡ��������
    * @param      path_str ��ȡ��·��
    * @param      val      ��ȡ���ص�ֵ
    */
    template<typename val_type>
    int path_get_leaf(const std::string &path_str,
                      const std::string &key_str,
                      val_type &val) const;



    /*!
    * @brief      ����ĺ�������չ������
    * @tparam     val_type �ο����溯��
    * @return     int
    * @param      path_str �ο����溯��
    * @param      key_str  �ο����溯��
    * @param      key_sequence key�����к�
    * @note
    */
    template<typename val_type>
    int pathseq_get_leaf(const std::string &path_str,
                         const std::string &key_str,
                         size_t key_sequence,
                         val_type &val) const
    {
        std::string seqkey_str = key_str + std::to_string(key_sequence);
        return path_get_leaf(path_str, seqkey_str, val);
    }

    /*!
    * @brief      ���������ػ���ģ��ߵ���һ�麯��,ģ�庯��,ֻ���岻ʵ��
    * @tparam     val_type
    * @return     void
    * @param      key_str
    * @param      val
    */
    template<typename val_type>
    void set_leaf(const std::string &key_str,
                  val_type val);


    ///����һ���µ�CHILD,��Ȼ����ȫ������ΪNULL,���ҷ��������Ľڵ�
    void add_child(const std::string &key_str,
                   ZCE_Conf_PropertyTree *&new_child_note);


    ///����
    void clear();


public:
    ///���÷ָ����,������������
    static void set_separator(char separator_char);

protected:

    ///Ĭ��ʹ��.��Ϊ�����ţ����ǵ�name������ܻ�ʹ��������ţ�
    ///�������Ҳ���������,û����const �������ṩ���޸ĺ���
    static char  SEPARATOR_STRING[2];

protected:

    ///Ҷ�ӽڵ��MAP
    LEAF_NOTE_TYPE   leaf_node_;

    ///�����ڵ��MAP
    CHILDREN_NOTE_TYPE     child_node_;

};

#endif //ZCE_LIB_CONFIG_PROPERTY_H_

