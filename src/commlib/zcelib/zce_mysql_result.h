/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_result.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004��7��24��
* @brief      MYSQL���ݿ������ķ�װ
*
* @details    ���˼·���£��Ҳ����̫���࣬��ʵ׼ȷ˵Ӧ�û���һ���װ���з�װ��OO
*             �ĽǶȿ�����������һЩ��
*             �����Ҳ�ϲ��̫�鷳�ˣ�һ����Ҳ��ֱ�Ӵӽ�������������еĽ����
*             ��MYSQL�ĺ���������֪��5.0���Ӻ���û�У�����4.0��CPI������ƣ�����ͨ��
*             һ��mysql_fetch_row�����õ���ǰ�У������һ���еĺڴ���
*             ��������Ƴ����������¼��ǰ�еģ���ǰ�У���ͨ��fetch_row_next������ǰ��+1��
*             Ȼ�������ͨ�� >> �������ţ�ȡ�ø����ֶ�ֵ��
*             ��Զ��Ի��ǱȽϼ򵥵ģ�
*             �����ڴ����������˺ܶౣ�������������֣���Щ����������ʵ�����ޣ������������
*
*             2013��1�£��Ҵ��»�ͷ����һ����δ����ע�ͣ��Ҿ��������ṩAPI���е�ƫִ���
*             ���򣬺ðɣ���8��ǰ���¶������¾���
*
* @note       �����в��������ֶ����ƴ���ĺ���������ʵһ���棬�����ƵĴ�������Ч��
*             һ������ʵ�ܶ��������ֶ����ƺܲ���׼�������Сд��һЩ�ֶ�����ϣ���
*             ������õ��Ľ���ȣ������Ҳ��Ƽ�ʹ�ã�
*
*/

#ifndef ZCE_LIB_MYSQL_DB_RESULT_H_
#define ZCE_LIB_MYSQL_DB_RESULT_H_

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_string.h"
#include "zce_log_logging.h"
#include "zce_mysql_field.h"

/*!
* @brief      MYSQL�Ľ������װ
*             ����һ�£�Ҳ���������NO Copyable�ˣ�
*
*/
class ZCE_Mysql_Result : ZCE_NON_Copyable
{
public:

    ///һ���սṹ�����ڶ��������ݵĵ�ת��������>>��������char *��2�������ݣ�
    struct BINARY
    {
    };

public:
    ///���캯��
    ZCE_Mysql_Result();
    ///���캯��
    ZCE_Mysql_Result(MYSQL_RES *sqlresult);
    ///��������
    ~ZCE_Mysql_Result();

    ///��������Ƿ�Ϊ��
    inline bool is_null();

    /*!
    * @brief      ���ý������
    * @return     void
    * @param      sqlresult ����������
    */
    void set_mysql_result(MYSQL_RES *sqlresult);

    ///����Field ID���ر���������,ע�����õ����е�����Ҳ�����ǿ�
    inline char *field_name(unsigned int fieldid) const;

    /*!
    * @brief      ����Field Name�õ�Field ID,�к�
    * @return     inline int 0�ɹ���-1ʧ��
    * @param[in]  fname      ������,
    * @param[out] fieldid    ���ص������ƶ�Ӧ��ID
    */
    inline int field_index(const char *fname,
                           unsigned int &fieldid) const;

    /*!
    * @brief      ���ؽ����������Ŀ
    * @return     unsigned int �е�����
    */
    inline unsigned int num_of_rows() const;

    /*!
    * @brief      ���ؽ����������Ŀ
    * @return     unsigned int �е�����
    */
    inline unsigned int num_of_fields() const;

    /*!
    * @brief      ���������ID�õ��ֶ�FIELD��[]�������ź������������ID,�Լ���֤����
    * @return     ZCE_MySQL_Field
    * @param[in]  fieldid          ȡ���ֶ��±�
    */
    ZCE_Mysql_Field operator[](unsigned int fieldid) const;

    /*!
    * @brief      ͨ����ID����ѯ��ǰ�е��ֶΣ����ܺã��±궨λ
    * @return     int      0�ɹ���-1ʧ��
    * @param[in]  fieldid  ��ID����0��ʼ
    * @param[out] ffield   �����е�ֵ
    */
    int get_field(unsigned int fieldid, ZCE_Mysql_Field &ffield) const;

    /*!
    * @brief      ͨ�������Ʋ�ѯ��ǰ�е�ĳ���ֶε�ֵ�������ܲ�����
    * @return     int     0�ɹ���-1ʧ��
    * @param      fname   ���ֶε����ƣ�SQL�е����ƣ�����Ǹ��ӵ�SQL SELECT�������ֻ�ȽϹ��죬
    * @param      ffield  ���ز����������ݵķ�װ
    */
    int get_field(const char *, ZCE_Mysql_Field &ffield) const;

    /*!
    * @brief      �ڵ�ǰ�У����������ID�õ��ֶ�ֵ,�����ݵ�ָ����Ϊ��Ϊ����ֵ
    * @return     const char* ���ݵ�ָ�룬����NULL��ʾȡ����
    * @param      fieldid     �±�
    * @note
    */
    inline const char *field_data(const unsigned int fieldid) const;

