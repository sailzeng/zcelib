
#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_mysql_process.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

//-----------------------------------------------------------------
//实例,为了SingleTon类准备
ZCE_Mysql_Process *ZCE_Mysql_Process::instance_ = NULL;

//构造函数，和析构函数
ZCE_Mysql_Process::ZCE_Mysql_Process():
    db_port_(MYSQL_PORT)
{

}
ZCE_Mysql_Process::~ZCE_Mysql_Process()
{
    //不用处理什么，相关的成员变量的析构都进行了处理
}

//初始化服务器,使用hostname进行连接,可以不立即连接和立即连接，你自己控制。
int ZCE_Mysql_Process::init_mysql_server(const char *host_name,
                                         const char *user,
                                         const char *pwd,
                                         unsigned int port,
                                         bool connect_atonce)
{

    db_hostname_ = host_name;
    db_user_name_ = user;
    db_password_ = pwd;
    db_port_ = port;

    if (connect_atonce)
    {
        return connect_mysql_server();
    }

    return 0;
}

//初始化MYSQL，使用UNIX socket file连接(UNIX下)，或者命名管道(Windows下),只初始化也可以了,只能用于本地
int ZCE_Mysql_Process::init_mysql_socketfile(const char *socket_file,
                                             const char *user,
                                             const char *pwd,
                                             bool connect_atonce)
{
    db_socket_file_ = socket_file;
    db_user_name_ = user;
    db_password_ = pwd;

    if (connect_atonce)
    {
        return connect_mysql_server();
    }

    return 0;
}

//
int ZCE_Mysql_Process::connect_mysql_server( )
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
                    db_connect_.get_error_no(),
                    db_connect_.get_error_message()
                   );
            return -1;
        }

        //
        db_command_.set_connection(&db_connect_);
    }

    return  0;
}

//断开链接
void ZCE_Mysql_Process::disconnect_mysql_server()
{

    if (db_connect_.is_connected() == true)
    {
        db_connect_.disconnect();
    }

}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004年9月22日
Function        : ZCE_Mysql_Process::db_process_query
Return          : int
Parameter List  :
Param1: const char* sql SQL语句
Param2: unsigned int& numaffect 返回的收到影响的记录条数
Param3: unsigned int& insertid  返回的插入的LAST_INSERT_ID
Param4: size_t sqllen           SQL语句长度,用于BIN的SQL语句,默认为0,表示使用STR的SQL
Description     : 执行家族的SQL语句,用于非SELECT语句(INSERT,UPDATE)
Calls           :
Called By       :
Other           : 注意:几个db_process_query函数连接周期就是对象生存周期,ZCE_Mysql_Connect对象再析构时断链接
Modify Record   : 自动重连功能加入
******************************************************************************************/
int ZCE_Mysql_Process::db_process_query(const char *sql,
                                        unsigned int &numaffect,
                                        unsigned int &insertid,
                                        size_t sqllen)
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    //TEXT类型的SQL
    if (sqllen == 0)
    {
        ZCE_LOGMSG_DBG(RS_DEBUG, "[db_process_query]SQL:[%s].", sql);
        db_command_.set_sql_command(sql, strlen(sql));
    }
    //BIN类型的SQL
    else
    {
        ZCE_LOGMSG_DBG(RS_DEBUG, "[db_process_query]SQL:[%.*s].", sqllen, sql);
        db_command_.set_sql_command(sql, sqllen);
    }

    ret = db_command_.execute(numaffect, insertid);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //成功
    return  0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004年9月22日
