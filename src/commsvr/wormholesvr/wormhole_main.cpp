/******************************************************************************************
Copyright           : 2000-2004, FXL Technology (Shenzhen) Company Limited.
FileName            : arbiter_main.cpp
Author              : Sail(ZENGXING)//Author name here
Version             :
Date Of Creation    : 2005Äê11ÔÂ21ÈÕ
Description         :

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
******************************************************************************************/

#include "arbiter_predefine.h"
#include "arbiter_application.h"

int main(int argc, const char *argv[])
{
    CfgSvrSdk::instance()->add_config("proxysvr_config.xml");
    CfgSvrSdk::instance()->set_service_type(SVC_PROXY_SERVER);

    return ArbiterAppliction::instance()->run(argc, argv);
}

