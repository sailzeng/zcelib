#pragma once
#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
namespace zce::pq
{
class result
{
public:

    ::PGresult* res = nullptr;
};
}
#endif //#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1