Function        : ZCE_Mysql_Process::db_process_query
Return          : int
Parameter List  :
Param1: const char* sql         SQL语句
Param2: unsigned int& numaffect 返回参数,返回的查询的记录个数
Param3: ZCE_Mysql_Result& dbresult 返回参数,查询的结果集合
Param4: size_t sqllen           SQL语句长度,用于BIN的SQL语句,默认为0,表示使用STR的SQL
Description     : 执行家族的SQL语句,用于SELECT语句,直接转储结果集合的方法
Calls           :
Called By       :
Other           : 注意:几个db_process_query函数的连接周期就是对象生存周期,ZCE_Mysql_Connect对象再析构时断链接
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_Process::db_process_query(const char *sql, unsigned int &numaffect, ZCE_Mysql_Result &dbresult, size_t sqllen)
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    //TEXT类型的SQL
    if (sqllen == 0)
    {
        ZCE_LOGMSG_DBG(RS_DEBUG, "[db_process_query]SQL:[%s]", sql);
        db_command_.set_sql_command(sql, strlen(sql));
    }
    //BIN类型的SQL
    else
    {
        ZCE_LOGMSG_DBG(RS_DEBUG, "[db_process_query]SQL:[%.*s]", sqllen, sql);
        db_command_.set_sql_command(sql, sqllen);
    }

    ret = db_command_.execute(numaffect, dbresult);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s.",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //成功
    return  0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004年9月22日
Function        : ZCE_Mysql_Process::db_process_query
Return          : int
Parameter List  :
Param1: const char*             SQL语句
Param2: ZCE_Mysql_Result& dbresult 返回的结果结合
Param3: size_t sqllen SQL       语句长度
Description     : 用于SELECT语句,用于use_result得到结果集合的方法
Calls           :
Called By       :
Other           : 用于结果集太多,会占用太多内存的的处理,不推荐使用,
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_Process::db_process_query(const char *sql, ZCE_Mysql_Result &dbresult, size_t sqllen)
{
    int ret = 0;

    //连接数据库
    if (db_connect_.is_connected() == false)
    {
        connect_mysql_server();
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        db_connect_.ping();
    }

    //TEXT类型的SQL
    if (sqllen == 0)
    {
        db_command_.set_sql_command(sql, strlen(sql));
    }
    //BIN类型的SQL
    else
    {
        db_command_.set_sql_command(sql, sqllen);
    }

    ret = db_command_.execute(dbresult);

    //如果错误
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] DB Error:[%u]:[%s]. SQL:%s",
                db_connect_.get_error_no(),
                db_connect_.get_error_message(),
                sql);
        return -1;
    }

    //成功
    return  0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年3月9日
Function        : ZCE_Mysql_Process::get_return_error
Return          : unsigned int 返回的错误Number
Parameter List  :
Param1: char* szerr   返回的错误信息
Param2: size_t buflen Buf的长度
Description     : 返回的的DB访问的错误信息
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
unsigned int ZCE_Mysql_Process::get_return_error(char *szerr, size_t buflen)
{
    snprintf(szerr, buflen, "[%d]:%s ", db_connect_.get_error_no(), db_connect_.get_error_message());
    return db_connect_.get_error_no();
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年8月18日
Function        : ZCE_Mysql_Process::get_query_sql
Return          : static const char*
Parameter List  :
Param1: void
Description     : 得到DB访问的语句
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
const char *ZCE_Mysql_Process::get_query_sql(void)
{
    return db_command_.get_sql_command();
}

//得到错误信息语句
const char *ZCE_Mysql_Process::get_return_error_str()
{
    return db_connect_.get_error_message();
}

//得到错误信息ID
unsigned int ZCE_Mysql_Process::get_return_error_id()
{
    return db_connect_.get_error_no();
}

//得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
//Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
unsigned int ZCE_Mysql_Process::make_real_escape_string(char *tostr,
                                                        const char *fromstr,
                                                        unsigned int fromlen)
{
    return mysql_real_escape_string(db_connect_.get_mysql_handle(), tostr, fromstr, fromlen);
}

//-----------------------------------------------------------------
//实例,为了SingleTon类准备

//实例赋值
void ZCE_Mysql_Process::instance(ZCE_Mysql_Process *instance)
{
    clean_instance();
    instance_ = instance;
}
//获得实例
ZCE_Mysql_Process *ZCE_Mysql_Process::instance()
{
    if (instance_)
    {
        delete instance_ ;
        instance_ = NULL;
    }

    instance_ = new ZCE_Mysql_Process();
    return  instance_;

}

//清除实例
void ZCE_Mysql_Process::clean_instance()
{
    if (instance_)
    {
        delete instance_ ;
        instance_ = NULL;
    }
}

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL

