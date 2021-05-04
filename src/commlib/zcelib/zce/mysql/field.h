/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/field.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MySQL的字段封装
*
* @details
*
*/

#ifndef ZCE_LIB_MYSQL_DB_FIELD_H_
#define ZCE_LIB_MYSQL_DB_FIELD_H_

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 26812)
#endif

/*!
@brief      MySQL的字段封装

*/
class ZCE_Mysql_Field
{
public:
    //构造函数
    ZCE_Mysql_Field():
        field_data_(NULL),
        field_length_(0),
        field_type_(FIELD_TYPE_NULL)
    {
    };

    //构造函数
    ZCE_Mysql_Field(const char *fdata,unsigned int flength,enum_field_types ftype):
        field_data_(fdata),
        field_length_(flength),
        field_type_(ftype)
    {
    };

    ///析构函数
    ~ZCE_Mysql_Field()
    {
    };

    /*!
    * @brief      设置字段数据
    * @param[in]  fdata    字段数据
    * @param[in]  flength  字段长度
    * @param[in]  ftype    字段类型
    */
    inline void set_field(const char *fdata,
                          unsigned int flength,
                          enum_field_types ftype);

    ///得到字段类型
    inline enum_field_types get_type() const;
    ///得到字段数据,Char*
    inline const char *get_data() const;
    ///得到字段的长度
    inline unsigned int get_length() const;

    ///是否数据为空
    inline bool is_null() const;

    //不提供BOOL类型的转换,因为MYSQL本身不提供这个类型，转换的感觉总是怪怪的
    //转换为BOOL类型,必须是数值类型 0表示false,非0表示真
    //inline bool AsBool() const;

    //高速的得到字符串,在字符串较为长时最好使用这个函数
    void get_string(std::string &) const;

    //用于转换的操作符号

    ///得到字符串，
    inline operator std::string() const;

    //转换为8位整型
    inline operator char() const;
    //转换为16位的整型
    inline operator short() const;
    ///转换为32为整形
    inline operator int() const;
    ///转换为长整型
    inline operator long() const;
    ///转换为64位整型
    inline operator long long() const;

    //转换为8位无符号整型
    inline operator unsigned char() const;
    //转换为16位的无符号整型
    inline operator unsigned short() const;
    ///转换为无符号32为整形
    inline operator unsigned int() const;
    ///转换为无符号长整型
    inline operator unsigned long() const;
    ///转换为64位无符号长整型
    inline operator unsigned long long() const;

    ///转换为double型
    inline operator double() const;
    ///转换为float型
    inline operator float() const;

protected:

    ///字段数据,指针，指向结果集的数据，所以结果集必须存在
    const char *field_data_;
    ///字段长度
    unsigned int     field_length_;
    ///字段类型
    enum_field_types field_type_;
};

//设置值
inline void ZCE_Mysql_Field::set_field(const char *fdata,unsigned int flength,enum_field_types ftype)
{
    field_data_ = fdata;
    field_length_ = flength;
    field_type_ = ftype;
};

//得到Field 类型
inline enum_field_types ZCE_Mysql_Field::get_type() const
{
    return field_type_;
}

//得到Field长度
inline unsigned int ZCE_Mysql_Field::get_length() const
{
    return field_length_;
}

//
inline const char *ZCE_Mysql_Field::get_data() const
{
    return field_data_;
};

//是否数据为空
inline bool ZCE_Mysql_Field::is_null() const
{
    if (field_data_ == NULL)
    {
        return true;
    }

    return false;
};

//高速的得到字符串,在字符串较为长时最好使用这个函数
inline ZCE_Mysql_Field::operator std::string() const
{
    std::string tmp_str;
    tmp_str.reserve(field_length_);
    tmp_str.assign(field_data_,field_length_);
    return tmp_str;
}

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined MYSQL_VERSION_ID

#endif //ZCE_LIB_MYSQL_DB_FIELD_H_
