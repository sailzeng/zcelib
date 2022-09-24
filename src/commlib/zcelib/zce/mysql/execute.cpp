#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/mysql/execute.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

namespace zce::mysql
{
//-----------------------------------------------------------------

//初始化服务器,使用hostname进行连接,可以不立即连接和立即连接，你自己控制。
int execute::init_connect(const char* host_name,
                          const char* user,
                          const char* pwd,
                          unsigned int port,
                          bool connect_atonce)
{
    db_hostname_ = host_name;
    db_user_name_ = user;
    db_password_ = pwd;
    db_port_ = port;

    if (connect_atonce)
    {
        return connect();
    }

    return 0;
}

//
int execute::connect()
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        //如果设置过HOST，用HOST NAME进行连接
        if (db_hostname_.length() > 0)
        {
            ret = db_connect_.connect_by_host(db_hostname_.c_str(),
                                              db_user_name_.c_str(),
                                              db_password_.c_str(),
                                              NULL,
                                              db_port_);
        }
        else if (db_socket_file_.length() > 0)
        {
            ret = db_connect_.connect_by_socketfile(db_socket_file_.c_str(),
                                                    db_user_name_.c_str(),
                                                    db_password_.c_str());
        }
        else
        {
            ZCE_ASSERT(false);
        }

        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error : [%u]:%s.",
                    db_connect_.error_no(),
                    db_connect_.error_message()
            );
            return -1;
        }

        //
        db_command_.set_connection(&db_connect_);
    }

    return  0;
}

//断开链接
void execute::disconnect()
{
    if (db_connect_.is_connected() == true)
    {
        db_connect_.disconnect();
    }
}

//用于非SELECT语句(INSERT,UPDATE)，
int execute::query(const char* sql,
                   size_t sql_len,
                   uint64_t& num_affect,
                   uint64_t& insert_id)
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    //
    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sql_len, sql);
    db_command_.set_sql_command(sql, sql_len);

    ret = db_command_.query(num_affect, insert_id);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.error_no(),
                db_connect_.error_message(),
                sql);
        return -1;
    }
    //成功
    return 0;
}

//执行家族的SQL语句,用于SELECT语句,直接转储结果集合的方法
int execute::query(const char* sql,
                   size_t sql_len,
                   uint64_t& num_affect,
                   zce::mysql::result& db_result)
{
    int ret = 0;
    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql_len, sql);
    db_command_.set_sql_command(sql, sql_len);

    ret = db_command_.query(num_affect, db_result);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                db_connect_.error_no(),
                db_connect_.error_message(),
                sql);
        return -1;
    }
    //成功
    return 0;
}

//
int execute::query(const char* sql,
                   size_t sql_len,
                   zce::mysql::result& db_result)
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql_len, sql);
    db_command_.set_sql_command(sql, sql_len);

    ret = db_command_.query(db_result);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.error_no(),
                db_connect_.error_message(),
                sql);
        return -1;
    }
    //成功
    return 0;
}

//返回的的DB访问的错误信息
unsigned int execute::error_message(char* szerr, size_t buflen)
{
    snprintf(szerr, buflen, "[%d]:%s ",
             db_connect_.error_no(),
             db_connect_.error_message());
    return db_connect_.error_no();
}

//得到DB访问的语句
const char* execute::get_query_sql(void)
{
    return db_command_.get_sql_command();
}

//得到错误信息语句
const char* execute::error_message()
{
    return db_connect_.error_message();
}

//得到错误信息ID
unsigned int execute::error_id()
{
    return db_connect_.error_no();
}

//得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
//Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
unsigned int execute::make_real_escape_string(char* tostr,
                                              const char* fromstr,
                                              unsigned int fromlen)
{
    return ::mysql_real_escape_string(db_connect_.get_mysql_handle(),
                                      tostr, fromstr, fromlen);
}
}//namesapce zce::mysql

namespace zce::mysql::exe
{
int connect(zce::mysql::connect* db_connect,
            const char* host_name,
            const char* user,
            const char* pwd,
            unsigned int port)
{
    int ret = 0;

    //连接数据库
    if (db_connect->is_connected() == false)
    {
        //如果设置过HOST，用HOST NAME进行连接

        ret = db_connect->connect_by_host(host_name,
                                          user,
                                          pwd,
                                          NULL,
                                          port);

        //如果错误
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zcelib] DB Error : [%u]:%s.",
                    db_connect->error_no(),
                    db_connect->error_message());
            return -1;
        }
    }
    return 0;
}

//!断开链接
void disconnect(zce::mysql::connect* db_connect)
{
    if (db_connect->is_connected() == true)
    {
        db_connect->disconnect();
    }
}

//!查询，
int query(zce::mysql::connect* db_connect,
          const char* sql,
          size_t sqllen,
          uint64_t* num_affect,
          uint64_t* insert_id)
{
    //连接数据库
    if (db_connect->is_connected() == false)
    {
        -1;
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect->ping();
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sqllen, sql);
    zce::mysql::command db_command;
    db_command.set_connection(db_connect);
    db_command.set_sql_command(sql, sqllen);
    int ret = db_command.query(*num_affect, *insert_id);
    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect->error_no(),
                db_connect->error_message(),
                sql);
        return -1;
    }

    //成功
    return 0;
}

//!
int query(zce::mysql::connect* db_connect,
          const char* sql,
          size_t sql_len,
          uint64_t* num_affect,
          zce::mysql::result* db_result)
{
    int ret = 0;
    //连接数据库
    if (db_connect->is_connected() == false)
    {
        -1;
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect->ping();
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql_len, sql);
    zce::mysql::command db_command;
    db_command.set_connection(db_connect);
    db_command.set_sql_command(sql, sql_len);
    ret = db_command.query(*num_affect, *db_result);
    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                db_connect->error_no(),
                db_connect->error_message(),
                sql);
        return -1;
    }

    //成功
    return 0;
}

//!
int query(zce::mysql::connect* db_connect,
          const char* sql,
          size_t sql_len,
          zce::mysql::result* db_result)
{
    int ret = 0;
    //连接数据库
    if (db_connect->is_connected() == false)
    {
        -1;
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect->ping();
    }

    ZCE_LOG_DEBUG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sql_len, sql);
    zce::mysql::command db_command;
    db_command.set_connection(db_connect);
    db_command.set_sql_command(sql, sql_len);
    ret = db_command.query(*db_result);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                db_connect->error_no(),
                db_connect->error_message(),
                sql);
        return -1;
    }
    //成功
    return 0;
}
}//namesapce exe

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL