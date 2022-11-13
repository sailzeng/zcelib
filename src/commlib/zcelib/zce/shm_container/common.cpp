#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/shm_container/common.h"

namespace zce
{
//做适当的放大处理，
static size_t zoomin_nodenum(size_t node_num)
{
    size_t zoomout_num = 0;
    //根据输入的数据，做适当的放大，再进行处理
    //如果大于10亿，增加1亿，其他以此类推,再开始计算最靠近的质数
    if (node_num > 1000000000ul)
    {
        zoomout_num = node_num + 100000000ul;
    }
    else if (node_num > 100000000ul)
    {
        zoomout_num = node_num + 10000000ul;
    }
    else  if (node_num > 10000000ul)
    {
        zoomout_num = node_num + 1000000ul;
    }
    else  if (node_num > 1000000ul)
    {
        zoomout_num = node_num + 100000ul;
    }
    else  if (node_num > 660000ul)
    {
        zoomout_num = node_num + 30000ul;
    }
    else  if (node_num > 330000ul)
    {
        zoomout_num = node_num + 20000ul;
    }
    else  if (node_num > 100000ul)
    {
        zoomout_num = node_num + 15000ul;
    }
    else  if (node_num > 66000ul)
    {
        zoomout_num = node_num + 5000ul;
    }
    else  if (node_num > 33000ul)
    {
        zoomout_num = node_num + 3000ul;
    }
    else  if (node_num > 10000ul)
    {
        zoomout_num = node_num + 2000ul;
    }
    else
    {
        zoomout_num = node_num + 500ul;
    }
    return zoomout_num;
}

//
void hash_prime(const size_t node_num, size_t& real_num)
{
    real_num = 0;
    //做一定的放大处理
    size_t zoomout_num = zce::zoomin_nodenum(node_num);
    real_num = zce::nearest_prime(zoomout_num);
    return;
}

//
void hash_prime_ary(const size_t num_node, size_t& real_num, size_t row, size_t prime_ary[])
{
    //做一定的放大处理
    size_t zoomout_num = zce::zoomin_nodenum(num_node);
    size_t per_row_num = zoomout_num / row + 1;

    //制造一个质数队列，
    size_t test_num = per_row_num;
    for (size_t i = 0; i < row; ++i)
    {
        prime_ary[i] = zce::nearest_prime(test_num);
        test_num = prime_ary[i];
    }

    //总计放入最后一个列
    real_num = 0;
    for (size_t i = 0; i < row; ++i)
    {
        real_num += prime_ary[i];
    }
}
}