#ifndef ZCE_LIB_BUS_ONE_WAY_H_
#define ZCE_LIB_BUS_ONE_WAY_H_

#include "zce_bus_mmap_pipe.h"

class dequechunk_node;
class shm_dequechunk;

//双行道
class ZCE_BusPipe_OneWay : public ZCE_Bus_MMAPPipe
{
public:

protected:
    //instance函数使用的东西
    static ZCE_BusPipe_OneWay *one_way_instance_;

public:

    //构造函数,
    ZCE_BusPipe_OneWay();
    //析购函数
    ~ZCE_BusPipe_OneWay();

public:

public:

    //实例的赋值
    static void instance(ZCE_BusPipe_OneWay *);
    //实例的获得
    static ZCE_BusPipe_OneWay *instance();
    //清除实例
    static void clean_instance();

};

#endif //ZCE_LIB_BUS_ONE_WAY_H_

