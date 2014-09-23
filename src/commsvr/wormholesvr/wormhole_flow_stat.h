/// @file       arbiter_flow_stat.h
/// @date       2012/07/19
///
/// @author     stefzhou
///
/// @brief      统计处理
///

#ifndef ARBITER_FLOW_STAT_H_
#define ARBITER_FLOW_STAT_H_

// 根据svc_type和svr_id创建64位key, 同于数据统计
#define MAKE_STAT_KEY(svc_type, svc_id) ((((uint64_t)svc_type) << 32) + ((uint64_t)(svc_id)))

/******************************************************************************************
账单信息
******************************************************************************************/
enum ProxyLogType
{
    LOGTYPE_CHANNEL_FLOW_STAT = 1,
};

/******************************************************************************************
通道信息, 每个通道由<from_type, to_type>确定
******************************************************************************************/
struct ChannelInfo
{
public:
    // 数据包源type
    unsigned short  from_type_;

    // 数据包目的type
    unsigned short  to_type_;

public:
    ChannelInfo(unsigned short from_type, unsigned short to_type);
    ~ChannelInfo();

    // 需要重载==操作符
    bool operator ==(const ChannelInfo &others) const;

    // 需要重载!=操作符
    bool operator !=(const ChannelInfo &others) const;

    // 需要重载<操作符
    bool operator <(const ChannelInfo &others) const;
};

// 得到KEY的HASH函数
class KeyofChannelInfo
{
public:
    // 这里如果不改成size_t会有编译告警，
    size_t operator()(const ChannelInfo &channel_info) const
    {
        return (((unsigned int)(channel_info.from_type_)) << 16) + channel_info.to_type_;
    }
};

typedef hash_map<ChannelInfo, size_t, KeyofChannelInfo> ChannelFlowInfo;

class ArbiterFlowStat
{
public:
    // 单子实例函数
    static ArbiterFlowStat *instance();

    // 清理单子实例
    static void clean_instance();

private:
    static ArbiterFlowStat *instance_;

    ArbiterFlowStat();
    ~ArbiterFlowStat();

public:
    // stat账单输出
    void write_flow_stat();

    // 增加通道数据包量
    void increase_channel_pkg_num(unsigned short from_type, unsigned short to_type);

public:
    // 通道包量统计
    ChannelFlowInfo  channel_flow_info_;
};



// 用于统计每个发送id的数据量
class ArbiterSendStat
{
public:
    // 单子实例函数
    static ArbiterSendStat *instance();

    // 清理单子实例
    static void clean_instance();


public:
    // 计数器自增
    void send_flow_increase_once(uint32_t svc_type, uint32_t svc_id);

    // 统计输出
    void show_flow_info(void);

private:
    static ArbiterSendStat *instance_;

    ArbiterSendStat();
    ~ArbiterSendStat();

private:
    // 用于统计通道负载差
    struct SendIdStatInfo
    {
        uint64_t max_val_;
        uint64_t min_val_;

        SendIdStatInfo()
        {
            max_val_ = 0ULL;
            min_val_ = -1ULL;
        }
    };

    // 发送通道统计
    typedef hash_map<uint64_t, uint64_t> SendFlotStatType;

    // 结果统计
    typedef hash_map<uint64_t, SendIdStatInfo> SendFlotCalType;

    // 统计存储结构
    SendFlotStatType flow_stat_;

    SendFlotCalType cal_stat_;

};




#endif // ARBITER_FLOW_STAT_H_
