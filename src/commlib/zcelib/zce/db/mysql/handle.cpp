#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/handle.h"
#include "zce/db/mysql/stmt_bind.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
handle::handle() noexcept
{
    //置开始状态
    if_connected_ = false;
}

handle::~handle() noexcept
{
    // disconnect if if_connected_ to handle
    disconnect();
}

//连接数据服务器
int handle::connect_i(CONNECT_BY by,
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
    if (if_connected_ == true)
    {
        disconnect();
    }

    //初始化MYSQL句柄
    ::mysql_init(&mysql_handle_);
    //设置连接的timeout
    if (timeout != 0)
    {
        ::mysql_options(&mysql_handle_, MYSQL_OPT_CONNECT_TIMEOUT, (char*)(&timeout));
    }

    //50013,版本后，提供了这个选项，而原来的版本，这个选项是默认打开的。
#if MYSQL_VERSION_ID >= 50013
    ::mysql_options(&mysql_handle_, MYSQL_OPT_RECONNECT, "1");
#endif

    unsigned long client_flag = 0;

#if MYSQL_VERSION_ID > 40100

    if (if_multi_sql)
    {
        client_flag |= CLIENT_MULTI_STATEMENTS;
    }

#endif

    //连接数据库
    MYSQL* ret = nullptr;
    switch (by)
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
        if (optfile != nullptr)
        {
            int opret = ::mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_FILE, optfile);

            //如果使group==nullptr,将读写optfile的[client]配置,否则读写group下的配置
            if (group != nullptr)
            {
                opret = ::mysql_options(&mysql_handle_, MYSQL_READ_DEFAULT_GROUP, group);
            }

            if (opret != 0)
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
    if (ret != nullptr)
    {
        return -1;
    }

    if (stmt_ != nullptr)
    {
        int tmpret = ::mysql_stmt_close(stmt_);
        if (tmpret != 0)
        {
            return tmpret;
        }
        stmt_ = nullptr;
    }
    stmt_ = ::mysql_stmt_init(&mysql_handle_);
    if (nullptr == stmt_)
    {
        return -1;
    }

    if_connected_ = true;
    //返回成功 0=0
    return 0;
}

