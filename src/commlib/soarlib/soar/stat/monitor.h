#pragma once

#include "soar/zerg/services_info.h"

namespace soar
{

//原来是使用模版策略锁，后来发现模版容易将问题扩大化，
//改成多态策略
class Stat_Monitor: public zce::Server_Status
{
public:
    //gunner里面有接收多个cgi统计上报的需求
    //单件无法支持
    Stat_Monitor();

    ~Stat_Monitor();

    /*!
    * @brief      初始化,由于小虫和业务服务器以相同ID的共存，所以用了一个前缀
    * @return     int
    * @param[in]  app_base_name 进程的名称
    * @param[in]  business_id   业务ID,
    * @param[in]  service_info  服务ID
    * @param[in]  num_stat_item 统计项目数量
    * @param[in]  item_ary      统计项队列，包括名称
    * @param[in]  mutli_thread  是否在多线程环境下使用
    */
    int initialize(const char *app_base_name,
                   unsigned int business_id,
                   const soar::SERVICES_ID &service_id,
                   size_t num_stat_item,
                   const zce::STATUS_ITEM_WITHNAME item_ary[],
                   bool mutli_thread);

    /*!
    * @brief      从stat文件名取出service_id ，app name等信息
    * @return     int ==0 表示成功，否则表示文件名称错误
    * @param[in]  stat_file_name 文件名称
    * @param[out] service_id     服务器的SVC ID
    * @param[out] app_base_name  进程的名称
    * @note
    */
    static int get_info_from_fname(const char *stat_file_name,
                                   unsigned int *business_id,
                                   soar::SERVICES_ID *service_info,
                                   char *app_base_name);

protected:

    //生产stat文件名称
    void create_stat_fname(const char *app_base_name,
                           unsigned int business_id,
                           const soar::SERVICES_ID &service_info);

    //单子的函数群，不是我不知道可以用BOOST的模板使用单子，是这样更加直接清爽，容易扩张修改一些
    //我不会为了单子考虑所谓的保护问题，你自己保证你的初始化函数不会重入
public:
    //
    static Stat_Monitor *instance();
    //
    static void clean_instance();

protected:
    // 监控mmap文件名最大长度
    static const size_t STAT_MMAP_FILENAME_LEN = 128;

protected:
    // 存放mmap文件名
    char stat_mmap_filename_[STAT_MMAP_FILENAME_LEN + 1];

protected:
    //单子实例
    static Stat_Monitor *instance_;
};

}


