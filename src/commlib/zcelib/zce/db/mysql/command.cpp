#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/connect.h"
#include "zce/db/mysql/command.h"

namespace zce::mysql
{
///****************************************************************************************
/// class command
///****************************************************************************************
//构造函数

//析构函数
bind_data::~bind_data()
{
    clear();
}

bind_data::bind_data(bind_data&& bind) noexcept :
    num_bind_(bind.num_bind_),
    stmt_bind_(bind.stmt_bind_)
{
    bind.num_bind_ = 0;
    bind.stmt_bind_ = nullptr;
}

bind_data& bind_data::operator=(bind_data&& bind) noexcept
{
    clear();
    num_bind_ = bind.num_bind_;
    stmt_bind_ = bind.stmt_bind_;
    bind.num_bind_ = 0;
    bind.stmt_bind_ = nullptr;
    return *this;
}

bind_data::bind_data(const bind_data& bind) :
    num_bind_(bind.num_bind_)
{
    stmt_bind_ = new MYSQL_BIND[num_bind_];
    memcpy(stmt_bind_, bind.stmt_bind_, sizeof(MYSQL_BIND) * num_bind_);
}

bind_data& bind_data::operator=(const bind_data& bind)
{
    clear();
    num_bind_ = bind.num_bind_;
    stmt_bind_ = new MYSQL_BIND[num_bind_];
    memcpy(stmt_bind_, bind.stmt_bind_, sizeof(MYSQL_BIND) * num_bind_);
    return *this;
}

void bind_data::clear()
{
    if(stmt_bind_)
    {
        delete[] stmt_bind_;
        stmt_bind_ = nullptr;
        num_bind_ = 0;
    }
}

void bind_data::initialize(size_t num_bind)
{
    clear();
    num_bind_ = num_bind;
    if(num_bind_ == 0)
    {
        return;
    }
    stmt_bind_ = new MYSQL_BIND[num_bind_];
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND) * num_bind_);
}

