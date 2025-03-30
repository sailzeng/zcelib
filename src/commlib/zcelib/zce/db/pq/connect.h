#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

#include "zce/db/base.h"

namespace zce::pq
{
/*!
@brief
*/
class connect
{
public:

    //构造函数,析构函数
    connect() noexcept;
    ~connect() noexcept;

    //避免拷贝
    connect(connect&&) noexcept = delete;
    connect& operator=(connect&&) noexcept = delete;
    connect(const connect&) = delete;
    connect& operator=(const connect&) = delete;

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

    ::PGconn* get_handle()
    {
        return conn_;
    }

protected:
    //
    ::PGconn* conn_ = nullptr;
};
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
