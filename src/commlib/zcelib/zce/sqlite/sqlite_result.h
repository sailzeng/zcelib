#pragma once

#if SQLITE_VERSION_NUMBER >= 3005000

#include "zce/string/from_string.h"

namespace zce
{
class sqlite_hdl;
/*!
* @brief      get_table 函数返回的结果参数
*             其实就是sqlite3_get_table 的结果参数的封装
* @note       请注意，sqlite3_get_table 只是应该向后兼容的函数
*/
class sqlite_result
{
    friend class zce::sqlite_hdl;

public:

    sqlite_result() = default;
    ~sqlite_result();

    //!结果集合释放为NULL
    bool is_null();

    //!释放结果集合
    void free_result();

    /*!
    * @brief      返回一个字段的名称
    * @return     const char* 字段的名称
    * @param      column 字段的列号,从1开始
    */
    const char* field_name(size_t column);

    /*!
    * @brief      返回一个字段的数据，
    * @return     const char* 字段的数据
    * @param      row    字段的列号,从1开始
    * @param      column 字段的行号,从1开始
    */
    const char* field_cstr(size_t row, size_t column);

    /*!
    * @brief      根据类型，返回一个字段的数据，
    * @tparam     value_type
    * @return     value_type
    * @param      row    字段的列号,从1开始
    * @param      column 字段的行号,从1开始
    * @note
    */
    template <typename value_type>
    value_type field_data(size_t row, size_t column)
    {
        return zce::from_str<value_type>(result_[row * column_ + column - 1]);
    }

    //!行的数量
    inline int row_number()
    {
        return row_;
    }

    //!列的数量
    inline int column_number()
    {
        return column_;
    }

protected:

    //! Results of the query
    char** result_ = NULL;
    //! Number of result rows written here ，row_也是从1开始
    //! 使用int的原因是内部函数用的int
    int row_ = 0;

    //! Number of result columns written here ,column_ 从1开始
    int column_ = 0;
};
}

#endif //SQLITE_VERSION_NUMBER >= 3005000