#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

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

    bool is_connected()
    {
        return if_connected_;
    }

    int connect_by_host(const char* host_name,
                        const unsigned int port = POSTGRESQL_PORT,
                        const char* user = "postgres",
                        const char* pwd = "",
                        const char* db = nullptr);

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

    static const short POSTGRESQL_PORT = 5432;

protected:
    //!
    ::PGconn* conn_ = nullptr;
    //!
    bool if_connected_ = false;
};
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
