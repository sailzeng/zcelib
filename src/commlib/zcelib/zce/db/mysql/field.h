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

#pragma once

#include "zce/string/from_string.h"

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 26812)
#endif

namespace zce::mysql
{
/*!
@brief      MySQL的字段封装
*/
class field
{
public:
    //构造函数,析构函数
    field() = default;
    field(const char* fdata,
        unsigned int flength,
        enum_field_types ftype);
    ~field() = default;
    field(const field& others);
    field& operator=(const field& others);

    /*!
    * @brief      设置字段数据
    * @param[in]  fdata    字段数据
    * @param[in]  flength  字段长度
    * @param[in]  ftype    字段类型
    */
    inline void set_field(const char* fdata,
        unsigned int flength,
        enum_field_types ftype)
    {
        field_data_ = fdata;
        field_length_ = flength;
        field_type_ = ftype;
    }

    ///得到字段类型
    inline enum_field_types get_type() const
    {
        return field_type_;
    }

    ///得到字段数据,Char*
    inline const char* get_data() const
    {
        return field_data_;
    }

    ///得到字段的长度
    inline unsigned int get_length() const
    {
        return field_length_;
    }

    ///是否数据为空
    inline bool is_null() const
    {
        return field_data_ == nullptr ? true : false;
    };

    ///取得Field里面的数据
    template <typename T>
    inline int get(T& data) const
    {
        return zce::from_str(field_data_, data);
    }

    //高速的得到字符串,在字符串较为长时最好使用这个函数
    void get_string(std::string&) const;

protected:

    ///字段数据,指针，指向结果集的数据，所以结果集必须存在
    const char* field_data_ = nullptr;
    ///字段长度
    unsigned int     field_length_ = 0;
    ///字段类型
    enum_field_types field_type_ = FIELD_TYPE_NULL;
};
}

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
