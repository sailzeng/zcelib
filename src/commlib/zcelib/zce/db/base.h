#pragma once

namespace zce::db
{
///连接器基类
class connect_base
{
public:

    void set_sql(std::string_view sql_cmd)
    {
        sql_cmd_ = sql_cmd;
    }

    template<typename... Args>
    void format_sql(std::string_view rt_fmt_str, Args&&... args)
    {
        sql_cmd_ = std::vformat(rt_fmt_str, std::make_format_args(args...));
    }

    void get_sql(std::string &sql_cmd) const
    {
        sql_cmd = sql_cmd_;
    }

protected:
    ///SQL
    std::string sql_cmd_;
};

///查询结果基类
class result_base
{
public:
};

class stmt_base
{
public:
};
}
