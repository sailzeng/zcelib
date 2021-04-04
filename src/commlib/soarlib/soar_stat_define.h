/// @file       soar_stat_define.h
/// @date       2012/07/21 15:04
///
/// @author     yunfeiyang
///
/// @brief      ��ܼ������ID����
///

#ifndef SOARING_LIB_MONITOR_DEFINE_H_
#define SOARING_LIB_MONITOR_DEFINE_H_


// �������ID��9100��ʼ
enum CommStatFeatureID
{
    // ��Ч��ͳ��ID
    COMM_STAT_INVALID_ID        = 0,
    // ��������IDֻ������ӣ��������м���룬��Ϊ��Щ����ֵ�Ѿ���monitor��������
    COMM_STAT_FEATURE_ID_BEGIN  = 9100,
    // ������ͳ��
    // app�յ��İ���
    COMM_STAT_RECV_PKG_COUNT    = 9101,
    // app���ͳɹ��İ���
    COMM_STAT_SEND_PKG_SUCC     = 9102,
    // app����ʧ�ܵİ���
    COMM_STAT_SEND_PKG_FAIL     = 9103,

    // ����������ͳ��
    // app������ɹ�����
    COMM_STAT_TRANS_END_SUCC   = 9104,
    // app������ʧ�ܵ���
    COMM_STAT_TRANS_END_FAIL   = 9105,
    // app������ʱ����
    COMM_STAT_TRANS_PROC_TIMEOUT = 9106,
    // app���������Ĵ�����
    COMM_STAT_TRANS_PROC_ERRNO  = 9107,

    // app���񻨷ѵ�ʱ��
    COMM_STAT_TRANS_USE_TIME    = 9108,

    // app���̴��״̬,ֻҪ����ֵ�ϱ�����ʾ���̴���
    COMM_STAT_APP_ALIVE         = 9109,

    // app�������ʵ���ͳ��
    COMM_STAT_BILL_COUNT        = 9110,

    // ���̼�ϵͳ����ϱ���
    // ϵͳ��CPU����ǧ����
    COMM_STAT_SYS_CPU_RATIO     = 9111,
    // ���̵�CPU����ǧ����
    COMM_STAT_PROCESS_CPU_RATIO = 9112,
    // ϵͳ�ڴ�ʹ��ǧ����
    COMM_STAT_SYS_MEM_USE_RATIO = 9113,
    // ϵͳ��ǰ�����ڴ棨��λ��Byte��
    COMM_STAT_AVAILABLE_MEM_SIZE = 9114,

    // app�յ����ݵ��ֽ���
    COMM_STAT_RECV_PKG_BYTES        = 9115,
    // app���Ͱ����ֽ���
    COMM_STAT_SEND_PKG_BYTES        = 9116,

    // ��notify task�����з�������ʧ�ܵ���
    COMM_STAT_TASK_QUEUE_SEND_FAIL = 9117,


    COMM_STAT_APP_RESTART_TIMES     = 9200,
    COMM_STAT_APP_OFFLINE           = 9201,

};

// ��ܷ���id
enum CommStatClassfyID
{
    COMM_STAT_ZERG_CID = 1,
    COMM_STAT_APP_CID,
};


// ͳ��������Ϣ
static const ZCE_STATUS_ITEM_WITHNAME COMM_STAT_ITEM_WITH_NAME[] =
{
    DEF_ZCE_STATUS_ITEM(COMM_STAT_RECV_PKG_COUNT, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_SEND_PKG_SUCC, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_SEND_PKG_FAIL, STATICS_PER_FIVE_MINTUES),

    DEF_ZCE_STATUS_ITEM(COMM_STAT_TRANS_END_SUCC, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_TRANS_END_FAIL, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_TRANS_PROC_TIMEOUT, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_TRANS_PROC_ERRNO, STATICS_PER_FIVE_MINTUES),

    DEF_ZCE_STATUS_ITEM(COMM_STAT_TRANS_USE_TIME, STATICS_PER_FIVE_MINTUES),

    DEF_ZCE_STATUS_ITEM(COMM_STAT_APP_ALIVE, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_BILL_COUNT, STATICS_PER_FIVE_MINTUES),

    // ���̼�ϵͳ����ϱ���
    DEF_ZCE_STATUS_ITEM(COMM_STAT_SYS_CPU_RATIO, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_PROCESS_CPU_RATIO, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_SYS_MEM_USE_RATIO, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_AVAILABLE_MEM_SIZE, STATICS_PER_FIVE_MINTUES),

    // app�յ��İ��ֽ���
    DEF_ZCE_STATUS_ITEM(COMM_STAT_RECV_PKG_BYTES, STATICS_PER_FIVE_MINTUES),
    DEF_ZCE_STATUS_ITEM(COMM_STAT_SEND_PKG_BYTES, STATICS_PER_FIVE_MINTUES),

    // ��notify task�����з�������ʧ�ܵ���
    DEF_ZCE_STATUS_ITEM(COMM_STAT_TASK_QUEUE_SEND_FAIL, STATICS_PER_FIVE_MINTUES),

};

// �ϱ����������
static const unsigned int COMM_STAT_FRATURE_NUM = ZCE_ARRAY_SIZE(COMM_STAT_ITEM_WITH_NAME);

#endif //COMM_STAT_DEFINE_H_
