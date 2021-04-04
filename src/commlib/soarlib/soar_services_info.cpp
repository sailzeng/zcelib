#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_services_info.h"

/****************************************************************************************************
class SERVICES_ID ������Ϣ,
****************************************************************************************************/
//���캯������������
SERVICES_ID::SERVICES_ID(unsigned short svrtype, unsigned int svrid):
    services_type_(svrtype),
    services_id_(svrid)
{
}

SERVICES_ID::~SERVICES_ID()
{
}

//
void SERVICES_ID::set_svcid(unsigned short svrtype, unsigned int svrid )
{
    services_type_ = svrtype;
    services_id_ = svrid;
}

//�Ƚ��Ƿ���ͬ�ĺ���
bool SERVICES_ID::operator ==(const SERVICES_ID &others) const
{
    if (services_type_ == others.services_type_ &&   services_id_ == others.services_id_ )
    {
        return true;
    }

    return false;
}

//�Ƚ��Ƿ�ͬ�ĺ���
bool SERVICES_ID::operator !=(const SERVICES_ID &others) const
{
    if (services_type_ != others.services_type_ ||   services_id_ != others.services_id_ )
    {
        return true;
    }

    return false;
}

//�Ƚϴ�С�ĺ���
bool SERVICES_ID::operator <(const SERVICES_ID &others) const
{
    if (services_type_ + services_id_ < others.services_type_ + others.services_id_ )
    {
        return true;
    }

    return false;
}


///ת��string
const char *SERVICES_ID::to_str(char *str_buffer, size_t buf_len)
{
    snprintf(str_buffer,
             buf_len,
             "%hu.%u",
             services_type_,
             services_id_);
    return str_buffer;
}

int SERVICES_ID::from_str(const char *str_buffer, bool check_valid)
{
    //ע��.ǰ���%��
    int ret_num = sscanf(str_buffer,
                         "%hu.%u",
                         &services_type_,
                         &services_id_);
    if (check_valid
        && (INVALID_SERVICES_TYPE == services_type_ || INVALID_SERVICES_ID == services_id_))
    {
        return SOAR_RET::ERROR_SERVICES_ID_INVALID;
    }
    if (ret_num != 2)
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }
    else
    {
        return 0;
    }
}


/******************************************************************************************
struct SERVICES_IP_INFO ����ID��Ϣ + IP��Ϣ
******************************************************************************************/

///���ַ����л�ȡ
int SERVICES_INFO::from_str(const char *svc_info_str,
                            bool check_valid)
{
    const size_t SVC_INFO_STR_LEN = 512;
    if (strlen(svc_info_str) > SVC_INFO_STR_LEN - 1)
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }

    //ȥ���������еĿո񣬱�����Ⱦsscanf, ��ʽ���ַ���Ϊ"%hu.%u|%u.%u.%u.%u#%hu|%u|%u",
    //char pure_str[SVC_INFO_STR_LEN];
    //zce::str_replace(svc_info_str, pure_str," ","");

    uint32_t u[4] = {0};
    uint16_t port = 0;
    //���Է�����ʵ����Ҫ�ֶ�ȥ������ո�ĸ��ţ��������ַ�ǰ��Ҳ����%���ƾͿ����ˡ�
    int ret_num = sscanf(svc_info_str,
                         "%hu.%u | %u.%u.%u.%u # %hu | %u | %u",
                         &svc_id_.services_type_,
                         &svc_id_.services_id_,
                         &u[0], &u[1], &u[2], &u[3],
                         &port,
                         &idc_no_,
                         &business_id_ );
    //����9��ʾ�������ݶ���ȡ��
    if (ret_num != 9 || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF )
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }
    if (check_valid &&
        (SERVICES_ID::INVALID_SERVICES_TYPE == svc_id_.services_type_
         || SERVICES_ID::INVALID_SERVICES_ID == svc_id_.services_id_))
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }

    uint32_t u32_addr = u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3];
    ip_address_.set(u32_addr, port);
    return 0;
}

///ת��string
const char *SERVICES_INFO::to_str(char *str_buffer, size_t buf_len)
{
    char str_svc_id[32], str_inet_add[32];
    size_t use_buf = 0;
    snprintf(str_buffer,
             buf_len,
             "%16s|%24s|%6u|%6u",
             svc_id_.to_str(str_svc_id, sizeof(str_svc_id) - 1),
             ip_address_.to_string(str_inet_add, sizeof(str_inet_add) - 1,use_buf),
             idc_no_,
             business_id_
            );
    return str_buffer;
}
