#include "wormhole_predefine.h"
#include "wormhole_application.h"
#include "wormhole_configture.h"

int main(int argc, const char* argv[])
{
    //利用模板开始启动
    return soar::soar_svrd_main < Wormhole_Proxy_App,
        Wormhole_Server_Config,
        soar::Server_Timer >(argc, argv);
}