//连接数据服务器,通过IP地址，主机名称
int handle::connect_by_host(const char* host_name,
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
int handle::connect_by_socketfile(const char* socket_file,
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
int handle::connect_by_optionfile(const char* optfile, const char* group)
{
    return connect_i(CONNECT_BY::SOCKET_FILE,
        nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0,
        false, optfile, group);
}

//断开数据库服务器连接
void handle::disconnect()
{
    //没有连接
    if (if_connected_ == false)
    {
        return;
    }
    if (nullptr != stmt_)
    {
        int tmpret = ::mysql_stmt_free_result(stmt_);
        tmpret = ::mysql_stmt_close(stmt_);
        ZCE_UNUSED_ARG(tmpret);
    }
    ::mysql_close(&mysql_handle_);
    if_connected_ = false;
}

//选择一个默认数据库,参数是数据库的名称
int handle::select_database(const char* db)
{
    int ret = ::mysql_select_db(&mysql_handle_, db);

    //检查结果,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//如果连接断开，重新连接，低成本的好方法,否则什么都不做，
int handle::ping()
{
    int ret = ::mysql_ping(&mysql_handle_);

    //检查结果,
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

//得到当前数据服务器的状态
const char* handle::get_mysql_status()
{
    return ::mysql_stat(&mysql_handle_);
}

//得到转意后的Escaple String ,没有根据当前的字符集合进行操作,
unsigned int handle::escape_string(char* tostr,
    const char* fromstr,
    unsigned int fromlen)
{
    return ::mysql_escape_string(tostr,
        fromstr,
        fromlen);
}

unsigned int handle::real_escape_string(char* tostr,
    const char* fromstr,
    unsigned int fromlen)
{
    return ::mysql_real_escape_string(&mysql_handle_,
        tostr,
        fromstr,
        fromlen);
}

//int 返回是否成功还是失败 MYSQL_RETURN_FAIL表示失败
//执行SQL语句，功能全集，不对外使用
int handle::query_i(size_t* num_affect,
    size_t* last_id,
    zce::mysql::result* sql_result,
    bool bstore)
{
    //如果没有设置连接或者没有设置命令
    if (sql_cmd_.empty())
    {
        return -1;
    }

    //执行SQL命令
    int tmpret = ::mysql_real_query(&mysql_handle_,
        sql_cmd_.c_str(),
        (unsigned long)sql_cmd_.length());
    if (tmpret != 0)
    {
        return tmpret;
    }

    //如果用户要求转储结果集
    if (sql_result)
    {
        MYSQL_RES* tmp_res = nullptr;
        if (bstore)
        {
            //转储结果
            tmp_res = ::mysql_store_result(&mysql_handle_);
        }
        else
        {
            //转储结果
            tmp_res = ::mysql_use_result(&mysql_handle_);
        }

        //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
        //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回nullptr? "
        //如果是INSERT语句，那么mysql_store_result就是返回nullptr，mysql_field_count也应该等于0，
        //如果MYSQL内部发生某个错误，那么mysql_store_result 返回nullptr，但mysql_field_count 会大于0，此时是个错误
        if (tmp_res == nullptr && mysql_field_count(&mysql_handle_) > 0)
        {
            return -1;
        }

        //得到结果集,查询结果集信息
        sql_result->set_mysql_result(tmp_res);
    }

    //执行SQL命令影响了多少行,mysql_affected_rows 必须在转储结果集后,所以你要注意输入的参数
    if (num_affect)
    {
        *num_affect = (uint64_t) ::mysql_affected_rows(&mysql_handle_);
    }

    if (last_id)
    {
        *last_id = (uint64_t) ::mysql_insert_id(&mysql_handle_);
    }

    //成功
    return 0;
}

//执行SQL语句,不用输出结果集合的那种,非SELECT语句
//num_affect 为返回参数,告诉你修改了几行
int handle::execute(size_t& num_affect, uint64_t& last_id)
{
    return query_i(&num_affect, &last_id, nullptr, false);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
//num_affect 为返回参数,告诉你修改了几行,SELECT了几行
int handle::execute(size_t& num_affect, zce::mysql::result& res)
{
    return query_i(&num_affect, nullptr, &res, true);
}

//再取一次结果，注意其调用的是mysql_use_result,num_affect对它无效

//但不推荐使用,一次取一行,交互太多
int handle::execute(zce::mysql::result& res)
{
    return query_i(nullptr, nullptr, &res, false);
}

//SQL 执行命令，这个事一个基础函数，内部调用
int handle::stmt_query_i(size_t* num_affect,
    size_t* last_id)
{
    int tmpret = 0;

    //执行
    tmpret = ::mysql_stmt_execute(stmt_);
    if (tmpret != 0)
    {
        return tmpret;
    }

    //如果要返回结果,进行转储
    if (is_bind_result_)
    {
        tmpret = ::mysql_stmt_store_result(stmt_);
        if (tmpret != 0)
        {
            return tmpret;
        }
    }

    //执行SQL命令影响了多少行,mysql_affected_rows
    //必须在转储结果集后,所以你要注意输入的参数
    if (num_affect)
    {
        *num_affect = (uint64_t) ::mysql_stmt_affected_rows(stmt_);
    }

    if (last_id)
    {
        *last_id = (uint64_t)::mysql_stmt_insert_id(stmt_);
    }

    //成功
    return 0;
}

//执行SQL语句,不用输出结果集合的那种
int handle::stmt_query(size_t& num_affect, size_t& last_id)
{
    return stmt_query_i(&num_affect, &last_id);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,
int handle::stmt_query(size_t& num_affect)
{
    return stmt_query_i(&num_affect, nullptr);
}

//准备SQL,并且分析绑定的变量
int handle::stmt_prepare_bind(zce::mysql::stmt_bind* bind_param,
    zce::mysql::stmt_bind* bind_result)
{
    int tmpret = ::mysql_stmt_prepare(stmt_,
        sql_cmd_.c_str(),
        static_cast<unsigned long>(sql_cmd_.size()));
    if (tmpret != 0)
    {
        return tmpret;
    }

    is_bind_result_ = false;

    //原打算检查语句的绑定变量个数,后来决定还是让MySQL自己检查,
    //unsigned long paramcount = mysql_stmt_param_count(stmt_);
    //ZASSERT(paramcount > 0 &&  bindparam!=nullptr || paramcount == 0 && bindparam == nullptr);

    //绑定的参数
    if (bind_param)
    {
        tmpret = ::mysql_stmt_bind_param(stmt_,
            bind_param->get_stmt_bind());
        if (tmpret != 0)
        {
            return tmpret;
        }
        bind_param->set_stmt(stmt_);
    }

    //绑定的结果
    if (bind_result)
    {
        tmpret = ::mysql_stmt_bind_result(stmt_,
            bind_result->get_stmt_bind());
        //出错返回,或者处理
        if (tmpret != 0)
        {
            return tmpret;
        }
        bind_result->set_stmt(stmt_);
        is_bind_result_ = true;
    }

    return 0;
}

//用于 multiple-statement executions 中得到多个
//如果
int handle::fetch_next_result(zce::mysql::result& res, bool bstore)
{
    int tmpret = ::mysql_next_result(&mysql_handle_);

    //tmpret == -1表示没有结果集,其他<0的值表示错误
    if (tmpret < 0)
    {
        return -1;
    }

    MYSQL_RES* my_res = nullptr;
    if (bstore)
    {
        //转储结果
        my_res = ::mysql_store_result(&mysql_handle_);
    }
    else
    {
        //转储结果
        my_res = ::mysql_use_result(&mysql_handle_);
    }

    //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
    //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回nullptr? "
    if (my_res == nullptr && ::mysql_field_count(&mysql_handle_) > 0)
    {
        return -1;
    }

    //得到结果集,查询结果集信息
    res.set_mysql_result(my_res);

    //成功
    return 0;
}

//设置是否自动提交
int handle::set_auto_commit(bool bauto)
{
    //my_bool其实是char
    my_bool mode = (bauto == true) ? 1 : 0;

    int ret = ::mysql_autocommit(&mysql_handle_, mode);

    //检查结果,
    if (0 != ret)
    {
        return ret;
    }
    return 0;
}

//提交事务Commit Transaction
int handle::trans_commit()
{
    int ret = ::mysql_commit(&mysql_handle_);

    //检查结果,
    if (0 != ret)
    {
        return ret;
    }
    return 0;
}

//回滚事务Rollback Transaction
int handle::trans_rollback()
{
    int ret = ::mysql_rollback(&mysql_handle_);

    //检查结果,
    if (0 != ret)
    {
        return ret;
    }
    return 0;
}
}

//如果你要用MYSQL的库
#endif 