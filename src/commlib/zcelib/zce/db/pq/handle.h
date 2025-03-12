#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
/*!
@brief
*/
class handle :public zce::db::handle_base
{
public:

    //构造函数,析构函数
    handle() noexcept;
    ~handle() noexcept;

    //避免拷贝
    handle(handle&&) noexcept = delete;
    handle& operator=(handle&&) noexcept = delete;
    handle(const handle&) = delete;
    handle& operator=(const handle&) = delete;

    int connect_by_info(const char* conninfo);

    int connect_by_params(const char* const* keywords,
        const char* const* values,
        int expand_dbname);

    int connect_by_host(const char* pghost,
        const char* pgport,
        const char* pgoptions,
        const char* pgtty,
        const char* dbName,
        const char* login,
        const char* pwd);

    const char* error_message()
    {
        return mysql_error(&mysql_handle_);
    }

    unsigned int error_no()
    {
        return mysql_errno(&mysql_handle_);
    }

    PGconn* get_handle()
    {
        return conn_;
    }
protected:

    int connect_i();

protected:
    //
    PGconn* conn_ = nullptr;
};
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
