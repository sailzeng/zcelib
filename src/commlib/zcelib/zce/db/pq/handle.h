#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

#include "zce/db/base.h"

namespace zce::pq
{
class result;
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

    void disconnect();

    int connect_by_host(const char* pghost,
                        const char* pgport,
                        const char* pgoptions,
                        const char* dbname,
                        const char* user,
                        const char* pwd);

    const char* error_message()
    {
        return ::PQerrorMessage(conn_);
    }

    unsigned int error_no()
    {
        return 0;
    }

    int execute(size_t& num_affect, uint64_t* last_id);

    int execute(size_t& num_affect, zce::pq::result* pq_res);

    PGconn* get_handle()
    {
        return conn_;
    }

protected:
    //
    ::PGconn* conn_ = nullptr;
};
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