    /*!
    * @brief      �ڵ�ǰ�У���ǰ�У��õ��ֶ�ֵ,�����ݵ�ָ����Ϊ��Ϊ����ֵ
    * @return     const char* ���ݵ�ָ�룬����NULL��ʾȡ����
    * @param      fname       �У��ֶΣ�����
    */
    const char *field_data(const char *fname) const;

    /*!
    * @brief      ���������ID�õ���ǰ�е��ֶ�ֵ,
    * @return     int       0�ɹ���-1ʧ��
    * @param      fieldid   ��ID
    * @param      pfdata    �����ݵ�ָ��
    */
    inline int field_data(unsigned int fieldid, char *pfdata) const;

    /*!
    * @brief      ���������ֵõ��ֶ�ֵ
    * @return     int      0�ɹ���-1ʧ��
    * @param      fname    ������
    * @param      pfdata   �����ݵ�ָ��
    */
    int field_data(const char *fname, char *pfdata) const;

    /*!
    * @brief      ��������ŵõ��ֶεĳ���
    * @return     int
    * @param      fieldid
    * @param      flength
    * @note
    */
    inline int field_length(unsigned int fieldid, unsigned int &flength) const;

    /*!
    * @brief      ���������ֵõ��ֶεĳ���
    * @return     int      0�ɹ���-1ʧ��
    * @param      fname    �����ƣ�SELECT�ֶ�����
    * @param      flength  �����ݵĳ���
    */
    int field_length(const char *fname, unsigned int &flength) const;

    ///ȡ�õ�ǰ���ֶεĳ���
    inline unsigned int get_cur_field_length();

    /*!
    * @brief      ���������ID�õ��ֶε�����
    * @return     inline int   0�ɹ���-1ʧ��
    * @param      fieldid      ���ֶ�ID
    * @param      ftype        �����ݵĳ��ȣ�Ҫ�ο�MYSQL CAPI ��enum_field_types
    */
    inline int field_type(unsigned int fieldid, enum_field_types &ftype) const;

    /*!
    * @brief      ���������ֵõ��ֶε�����
    * @return     int    0�ɹ���-1ʧ��
    * @param      fname  �����ƣ�SELECT�ֶ�����
    * @param      ftype  �����ͣ�Ҫ�ο�MYSQL CAPI ��enum_field_types
    */
    int field_type(const char *fname, enum_field_types &ftype) const;

    /*!
    * @brief      �õ��ֶα�ṹ����ĳ���
    * @return     int
    * @param      fieldid ���ֶ�ID
    * @param      flength �ж���ĳ��ȣ�
    */
    int field_define_size(unsigned int fieldid, unsigned int &flength) const;

    /*!
    * @brief      �õ��ֶα�ṹ����ĳ���
    * @return     int
    * @param      name
    * @param      flength
    * @note
    */
    int field_define_size(const char *name, unsigned int &flength) const;

    /*!
    * @brief      �������������У������ƶ���ĳ��
    * @return     int  0�ɹ���-1ʧ��
    * @param      row_id ��ID
    */
    int seek_row(unsigned int row_id);

    /*!
    * @brief      ��������һ�У�����true,��ʵ�е�����Orale�Ĺ�괦���Ǻ�
    * @return     bool true���н�����ϣ�falseû�н��������
    */
    bool fetch_row_next();

    /*!
    * @brief      ����Ѿ��н����, �ͷ�ԭ�еĽ����,
    */
    void free_result();

    /// >> ��������,���ڽ���������val��
    ///����Ϊ�˰�ȫ��>>����ǰ�����˸��ַ�ֹ����ļ�飬�����������һ��bug��
    ///���Ժ�����Ϊ�����ɵ����߰�װ�߽簲ȫ��
    ZCE_Mysql_Result &operator >> (char &val);
    ZCE_Mysql_Result &operator >> (short &val);
    ZCE_Mysql_Result &operator >> (int &val);
    ZCE_Mysql_Result &operator >> (long &val);
    ZCE_Mysql_Result &operator >> (long long &val);

    ZCE_Mysql_Result &operator >> (unsigned char &val);
    ZCE_Mysql_Result &operator >> (unsigned short &val);
    ZCE_Mysql_Result &operator >> (unsigned int &val);
    ZCE_Mysql_Result &operator >> (unsigned long &val);
    ZCE_Mysql_Result &operator >> (unsigned long long &val);

    ZCE_Mysql_Result &operator >> (float &val);
    ZCE_Mysql_Result &operator >> (double &val);

    ZCE_Mysql_Result &operator >> (bool &val);

    ZCE_Mysql_Result &operator >> (char *val);
    ZCE_Mysql_Result &operator >> (unsigned char *val);
    ZCE_Mysql_Result &operator >> (std::string &val);

    ///�����Ƶ�����Ҫ�ر���һ��,�ַ������ر�+1��,�����������ݲ�Ҫ��������
    ZCE_Mysql_Result &operator >> (BINARY *);

private:
    ///�������
    MYSQL_RES       *mysql_result_;

