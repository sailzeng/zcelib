#pragma once

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

#include "zce/logger/logging.h"
#include "zce/string/from_string.h"
#include "zce/buffer/string_buffer.h"

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
    friend class command;
public:
    result() = default;
    result(char** res, int row_num, int col_um);
    ~result();
    //拷贝构造函数，不使用
    result(const result&) = delete;
    result& operator=(const result&) = delete;
    //移动构造函数
    result(result&&) noexcept;
    result& operator=(result&&) noexcept;

protected:

    void move_result(result&& res) noexcept;

public:
    //!结果集合释放为nullptr
    bool is_null();

    //!释放结果集合
    void free();

    //! @brief      取下一行作为光标
    bool cursor_next();

    //! @brief 将结果集处理的行，检索移动到某行
    bool cursor_seek(size_t row_id);

    template <typename T>
    int cursor_field(size_t col, T& val)
    {
        return field(cursor_row_, col, val);
    }

    template <typename... Types>
    std::tuple<Types...> cursor_make_tuple(size_t row)
    {
        return make_tuple<Types...>(cursor_row_);
    }

    /*!
    * @brief      返回一个字段的名称
    * @return     const char* 字段的名称
    * @param      col 字段的列号,从0开始
    */
    const char* field_name(size_t col);

    /*!
    * @brief      返回一个字段的数据，
    * @return     const char* 字段的数据
    * @param      row    字段的列号,从0开始
    * @param      col 字段的行号,从0开始
    */
    const char* field_data(size_t row, size_t col);

    /*!
    * @brief      根据类型，返回一个字段的数据，
    * @tparam     T
    * @return     T
    * @param      row    字段的列号,从0开始
    * @param      col 字段的行号,从0开始
    * @note
    */
    template <typename T>
    T field(size_t row, size_t col) const
    {
        T val = T{};
        field(row, col, val);
        return val;
    }

    template <typename T>
    int field(size_t row, size_t col, T& val) const
    {
        ZCE_ASSERT(col < num_result_column_ && row < num_result_row_);
        if (sq_result_ == nullptr || col >= num_result_column_ || row >= num_result_row_)
        {
            return -1;
        }
		//! sq_result_ 第0行是列名，所以从1行开始
        val = zce::from_str_to<T>(sq_result_[(row + 1) * num_result_column_ + col]);
		return 0;
    }

    //!行的数量
    inline size_t num_of_rows()
    {
        return num_result_row_;
    }

    //!列的数量
    inline size_t num_of_columns()
    {
        return num_result_column_;
    }

    //将某行的数据全部转化一个tuple
    template <typename... Types>
    std::tuple<Types...> make_tuple(size_t row)
    {
        return _make_tuple_i<Types...>(row, std::index_sequence_for<Types...>{});
    }

protected:

    template<typename ...Types, std::size_t... Is>
    std::tuple<Types...> _make_tuple_i(size_t row, std::index_sequence<Is...>)
    {
        return std::make_tuple(field<Types>(row, Is)...);
    }
protected:

    //! Results of the query
    char** sq_result_ = nullptr;
    //! Number of result rows written here ，
    //! 使用int的原因是内部函数用的int
    int num_result_row_ = 0;

    //! Number of result columns written here ,
    int num_result_column_ = 0;

    //! 光标所在行
    size_t  cursor_row_ = (size_t)-1;
};

/*!
* @brief      sqlite3_step 函数返回后，处理的数据结果，stmt的结果集处理
*             真正取数据其实是调用，sqlite3_column_xxx 进行返回
*             取出一行数据后，调用cursor_next or fectch_next 函数（内部调用sqlite3_step）
*             进行下一行数据的处理
* @note
*/
class stmt_result
{
    friend class command;
public:
    stmt_result() = default;
    stmt_result(zce::sqlite::command& cmd);
    ~stmt_result() = default;

    //拷贝构造函数，不使用
    stmt_result(const stmt_result&) = delete;
    stmt_result& operator=(const stmt_result&) = delete;
    //移动构造函数
    stmt_result(stmt_result&&) noexcept;
    stmt_result& operator=(stmt_result&&) noexcept;

    stmt_result& operator=(sqlite3_stmt* statement) noexcept;

    //!结果集合释放为nullptr
    bool is_null()
    {
        return statement_ == nullptr ? true : false;
    }

    //! 取下一行数据
    //! do { dosomesth; }while(fectch_next());
    bool fectch_next();

    //! 将光标至于下一行
    //! while(cursor_next()) { dosomesth; }
    bool cursor_next();

    void field(size_t col, char& val) const;
    void field(size_t col, short& val) const;
    void field(size_t col, int& val) const;
    void field(size_t col, long& val) const;
    void field(size_t col, long long& val) const;
    void field(size_t col, unsigned char& val) const;
    void field(size_t col, unsigned short& val) const;
    void field(size_t col, unsigned int& val) const;
    void field(size_t col, unsigned long& val) const;
    void field(size_t col, unsigned long long& val) const;
    void field(size_t col, float& val) const;
    void field(size_t col, double& val) const;
    void field(size_t col, char* val) const;
    void field(size_t col, zce::string_buf& val) const;
    void field(size_t col, std::string& val) const;

    /*!
    * @brief      返回一个字段的名称
    * @return     const char* 字段的名称
    * @param      col 字段的列号,从0开始
    */
    const char* field_name(size_t col);

    template <typename T>
    T field(size_t col) const
    {
        T val = T{};
        field(col, val);
        return val;
    }

    /*!
    * @brief      取得列的数量
    * @return     int
    * @param      num_col
    */
    inline size_t column_count()
    {
        return num_result_column_;
    }

    //!导出结果,列号自动++
    template <class T>
    stmt_result& operator >> (T& val)
    {
        field(current_col_, val);
        ++current_col_;
        return *this;
    }

    size_t cur_field_length()
    {
        return field_length(current_col_);
    }

    /*!
    * @brief      得到当前返回列的长度
    * @return     int 长度
    * @param[in]  col
    * @note
    */
    int field_length(size_t col);

    //将当前行的数据全部转化一个tuple
    template <typename... Types>
    std::tuple<Types...> make_tuple()
    {
        return _make_tuple_i<Types...>(std::index_sequence_for<Types...>{});
    }

protected:

    template<typename ...Types, std::size_t... Is>
    std::tuple<Types...> _make_tuple_i(std::index_sequence<Is...>)
    {
        return std::make_tuple(field<Types>(Is)...);
    }

protected:
    //! SQLite原声的STMT的句柄
    sqlite3_stmt* statement_ = nullptr;

    //! Number of result columns written here ,
    size_t num_result_column_ = 0;

    //!当前取结果的列,用于>>函数,从0开始技术
    size_t current_col_ = 0;

    //! 光标所在行
    size_t  cursor_row_ = (size_t)-1;
};
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
