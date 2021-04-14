#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_services_info.h"
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


//设置相应的SVC INFO,
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

    ret = lolo_svc_ip_.set(svc_ip, svc_port);
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

