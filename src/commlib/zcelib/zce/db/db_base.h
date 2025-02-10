#pragma once

namespace zce::db
{
///连接器基类
class connect_base
{
public:
    /*!
    * @brief      设置SQL Command语句,为BIN型的SQL语句准备
    * @param      sqlcmd SQL语句
    * @param      szsql  SQL语句长度
    */
    int set_sql(const char* sqlcmd, size_t szsql);
    int set_sql(const std::string& sqlcmd);
    int set_sql(const char* sql_format, ...);

    int get_sql_command(char* cmdbuf, size_t& szbuf) const;
    const char* get_sql_command() const;
    void get_sql_command(std::string&) const;
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
