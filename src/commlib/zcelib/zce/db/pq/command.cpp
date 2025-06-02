#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/buffer/string_buffer.h"
#include "zce/db/pq/command.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
///****************************************************************************************
/// class bind_data
///****************************************************************************************
//构造函数
bind::bind(size_t num_bind)
{
    initialize(num_bind);
}
//析构函数
bind::~bind()
{
    terminate();
}

bind::bind(bind&& bind) noexcept :
    num_bind_(bind.num_bind_),
    param_value_(bind.param_value_),
    param_type_(bind.param_type_),
    param_len_(bind.param_len_),
    param_fmt_(bind.param_fmt_)
{
    bind.num_bind_ = 0;
    bind.param_value_ = nullptr;
    bind.param_type_ = nullptr;
    bind.param_len_ = nullptr;
    bind.param_fmt_ = nullptr;
}

bind& bind::operator=(bind&& bind) noexcept
{
    terminate();
    num_bind_ = bind.num_bind_;
    param_value_ = bind.param_value_;
    param_type_ = bind.param_type_;
    param_len_ = bind.param_len_;
    param_fmt_ = bind.param_fmt_;
    bind.num_bind_ = 0;
    bind.param_value_ = nullptr;
    bind.param_type_ = nullptr;
    bind.param_len_ = nullptr;
    bind.param_fmt_ = nullptr;
    return *this;
}

bind::bind(const bind& bind) :
    num_bind_(bind.num_bind_)
{
    param_value_ = new char* [num_bind_];
    param_type_ = new ::Oid[num_bind_];
    param_len_ = new int[num_bind_];
    param_fmt_ = new int[num_bind_];
    memcpy(param_value_, bind.param_value_, sizeof(char*) * num_bind_);
    memcpy(param_type_, bind.param_type_, sizeof(::Oid) * num_bind_);
    memcpy(param_len_, bind.param_len_, sizeof(int) * num_bind_);
    memcpy(param_fmt_, bind.param_fmt_, sizeof(int) * num_bind_);
}

bind& bind::operator=(const bind& bind)
{
    terminate();
    num_bind_ = bind.num_bind_;
    param_value_ = new char* [num_bind_];
    param_type_ = new ::Oid[num_bind_];
    param_len_ = new int[num_bind_];
    param_fmt_ = new int[num_bind_];
    memcpy(param_value_, bind.param_value_, sizeof(char*) * num_bind_);
    memcpy(param_type_, bind.param_type_, sizeof(::Oid) * num_bind_);
    memcpy(param_len_, bind.param_len_, sizeof(int) * num_bind_);
    memcpy(param_fmt_, bind.param_fmt_, sizeof(int) * num_bind_);
    return *this;
}

void bind::terminate()
{
    delete[] param_value_;
    param_value_ = nullptr;
    delete[] param_type_;
    param_type_ = nullptr;
    delete[] param_len_;
    param_len_ = nullptr;
    delete[] param_fmt_;
    param_fmt_ = nullptr;
    num_bind_ = 0;
}

void bind::initialize(size_t num_bind)
{
    terminate();
    num_bind_ = num_bind;
    if (num_bind_ == 0)
    {
        return;
    }
    param_value_ = new char* [num_bind_];
    param_type_ = new ::Oid[num_bind_];
    param_len_ = new int[num_bind_];
    param_fmt_ = new int[num_bind_];
    memset(param_value_, 0, sizeof(char*) * num_bind_);
    memset(param_type_, 0, sizeof(::Oid) * num_bind_);
    memset(param_len_, 0, sizeof(int) * num_bind_);
    memset(param_fmt_, 0, sizeof(int) * num_bind_);
}

int bind::tie_one_param(size_t id,
    char* paramdata,
    Oid type,
    int len,
    int fmt)
{
    if (id > num_bind_)
    {
        return -1;
    }
    param_value_[id] = paramdata;
    param_type_[id] = type;
    param_len_[id] = len;
    param_fmt_[id] = fmt;
    return 0;
}

void bind::tie(size_t id, bool& val)
{
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(bool);
    param_fmt_[id] = FMT_BINARY;
    return;
}

