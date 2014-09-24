

#include "ogre_predefine.h"
#include "ogre_application.h"

int main(int argc, const char *argv[])
{
    CfgSvrSdk::instance()->add_config("zergsvrd.xml");
    CfgSvrSdk::instance()->add_config("framework.xml");
    // 设置服务名
    Ogre4a_Service_App::instance()->set_service_info("Tencent Zerg Svr", "腾讯游戏安全引擎通讯处理服务");
    return Ogre4a_Service_App::instance()->run(argc, argv);
}

