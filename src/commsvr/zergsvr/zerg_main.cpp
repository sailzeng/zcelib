/******************************************************************************************
Copyright           : 2000-2004, Tencent Technology (Shenzhen) Company Limited.
FileName            : zerg_main.cpp
Author              : Sail(ZENGXING)//Author name here
Version             :
Date Of Creation    : 2005年11月27日
Description         :

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
******************************************************************************************/

#include "zerg_predefine.h"
#include "zerg_application.h"
#include "comm_service_info.h"
#include "comm_enum_define.h"

int main(int argc, const char *argv[])
{
    // 设置服务名
    Zerg_Service_App::instance()->set_service_info("Tencent Zerg Svr", "腾讯游戏安全引擎通讯处理服务");
    return Zerg_Service_App::instance()->run(argc, argv);
}

