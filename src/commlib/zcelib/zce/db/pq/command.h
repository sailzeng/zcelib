#pragma once

#if defined ZCE_USE_PQ && ZCE_USE_PQ == 1

namespace zce::pq
{
class result;

class command
{
public:

    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                uint64_t* last_id);

    int execute(std::string_view sqlcmd,
                size_t& num_affect,
                zce::pq::result* pq_res);

protected:
    //
    ::PGconn* conn_ = nullptr;
};
}

#endif
