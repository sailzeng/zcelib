/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   define.h
* @author     yunfeiyang
* @version
* @date       2021年5月5日
* @brief
*
*
* @details    框架监控特性ID定义
*
*
*
* @note
*
*/

#ifndef SOARING_LIB_MONITOR_DEFINE_H_
#define SOARING_LIB_MONITOR_DEFINE_H_

// 框架特性ID从100开始,到1000,
enum COMM_STAT_FEATURE_ID
{
    // 以下特性ID只能往后加，不能在中间插入，因为这些特性值已经在monitor上配置了
    COMM_STAT_ID_BEGIN                       = 100,

    // BUS的统计
    // BUS收到的包量
    COMM_RECV_BUS_POP_COUNT                  = 102,
    COMM_SEND_BUS_POP_COUNT                  = 103,
    // BUS发送成功的包量
    COMM_RECV_BUS_PUSH_SUCC                  = 104,
    COMM_SEND_BUS_PUSH_SUCC                  = 105,
    // BUS发送失败的包量
    COMM_RECV_BUS_PUSH_FAIL                  = 106,
    COMM_SEND_BUS_PUSH_FAIL                  = 107,
    // BUS收到数据的字节数
    COMM_RECV_BUS_POP_BYTES                  = 110,
    COMM_SEND_BUS_POP_BYTES                  = 111,
    // BUS发送包的字节数
    COMM_RECV_BUS_PUSH_BYTES                 = 112,
    COMM_SEND_BUS_PUSH_BYTES                 = 113,

    // 往notify task队列中发送数据失败的量
    //
    COMM_STAT_TASK_QUEUE_RECV_COUNT          = 120,
    //
    COMM_STAT_TASK_QUEUE_SEND_FAIL           = 121,
    //
    COMM_STAT_TASK_QUEUE_SEND_SUCC           = 122,

    // 包处理结果的统计
    COMM_STAT_TRANS_START_SUCC               = 130,
    // app事务处理成功的量
    COMM_STAT_TRANS_END_SUCC                 = 131,
    // app事务处理失败的量
    COMM_STAT_TRANS_END_FAIL                 = 132,
    // app事务处理出错的错误码
    COMM_STAT_TRANS_END_FAIL_BY_SVCTYPE      = 133,
    // app事务处理超时的量
    COMM_STAT_TRANS_PROC_TIMEOUT             = 134,
    // app事务花费的时间
    COMM_STAT_TRANS_USE_TIME                 = 135,

    // app进程存活状态,只要有数值上报，表示进程存在,采用绝对值保存
    COMM_STAT_APP_ALIVE                      = 140,
    //服务器重启的次数，按小时统计
    COMM_STAT_APP_RESTART_TIMES              = 141,
    //服务器离线的次数，正常退出，按小时统计
    COMM_STAT_APP_EXIT_TIMES                 = 142,

    // 进程及系统监控上报项
    // 系统的CPU利用千分率
    COMM_STAT_SYS_CPU_RATIO                  = 150,
    // 进程的CPU利用千分率
    COMM_STAT_PROCESS_CPU_RATIO              = 151,
    // 系统内存使用千分率
    COMM_STAT_SYS_MEM_USE_RATIO              = 152,
    // 系统当前可用内存（单位：Byte）
    COMM_STAT_AVAILABLE_MEM_SIZE             = 153,

    COMM_STAT_ID_END                         = 999,
};

// 统计配置信息
static const zce::STATUS_ITEM_WITHNAME COMM_STAT_ITEM_WITH_NAME[] =
{
    DEF_STATUS_ITEM(COMM_RECV_BUS_POP_COUNT,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_SEND_BUS_POP_COUNT,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_RECV_BUS_PUSH_SUCC,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_SEND_BUS_PUSH_SUCC,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_RECV_BUS_PUSH_FAIL,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_SEND_BUS_PUSH_FAIL,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_RECV_BUS_POP_BYTES,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_SEND_BUS_POP_BYTES,                 zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_RECV_BUS_PUSH_BYTES,                zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_SEND_BUS_PUSH_BYTES,                zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_STAT_TASK_QUEUE_RECV_COUNT,         zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TASK_QUEUE_SEND_FAIL,          zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TASK_QUEUE_SEND_FAIL,          zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_STAT_TRANS_START_SUCC,              zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TRANS_END_SUCC,                zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TRANS_END_FAIL,                zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TRANS_PROC_TIMEOUT,            zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TRANS_END_FAIL_BY_SVCTYPE,     zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(COMM_STAT_TRANS_USE_TIME,                zce::STATUS_STATICS::PER_FIVE_MINTUES),

    DEF_STATUS_ITEM(COMM_STAT_APP_ALIVE,                     zce::STATUS_STATICS::ABSOLUTE_VALUE),
    DEF_STATUS_ITEM(COMM_STAT_APP_RESTART_TIMES,             zce::STATUS_STATICS::PER_HOUR),
    DEF_STATUS_ITEM(COMM_STAT_APP_EXIT_TIMES,                zce::STATUS_STATICS::PER_HOUR),

    DEF_STATUS_ITEM(COMM_STAT_SYS_CPU_RATIO,                 zce::STATUS_STATICS::ABSOLUTE_VALUE),
    DEF_STATUS_ITEM(COMM_STAT_PROCESS_CPU_RATIO,             zce::STATUS_STATICS::ABSOLUTE_VALUE),
    DEF_STATUS_ITEM(COMM_STAT_SYS_MEM_USE_RATIO,             zce::STATUS_STATICS::ABSOLUTE_VALUE),
    DEF_STATUS_ITEM(COMM_STAT_AVAILABLE_MEM_SIZE,            zce::STATUS_STATICS::ABSOLUTE_VALUE),
};

// 上报数据项个数
static const unsigned int COMM_STAT_FRATURE_NUM = ZCE_ARRAY_SIZE(COMM_STAT_ITEM_WITH_NAME);

#endif //COMM_STAT_DEFINE_H_
