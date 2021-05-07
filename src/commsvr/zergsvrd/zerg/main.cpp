#include "zerg/predefine.h"
#include "zerg/application.h"
#include "zerg/configure.h"
#include "zerg/app_timer.h"

int main(int argc,const char *argv[])
{
    //利用模板开始启动
    return soar::svrd_main < Zerg_Service_App,
        Zerg_Server_Config,
        Zerg_App_Timer >(argc,argv);
}