    ///������ϵĵ�ǰ��
    MYSQL_ROW        current_row_;
    ///��ǰ�У�
    unsigned int     current_field_;

    ///����ָ��,ָ�������ϵĵ�ǰ�еĸ����ֶ����ݵĳ���
    unsigned long   *fields_length_;

    ///�����������
    unsigned int     num_result_row_;

    ///�����������
    unsigned int     num_result_field_;

    ///MYSQL_FIELD����ָ��,ָ�������ϵ�����Field˵��.
    MYSQL_FIELD      *mysql_fields_;

};

//Description     : ��ѯ��������Ƿ�Ϊ��
inline bool ZCE_Mysql_Result::is_null()
{
    if (mysql_result_)
    {
        return false;
    }

    return true;
}

//���������õ���ID,��0��ʼ����
//ѭ���Ƚ�,Ч�ʱȽϵ�
inline int ZCE_Mysql_Result::field_index(const char *fname, unsigned int &field_id) const
{
    //ѭ���Ƚ����е�����,Ч�ʱȽϵ���
    for (unsigned int i = 0; i < num_result_field_; ++i)
    {
        //MYSQL�������ǲ����ִ�Сд��
        if (!strcasecmp(fname, mysql_fields_[i].name))
        {
            field_id = i;
            return 0;
        }
    }

    return -1;
}

//������Field ID ���ر���������,��������,����Ϊ��
//����õ����е�������Ҳ�����ǿ�,
inline char *ZCE_Mysql_Result::field_name(unsigned int fieldid) const
{
    //���������Ϊ��,���߲���nfield����
    if (mysql_result_ == NULL || fieldid >= num_result_field_)
    {
        return NULL;
    }

    //ֱ�ӵõ����������
    return mysql_fields_[fieldid].name;
}

//���ؽ����������Ŀ,num_result_row_ �����execute������Ҳ���Եõ�
inline unsigned int ZCE_Mysql_Result::num_of_rows() const
{
    return num_result_row_;
}

//���ؽ����������Ŀ
inline unsigned int ZCE_Mysql_Result::num_of_fields() const
{
    return num_result_field_;
}

//�����ֶ���ID,�õ��ֶ�ֵ
const char *ZCE_Mysql_Result::field_data(const unsigned int fieldid) const
{
    if (current_row_ == NULL || fieldid >= num_result_field_ )
    {
        return NULL;
    }

    return current_row_[fieldid];
}

//�����ֶ���ID,�õ��ֶ�ֵ��ָ�룬�������Լ���֤
inline int ZCE_Mysql_Result::field_data(unsigned int fieldid, char *pfdata) const
{
    //��������ϵĵ�ǰ��Ϊ��(����û��fetch_row_next),���߲���fieldid����
    if (current_row_ == NULL || fieldid >= num_result_field_ || pfdata == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    memcpy(pfdata, current_row_[fieldid], fields_length_[fieldid]);
    return 0;
}

//�����ֶ�˳��ID,�õ��ֶα�ṹ���������
inline int ZCE_Mysql_Result::field_type(unsigned int fieldid, enum_field_types &ftype) const
{

    //���������Ϊ��,���߲���nfield����
    if (current_row_ == NULL || fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ftype =  mysql_fields_[fieldid].type;
    return 0;

}

//����Field ID �õ�����ֵ��ʵ�ʳ���
inline int ZCE_Mysql_Result::field_length(unsigned int fieldid, unsigned int  &flength) const
{
    //��������ϵĵ�ǰ��Ϊ��(����û��fetch_row_next),���߲���fieldid����
    if (current_row_ == NULL && fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = fields_length_[fieldid];
    return 0;
}

inline unsigned int ZCE_Mysql_Result::get_cur_field_length()
{
    return static_cast<unsigned int>(fields_length_[current_field_]);
}

//�����ֶε�����ֵ�õ��ֶ�ֵ
inline int ZCE_Mysql_Result::get_field(unsigned int fieldid, ZCE_Mysql_Field &ffield) const
{
    //���а�ȫ��飬������󷵻�
    if (current_row_ == NULL || fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ffield.set_field(current_row_[fieldid], fields_length_[fieldid], mysql_fields_[fieldid].type);
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004��7��22��
Function        : ZCE_Mysql_Result::operator[]
Return          : ZCE_MySQL_Field
Parameter List  :
  Param1: unsigned int fieldid
Description     : ���������ID�õ��ֶ�FIELD��
Calls           :
Called By       :
Other           : []�������ź������������ID,�Լ���֤����
Modify Record   :
******************************************************************************************/
inline ZCE_Mysql_Field ZCE_Mysql_Result::operator[](unsigned int fieldid) const
{
    ZCE_Mysql_Field ffield(current_row_[fieldid], fields_length_[fieldid], mysql_fields_[fieldid].type);
    return ffield;
}

//�����Ҫ��MYSQL�Ŀ�
#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_DB_RESULT_H_

