/// @file       arbiter_type_define.h
/// @date       2012/07/19

/// @author     stefzhou

/// @brief      数据和类型定义
/// @details

#ifndef ARBITER_STAT_DEFINE_H_
#define ARBITER_STAT_DEFINE_H_

/******************************************************************************************
监控信息
******************************************************************************************/
enum ArbiterMonitorFeatureID
{
    ARBITER_SERVICE_STAT_BEGIN = SVC_PROXY_SERVER * 10000, // proxysvr start
    ARBITER_TRANS_PKG_ERROR,                  // 转发的数据包错误
    ARBITER_TRANS_SEND_UNBALANCE_RATE,        // 发送端负载不均衡比例
    ARBITER_SERVICE_STAT_END,
};

// 上报数据项个数
static const unsigned int ARBITER_FRATURE_NUM = 
    ARBITER_SERVICE_STAT_END - ARBITER_SERVICE_STAT_BEGIN - 1;

// 统计信息字符串
const ZEN_STATUS_ITEM_WITHNAME STR_ARBITER_STAT[ARBITER_FRATURE_NUM] =
{
    DEF_ZEN_STATUS_ITEM(ARBITER_TRANS_PKG_ERROR, STATICS_PER_FIVE_MINTUES),
    DEF_ZEN_STATUS_ITEM(ARBITER_TRANS_SEND_UNBALANCE_RATE, STATICS_PER_FIVE_MINTUES),
};

#endif // ARBITER_STAT_DEFINE_H_
