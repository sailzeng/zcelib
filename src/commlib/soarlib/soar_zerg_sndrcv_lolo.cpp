#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_svc_info.h"
#include "soar_zerg_frame.h"
#include "soar_zerg_sndrcv_lolo.h"

Lolo_SendRecv_Package::Lolo_SendRecv_Package()
{
    lolo_datagram_.open();
}

Lolo_SendRecv_Package::~Lolo_SendRecv_Package()
{
    lolo_datagram_.close();
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年4月25日
Function        : Zulu_SendRecv_Package::set_zulu_svcinfo
Return          : int
Parameter List  :
  Param1: const char* svc_ip
  Param2: unsigned short svc_port
  Param3: const SERVICES_ID& recv_service
  Param4: const SERVICES_ID& send_service
  Param5: const SERVICES_ID& proxy_service
  Param6: size_t frame_len
Description     : //设置相应的SVC INFO,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Lolo_SendRecv_Package::set_lolo_svcinfo(const char *svc_ip,
                                            unsigned short svc_port,
                                            const SERVICES_ID &recv_service,
                                            const SERVICES_ID &send_service,
                                            const SERVICES_ID &proxy_service,
                                            size_t frame_len,
                                            bool is_check_conn_info)
{
    int ret = 0;

    set_services_id(recv_service, send_service, proxy_service, frame_len);

    lolo_svc_ip_.set(svc_ip, svc_port);

    if ( ret != 0)
    {
        return SOAR_RET::ERROR_ZULU_INPUT_IPADDRESS_ERROR;
    }

    if ( is_check_conn_info )
    {
        lolo_datagram_.connect(&lolo_svc_ip_);
    }

    return 0;
}

