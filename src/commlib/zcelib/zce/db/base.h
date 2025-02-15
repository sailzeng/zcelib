#pragma once

namespace zce::db
{
///连接器基类
class handle_base
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

    virtual bool is_connected()
    {
        return if_connected_;
    }

protected:
    ///SQL 命令
    std::string sql_cmd_;

    ///是否连接MYSQL数据库
    bool      if_connected_ = false;
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