//绑定一个char
void bind::tie(size_t id, char& val)
{
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(char);
    param_fmt_[id] = FMT_BINARY;

    return;
}

void bind::tie(size_t id, short& val)
{
    val = ::htons(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(short);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, int& val)
{
    val = ::htonl(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(int);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, long& val)
{
#if defined (ZCE_OS_WINDOWS)
    val = ::htonl(val);
#elif defined (ZCE_OS_LINUX)
    val = ::htonll(val);
#endif
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(long);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, long long& val)
{
    val = ::htonll(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(long long);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, unsigned char& val)
{
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(unsigned char);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, unsigned short& val)
{
    val = ::htons(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(unsigned short);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, unsigned int& val)
{
    val = ::htonl(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(unsigned int);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, unsigned long& val)
{
#if defined (ZCE_OS_WINDOWS)
    val = ::htonl(val);
#elif defined (ZCE_OS_LINUX)
    val = ::htonll(val);
#endif
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(unsigned long);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, unsigned long long& val)
{
    val = ::htonll(val);
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(unsigned long long);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, float& val)
{
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(float);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, double& val)
{
    param_value_[id] = (char*)(&val);
    param_type_[id] = InvalidOid;
    param_len_[id] = sizeof(double);
    param_fmt_[id] = FMT_BINARY;
    return;
}

void bind::tie(size_t id, const std::string& val)
{
    param_value_[id] = (char*)(val.c_str());
    param_type_[id] = InvalidOid;
    param_len_[id] = (int)val.length();
    param_fmt_[id] = FMT_TEXT;
    return;
}
//! 绑定string_view 作为参数，只能作为参数绑定，不能作为结果绑定
void bind::tie(size_t id, const std::string_view& val)
{
    param_value_[id] = (char*)(val.data());
    param_type_[id] = InvalidOid;
    param_len_[id] = (int)val.length();
    param_fmt_[id] = FMT_TEXT;
    return;
}

void bind::tie(size_t id, const zce::string_buf& val)
{
    param_value_[id] = (char*)(val.data());
    param_type_[id] = InvalidOid;
    param_len_[id] = (int)val.size();
    param_fmt_[id] = FMT_TEXT;
    return;
}

///****************************************************************************************
/// class zce::pq::command
///****************************************************************************************

command::command(zce::pq::connect& conn) :
    conn_(conn.get_handle())
{
    ZCE_ASSERT(conn_);
    stmt_name_[0] = 0;
}
command::~command() noexcept
{
    stmt_clear();
}

//! 开始一个事务，Begin Transaction，返回0标识成功
int command::trans_begin()
{
    return execute("BEGIN");
}
//! 提交事务Commit Transaction
int command::trans_commit()
{
    return execute("COMMIT");
}
//回滚事务Rollback Transaction
int command::trans_rollback()
{
    return execute("ROLLBACK");
}

int command::execute(std::string_view sqlcmd)
{
    int ret = 0;
    ::PGresult* res = ::PQexec(conn_, sqlcmd.data());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        ZCE_LOG(RS_ERROR,
            "[pq] commit transaction failed : %s\n",
            ::PQerrorMessage(conn_));
        ret = -1;
    }
    ::PQclear(res);
    return ret;
}

int command::execute(std::string_view sql_cmd,
    size_t& num_affect,
    uint64_t* last_id)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
    return get_result(res,
        num_affect,
        last_id,
        nullptr);
}

int command::execute(std::string_view sql_cmd,
    size_t& num_affect,
    zce::pq::result& pq_res)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
    return get_result(res,
        num_affect,
        nullptr,
        &pq_res);
}

void command::stmt_clear()
{
    if (stmt_name_[0] != 0)
    {
        ::PQclear(::PQprepare(conn_, stmt_name_, nullptr, 0, nullptr));
        stmt_name_[0] = 0;
    }
    bind_param_.terminate();
    current_bind_ = 0;
}

void command::bind_reset()
{
    current_bind_ = 0;
}

int command::stmt_prepare(std::string_view sql_cmd)
{
    stmt_clear();
    zce::unique_name("STMT",
        stmt_name_,
        sizeof(stmt_name_));
    size_t param_num = stmt_count_sql_param(sql_cmd);
    PGresult* res = ::PQprepare(conn_,
        stmt_name_,
        sql_cmd.data(),
        (int)param_num,
        nullptr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        ZCE_LOG(RS_ERROR,
            "Failed to prepare SQL : %s : %s\n", sql_cmd.data(),
            ::PQerrorMessage(conn_));
        return -1;
    }
    ::PQclear(res);
    bind_param_.initialize(param_num);
    return 0;
}

int command::stmt_execute(size_t& num_affect,
    size_t* last_id,
    int res_fmt)
{
    PGresult* res = ::PQexecPrepared(conn_,
        stmt_name_,
        (int)bind_param_.num_bind_,
        bind_param_.param_value_,
        bind_param_.param_len_,
        bind_param_.param_fmt_,
        res_fmt);
    return get_result(res,
        num_affect,
        last_id,
        nullptr);
}

int command::stmt_execute(size_t& num_affect,
    zce::pq::result& pq_res,
    int res_fmt)
{
    PGresult* res = ::PQexecPrepared(conn_,
        stmt_name_,
        (int)bind_param_.num_bind_,
        bind_param_.param_value_,
        bind_param_.param_len_,
        bind_param_.param_fmt_,
        res_fmt);
    return get_result(res,
        num_affect,
        nullptr,
        &pq_res);
}

int command::get_result(PGresult* res,
    size_t& num_affect,
    size_t* last_id,
    zce::pq::result* pq_res)
{
    num_affect = 0;
    int ret = 0;
    ExecStatusType status = ::PQresultStatus(res);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK)
    {
        ZCE_LOG(RS_ERROR,
            "Failed to prepare SQL : %s : %s\n", stmt_name_,
            ::PQerrorMessage(conn_));
        ret = -1;
    }
    else
    {
        if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK)
        {
            auto s_num = ::PQcmdTuples(res);
            num_affect = std::stoull(s_num);
        }
        //如果想得到sequence id，请使用RETURNING子句
        //INSERT INTO example_table (name) VALUES ('Sample Name') RETURNING id;
        if (last_id)
        {
            *last_id = 0;
            char* id_str = ::PQgetvalue(res, 0, 0);
            if (id_str != nullptr)
            {
                *last_id = atoll(id_str);
            }
        }
        if (pq_res)
        {
            pq_res->set_result(res);
        }
    }
    //如果不需要结果集，直接清理掉结果集
    if (pq_res == nullptr && res != nullptr)
    {
        ::PQclear(res);
    }
    return ret;
}

