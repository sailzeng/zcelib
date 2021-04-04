/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_enum_define.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��6��22��
* @brief      ���.h����ر��ָɾ�.
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


//ҵ��ID
enum SOAR_BUSINESS_ID
{
    //ҵ��ID
    INVALID_BUSINESS_ID = 0,

    //����ҵ��ID��1-1000
    SOAR_PUBLIC_BUSINESS_ID = 100,


    //��Ϸҵ���1000��ʼ
    GAME_BUSINESS_ID_BEGIN =  1000,


};





// ͨ�÷������Ͷ��壬 ͨ�÷�������<30 30���ϵ�ҵ���������
enum SOAR_SERVICES_TYPE
{
    //��Ч�ķ��������-----------------------------------------------------
    SVC_INVALID_SERVER      = 0,

    //�������˱��----------------------------------------------------------

    // ���÷�����
    SVC_CFG_SVR             = 1,

    // ��־������
    SVC_LOG_SVR             = 2,

    // ��ط�����
    SVC_MONITOR_SVR         = 3,

    // proxy
    SVC_PROXY_SERVER        = 4,

    // ZERG SVR
    SVC_ZERGLING_SVR        = 5,

    // ��ʱservice type, ��zerg����Ķ�̬service id����ʹ��
    SVC_TMP_TYPE            = 6,

    // GunnerSvr
    SVR_GUNNER_TYPE         = 7,

    // store adapt,�洢����
    SVC_STORE_ADAPT         = 8,

    // UDP��������Type��ʼֵ
    SVC_UDP_SERVER_BEGIN = 30000,

    //����ͨѶ�������������ʾ
    SVC_SPECIAL_BEGIN    = 49000,
    //TCPSVRD,ZERGINGͨѶ������
    SVR_ZERGING_TCPSVRD  = 49001,
    //OG4A,ͨ�õ�ͨѶ������ģ��
    SVR_OGRE4A_COMM_SVRD = 49002,

};


//�ͻ�������ķ������б�
struct CLIENT_NEED_SVC_TYPE
{
    //����������
    unsigned short    svc_type_;
    //�Ƿ����
    bool              if_must_need_;
};

//���ʵ�DB����
enum QUERY_DATABASE_TYPE
{
    DB_TYPE_MYSQL,
    DB_TYPE_SQLITE,
};

//��Ч���±�ֵ
static const unsigned int   PTR_INVALID = 0xFFFFFFFF;

//������־�ĸ���
static const unsigned int  RESERVE_LOGFILE_NUM = 60;

//CONST ��ֵ����

// ��ҳ��ת����
static const size_t   LEN_OF_WEB_SIGNATURE_KEY = 32;

//Ĭ�Ͽͻ��˰汾
static const short    DEFAULT_QQPET2_CLIENT_VER = 6868;

// ����ǩ���ĳ���
static const size_t   LEN_OF_SIGNATURE_KEY_56 = 56;
static const size_t   LEN_OF_SIGNATURE_KEY_64 = 64;

static const size_t   LEN_OF_UIN_SESSION = 16;


//
static const size_t   MAX_SHORTNAME_STRING = 16;
//���Ƶ���󳤶�,ע�ⲻ��BUFFER����
static const size_t   MAX_NAME_LEN_STRING = 32;
//�����Ƶ���󳤶�,ע�ⲻ��BUFFER����
static const size_t   MAX_LONGNAME_LEN_STRING = 64;
//ע����Ϣ����󳤶�,ע�ⲻ��BUFFER����
static const size_t   MAX_COMMENT_LEN_STRING = 128;

//����TXT����
static const size_t   MAX_SHORT_TEXT_STRING = 256;
//���TXT����
static const size_t   MAX_LONG_TEXT_STRING = 512;

//�ı��ĳ���
static const size_t   MAX_LONG_LONG_TEXT_STRING = 1024;
//������ı�����
static const size_t   MAX_HUGE_TEXT_STRING = 2048;
//�������ı�����
static const size_t   MAX_TOO_HUGE_TEXT_STRING = 4096;
//����������ı����ȡ��������ǲ��ں�.��ΪҪ�ڶ���ʵ��
static const size_t   MAX_DO_NOT_CARE_TEXT_STRING = 12 * 1024;

//��������ַ�����󳤶�
static const size_t   MARRIAGE_VOW_MAX_LEN = 32;
//��������ַ�����������󳤶�
static const size_t   MARRIAGE_VOW_BUFFER_LEN = MARRIAGE_VOW_MAX_LEN + 1;

//md5У��ֵ����������
static const size_t   LEN_OF_MD5_CHECK_SUM_BUFFER = 64;

//����ְҵ���Ƶ���󳤶�
static const size_t   WORK_TITLE_STR_MAX_LEN = 16;
//ѧϰ�γ����Ƶ���󳤶�
static const size_t   TRAIN_SUBJECT_STR_MAX_LEN = 16;
//ְҵ������󳤶�
static const size_t   CAREER_TITLE_STR_MAX_LEN = 16;

//ʱЧ����+400���� ��ʹ�õ��ߺͼ����׼��ʱ��Ҫ�ӣ�����д������
static const unsigned short ADD_400_MENTEL_BENCH_MARK = 400;

//�������е��õ�epsilon����
static const double  EPSILON = 1E-6;

//���ֹ���ʱ��Ķ���
static const unsigned int   ONE_WEEK_DAYS = 7;

//һ���ӵ�����
static const unsigned int   ONE_MINUTE_SECOND = 60;
//һСʱ������
static const unsigned int   ONE_HOUR_SECOND = 3600;
//һ�������86400
static const unsigned int   ONE_DAY_SECOND = ONE_HOUR_SECOND * 24;
//
//һ�ܵ�����
static const unsigned int   ONE_WEEK_SECONDS = ONE_DAY_SECOND * ONE_WEEK_DAYS;


//����2����
//ȡ��1970�����ڵ�ǰ������
#define DATE_NOW(x)  (( (x) + ONE_HOUR_SECOND * 8)/ONE_DAY_SECOND)

//��ǰ���ڵ�����
#define SECOND_NOW(x)  (( (x) + ONE_HOUR_SECOND * 8)%ONE_DAY_SECOND)

//��ǰ���ڵĵ���
#define HOUR_NOW(x) (SECOND_NOW(x) / ONE_HOUR_SECOND)

//��ǰʱ���8Сʱ
#define TIMESTAMP_NOW(X) ( (X) + (ONE_HOUR_SECOND * 8) )

//��ǰ�ܵ����� ����ΪɶҪ+3����Ϊ19700101������ġ�����ע������ɣ�����һ������
#define  WEEKDAY_NOW(x)  (( TIMESTAMP_NOW(x) + ONE_DAY_SECOND * 3)%ONE_WEEK_SECONDS)

#endif //_SOARING_LIB_PER_DEFINE_BASE_H_
