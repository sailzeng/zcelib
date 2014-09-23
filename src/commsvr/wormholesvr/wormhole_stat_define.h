/// @file       arbiter_type_define.h
/// @date       2012/07/19

/// @author     stefzhou

/// @brief      数据和类型定义
/// @details

#ifndef WORMHOLE_STAT_DEFINE_H_
#define WORMHOLE_STAT_DEFINE_H_

/******************************************************************************************
监控信息
******************************************************************************************/
enum ArbiterMonitorFeatureID
{
    WORMHOLE_SERVICE_STAT_BEGIN = SVC_PROXY_SERVER * 10000, // proxysvr start
    WORMHOLE_TRANS_PKG_ERROR,                  // 转发的数据包错误
    WORMHOLE_TRANS_SEND_UNBALANCE_RATE,        // 发送端负载不均衡比例
    WORMHOLE_SERVICE_STAT_END,
};

// 上报数据项个数
static const unsigned int WORMHOLE_FRATURE_NUM = 
    WORMHOLE_SERVICE_STAT_END - WORMHOLE_SERVICE_STAT_BEGIN - 1;

// 统计信息字符串
const ZCE_STATUS_ITEM_WITHNAME STR_WORMHOLE_STAT[WORMHOLE_FRATURE_NUM] =
{
    DEF_ZCE_STATUS_ITEM(WORMHOLE_TRANS_PKG_ERROR, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(WORMHOLE_TRANS_SEND_UNBALANCE_RATE, STATICS_PER_FIVE_MINTUES),
};

#endif // WORMHOLE_STAT_DEFINE_H_
