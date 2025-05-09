#pragma once

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

#include "zce/string/from_string.h"

namespace zce::sqlite
{
class command;
/*!
* @brief      get_table 函数返回的结果参数
*             其实就是sqlite3_get_table 的结果参数的封装
* @note       请注意，sqlite3_get_table 只是应该向后兼容的函数
*/
class result
{
    friend class zce::sqlite::command;

public:

    result() = default;
    ~result();

    //!结果集合释放为nullptr
    bool is_null();

    //!释放结果集合
    void free();

    /*!
    * @brief      返回一个字段的名称
    * @return     const char* 字段的名称
    * @param      column 字段的列号,从0开始
    */
    const char* field_name(size_t column);

    /*!
    * @brief      返回一个字段的数据，
    * @return     const char* 字段的数据
    * @param      row    字段的列号,从0开始
    * @param      column 字段的行号,从0开始
    */
    const char* field_data(size_t row, size_t column);

    /*!
    * @brief      根据类型，返回一个字段的数据，
    * @tparam     value_type
    * @return     value_type
    * @param      row    字段的列号,从0开始
    * @param      column 字段的行号,从0开始
    * @note
    */
    template <typename value_type>
    value_type field(size_t row, size_t column)
    {
        return zce::from_str_to<value_type>(result_[(row + 1) * column_ + column]);
    }

    //!行的数量
    inline size_t num_of_rows()
    {
        return row_;
    }

    //!列的数量
    inline size_t num_of_fields()
    {
        return column_;
    }

protected:

    //! Results of the query
    char** result_ = nullptr;
    //! Number of result rows written here ，
    //! 使用int的原因是内部函数用的int
    int row_ = 0;

    //! Number of result columns written here ,
    int column_ = 0;
};
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
