

#include "zerg_predefine.h"
#include "zerg_application.h"
#include "zerg_configure.h"
#include "zerg_app_timer.h"


int main(int argc, const char *argv[])
{
    //利用模板开始启动  
    return SOAR_LIB::soar_svrd_main < Zerg_Service_App,
           Zerg_Server_Config,
           Zerg_App_Timer_Handler > (argc, argv);
}

