#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/mysql/stmt_cmd.h"
#include "zce/mysql/stmt_bind.h"
#include "zce/mysql/result.h"

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

namespace zce::mysql
{
stmt_cmd::stmt_cmd() :
    mysql_connect_(nullptr),
    mysql_stmt_(nullptr),
    is_bind_result_(false)
{
    //保留INITBUFSIZE的空间
    stmt_command_.reserve(SQL_INIT_BUFSIZE);
}

//指定一个connect
stmt_cmd::stmt_cmd(zce::mysql::connect* conn) :
    mysql_connect_(nullptr),
    mysql_stmt_(nullptr),
    is_bind_result_(false)
{
    //assert(conn != nullptr);
    if (conn != nullptr && conn->is_connected())
    {
        mysql_connect_ = conn;
        //处理
        mysql_stmt_ = mysql_stmt_init(mysql_connect_->get_mysql_handle());
    }

    //保留INITBUFSIZE的空间
    stmt_command_.reserve(SQL_INIT_BUFSIZE);
}

stmt_cmd::~stmt_cmd()
{
    if (nullptr != mysql_stmt_)
    {
        int tmpret = ::mysql_stmt_free_result(mysql_stmt_);
        tmpret = ::mysql_stmt_close(mysql_stmt_);
        ZCE_UNUSED_ARG(tmpret);
    }
}

//
int stmt_cmd::set_connect(zce::mysql::connect* conn)
{
    //检查参数
    if (conn != nullptr && conn->is_connected())
    {
        mysql_connect_ = conn;

        if (mysql_stmt_ != nullptr)
        {
            int tmpret = ::mysql_stmt_close(mysql_stmt_);
            if (tmpret != 0)
            {
                return tmpret;
            }
            mysql_stmt_ = nullptr;
        }

        mysql_stmt_ = ::mysql_stmt_init(mysql_connect_->get_mysql_handle());
        if (nullptr == mysql_stmt_)
        {
            return -1;
        }

        return 0;
    }

    //
    return 0;
}

//准备SQL,并且分析绑定的变量
int stmt_cmd::stmt_prepare_bind(zce::mysql::stmt_bind* bindparam,
                                zce::mysql::stmt_bind* bindresult)
{
    int tmpret = ::mysql_stmt_prepare(mysql_stmt_,
                                      stmt_command_.c_str(),
                                      static_cast<unsigned long>(stmt_command_.size()));
    if (tmpret != 0)
    {
        return tmpret;
    }

    is_bind_result_ = false;

    //原打算检查语句的绑定变量个数,后来决定还是让MySQL自己检查,
    //unsigned long paramcount = mysql_stmt_param_count(mysql_stmt_);
    //ZASSERT(paramcount > 0 &&  bindparam!=nullptr || paramcount == 0 && bindparam == nullptr);

    //绑定的参数
    if (bindparam)
    {
        tmpret = ::mysql_stmt_bind_param(mysql_stmt_,
                                         bindparam->get_stmt_bind_handle());
        if (tmpret != 0)
        {
            return tmpret;
        }
    }

    //绑定的结果
    if (bindresult)
    {
        tmpret = ::mysql_stmt_bind_result(mysql_stmt_,
                                          bindresult->get_stmt_bind_handle());
        //出错返回,或者处理
        if (tmpret != 0)
        {
            return tmpret;
        }

        is_bind_result_ = true;
    }

    return 0;
}

//设置SQL Command语句,为BIN型的SQL语句准备,同时绑定参数,结果
int stmt_cmd::set_stmt_command(const std::string& sqlcmd,
                               stmt_bind* bindparam,
                               stmt_bind* bindresult)
{
    stmt_command_ = sqlcmd;
    return stmt_prepare_bind(bindparam, bindresult);
}

//设置SQL Command语句,为BIN型的SQL语句准备,用于要帮定变量的SQL,结果
int stmt_cmd::set_stmt_command(const char* sqlcmd,
                               size_t szsql,
                               zce::mysql::stmt_bind* bindparam,
                               zce::mysql::stmt_bind* bindresult)
{
    ZCE_ASSERT(sqlcmd != nullptr);
    //
    stmt_command_.assign(sqlcmd, szsql);

    return stmt_prepare_bind(bindparam, bindresult);
}

//得到SQL Command语句
void stmt_cmd::get_stmt_command(std::string& sqlcmd) const
{
    sqlcmd = stmt_command_;
}

//SQL 执行命令，这个事一个基础函数，内部调用
int stmt_cmd::query_i(unsigned int* num_affect,
                      unsigned int* lastid)
{
    int tmpret = 0;

    //执行
    tmpret = ::mysql_stmt_execute(mysql_stmt_);
    if (tmpret != 0)
    {
        return tmpret;
    }

    //如果要返回结果,进行转储
    if (is_bind_result_)
    {
        tmpret = ::mysql_stmt_store_result(mysql_stmt_);
        if (tmpret != 0)
        {
            return tmpret;
        }
    }

    //执行SQL命令影响了多少行,mysql_affected_rows
    //必须在转储结果集后,所以你要注意输入的参数
    if (num_affect)
    {
        *num_affect = (unsigned int) ::mysql_stmt_affected_rows(mysql_stmt_);
    }

    if (lastid)
    {
        *lastid = (unsigned int) ::mysql_stmt_insert_id(mysql_stmt_);
    }

    //成功
    return 0;
}

//执行SQL语句,不用输出结果集合的那种
int stmt_cmd::query(unsigned int& num_affect, unsigned int& lastid)
{
    return query_i(&num_affect, &lastid);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,
int stmt_cmd::query(unsigned int& num_affect)
{
    return query_i(&num_affect, nullptr);
}

//
int stmt_cmd::fetch_row_next() const
{
    int tmpret = ::mysql_stmt_fetch(mysql_stmt_);
    if (tmpret != 0)
    {
        return -1;
    }

    return 0;
}

//
int stmt_cmd::fetch_column(MYSQL_BIND* bind,
                           unsigned int column,
                           unsigned int offset) const
{
    int tmpret = ::mysql_stmt_fetch_column(mysql_stmt_, bind, column, offset);
    if (0 != tmpret)
    {
        return -1;
    }

    return 0;
}

//
int stmt_cmd::seek_result_row(unsigned int nrow) const
{
    //检查结果集合为空,或者参数row错误
    ::mysql_stmt_data_seek(mysql_stmt_, nrow);
    int tmpret = ::mysql_stmt_fetch(mysql_stmt_);
    if (0 != tmpret)
    {
        return -1;
    }

    return 0;
}

//
void stmt_cmd::param_2_metadata(zce::mysql::result* tmpres) const
{
    MYSQL_RES* myres = ::mysql_stmt_param_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return;
}

//
void stmt_cmd::result_2_metadata(zce::mysql::result* tmpres) const
{
    MYSQL_RES* myres = ::mysql_stmt_result_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return;
}
}

#endif //MYSQL_VERSION_ID >= 40100