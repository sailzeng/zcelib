#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/pq/command.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
///****************************************************************************************
/// class bind_data
///****************************************************************************************
//构造函数
bind_data::bind_data(size_t num_bind)
{
    initialize(num_bind);
}
//析构函数
bind_data::~bind_data()
{
    clear();
}

bind_data::bind_data(bind_data&& bind) noexcept :
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

bind_data& bind_data::operator=(bind_data&& bind) noexcept
{
    clear();
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

bind_data::bind_data(const bind_data& bind) :
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

bind_data& bind_data::operator=(const bind_data& bind)
{
    clear();
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

void bind_data::clear()
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

void bind_data::initialize(size_t num_bind)
{
    clear();
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

int bind_data::bind_one_param(size_t id,
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

///****************************************************************************************
/// class command
///****************************************************************************************

int command::execute(std::string_view sql_cmd,
                     size_t& num_affect,
                     uint64_t* last_id)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
    ::ExecStatusType status = ::PQresultStatus(res);
    if (status != ::PGRES_TUPLES_OK && status != ::PGRES_COMMAND_OK)
    {
        ::PQclear(res);
        return -1;
    }
    if (status == ::PGRES_TUPLES_OK)
    {
        auto s_num = ::PQcmdTuples(res);
        num_affect = std::stoull(s_num);
    }
    if (last_id)
    {
        if (PQresultStatus(res) == PGRES_TUPLES_OK)
        {
            char* id_str = PQgetvalue(res, 0, 0);
            *last_id = atoll(id_str);
        }
        else
        {
            ZCE_LOG(RS_ALERT,
                    "[pq] execute sql don't have last id [%s].",
                    ::PQresultErrorMessage(res));
            return -1;
        }
    }
    PQclear(res);
    return 0;
}

int command::execute(std::string_view sql_cmd,
                     size_t& num_affect,
                     zce::pq::result* /*pq_res*/)
{
    num_affect = 0;
    ::PGresult* res = ::PQexec(conn_, sql_cmd.data());
    ::ExecStatusType status = ::PQresultStatus(res);
    if (status != ::PGRES_TUPLES_OK && status != ::PGRES_COMMAND_OK)
    {
        ::PQclear(res);
        return -1;
    }
    if (status == ::PGRES_TUPLES_OK)
    {
        auto s_num = ::PQcmdTuples(res);
        num_affect = std::stoull(s_num);
    }
    //pq_res->set_pg_result(res);
    return 0;
}
}

#endif
