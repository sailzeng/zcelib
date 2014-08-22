#ifndef SOARING_LIB_MONITOR_STAT_H_
#define SOARING_LIB_MONITOR_STAT_H_

#include "soar_zerg_svc_info.h"

//原来是使用模版策略锁，后来发现模版容易将问题扩大化，
//改成多态策略
class Comm_Stat_Monitor : public ZCE_Server_Status
{

    // 监控mmap文件名最大长度
    static const size_t STAT_MMAP_FILENAME_LEN = 64;

protected:

    //生产stat文件名称
    void create_stat_fname(bool if_zerg, const SERVICES_ID &service_info);

public:
    //gunner里面有接收多个cgi统计上报的需求
    //单件无法支持
    Comm_Stat_Monitor();

    ~Comm_Stat_Monitor();

    //初始化,由于小虫和业务服务器以相同ID的共存，所以用了一个前缀
    int initialize(bool if_zerg,
                   const SERVICES_ID &service_info,
                   size_t num_stat_item,
                   const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                   bool mutli_thread);

    // 从stat文件名取出service_info
    static int get_svr_info_from_stat_fname(const char *stat_file_name,
                                            SERVICES_ID *service_info, bool *if_zerg = NULL);

    //单子的函数群，不是我不知道可以用BOOST的模板使用单子，是这样更加直接清爽，容易扩张修改一些
    //我不会为了单子考虑所谓的保护问题，你自己保证你的初始化函数不会重入
public:
    //
    static Comm_Stat_Monitor *instance();
    //
    static void clean_instance();

protected:
    // 存放mmap文件名
    char                      stat_mmap_filename_[STAT_MMAP_FILENAME_LEN + 1];

protected:
    //单子实例
    static Comm_Stat_Monitor *instance_;
};

#endif //SOARING_LIB_MONITOR_STAT_H_