//得到SQL语句中的参数个数
size_t command::stmt_count_sql_param(const std::string_view& sql)
{
    size_t max_param = 0;
    size_t i = 0;
    const size_t len = sql.length();

    while (i < len)
    {
        char ch = sql[i];

        // 跳过单引号字符串
        if (ch == '\'')
        {
            i++;
            while (i < len) {
                if (sql[i] == '\'')
                {
                    if (i + 1 < len && sql[i + 1] == '\'')
                    {
                        i += 2; // 跳过转义的 ''
                    }
                    else
                    {
                        i++;
                        break;
                    }
                }
                else
                {
                    i++;
                }
            }
        }
        // 跳过 -- 注释
        else if (ch == '-' && i + 1 < len && sql[i + 1] == '-')
        {
            i += 2;
            while (i < len && sql[i] != '\n') i++;
        }
        // 跳过 /* ... */ 注释
        else if (ch == '/' && i + 1 < len && sql[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < len && !(sql[i] == '*' && sql[i + 1] == '/'))
            {
                i++;
            }
            i += 2; // skip */
        }

        // 查找 $n 参数
        else if (ch == '$' && i + 1 < len && isdigit(sql[i + 1]))
        {
            size_t start = i + 1;
            while (start < len && isdigit(sql[start])) start++;
            int param = atoi(sql.substr(i + 1, start - (i + 1)).data());
            if (param > max_param)
            {
                max_param = param;
            }
            i = start;
        }
        else
        {
            i++;
        }
    }

    return max_param;
}
}

#endif
