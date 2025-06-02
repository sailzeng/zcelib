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
    connect() noexcept = default;
    ~connect() noexcept = default;

    //避免拷贝
    connect(connect&&) noexcept = delete;
    connect& operator=(connect&&) noexcept = delete;
    connect(const connect&) = delete;
    connect& operator=(const connect&) = delete;

	//! 注意，PQ的 DATABASE 概念和 MySQL 的 DATABASE 概念不同， 
    int connect_host(const char* host_name,
                     const unsigned int port = POSTGRESQL_PORT,
                     const char* user = "postgres",
                     const char* pwd = "",
                     const char* db = nullptr);

    int connect_url(const char* url);

    int connect_by_info(const char* conninfo);

    void disconnect();

    bool is_connected()
    {
        return if_connected_;
    }

    const char* error_message()
    {
        return ::PQerrorMessage(conn_);
    }

    int error_no()
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