//绑定一个参数
int bind_data::bind_one_param(size_t col,
                              ::enum_field_types paramtype,
                              my_bool* is_null,
                              void* paramdata,
                              unsigned long szparam)
{
    ZCE_ASSERT(col < num_bind_);
    if(col >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[col].buffer_type = paramtype;
    stmt_bind_[col].buffer = paramdata;

    stmt_bind_[col].is_null = is_null;
    stmt_bind_[col].length = nullptr;
    stmt_bind_[col].buffer_length = szparam;
    return 0;
}

int bind_data::bind_one_result(size_t col,
                               ::enum_field_types paramtype,
                               void* paramdata,
                               unsigned long* szparam)
{
    ZCE_ASSERT(col < num_bind_);
    if(col >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[col].buffer_type = paramtype;
    stmt_bind_[col].buffer = paramdata;

    stmt_bind_[col].buffer_length = *szparam;
    //长度指针保存返回值
    stmt_bind_[col].length = szparam;
    return 0;
}

template<>
void bind_data::bind(size_t col, bool& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(char);

    return;
}

//绑定一个char
template<>
void bind_data::bind(size_t col, char& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(char);

    return;
}

template<>
void bind_data::bind(size_t col, short& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(short);
    return;
}

template<>
void bind_data::bind(size_t col, int& val)
{
    //MYSQL_TYPE_LONG 长度为4
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);

    return;
}

template<>
void bind_data::bind(size_t col, long& val)
{
#if defined (ZCE_OS_WINDOWS)
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
#elif defined (ZCE_OS_LINUX)
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
#endif
    stmt_bind_[col].buffer = (void*)(&val);
    return;
}

template<>
void bind_data::bind(size_t col, long long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[col].buffer = (void*)(&val);

    return;
}

template<>
void bind_data::bind(size_t col, unsigned char& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void bind_data::bind(size_t col, unsigned short& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void bind_data::bind(size_t col, unsigned int& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[col].buffer = (void*)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void bind_data::bind(size_t col, unsigned long& val)
{
#if defined (ZCE_OS_WINDOWS)
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONG;
#elif defined (ZCE_OS_LINUX)
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
#endif
    stmt_bind_[col].buffer = (void*)(&val);
    stmt_bind_[col].buffer_length = sizeof(unsigned long);
    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void bind_data::bind(size_t col, unsigned long long& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);
    stmt_bind_[col].buffer_length = sizeof(unsigned long long);
    //无符号,绑定结果时应该不用
    stmt_bind_[col].is_unsigned = 1;

    return;
}

template<>
void bind_data::bind(size_t col, float& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_FLOAT;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);

    return;
}

template<>
void bind_data::bind(size_t col, double& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_DOUBLE;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(&val);

    return;
}

template<>
void bind_data::bind(size_t col, zce::mysql::blob& blob_data)
{
    stmt_bind_[col].buffer_type = blob_data.bind_type_;
    stmt_bind_[col].buffer = blob_data.blob_ptr_;

    //这个可能既是绑定参数,也是绑定结果
    stmt_bind_[col].buffer_length = *blob_data.blob_len_;
    stmt_bind_[col].length = blob_data.blob_len_;

    return;
}

template<>
void bind_data::bind(size_t col, zce::mysql::time& val)
{
    stmt_bind_[col].buffer_type = val.stmt_timetype_;
    stmt_bind_[col].buffer = reinterpret_cast<void*>(val.stmt_ptime_);

    stmt_bind_[col].buffer_length = sizeof(MYSQL_TIME);
    stmt_bind_[col].length = nullptr;

    return;
}

//绑定一个空参数
template<>
void bind_data::bind(size_t col, zce::mysql::null& val)
{
    stmt_bind_[col].buffer_type = MYSQL_TYPE_NULL;
    stmt_bind_[col].is_null = val.is_null_;
    return;
}

///****************************************************************************************
/// class command
///****************************************************************************************
command::command(zce::mysql::connect& conn)
{
    mysql_ = conn.get_handle();
    stmt_ = ::mysql_stmt_init(mysql_);
    ZCE_ASSERT(stmt_);
}
command::~command() noexcept
{
    if(nullptr != stmt_)
    {
        [[maybe_unused]]
        int ret = ::mysql_stmt_free_result(stmt_);
        ret = ::mysql_stmt_close(stmt_);
    }
}

//得到转意后的Escaple String ,没有根据当前的字符集合进行操作,
size_t command::escape_string(char* tostr,
                                    const char* fromstr,
                                    unsigned int fromlen)
{
    return static_cast<size_t>(
        ::mysql_escape_string(tostr,
        fromstr,
        fromlen));
}

size_t command::real_escape_string(char* tostr,
                                         const char* fromstr,
                                         unsigned int fromlen)
{
    return static_cast<size_t>(
        ::mysql_real_escape_string(mysql_,
        tostr,
        fromstr,
        fromlen));
}

//提交事务Commit Transaction
int command::trans_commit()
{
    int ret = ::mysql_commit(mysql_);
    if(0 != ret)
    {
        return ret;
    }
    return 0;
}

//回滚事务Rollback Transaction
int command::trans_rollback()
{
    int ret = ::mysql_rollback(mysql_);
    if(0 != ret)
    {
        return ret;
    }
    return 0;
}

//执行SQL语句,不用输出结果集合的那种,非SELECT语句
//num_affect 为返回参数,告诉你修改了几行
int command::execute(std::string_view sqlcmd,
                     size_t& num_affect,
                     uint64_t& last_id)
{
    int ret = 0;
    if((ret = query(sqlcmd)) == 0)
    {
        ret = get_result(&num_affect, &last_id, nullptr, false);
    }
    return ret;
}

//执行SQL语句,SELECT语句,转储结果集合的那种,注意这个函数条用的是mysql_store_result.
//num_affect 为返回参数,告诉你修改了几行,SELECT了几行
int command::execute(std::string_view sqlcmd,
                     size_t& num_affect,
                     zce::mysql::result& my_res)
{
    int ret = 0;
    if((ret = query(sqlcmd)) == 0)
    {
        ret = get_result(&num_affect, nullptr, &my_res, false);
    }
    return ret;
}

//但不推荐使用,一次取一行,交互太多
int command::query(std::string_view sqlcmd)
{
    //如果没有设置连接或者没有设置命令
    if(if_connected_ == false || sqlcmd.empty())
    {
        return -1;
    }

    //执行SQL命令
    int ret = ::mysql_real_query(mysql_,
                                 sqlcmd.data(),
                                 (unsigned long)sqlcmd.length());
    if(ret != 0)
    {
        return ret;
    }
    return 0;
}

int command::get_result(size_t* num_affect,
                        size_t* last_id,
                        zce::mysql::result* my_res,
                        bool use_result)
{
    //如果用户要求转储结果集
    if(my_res)
    {
        MYSQL_RES* res = nullptr;
        if(use_result)
        {
            //转储结果,但只能一次取一行
            res = ::mysql_use_result(mysql_);
        }
        else
        {
            //转储结果
            res = ::mysql_store_result(mysql_);
        }

        //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
        //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回nullptr? "
        //如果是INSERT语句，那么mysql_store_result就是返回nullptr，mysql_field_count也应该等于0，
        //如果MYSQL内部发生某个错误，那么mysql_store_result 返回nullptr，但mysql_field_count 会大于0，此时是个错误
        if(res == nullptr && mysql_field_count(mysql_) > 0)
        {
            return -1;
        }

        //得到结果集,查询结果集信息
        my_res->set_mysql_result(res);
    }
    //执行SQL命令影响了多少行,如果是查询语句，mysql_affected_rows 必须在转储结果集后,所以你要注意输入的参数
    if(num_affect)
    {
        *num_affect = (uint64_t) ::mysql_affected_rows(mysql_);
        //注意如果调用的是mysql_use_result,num_affect 不是总数，而只是1
    }
    if(last_id)
    {
        *last_id = (uint64_t) ::mysql_insert_id(mysql_);
    }
    return 0;
}

//
int command::fetch_next_row(zce::mysql::result& res)
{
    MYSQL_RES* my_res = ::mysql_use_result(mysql_);
    if(my_res == nullptr)
    {
        return -1;
    }

    //得到结果集,查询结果集信息
    res.set_mysql_result(my_res);
    return 0;
}

//用于 multiple-statement executions 中得到多个
int command::fetch_next_result(zce::mysql::result& res,
                                bool use_result)
{
    int ret = ::mysql_next_result(mysql_);
    //ret == -1表示没有结果集,其他<0的值表示错误
    if(ret < 0)
    {
        return -1;
    }

    MYSQL_RES* my_res = nullptr;
    if(use_result)
    {
        //转储一行结果
        my_res = ::mysql_use_result(mysql_);
    }
    else
    {
        //转储结果
        my_res = ::mysql_store_result(mysql_);
    }

    //比如你用INSERT语句但是,你要取回结果集,我暂时认为你是对的,只是返回的结果集为空或者你不看注释
    //如果转储失败,为什么这样作,见MySQL文档"为什么在mysql_query()返回成功后mysql_store_result()有时返回nullptr? "
    if(my_res == nullptr && ::mysql_field_count(mysql_) > 0)
    {
        return -1;
    }
    //得到结果集,查询结果集信息
    res.set_mysql_result(my_res);

    return 0;
}

int command::stmt_query(std::string_view sqlcmd,
               zce::mysql::bind_data* bind_param,
               zce::mysql::bind_data* bind_result)
{
    //如果没有设置连接或者没有设置命令
    if(sqlcmd.empty())
    {
        return -1;
    }
    //执行SQL命令
    int ret = ::mysql_stmt_prepare(stmt_,
                                   sqlcmd.data(),
                                   static_cast<unsigned long>(sqlcmd.length()));
    if(ret != 0)
    {
        return ret;
    }
    //绑定参数
    if(bind_param)
    {
        bind_param_ = std::move(*bind_param);
        ret = ::mysql_stmt_bind_param(stmt_, bind_param_.get_stmt_bind());
        if(ret != 0)
        {
            return ret;
        }
    }
    //绑定结果
    if(bind_result)
    {
        bind_result_ = std::move(*bind_result);
        ret = ::mysql_stmt_bind_result(stmt_, bind_result_.get_stmt_bind());
        if(ret != 0)
        {
            return ret;
        }
        is_bind_result_ = true;
    }
    //执行SQL命令
    ret = ::mysql_stmt_execute(stmt_);
    if(ret != 0)
    {
        return ret;
    }
    return 0;
}

//SQL 执行命令，这个事一个基础函数，内部调用
int command::stmt_get_result(size_t* num_affect,
                          size_t* last_id)
{
    int ret = 0;
    //如果要返回结果,进行转储
    if(is_bind_result_)
    {
        ret = ::mysql_stmt_store_result(stmt_);
        if(ret != 0)
        {
            return ret;
        }
    }

    //执行SQL命令影响了多少行,mysql_affected_rows
    //必须在转储结果集后,所以你要注意输入的参数
    if(num_affect)
    {
        *num_affect = (uint64_t) ::mysql_stmt_affected_rows(stmt_);
    }

    if(last_id)
    {
        *last_id = (uint64_t)::mysql_stmt_insert_id(stmt_);
    }

    return 0;
}

//
int command::stmt_fetch_next_row() const
{
    int tmpret = ::mysql_stmt_fetch(stmt_);
    if(tmpret != 0)
    {
        return -1;
    }
    return 0;
}

//用bind_data取出一列的数据
int command::stmt_fetch_column(size_t column,
                               size_t offset,
                               zce::mysql::bind_data* bind_colum) const
{
    int tmpret = ::mysql_stmt_fetch_column(stmt_,
                                           bind_colum->get_stmt_bind(),
                                           static_cast<unsigned int>(column),
                                           static_cast<unsigned long>(offset));
    if(0 != tmpret)
    {
        return -1;
    }

    return 0;
}

//
int command::stmt_seek_result_row(size_t nrow) const
{
    //检查结果集合为空,或者参数row错误
    ::mysql_stmt_data_seek(stmt_, nrow);
    int tmpret = ::mysql_stmt_fetch(stmt_);
    if(0 != tmpret)
    {
        return -1;
    }

    return 0;
}
}
