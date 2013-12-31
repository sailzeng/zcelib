#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_mysql_connect.h"

//如果你要用MYSQL的库
#if defined MYSQL_VERSION_ID

/*********************************************************************************
class ZCE_Mysql_Connect
*********************************************************************************/

ZCE_Mysql_Connect::ZCE_Mysql_Connect()
{
    mysql_init(&mysql_handle_);
    //置开始状态
    if_connected_ = false;
}

ZCE_Mysql_Connect::~ZCE_Mysql_Connect()
{
    // disconnect if if_connected_ to ZCE_Mysql_Connect
    disconnect();
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004年8月15日
Function        : ZCE_Mysql_Connect::connect_by_optionfile
Return          : int
Parameter List  :
  Param1: const char* optfile 选项文件
  Param2: const char* groupconst 选项文件要读取的SECTION，你的配置都要在这个SECTION下,如果为空，
                  读取[client]下的配置项
Description     : 如果使用选项文件进行连接,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_Connect::connect_by_optionfile(const char *optfile, const char *group)
{
    //如果已经连接,关闭原来的连接
    if (if_connected_ == true)
    {
        disconnect();
    }

    //初始化MYSQL句柄
    mysql_init(&mysql_handle_);

    if (optfile != NULL)
    {
        int opret = mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_FILE, optfile);

        //如果使group==NULL,将读写optfile的[client]配置,否则读写group下的配置
        if (group != NULL)
        {
            opret = mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_GROUP, group);
        }

        MYSQLCONNECTCHECK(opret == 0);
    }

    //连接数据库
    MYSQL *ret = mysql_real_connect(&mysql_handle_, NULL, NULL, NULL, NULL, 0, NULL, 0);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    if_connected_ = true;
    //返回成功 MYSQL_RETURN_OK=0
    return MYSQL_RETURN_OK;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2004年8月15日
Function        : ZCE_Mysql_Connect::connect
Return          : int
Parameter List  :
  Param1: const char* host 数据库服务器
  Param2: const char *socket_file,UNIX SOCKET文件名称或者命名管道名称
  Param2: const char* user 用户,默认为mysql
  Param3: const char* pwd  用户密码,默认为""
  Param4: unsigned int port 端口,默认为MYSQL_PORT
  Param5: const char* db    使用的默认数据库,默认为空表示不选择
  Param6: const const unsigned int timeout 连接数据库的超时时间，默认为0,表示不设置
  Param7: bool bmultisql 是否使用多语句同时执行的方式,默认为false,可能在事物等处理上有些效果
Description     : 连接数据服务器，
Calls           :
Called By       :
Other           : 可以设置选项文件
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_Connect::connect_i(const char *host_name,
                                 const char *socket_file,
                                 const char *user,
                                 const char *pwd,
                                 const char *db,
                                 const unsigned int port,
                                 const unsigned int timeout,
                                 bool if_multi_sql)
{

    //如果已经连接,关闭原来的连接
    if (if_connected_ == true)
    {
        disconnect();
    }

    //初始化MYSQL句柄
    mysql_init(&mysql_handle_);

    //设置连接的timeout
    if (timeout != 0)
    {
        mysql_options(&mysql_handle_, MYSQL_OPT_CONNECT_TIMEOUT, (char *)(&timeout));
    }

    //50013,版本后，提供了这个选项，而原来的版本，这个选项是默认打开的。
#if MYSQL_VERSION_ID >= 50013
    mysql_options(&mysql_handle_, MYSQL_OPT_RECONNECT, "1");
#endif

    unsigned long client_flag = 0;

#if MYSQL_VERSION_ID > 40100

    if (if_multi_sql)
    {
        client_flag |= CLIENT_MULTI_STATEMENTS;
    }

#endif

    //连接数据库
    MYSQL *ret = NULL;

    //如果使用域名或者IP地址进行连接
    if (host_name)
    {
        ret = mysql_real_connect(&mysql_handle_, host_name, user, pwd, db, port, NULL, client_flag);
    }
    //如果使用UNIXSOCKET或者命名管道进行本地连接
    else if (socket_file)
    {
        //这个地方必须注意一下，WINDOWS下，对于mysql_real_connect函数如果host_name参数为NULL，是先进行命名管道连接，如果不行用TCP/IP连接本地
        //如果要不保证绝对使用命名管道，则参数host_name=".",
        ret =  mysql_real_connect(&mysql_handle_, NULL, user, pwd, db, port, socket_file, client_flag);
    }
    //参数使用错误，不能host和unixsocket都为NULL
    else
    {
        ZCE_ASSERT(false);
    }

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    if_connected_ = true;
    //返回成功 MYSQL_RETURN_OK=0
    return MYSQL_RETURN_OK;
}

