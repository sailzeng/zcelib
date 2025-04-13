#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

#include "zce/bytes/serialize.h"
#include "zce/string/from_string.h"

namespace zce::pq
{
static const int FMT_BINARY = 0x1;

static const int FMT_TEXT = 0x0;

/*!
 * @brief PQ 的结果集
 */
class result
{
public:

    //!构造函数,析构函数
    result() noexcept = default;
    result(::PGresult* res) noexcept;
    ~result() noexcept;

    //拷贝构造函数，不使用
    result(const result&) = delete;
    result& operator=(const result&) = delete;
    //移动构造函数
    result(result&&) noexcept;
    result& operator=(result&&) noexcept;

public:
    //! @brief   设置PQ的结果集
    void set_result(::PGresult* res);

    //! @brief   释放结果集
    void clear();

    ///结果集合是否为空
    inline bool is_null()
    {
        return pq_result_ ? true : false;
    }

    //! @brief      返回结果集的行数目
    inline size_t num_of_rows() const
    {
        return num_result_row_;
    }

    //! @brief      返回结果集的列数目
    inline size_t num_of_fields() const
    {
        return num_result_field_;
    }

    /*!
    * @brief      检索到下一行，返回true,其实有点类似Orale的光标处理，呵呵
    * @return     bool true还有结果集合，false没有结果集合了
    * @note       PQ 其实没有光标概念，我们增加了一个当前行作为匹配。
    */
    bool cursor_fetch();

    //! @brief 将结果集处理的行，检索移动到某行
    bool cursor_seek(size_t row_id);

    template <typename T>
    int cursor_field(size_t colum, T& val)
    {
        return field(cursor_row_, colum, val);
    }

    //! @brief 根据colum返回表定义列域名,注意计算得到的列的名字也可能是空
    const char* field_name(size_t colum) const;

    //! @brief 根据Field Name得到Field ID,列号 返回-1表示没有找到
    size_t field_index(const char* fname) const;

    //! 根据列号 （colum）取得字段定义长度
    size_t field_def_size(size_t colum) const;

    //! 根据列号取得其格式，返回0文本，1二进制
    int field_format(size_t colum) const;

    //! 根据列号取得类型Oid
    ::Oid field_type(size_t colum) const;

    //! 根据行号，列号，取得字段的（实际）长度
    size_t field_length(size_t row, size_t colum) const;

    //! 根据行号，列号，取得字段的的数据
    const char* field_data(size_t row, size_t colum) const;

    template <typename T>
    int field(size_t row, size_t colum, T& val) const
    {
        //返回字段为文本类型
        auto ffmt = field_format(colum) == 0;
        if (ffmt == FMT_TEXT)
        {
            return zce::from_str(::PQgetvalue(pq_result_, (int)row, (int)colum), val);
        }
        else if (ffmt == FMT_BINARY)
        {
            zce::ser::decode dc(::PQgetvalue(pq_result_, (int)row, (int)colum),
                                (size_t)::PQgetlength(pq_result_, (int)row, (int)colum));
            dc.read(val);
            return 0;
        }
        else
        {
            return -1;
        }
    }

    template <typename T>
    T field(size_t row, size_t colum) const
    {
        T val;
        field(row, colum, val);
        return val;
    }

    //将光标的数据全部转化一个tuple
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

    ///PG 结果集
    ::PGresult* pq_result_ = nullptr;

    ///结果集的行数
    size_t  num_result_row_ = 0;

    ///结果集的列数
    size_t  num_result_field_ = 0;

    /// 光标所在行
    size_t  cursor_row_ = (size_t)-1;
};
}
#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
