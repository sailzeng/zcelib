#include "zce/predefine.h"
#include "zce/db/pq/handle.h"

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
//构造函数,析构函数
int handle::connect_by_info(const char* conninfo)
{
    if (conninfo == nullptr)
    {
        return -1;
    }
    //连接数据库
    conn_ = ::PQconnectdb(conninfo);
    if (conn_ == nullptr)
    {
        return -1;
    }
    //检查连接状态
    if (::PQstatus(m_conn) != ::CONNECTION_OK)
    {
        ::PQfinish(m_conn);
        conn_ = nullptr;
        return -1;
    }
    return 0;
}
}

#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1