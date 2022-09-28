#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/mysql/command.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

namespace zce::mysql
{
command::command() :
    mysql_connect_(NULL)
{
    //保留INITBUFSIZE的空间
    mysql_command_.reserve(INITBUFSIZE);
    sql_buffer_ = new char[INITBUFSIZE];
}

command::command(zce::mysql::connect* conn) :
    mysql_connect_(NULL)
{
    //assert(conn != NULL);
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
    }

    //保留INITBUFSIZE的空间
    mysql_command_.reserve(INITBUFSIZE);
    sql_buffer_ = new char[INITBUFSIZE];
}

command::~command()
{
    if (sql_buffer_)
    {
        delete sql_buffer_;
        sql_buffer_ = NULL;
    }
}

//为Command设置相关的连接对象，而且是必须已经成功连接上数据的
int command::set_connect(zce::mysql::connect* conn)
{
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
        return 0;
    }

    return -1;
}

///设置SQL Command语句,动态参数版本
int command::set_sql_command(const char* sql_format, ...)
{
    va_list args;
    va_start(args, sql_format);

    //_vsnprintf不是ANSI C标准函数,但是大部分函数库应该实现了它,毕竟vsprintf缺乏基本的安全感
    int ret = vsnprintf(sql_buffer_, INITBUFSIZE, sql_format, args);

    va_end(args);

    //如果返回结果错误,_vsnprintf =-1表示Buf长度不够
    if (ret < 0)
    {
        return -1;
    }

    //设置
    mysql_command_.assign(sql_buffer_);

    //成功
    return 0;
}

//得到SQL command. 重载多种形式,用于文本类型
const char* command::get_sql_command() const
{
    return mysql_command_.c_str();
}

