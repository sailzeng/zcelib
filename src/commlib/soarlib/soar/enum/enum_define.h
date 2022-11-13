/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar/enum/enum_define.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年6月22日
* @brief      这个.h请务必保持干净.
*
*
* @details
*
*
*
* @note
*
*/

#ifndef SOARING_LIB_PER_DEFINE_BASE_H_
#define SOARING_LIB_PER_DEFINE_BASE_H_

//业务ID
enum SOAR_BUSINESS_ID
{
    //业务ID
    INVALID_BUSINESS_ID = 0,

    //公用业务ID从1-1000
    SOAR_PUBLIC_BUSINESS_ID = 100,

    //游戏业务从1000开始
    GAME_BUSINESS_ID_BEGIN = 1000,
};

// 通用服务类型定义， 通用服务类型<30 30以上的业务服务类型
enum SOAR_SERVICES_TYPE
{
    //无效的服务器编号-----------------------------------------------------
    SVC_INVALID_SERVER = 0,

    //服务器端编号----------------------------------------------------------

    // 配置服务器
    SVC_CFG_SVR = 1,

    // 日志服务器
    SVC_LOG_SVR = 2,

    // 监控服务器
    SVC_MONITOR_SVR = 3,

    // proxy
    SVC_PROXY_SERVER = 4,

    // ZERG SVR
    SVC_ZERGLING_SVR = 5,

    // 临时service type, 和zerg分配的动态service id搭配使用
    SVC_TMP_TYPE = 6,

    // GunnerSvr
    SVR_GUNNER_TYPE = 7,

    // store adapt,存储代理
    SVC_STORE_ADAPT = 8,

    // UDP服务器的Type开始值
    SVC_UDP_SERVER_BEGIN = 30000,

    //几个通讯服务器的特殊标示
    SVC_SPECIAL_BEGIN = 49000,
    //TCPSVRD,ZERGING通讯服务器
    SVR_ZERGING_TCPSVRD = 49001,
    //OG4A,通用的通讯服务器模型
    SVR_OGRE4A_COMM_SVRD = 49002,
};

//客户端所需的服务器列表
struct CLIENT_NEED_SVC_TYPE
{
    //服务器类型
    unsigned short    svc_type_;
    //是否必须
    bool              if_must_need_;
};

//访问的DB类型
enum QUERY_DATABASE_TYPE
{
    DB_TYPE_MYSQL,
    DB_TYPE_SQLITE,
};

//无效的下标值
static const unsigned int   PTR_INVALID = 0xFFFFFFFF;

//保留日志的个数
static const unsigned int  RESERVE_LOGFILE_NUM = 60;

//CONST 数值定义

// 网页跳转代码
static const size_t   LEN_OF_WEB_SIGNATURE_KEY = 32;

//默认客户端版本
static const short    DEFAULT_QQPET2_CLIENT_VER = 6868;

// 各种签名的长度
static const size_t   LEN_OF_SIGNATURE_KEY_56 = 56;
static const size_t   LEN_OF_SIGNATURE_KEY_64 = 64;

static const size_t   LEN_OF_UIN_SESSION = 16;

//
static const size_t   MAX_SHORTNAME_STRING = 16;
//名称的最大长度,注意不是BUFFER长度
static const size_t   MAX_NAME_LEN_STRING = 32;
//长名称的最大长度,注意不是BUFFER长度
static const size_t   MAX_LONGNAME_LEN_STRING = 64;
//注释信息的最大长度,注意不是BUFFER长度
static const size_t   MAX_COMMENT_LEN_STRING = 128;

//最大短TXT长度
static const size_t   MAX_SHORT_TEXT_STRING = 256;
//最大长TXT长度
static const size_t   MAX_LONG_TEXT_STRING = 512;

//文本的长度
static const size_t   MAX_LONG_LONG_TEXT_STRING = 1024;
//更大的文本长度
static const size_t   MAX_HUGE_TEXT_STRING = 2048;
//更大大的文本长度
static const size_t   MAX_TOO_HUGE_TEXT_STRING = 4096;
//更更更大的文本长度。恩，我们不在乎.因为要在堆上实现
static const size_t   MAX_DO_NOT_CARE_TEXT_STRING = 12 * 1024;

//结婚誓言字符串最大长度
static const size_t   MARRIAGE_VOW_MAX_LEN = 32;
//结婚誓言字符串缓冲区最大长度
static const size_t   MARRIAGE_VOW_BUFFER_LEN = MARRIAGE_VOW_MAX_LEN + 1;

//md5校验值缓冲区长度
static const size_t   LEN_OF_MD5_CHECK_SUM_BUFFER = 64;

//工作职业名称的最大长度
static const size_t   WORK_TITLE_STR_MAX_LEN = 16;
//学习课程名称的最大长度
static const size_t   TRAIN_SUBJECT_STR_MAX_LEN = 16;
//职业名称最大长度
static const size_t   CAREER_TITLE_STR_MAX_LEN = 16;

//时效道具+400心情 在使用道具和计算基准的时候都要加，所以写在这里
static const unsigned short ADD_400_MENTEL_BENCH_MARK = 400;

//浮点数判等用的epsilon邻域
static const double  EPSILON = 1E-6;

//各种关于时间的定义
static const unsigned int   ONE_WEEK_DAYS = 7;

//一分钟的秒数
static const unsigned int   ONE_MINUTE_SECOND = 60;
//一小时的秒数
static const unsigned int   ONE_HOUR_SECOND = 3600;
//一天的秒数86400
static const unsigned int   ONE_DAY_SECOND = ONE_HOUR_SECOND * 24;
//
//一周的秒数
static const unsigned int   ONE_WEEK_SECONDS = ONE_DAY_SECOND * ONE_WEEK_DAYS;

//定义2个宏
//取从1970到现在当前的日期
#define DATE_NOW(x)  (( (x) + ONE_HOUR_SECOND * 8)/ONE_DAY_SECOND)

//当前日期的秒数
#define SECOND_NOW(x)  (( (x) + ONE_HOUR_SECOND * 8)%ONE_DAY_SECOND)

//当前日期的点数
#define HOUR_NOW(x) (SECOND_NOW(x) / ONE_HOUR_SECOND)

//当前时间戳8小时
#define TIMESTAMP_NOW(X) ( (X) + (ONE_HOUR_SECOND * 8) )

//当前周的秒数 这里为啥要+3，因为19700101是礼拜四。还是注释清楚吧，从周一到现在
#define  WEEKDAY_NOW(x)  (( TIMESTAMP_NOW(x) + ONE_DAY_SECOND * 3)%ONE_WEEK_SECONDS)

#endif //_SOARING_LIB_PER_DEFINE_BASE_H_