//连接数据服务器,通过IP地址，主机名称
int ZCE_Mysql_Connect::connect_by_host(const char *host_name,
                                       const char *user,
                                       const char *pwd,
                                       const char *db,
                                       const unsigned int port,
                                       unsigned int timeout,
                                       bool if_multi_sql)
{
    return connect_i(host_name, NULL, user, pwd, db, port, timeout, if_multi_sql);
}

//连接数据库服务器，通过UNIXSOCKET文件（UNIX下）或者命名管道（WINDOWS下）进行通信，只能用于本机
int ZCE_Mysql_Connect::connect_by_socketfile(const char *socket_file,
                                             const char *user,
                                             const char *pwd,
                                             const char *db,
                                             unsigned int timeout,
                                             bool if_multi_sql)
{
    return connect_i(NULL, socket_file, user, pwd, db, 0, timeout, if_multi_sql);
}

//断开数据库服务器连接
void ZCE_Mysql_Connect::disconnect()
{
    //没有连接
    if (if_connected_ == false)
    {
        return;
    }

    mysql_close(&mysql_handle_);
    if_connected_ = false;
}

//选择一个默认数据库,参数是数据库的名称
int ZCE_Mysql_Connect::select_database(const char *db)
{
    int ret = mysql_select_db(&mysql_handle_, db);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    return MYSQL_RETURN_OK;
}

//如果连接断开，重新连接，低成本的好方法,否则什么都不做，
int ZCE_Mysql_Connect::ping()
{
    int ret = mysql_ping(&mysql_handle_);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    return MYSQL_RETURN_OK;
}

//得到当前数据服务器的状态
const char *ZCE_Mysql_Connect::get_mysql_status()
{
    return mysql_stat(&mysql_handle_);
}

//得到转意后的Escaple String ,没有根据当前的字符集合进行操作,
unsigned int ZCE_Mysql_Connect::make_escape_string(char *tostr, const char *fromstr, unsigned int fromlen)
{
    return mysql_escape_string(tostr, fromstr, fromlen);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年3月8日
Function        : ZCE_Mysql_Connect::make_real_escape_string
Return          : unsigned int 编码的长度
Parameter List  :
  Param1: char* tostr          : 转换得到的字符串
  Param2: const char* fromstr  : 进行转换的字符串
  Param3: unsigned int fromlen : 转换的字符串长度
  Description     : 得到Real Escape String ,Real表示根据当前的MYSQL connect的字符集,得到Escape String
Calls           : 为什么采用这样的奇怪参数顺序,因为mysql_real_escape_string
Called By       :
Other           : Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
Modify Record   :
******************************************************************************************/
unsigned int ZCE_Mysql_Connect::make_real_escape_string(char *tostr, const char *fromstr, unsigned int fromlen)
{
    return mysql_real_escape_string(&mysql_handle_, tostr, fromstr, fromlen);
}

//这些函数都是4.1后的版本功能
#if MYSQL_VERSION_ID > 40100

//设置是否自动提交
int ZCE_Mysql_Connect::set_auto_commit(bool bauto)
{
    //my_bool其实是char
    my_bool mode =  (bauto == true ) ? 1 : 0;

    int ret = mysql_autocommit(&mysql_handle_, mode);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    return MYSQL_RETURN_OK;
}

//提交事务Commit Transaction
int ZCE_Mysql_Connect::trans_commit()
{

    int ret = mysql_commit(&mysql_handle_);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    return MYSQL_RETURN_OK;
}

//回滚事务Rollback Transaction
int ZCE_Mysql_Connect::trans_rollback()
{
    int ret = mysql_rollback(&mysql_handle_);

    //检查结果,返回错误,或者抛出异常
    MYSQLCONNECTCHECK(ret);

    return MYSQL_RETURN_OK;
}

#endif // MYSQL_VERSION_ID > 40100

//如果你要用MYSQL的库
#endif //#if defined MYSQL_VERSION_ID

