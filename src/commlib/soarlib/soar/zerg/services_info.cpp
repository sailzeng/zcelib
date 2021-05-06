#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/services_info.h"

namespace soar
{
/******************************************************************************************
struct SERVICES_IP_INFO 服务ID信息 + IP信息
******************************************************************************************/

///从字符串中获取
int SERVICES_INFO::from_str(const char *svc_info_str,
                            bool check_valid)
{
    const size_t SVC_INFO_STR_LEN = 512;
    if (strlen(svc_info_str) > SVC_INFO_STR_LEN - 1)
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }

    //去掉里面所有的空格，避免污染sscanf, 格式化字符串为"%hu.%u|%u.%u.%u.%u#%hu|%u|%u",
    //char pure_str[SVC_INFO_STR_LEN];
    //zce::str_replace(svc_info_str, pure_str," ","");

    uint32_t u[4] = {0};
    uint16_t port = 0;
    //测试发现其实不需要手动去掉多余空格的干扰，把特殊字符前面也增加%控制就可以了。
    int ret_num = sscanf(svc_info_str,
                         "%hu.%u | %u.%u.%u.%u # %hu | %hu | %hu",
                         &svc_id_.services_type_,
                         &svc_id_.services_id_,
                         &u[0],&u[1],&u[2],&u[3],
                         &port,
                         &idc_no_,
                         &business_id_);
    //返回9表示所有数据都读取了
    if (ret_num != 9 || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF)
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }
    if (check_valid &&
        (soar::SERVICES_ID::INVALID_SERVICES_TYPE == svc_id_.services_type_
        || soar::SERVICES_ID::INVALID_SERVICES_ID == svc_id_.services_id_))
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }

    uint32_t u32_addr = u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3];
    ip_address_.set(u32_addr,port);
    return 0;
}

///转换string
const char *SERVICES_INFO::to_str(char *str_buffer,size_t buf_len)
{
    char str_svc_id[32],str_inet_add[32];
    size_t use_buf = 0;
    snprintf(str_buffer,
             buf_len,
             "%16s|%24s|%6hu|%6hu",
             svc_id_.to_str(str_svc_id,sizeof(str_svc_id) - 1),
             ip_address_.to_string(str_inet_add,sizeof(str_inet_add) - 1,use_buf),
             idc_no_,
             business_id_
    );
    return str_buffer;
}
} //namespace soar