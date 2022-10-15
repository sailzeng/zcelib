#include "zce_predefine.h"
#include "zce_mysql_stmtcmd.h"
#include "zce_mysql_stmtbind.h"
#include "zce_mysql_result.h"

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

ZCE_Mysql_STMT_Command::ZCE_Mysql_STMT_Command():
    mysql_connect_(NULL),
    mysql_stmt_(NULL),
    is_bind_result_(false)
{
    //保留INITBUFSIZE的空间
    stmt_command_.reserve(SQL_INIT_BUFSIZE);
}

//指定一个connect
ZCE_Mysql_STMT_Command::ZCE_Mysql_STMT_Command(ZCE_Mysql_Connect *conn ):
    mysql_connect_(NULL),
    mysql_stmt_(NULL),
    is_bind_result_(false)
{

    //assert(conn != NULL);
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
        //处理
        mysql_stmt_ = mysql_stmt_init(mysql_connect_->get_mysql_handle());
    }

    //保留INITBUFSIZE的空间
    stmt_command_.reserve(SQL_INIT_BUFSIZE);
}

ZCE_Mysql_STMT_Command::~ZCE_Mysql_STMT_Command()
{
    if ( NULL  != mysql_stmt_)
    {
        int tmpret = ::mysql_stmt_free_result(mysql_stmt_);
        tmpret = ::mysql_stmt_close(mysql_stmt_);
        ZCE_UNUSED_ARG(tmpret);
    }
}

//
int ZCE_Mysql_STMT_Command::set_connection(ZCE_Mysql_Connect *conn)
{
    //检查参数
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;

        if (mysql_stmt_ != NULL)
        {
            int tmpret = ::mysql_stmt_close(mysql_stmt_);
            if (tmpret != 0)
            {
                return tmpret;
            }
            mysql_stmt_ = NULL;
        }

        mysql_stmt_ = ::mysql_stmt_init(mysql_connect_->get_mysql_handle());
        if (NULL == mysql_stmt_)
        {
            return -1;
        }

        return 0;
    }

    //
    return 0;
}


//准备SQL,并且分析绑定的变量
int ZCE_Mysql_STMT_Command::stmt_prepare_bind(ZCE_Mysql_STMT_Bind *bindparam,
                                              ZCE_Mysql_STMT_Bind *bindresult)
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
    //ZASSERT(paramcount > 0 &&  bindparam!=NULL || paramcount == 0 && bindparam == NULL);

    //绑定的参数
    if (bindparam)
    {
        tmpret = ::mysql_stmt_bind_param(mysql_stmt_, bindparam->get_stmt_bind_handle());
        if (tmpret != 0)
        {
            return tmpret;
        }
    }

    //绑定的结果
    if (bindresult)
    {
        tmpret = ::mysql_stmt_bind_result(mysql_stmt_, bindresult->get_stmt_bind_handle());
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
int ZCE_Mysql_STMT_Command::set_stmt_command(const std::string &sqlcmd, ZCE_Mysql_STMT_Bind *bindparam, ZCE_Mysql_STMT_Bind *bindresult)
{
    stmt_command_ = sqlcmd;
    return stmt_prepare_bind(bindparam, bindresult);
}

//设置SQL Command语句,为BIN型的SQL语句准备,用于要帮定变量的SQL,结果
int ZCE_Mysql_STMT_Command::set_stmt_command(const char *sqlcmd, size_t szsql, ZCE_Mysql_STMT_Bind *bindparam, ZCE_Mysql_STMT_Bind *bindresult)
{
    ZCE_ASSERT(sqlcmd != NULL);
    //
    stmt_command_.assign(sqlcmd, szsql);

    return stmt_prepare_bind(bindparam, bindresult);
}

//SQL 执行命令，这个事一个基础函数，内部调用
int ZCE_Mysql_STMT_Command::_execute(unsigned int *num_affect,
                                     unsigned int *lastid)
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
int ZCE_Mysql_STMT_Command::execute(unsigned int &num_affect, unsigned int &lastid)
{
    return _execute(&num_affect, &lastid);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,
int ZCE_Mysql_STMT_Command::execute(unsigned int &num_affect)
{
    return _execute(&num_affect, NULL);
}

//
int ZCE_Mysql_STMT_Command::fetch_row_next() const
{
    int tmpret = ::mysql_stmt_fetch(mysql_stmt_);

    if (tmpret != 0)
    {
        return -1;
    }

    return 0;

}

//
int  ZCE_Mysql_STMT_Command::fetch_column(MYSQL_BIND *bind, unsigned int column, unsigned int offset) const
{
    int tmpret = ::mysql_stmt_fetch_column(mysql_stmt_, bind, column, offset);
    if (0 != tmpret )
    {
        return -1;
    }

    return 0;
}

//
int ZCE_Mysql_STMT_Command::seek_result_row(unsigned int nrow) const
{
    //检查结果集合为空,或者参数row错误
    ::mysql_stmt_data_seek(mysql_stmt_, nrow);
    int tmpret  = ::mysql_stmt_fetch(mysql_stmt_);
    if (0 != tmpret)
    {
        return -1;
    }

    return 0;
}

//
void ZCE_Mysql_STMT_Command::param_2_metadata(ZCE_Mysql_Result *tmpres) const
{
    MYSQL_RES *myres =  ::mysql_stmt_param_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return ;
}

//
void ZCE_Mysql_STMT_Command::result_2_metadata(ZCE_Mysql_Result *tmpres) const
{
    MYSQL_RES *myres =  ::mysql_stmt_result_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return ;
}

#endif //MYSQL_VERSION_ID >= 40100

