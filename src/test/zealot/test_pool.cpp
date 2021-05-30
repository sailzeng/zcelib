#include "predefine.h"

int test_pool(int /*argc*/, char* /*argv*/[])
{
    typedef zce::object_pool<zce::buffer_queue>  buffer_pool;
    buffer_pool pool;
    std::function<bool(zce::buffer_queue *)> init_fun =
        std::bind(&zce::buffer_queue::initialize, std::placeholders::_1, 1024);
    std::function<void(zce::buffer_queue *)> clear_fun =
        std::bind(&zce::buffer_queue::clear, std::placeholders::_1);
    pool.initialize(100,
                    1024,
                    init_fun,
                    clear_fun);
    return 0;
}