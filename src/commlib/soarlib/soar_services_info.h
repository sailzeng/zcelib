#ifndef SOARING_LIB_SERVICES_INFO_H_
#define SOARING_LIB_SERVICES_INFO_H_

#include "soar_enum_define.h"

#pragma pack (1)
/****************************************************************************************************
class  SERVICES_ID ����������Ϣ,
****************************************************************************************************/
struct SOARING_EXPORT SERVICES_ID
{

public:

    ///����,��������,Ĭ��Ϊ0
    explicit SERVICES_ID(uint16_t svrtype = INVALID_SERVICES_TYPE,
                         uint32_t svrno = INVALID_SERVICES_ID);
    //
    ~SERVICES_ID();

    void  clear()
    {
        services_type_ = 0;
        services_id_ = 0;
    }


    /*!
    * @brief      ��string�еõ�SVC ID
    * @return     int == 0 ��ʾ�ɹ�
    * @param      str ת�����ַ���
    * @param      check_valid ����Ƿ���Ч,svcid��type��id ������0
    */
    int from_str(const char *str,
                 bool check_valid = false);

    ///ת��string
    const char *to_str(char *str_buffer, size_t buf_len);

    ///����
    void set_svcid(uint16_t svrtype, uint32_t svrid);

    ///�ȽϺ���,services_type_,services_id_
    bool operator ==(const SERVICES_ID &others) const;
    ///�Ƚϲ�ͬ����,
    bool operator !=(const SERVICES_ID &others) const;
    ///�и���������,
    bool operator <(const SERVICES_ID &others) const;

public:

    ///��Ч��SERVICE ����
    static const uint16_t INVALID_SERVICES_TYPE = 0;

    ///��Ч��SERVICE ID
    static const uint32_t INVALID_SERVICES_ID   = 0;

    ///��ZERG��æ��̬����һ��SERVICES_ID ������ĳЩ�ͻ�������
    static const uint32_t DYNAMIC_ALLOC_SERVICES_ID = static_cast<uint32_t>(-1);

    ///��ĳ��type���й㲥��
    static const uint32_t BROADCAST_SERVICES_ID = static_cast<uint32_t>(-2);

    ///ʹ�þ��⸺�صķ�ʽ��̬��Ƭ��SERVICE ID
    static const uint32_t LOAD_BALANCE_DYNAMIC_ID = static_cast<uint32_t>(-3);

    ///ʹ�����ӵķ�ʽ������AUTO CONNECT��˳�򣩶�̬�����SERVICE ID
    static const uint32_t MAIN_STANDBY_DYNAMIC_ID = static_cast<uint32_t>(-4);


public:

    ///���������,�ο�ö��ֵSERVICES_TYPE����Чʱ��INVALID_SERVICES_TYPE
    uint16_t           services_type_ = INVALID_SERVICES_TYPE;

    ///����ı��,�������ֱ��ַ�ʽ,�����ſ�����һ������������ֵ,Ҳ������һ��UIN.
    uint32_t           services_id_ = INVALID_SERVICES_ID;


};

#pragma pack ()

//HASH����,���ڵõ�HASH Key

struct SOARING_EXPORT HASH_OF_SVCID
{
public:
    size_t operator()(const SERVICES_ID &svrinfo) const
    {
        return (size_t (svrinfo.services_type_) << 16) + svrinfo.services_id_ ;
    }
};

/******************************************************************************************
struct SERVICES_IP_INFO ����ID��Ϣ + IP��Ϣ
******************************************************************************************/
struct SOARING_EXPORT SERVICES_INFO
{
public:

    ///���ַ����л�ȡ
    int from_str(const char *str,
                 bool check_valid = false);

    ///ת��string
    const char *to_str(char *str_buffer, size_t buf_len);

public:

    ///����ID��Ϣ
    SERVICES_ID      svc_id_ = SERVICES_ID(0, 0);


    ///����IP,����˿�
    ZCE_Sockaddr_In  ip_address_;

    ///ҵ��ID
    unsigned int     business_id_ = 0;

    ///���������� IDC�ı��
    unsigned int     idc_no_ = 0;
};

//�õ�KEY��HASH����
struct SOARING_EXPORT HASH_OF_SVCINFO
{
public:
    size_t operator()(const SERVICES_INFO &svripinfo) const
    {
        return (size_t (svripinfo.svc_id_.services_type_) << 16) + svripinfo.svc_id_.services_id_ ;
    }
};

struct SOARING_EXPORT EQUAL_OF_SVCINFO
{
public:
    //ע���ж������������еı���
    bool operator()(const SERVICES_INFO &right, const SERVICES_INFO &left) const
    {
        //���SVC INFO�����,����Ϊ���
        if (right.svc_id_ == left.svc_id_ )
        {
            return true;
        }

        return false;
    }
};

#endif //#ifndef SOARING_LIB_SERVICES_INFO_H_

