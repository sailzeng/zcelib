#include "wormhole_predefine.h"
#include "wormhole_flow_stat.h"
#include "wormhole_stat_define.h"

ArbiterFlowStat *ArbiterFlowStat::instance_ = NULL;

ArbiterSendStat *ArbiterSendStat::instance_ = NULL;

ChannelInfo::ChannelInfo(unsigned short from_type, unsigned short to_type):
    from_type_(from_type),
    to_type_(to_type)
{

}

ChannelInfo::~ChannelInfo()
{

}

// 比较是否相同的函数
bool ChannelInfo::operator ==(const ChannelInfo &others) const
{
    if (from_type_ == others.from_type_
        && to_type_ == others.to_type_ )
    {
        return true;
    }

    return false;
}

// 比较是否相同的函数
bool ChannelInfo::operator !=(const ChannelInfo &others) const
{
    if (from_type_ != others.from_type_
        || to_type_ != others.to_type_ )
    {
        return true;
    }

    return false;
}

// 比较大小的函数
bool ChannelInfo::operator <(const ChannelInfo &others) const
{
    if (((((unsigned int)(from_type_)) << 16) + to_type_)
        < ((((unsigned int)(others.from_type_)) << 16) + others.to_type_))
    {
        return true;
    }

    return false;
}

// 单子实例函数
ArbiterFlowStat *ArbiterFlowStat::instance()
{
    if (NULL == instance_)
    {
        instance_ = new ArbiterFlowStat();
    }

    return instance_;
}

// 清理单子实例
void ArbiterFlowStat::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
}

ArbiterFlowStat::ArbiterFlowStat()
{
    channel_flow_info_.clear();

}

ArbiterFlowStat::~ArbiterFlowStat()
{

}

void ArbiterFlowStat::write_flow_stat()
{
    ChannelFlowInfo::iterator iter = channel_flow_info_.begin();

    for (; iter != channel_flow_info_.end(); iter++)
    {
        // 包量清0
        iter->second = 0;
    }
}

void
ArbiterFlowStat::increase_channel_pkg_num(unsigned short from_type, unsigned short to_type)
{
    ChannelInfo channle_info(from_type, to_type);

    ChannelFlowInfo::iterator iter = channel_flow_info_.find(channle_info);

    if (iter == channel_flow_info_.end())
    {
        // 新通道, 直接赋值
        channel_flow_info_[channle_info] = 1;
    }
    else
    {
        // 已有通道, 加1
        iter->second++;
    }
}


// 单子实例函数
ArbiterSendStat *ArbiterSendStat::instance()
{
    if (NULL == instance_)
    {
        instance_ = new ArbiterSendStat();
    }

    return instance_;
}

// 清理单子实例
void ArbiterSendStat::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
}

ArbiterSendStat::ArbiterSendStat()
{
    flow_stat_.clear();
}

ArbiterSendStat::~ArbiterSendStat()
{

}

void ArbiterSendStat::send_flow_increase_once(uint32_t svc_type, uint32_t svc_id)
{
    flow_stat_[MAKE_STAT_KEY(svc_type, svc_id)]++;
}

void ArbiterSendStat::show_flow_info(void)
{
    // 遍历hash_map所有节点
    cal_stat_.clear();
    SendFlotStatType::const_iterator iter = flow_stat_.begin();
    for ( ; iter != flow_stat_.end(); ++iter)
    {
        uint32_t svc_type = (uint32_t)(iter->first >> 32);
        uint32_t svc_id = (uint32_t)(iter->first & 0xffffffffULL);
        uint64_t stat_val = (uint64_t)iter->second;
        uint32_t game_id = 0;
        if (0 != svc_id)
        {
            game_id = svc_id / 10000;
        }

        ZLOG_INFO("gameid=%u, service:%u|%u, value=%llu", 
            game_id, svc_type, svc_id, stat_val);

        // 获得每个appid的最大值和最小值
        if (cal_stat_[MAKE_STAT_KEY(game_id, svc_type)].max_val_ < stat_val)
        {
            cal_stat_[MAKE_STAT_KEY(game_id, svc_type)].max_val_ = stat_val;
        }
        if (cal_stat_[MAKE_STAT_KEY(game_id, svc_type)].min_val_ > stat_val)
        {
            cal_stat_[MAKE_STAT_KEY(game_id, svc_type)].min_val_ = stat_val;
        }
    }

    // 上报各个gameid的负载不均衡比例
    SendFlotCalType::const_iterator iter_cal = cal_stat_.begin();
    for ( ; iter_cal != cal_stat_.end(); ++iter_cal)
    {
        uint32_t game_id = (uint32_t)(iter_cal->first >> 32);
        uint32_t svc_type = (uint32_t)(iter_cal->first & 0xffffffffULL);
        uint32_t unbalance_rate = 0;
        if (iter_cal->second.max_val_ > 0)
        {
            unbalance_rate = (iter_cal->second.max_val_ - iter_cal->second.min_val_) *
                100 / iter_cal->second.max_val_;
        }

        Comm_Stat_Monitor::instance()->set_by_statid(ARBITER_TRANS_SEND_UNBALANCE_RATE,
            game_id, svc_type, unbalance_rate);

        ZLOG_INFO("gameid=%u, svc_type=%u, max=%llu, min=%llu, unbalance_rate=%u", 
            game_id, svc_type,
            iter_cal->second.max_val_,
            iter_cal->second.min_val_,
            unbalance_rate);
    }

    // 每次显示完自动清除
    flow_stat_.clear();

}
