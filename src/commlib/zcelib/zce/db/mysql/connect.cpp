#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/connect.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
connect::connect()
{
    //置开始状态
    if_connected_ = false;
}

connect::~connect() noexcept
{
    // disconnect if if_connected_ to handle
    disconnect();
}

//连接数据服务器
int connect::connect_i(CONNECT_BY by,
                       const char* host_name,
                       const char* socket_file,
                       const char* user,
                       const char* pwd,
                       const char* db,
                       const unsigned int port,
                       const unsigned int timeout,
                       bool if_multi_sql,
                       const char* optfile,
                       const char* group)
{
    //如果已经连接,关闭原来的连接
    if(if_connected_ == true)
    {
        disconnect();
    }

    //初始化MYSQL句柄
    ::mysql_init(&mysql_handle_);
    //设置连接的timeout
    if(timeout != 0)
    {
        ::mysql_options(&mysql_handle_, MYSQL_OPT_CONNECT_TIMEOUT, (char*)(&timeout));
    }

    //50013,版本后，提供了这个选项，而原来的版本，这个选项是默认打开的。
#if MYSQL_VERSION_ID >= 50013
    ::mysql_options(&mysql_handle_, MYSQL_OPT_RECONNECT, "1");
#endif

    unsigned long client_flag = 0;

#if MYSQL_VERSION_ID > 40100

    if(if_multi_sql)
    {
        client_flag |= CLIENT_MULTI_STATEMENTS;
    }

#endif

    //连接数据库
    MYSQL* ret = nullptr;
    switch(by)
    {
        using enum CONNECT_BY;
    case HOST:
    ret = ::mysql_real_connect(&mysql_handle_,
                               host_name,
                               user,
                               pwd,
                               db,
                               port,
                               nullptr,
                               client_flag);
    break;
    case SOCKET_FILE:
    //这个地方必须注意一下，WINDOWS下，对于mysql_real_connect函数如果host_name参数为nullptr，
    // 是先进行命名管道连接，如果不行用TCP/IP连接本地
    //如果要不保证绝对使用命名管道，则参数host_name=".",
    ret = ::mysql_real_connect(&mysql_handle_,
                               nullptr,
                               user,
                               pwd,
                               db,
                               port,
                               socket_file,
                               client_flag);
    break;
    case OPTION_FILE:
    if(optfile != nullptr)
    {
        int opret = ::mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_FILE, optfile);

        //如果使group==nullptr,将读写optfile的[client]配置,否则读写group下的配置
        if(group != nullptr)
        {
            opret = ::mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_GROUP, group);
        }

        if(opret != 0)
        {
            return -1;
        }
    }
    ret = ::mysql_real_connect(&mysql_handle_,
                               nullptr,
                               nullptr,
                               nullptr,
                               nullptr,
                               0,
                               nullptr,
                               0);
    break;
    default:
    ZCE_ASSERT(false);
    return -1;
    }

    //检查结果,
    if(ret != nullptr)
    {
        return -1;
    }
    if_connected_ = true;
    //返回成功 0=0
    return 0;
}

//连接数据服务器,通过IP地址，主机名称
int connect::connect_by_host(const char* host_name,
                             const char* user,
                             const char* pwd,
                             const char* db,
                             const unsigned int port,
                             unsigned int timeout,
                             bool if_multi_sql)
{
    return connect_i(CONNECT_BY::HOST,
                     host_name, nullptr, user, pwd, db, port, timeout,
                     if_multi_sql);
}

//连接数据库服务器，通过UNIXSOCKET文件（UNIX下）或者命名管道（WINDOWS下）进行通信，只能用于本机
int connect::connect_by_socketfile(const char* socket_file,
                                   const char* user,
                                   const char* pwd,
                                   const char* db,
                                   unsigned int timeout,
                                   bool if_multi_sql)
{
    return connect_i(CONNECT_BY::SOCKET_FILE,
                     nullptr, socket_file, user, pwd, db, 0, timeout,
                     if_multi_sql);
}

//如果使用选项文件进行连接
int connect::connect_by_optionfile(const char* optfile, const char* group)
{
    return connect_i(CONNECT_BY::SOCKET_FILE,
                     nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0,
                     false, optfile, group);
}

//断开数据库服务器连接
void connect::disconnect()
{
    //没有连接
    if(if_connected_ == false)
    {
        return;
    }
    ::mysql_close(&mysql_handle_);
    if_connected_ = false;
}

//选择一个默认数据库,参数是数据库的名称
int connect::select_database(const char* db)
{
    int ret = ::mysql_select_db(&mysql_handle_, db);

    //检查结果,
    if(0 != ret)
    {
        return ret;
    }

    return 0;
}

//如果连接断开，重新连接，低成本的好方法,否则什么都不做，
int connect::ping()
{
    int ret = ::mysql_ping(&mysql_handle_);

    //检查结果,
    if(0 != ret)
    {
        return ret;
    }

    return 0;
}

//得到当前数据服务器的状态
const char* connect::get_mysql_status()
{
    return ::mysql_stat(&mysql_handle_);
}

//设置是否自动提交
int connect::set_auto_commit(bool bauto)
{
    //my_bool其实是char
    my_bool mode = (bauto == true) ? 1 : 0;

    int ret = ::mysql_autocommit(&mysql_handle_, mode);

    //检查结果,
    if(0 != ret)
    {
        return ret;
    }
    return 0;
}
}

//如果你要用MYSQL的库
#endif
