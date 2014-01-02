#include "soar_predefine.h"

/*
#include "soar_fsm_trans_mgr.h"
#include "soar_monitor_sta.h"
#include "soar_zerg_mmappipe.h"
#include "soar_svrd_config.h"

MonitorSta *MonitorSta::instance_ = NULL;

MonitorSta *MonitorSta::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new MonitorSta();
    }

    return instance_;
}

void MonitorSta::clean_instance()
{
    delete instance_;
    instance_ = NULL;
}

void MonitorSta::statistic_done(const ZCE_STATUS_ITEM *statistic_item, time_t report_time)
{
    report( statistic_item->item_id_.statics_id_,
            statistic_item->item_id_.classify_id_,
            statistic_item->counter_,
            report_time
          );

    return;
}

int MonitorSta::initialize(
    char *stat_mmap_prefix,
    const SERVICES_ID &service_info,
    bool restore_mmap,
    size_t num_stat_item,
    size_t num_classify_id,
    const ZCE_STATUS_ITEM item_ary[],
    unsigned int classify_id_ary[] = NULL
)
{
    // 获取服务器name和service_id
    // stat_mmap_filename_长度是STAT_MMAP_FILENAME_LEN + 1
    snprintf(stat_mmap_filename_, STAT_MMAP_FILENAME_LEN, "TSS_%s_%d_%d.STAT",
             stat_mmap_prefix,
             service_info.services_type_,
             service_info.services_id_);
    stat_mmap_filename_[STAT_MMAP_FILENAME_LEN] = '\0';

    // 将stat_mmap_filename_转换为大写
    ZCE_OS::strupr((char *)stat_mmap_filename_);

    int ret = ZCE_Server_Status<ZCE_Null_Mutex>::initialize(
                  stat_mmap_filename_,
                  restore_mmap,
                  num_stat_item,
                  num_classify_id,
                  item_ary,
                  classify_id_ary);

    return ret;
}

void MonitorSta::report(uint32_t statistic_id, uint32_t classify_id, uint64_t value, time_t report_time)
{
    sec_proto::MonitorSvrPkg pkg;
    pkg.head_.cmd_ = sec_proto::REQ_MONITORSVR_REPORT;
    pkg.head_.seq_ = 0;
    pkg.head_.uin_ = 0;
    pkg.head_.appid_ = 0;

    sec_proto::MonitorSvrReport &report = pkg.body_.report_;

    report.count_ = 1;
    report.item_[0].app_id_ = classify_id;
    report.item_[0].feature_id_ = statistic_id;
    report.item_[0].feature_value_ = value;

    SERVICES_ID selfsvc = *Transaction_Manager::instance()->self_svc_info();
    report.report_time_ = static_cast<uint32_t>(report_time);
    report.service_type_ = selfsvc.services_type_;
    report.service_id_ = selfsvc.services_id_;

    // 只有使用了trans action的才使用网络监控
    SERVICES_ID rcvsvc(SVC_MONITOR_SVR, 0);
    SERVICES_ID proxysvc(0, 0);

    //    int ret = Transaction_Manager::instance()->mgr_sendmsg_to_service
    //        (sec_proto::CMD_MONITORSVR_REPORT_SINGLE_REQ,
    //        0, 0, 0, rcvsvc, proxysvc, selfsvc, pkg, 0, 0);
    //

    //这里只能允许业务服务器这样使用，而zergsvr不能使用，否则会出现管道有两个写者的情况
    int ret = Zerg_MMAP_BusPipe::instance()->pipe_sendmsg_to_service(sec_proto::REQ_MONITORSVR_REPORT,
                                                                     0,
                                                                     0,
                                                                     0,
                                                                     selfsvc,
                                                                     rcvsvc,
                                                                     pkg);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("statistic report fail: send to monitorsvr fail. ret=%d", ret);
    }
}
*/
