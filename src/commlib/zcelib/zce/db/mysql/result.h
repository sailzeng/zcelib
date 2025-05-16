/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/result.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MYSQL数据库结果集的封装
*
* @details    设计思路如下，我不想搞太多类，其实准确说应该还有一层封装，列封装在OO
*             的角度看起会更加完美一些，
*             但是我不喜欢太麻烦了，一般人也会直接从结果集处理玩所有的结果，
*             而MYSQL的函数，（不知道5.0增加函数没有，我以4.0的CPI做的设计），是通过
*             一个mysql_fetch_row函数得到当前行，你必须一行行的黑醋栗
*             所以我设计成主动帮你记录当前行的，当前列，你通过fetch_row_next，将当前行+1，
*             然后你可以通过 >> 操作符号，取得个个字段值。
*             相对而言还是比较简单的，
*             早年在代码里面用了很多保护，但后来发现，这些保护的意义实在有限，不如让你崩溃
*
*             2013年1月，我从新回头整理一下这段代码的注释，我觉得我在提供API上有点偏执狂的
*             倾向，好吧，想8年前的懵懂少年致敬。
*
* @note       其中有不少用列字段名称处理的函数，但其实一方面，列名称的处理并不高效，
*             一方面其实很多结果的列字段名称很不标准，比如大小写，一些字段是组合，函
*             数运算得到的结果等，所以我不推荐使用，
*
*/

#pragma once

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

#include "zce/os_adapt/string.h"
#include "zce/logger/logging.h"
#include "zce/buffer/string_buffer.h"
#include "zce/string/from_string.h"

namespace zce::mysql
{
/*! //============================================================================
* @brief      MYSQL的结果的字段
*/
class field
{
public:
    //! 数据
    const char* data_ = nullptr;
    //! 数据长度
    unsigned long length_ = 0;
    //! 数据类型
    enum_field_types type_ = MYSQL_TYPE_NULL;
};

class result;
//============================================================================
/*!
* @brief      MYSQL的结果的游标,
*             注意游标其实是通过mysql_fetch_row获得的，再次调用mysql_fetch_row
*             或者 mysql_data_seek后，游标的数据就失效了
*/
class cursor
{
    friend zce::mysql::result;
public:
    ///构造函数,析构函数
    cursor() noexcept = default;
    ~cursor() noexcept = default;

    /*!
     * @brief 在当前行，根据列序号ID得到字段值,将数据的指针作为作为返回值
     * @param col 列号
     * @return 将数据的指针作为作为返回值 const char *
     */
    const char* field_data(size_t col) const;

    /*!
     * @brief  根据列序号ID得到当前行的字段值,
     * @param col  列号
     * @param pfdata 将数据拷贝到pfdata
     * @return 0成功，-1失败
     */
    int field_data(size_t col, char* pfdata) const;

    /*!
     * @brief 根据列ID （col）取得字段的（实际）长度
     * @param col 列号
     * @return 字段的（实际）长度
     */
    size_t field_length(size_t col) const;

    /*!
     * @brief 根据列ID （col）取得当前行（游标的）字段类型
     * @param col  列号
     * @return 返回类型要参考MYSQL CAPI 的enum_field_types
     */
    enum_field_types field_type(size_t col) const;

    /*!
     * @brief 根据列ID （col）取得当前行（游标的）字段值
     * @param col 列ID
     * @return 列数据
     */
    zce::mysql::field get_field(size_t col) const;

    /*!
     * @brief      普通情况的使用 from_str 进行转换，有几种情况进行了特化
     *             字符串 用char * ,unsigned char*,字符串都特别+1了,帮忙做了结尾
     *             二进制数据用 BINARY*,维持原长度
     * @return     int 表示成功转换
     * @param      col 列ID
     * @param      val   取得的列数据
     */
    template <typename T>
    int field(size_t col, T& val) const
    {
        return zce::from_str(cursor_row_[col], val);
    }
    template <typename T>
    T field(size_t row, size_t col) const
    {
        T val;
        field(row, col, val);
        return val;
    }

    //若干特化实现
    template<>
    int field(size_t col, zce::string_buf& val) const;
    template<>
    int field(size_t col, zce::ztm& val) const;
    template<>
    int field(size_t col, char*& val) const;
    template<>
    int field(size_t col, unsigned char*& val) const;

    //! 清理
    void clear();

    //将光标的数据全部转化一个tuple
    template <typename... Types>
    std::tuple<Types...> make_tuple()
    {
        return _make_tuple_i<Types...>(std::index_sequence_for<Types...>{});
    }

protected:

    //! 注意，两个模板参数的顺序是不能交换的
    template<typename ...Types, std::size_t... Is>
    std::tuple<Types...> _make_tuple_i(std::index_sequence<Is...>)
    {
        return std::make_tuple(field<Types>(Is)...);
    }
protected:

    //! 游标对应的行号
    size_t cursor_rowid_ = (size_t)-1;