// 得到SQL 语句. 类型数据,传入的char buf长度是否足够自己保证
int command::get_sql_command(char* cmdbuf, size_t& szbuf) const
{
    if (cmdbuf == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    size_t size_sql = mysql_command_.length();

    if (size_sql + 1 > szbuf)
    {
        return -1;
    }

    szbuf = size_sql;
    ::memcpy(cmdbuf, mysql_command_.c_str(), szbuf);
    return 0;
}

//
void command::get_sql_command(std::string& strcmd) const
{
    //预先分配内存,保证效率
    strcmd.reserve(mysql_command_.length());
    strcmd = mysql_command_;
}

//int 返回是否成功还是失败 MYSQL_RETURN_FAIL表示失败
//执行SQL语句，功能全集，不对外使用
int command::query(uint64_t* num_affect,
                   uint64_t* last_id,
                   zce::mysql::result* sql_result,
                   bool bstore)
{
    //如果没有设置连接或者没有设置命令
    if (mysql_connect_ == NULL || mysql_command_.empty())
    {
        return -1;
    }

    //执行SQL命令
    int tmpret = ::mysql_real_query(mysql_connect_->get_mysql_handle(),
                                    mysql_command_.c_str(),
                                    (unsigned long)mysql_command_.length());
    if (tmpret != 0)
    {
        return tmpret;
    }

    //如果用户要求转储结果集
    if (sql_result)
    {
        MYSQL_RES* tmp_res = NULL;
        if (bstore)
        {
            //转储结果
            tmp_res = ::mysql_store_result(mysql_connect_->get_mysql_handle());
        }
        else
        {
            //转储结果
            tmp_res = ::mysql_use_result(mysql_connect_->get_mysql_handle());
        }

        //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
        //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回NULL? "
        //如果是INSERT语句，那么mysql_store_result就是返回NULL，mysql_field_count也应该等于0，
        //如果MYSQL内部发生某个错误，那么mysql_store_result 返回NULL，但mysql_field_count 会大于0，此时是个错误
        if (tmp_res == NULL && mysql_field_count(mysql_connect_->get_mysql_handle()) > 0)
        {
            return -1;
        }

        //得到结果集,查询结果集信息
        sql_result->set_mysql_result(tmp_res);
    }

    //执行SQL命令影响了多少行,mysql_affected_rows 必须在转储结果集后,所以你要注意输入的参数
    if (num_affect)
    {
        *num_affect = (uint64_t) ::mysql_affected_rows(mysql_connect_->get_mysql_handle());
    }

    if (last_id)
    {
        *last_id = (uint64_t) ::mysql_insert_id(mysql_connect_->get_mysql_handle());
    }

    //成功
    return 0;
}

//执行SQL语句,不用输出结果集合的那种,非SELECT语句
//num_affect 为返回参数,告诉你修改了几行
int command::query(uint64_t& num_affect, uint64_t& last_id)
{
    return query(&num_affect, &last_id, NULL, false);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
//num_affect 为返回参数,告诉你修改了几行,SELECT了几行
int command::query(uint64_t& num_affect, zce::mysql::result& sql_result)
{
    return query(&num_affect, NULL, &sql_result, true);
}

//执行SQL语句,SELECT语句,USE结果集合的那种,注意其调用的是mysql_use_result,num_affect对它无效
//用于结果集太多的处理,如果一次转储结果集会占用太多内存的处理,可以考虑用它,
//但不推荐使用,一次取一行,交互太多
int command::query(zce::mysql::result& sql_result)
{
    return query(NULL, NULL, &sql_result, false);
}

//得到connect 的句柄
zce::mysql::connect* command::get_connect()
{
    return mysql_connect_;
}

//得到错误信息
const char* command::error_message()
{
    return mysql_connect_->error_message();
}

//得到错误的ID
unsigned int command::error_no()
{
    return mysql_connect_->error_no();
}

//SQL预计的赋值，
int command::set_sql_command(const char* sqlcmd, size_t szsql)
{
    //如果错误,返回
    if (sqlcmd == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    //
    mysql_command_.assign(sqlcmd, szsql);
    return 0;
}

//为TXT,BIN二进制的SQL命令提供的赋值方式 ,
int command::set_sql_command(const std::string& sqlcmd)
{
    mysql_command_ = sqlcmd;
    return 0;
}

//
command& command::operator =(const char* sqlcmd)
{
    set_sql_command(sqlcmd);
    return *this;
}
//
command& command::operator =(const std::string& sqlcmd)
{
    set_sql_command(sqlcmd);
    return *this;
}

//
command& command::operator +=(const char* sqlcmd)
{
    mysql_command_.append(sqlcmd);
    return *this;
}

command& command::operator +=(const std::string& sqlcmd)
{
    mysql_command_.append(sqlcmd);
    return *this;
}

#if MYSQL_VERSION_ID > 40100

//用于 multiple-statement executions 中得到多个
//如果
int command::fetch_next_multi_result(zce::mysql::result& sqlresult, bool bstore)
{
    int tmpret = ::mysql_next_result(mysql_connect_->get_mysql_handle());

    //tmpret == -1表示没有结果集,其他<0的值表示错误
    if (tmpret < 0)
    {
        return -1;
    }

    MYSQL_RES* tmp_res = NULL;

    if (bstore)
    {
        //转储结果
        tmp_res = ::mysql_store_result(mysql_connect_->get_mysql_handle());
    }
    else
    {
        //转储结果
        tmp_res = ::mysql_use_result(mysql_connect_->get_mysql_handle());
    }

    //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
    //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回NULL? "
    if (tmp_res == NULL && ::mysql_field_count(mysql_connect_->get_mysql_handle()) > 0)
    {
        return -1;
    }

    //得到结果集,查询结果集信息
    sqlresult.set_mysql_result(tmp_res);

    //成功
    return 0;
}

//得到错误信息
int command::set_auto_commit(bool bauto)
{
    return mysql_connect_->set_auto_commit(bauto);
}

//得到错误信息
int command::trans_commit()
{
    return mysql_connect_->trans_commit();
}

//得到错误信息
int command::trans_rollback()
{
    return mysql_connect_->trans_rollback();
}

#endif //MYSQL_VERSION_ID > 40100
}
//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL