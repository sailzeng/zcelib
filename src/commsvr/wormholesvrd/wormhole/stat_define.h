/// @file       arbiter_type_define.h
/// @date       2012/07/19

/// @brief      数据和类型定义
/// @details

#pragma once

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

// 统计信息字符串
static const zce::STATUS_ITEM_WITHNAME STR_WORMHOLE_STAT[] =
{
    DEF_STATUS_ITEM(WORMHOLE_TRANS_PKG_ERROR,          zce::STATUS_STATICS::PER_FIVE_MINTUES),
    DEF_STATUS_ITEM(WORMHOLE_TRANS_SEND_UNBALANCE_RATE,zce::STATUS_STATICS::PER_FIVE_MINTUES),
};