    //! 游标的当前行
    MYSQL_ROW  cursor_row_ = nullptr;

    //! 数组指针,指向结果集合的当前行的各个字段数据的长度
    unsigned long* fields_len_ = nullptr;

    //! 列数量
    size_t num_field_ = 0;
    //! 列定义
    MYSQL_FIELD* mysql_fields_ = nullptr;
};

//============================================================================
/*!
* @brief      MYSQL的结果集封装
*/
class result
{
public:
    ///构造函数,析构函数
    result() noexcept = default;
    result(::MYSQL_RES* res) noexcept;
    ~result() noexcept;

    //拷贝构造函数，不使用
    result(const result&) = delete;
    result& operator=(const result&) = delete;
    //移动构造函数
    result(result&&) noexcept;
    result& operator=(result&&) noexcept;

protected:

    void move_result(result&& res) noexcept;

public:
    ///结果集合是否为空
    inline bool is_null()
    {
        return mysql_result_ == nullptr ? true : false;
    }

    /*!
    * @brief      设置结果集合
    * @return     void
    * @param      sqlresult 放入结果集合
    */
    void save_result(MYSQL_RES* res);

    /*!
    * @brief      如果已经有结果集, 释放原有的结果集,
    */
    void free();

    /*!
    * @brief      检索到下一行，返回true,其实有点类似Orale的光标处理，呵呵
    * @return     bool true还有结果集合，false没有结果集合了
    */
    bool cursor_next();

    //! @brief 将结果集处理的行，检索移动到某行
    bool cursor_seek(size_t row_id);

    template <typename T>
    int cursor_field(size_t col, T& val)
    {
        return cursor_.field(col, val);
    }

    template <typename... Types>
    std::tuple<Types...> cursor_make_tuple()
    {
        return cursor_.make_tuple<Types...>();
    }

    //! @brief 取得当前的游标
    zce::mysql::cursor get_cursor()
    {
        return cursor_;
    }

    /// @brief 根据Field ID返回表定义列域名,注意计算得到的列的名字也可能是空
    const char* field_name(size_t col) const;

    //! @brief 根据Field Name得到Field ID,列号 返回-1表示没有找到
    size_t field_index(const char* fname) const;

    //! 根据列ID （col）取得当前行（游标的）字段定义长度
    size_t field_def_size(size_t col) const;

    //! 根据列ID （col）取得当前行（游标的）字段类型
    //! 返回类型要参考MYSQL CAPI 的enum_field_types
    enum_field_types field_type(size_t col) const;

    //! @brief      返回结果集的行数目
    inline size_t num_of_rows() const
    {
        return num_result_row_;
    }

    //! @brief      返回结果集的列数目
    inline size_t num_of_columns() const
    {
        return num_result_field_;
    }

    //! 警告：MySQL 其实是希望使用光标获取结果集的数据，所以没有提供直接用行号，列号
    //! 处理数据，但为了方便，我提供下面这些API，但因为一但处理的行发生变化，就要调用
    //! mysql_data_seek 和 mysql_fetch_row，重新定位游标，所以，还是建议一行行的
    //! 处理数据，处理完成一行后，再处理下一行。

    /*!
    * @brief      在当前行，根据列序号ID得到字段值,将数据的指针作为作为返回值
    * @return     const char* 数据的指针，返回nullptr表示取错误
    * @param      col     下标
    */
    const char* field_data(size_t row, size_t col);

    /*!
    * @brief      根据列序号ID得到当前行的字段值,
    * @return     int       0成功，-1失败
    * @param      row, col   行，列ID
    * @param      pfdata    列数据的指针
    */
    int field_data(size_t row, size_t col, char* pfdata);

    //! 根据列ID （col）或者列名称（fname）取得字段的（实际）长度
    size_t field_length(size_t row, size_t col);

    //! 根据列ID （col）取得当前行（游标的）字段值
    zce::mysql::field get_field(size_t row, size_t col);

    template <typename T>
    int field(size_t row, size_t col, T& val)
    {
        if (row != cursor_.cursor_rowid_)
        {
            if (cursor_seek(row))
            {
                return -1;
            }
        }
        return cursor_.field(col, val);
    }

    //将row行数据全部转化一个tuple
    template <typename... Types>
    std::tuple<Types...> make_tuple(size_t row)
    {
        if (row != cursor_.cursor_rowid_)
        {
            [[maybe_unused]]
            bool ret = cursor_seek(row);
            assert(ret == 0);
        }
        return cursor_.make_tuple<Types...>();
    }

private:
    ///结果集合
    MYSQL_RES* mysql_result_ = nullptr;

    ///结果集的行数
    size_t  num_result_row_ = 0;

    ///结果集的列数
    size_t  num_result_field_ = 0;

    ///MYSQL_FIELD数组指针,指向结果集合的所有Field说明.
    MYSQL_FIELD* mysql_fields_ = nullptr;

    ///游标，包括MYSQL_ROW等数据。
    zce::mysql::cursor  cursor_;
};

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
}
