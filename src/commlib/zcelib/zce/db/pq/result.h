#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

#include "zce/bytes/serialize.h"
#include "zce/string/from_string.h"

namespace zce::pq
{
static const int FMT_BINARY = 0x1;

static const int FMT_TEXT = 0x0;

enum PG_OID_TYPE
{
    PG_BOOL = 16,
    PG_BYTEA = 17,
    PG_INT8 = 20,
    PG_INT2 = 21,
    PG_INT4 = 23,
    PG_TEXT = 25,

    PG_FLOAT4 = 700,
    PG_FLOAT8 = 701,

    PG_CHAR = 1042,
    PG_VARCHAR = 1043,

    PG_DATE = 1082,
    PG_TIME = 1083,
    PG_TIMESTAMP = 1114,
    PG_TIMESTAMPTZ = 1184,
    PG_INTERVAL = 1186,

    PG_NUMERIC = 1700,

    PG_JSON = 114,
    PG_JSONB = 3802,

    PG_UUID = 2950,

    // 常用数组类型
    PG_INT4_ARRAY = 1007,
    PG_TEXT_ARRAY = 1009,
    PG_VARCHAR_ARRAY = 1015,
    PG_TIMESTAMP_ARRAY = 1115,
    PG_UUID_ARRAY = 2951,
};

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

    struct interval
    {
        int64_t time_usec = 0;  // 微秒
        int32_t days = 0;
        int32_t months = 0;
    };

    //! 当结果格式为二进制时，PQ的TIME，DATE，TIMESTAMP，TIMESTAMPTZ，INTERVAL类型的值要进行特殊处理
    class time
    {
    public:
        static const time_t PG_EPOCH = 946684800; // 2000-01-01 00:00:00
    public:
        //! @brief 将PG的date转换为zce::ztm
        static int parse_date(int32_t date, zce::ztm* pztm);
        //! @brief 将PG的time转换为zce::ztm
        static int parse_time(double time, zce::ztm* pztm);
        //! @brief 将PG的timestamp转换为zce::ztm
        static int parse_timestamp(int64_t timestamp, zce::ztm* pztm);
        //! @brief 将PG的interval转换为zce::ztm
        static int parse_interval(interval intvl_val,
                                  zce::ztm* pztm);
    };

    //! @brief   设置PQ的结果集
    void set_result(::PGresult* res);

    //! @brief   释放结果集
    void clear();

    ///结果集合是否为空
    inline bool is_null()
    {
        return pq_result_ == nullptr ? true : false;
    }

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

    /*!
    * @brief      检索到下一行，返回true,其实有点类似Orale的光标处理，呵呵
    * @return     bool true还有结果集合，false没有结果集合了
    * @note       PQ 其实没有光标概念，我们增加了一个当前行作为匹配。
    */
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

    //! @brief 根据col返回表定义列域名,注意计算得到的列的名字也可能是空
    const char* field_name(size_t col) const;

    //! @brief 根据Field Name得到Field ID,列号 返回-1表示没有找到
    size_t field_index(const char* fname) const;

    //! 根据列号 （col）取得字段定义长度
    size_t field_def_size(size_t col) const;

    //! 根据列号取得其格式，返回0文本，1二进制
    int field_format(size_t col) const;

    //! 根据列号取得类型Oid
    ::Oid field_type(size_t col) const;

    //! 根据行号，列号，取得字段的（实际）长度
    size_t field_length(size_t row, size_t col) const;

    //! 根据行号，列号，取得字段的的数据
    const char* field_data(size_t row, size_t col) const;

    template <typename T>
    int field(size_t row, size_t col, T& val) const
    {
        //返回字段为文本类型
        auto ffmt = field_format(col) == 0;
        if (ffmt == FMT_TEXT)
        {
            return zce::from_str(::PQgetvalue(pq_result_, (int)row, (int)col), val);
        }
        else if (ffmt == FMT_BINARY)
        {
            zce::ser::decode dc(::PQgetvalue(pq_result_, (int)row, (int)col),
                                (size_t)::PQgetlength(pq_result_, (int)row, (int)col));
            //dc.read(val);
            return 0;
        }
        else
        {
            return -1;
        }
    }

    template <>
    int field(size_t row, size_t col, zce::ztm& val) const;

    template <typename T>
    T field(size_t row, size_t col) const
    {
        T val;
        field(row, col, val);
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

    //! PG 结果集
    ::PGresult* pq_result_ = nullptr;

    //! 结果集的行数
    size_t  num_result_row_ = 0;

    //! 结果集的列数
    size_t  num_result_field_ = 0;

    //! 光标所在行
    size_t  cursor_row_ = (size_t)-1;
};
}
#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
