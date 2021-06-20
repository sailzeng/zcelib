#include "predefine.h"

#include <zce/util/random.h>
#include <zce/rudp/rudp.h>

int test_rudp(int argc, char* argv[])
{
    if (argc > 1)
    {
        return test_rudp_core(argc, argv);
    }
}

int test_rudp_core(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::rudp::CORE core;
    sockaddr_in core_addr;
    zce::set_sockaddr_in(&core_addr, "0.0.0.0", 888);
    ret = core.initialize((sockaddr *)&core_addr,
                          1024,
                          1024,
                          64 * 1024,
                          64 * 1024);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int test_rudp_peer(int /*argc*/, char* /*argv*/[])
{
    return 0;
}