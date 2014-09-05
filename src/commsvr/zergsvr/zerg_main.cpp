

#include "zerg_predefine.h"
#include "zerg_application.h"


int main(int argc, const char *argv[])
{
    return SOAR_LIB::soar_svrd_main<Zerg_Service_App,
        Zerg_Server_Config,
        Zerg_App_Timer_Handler>(argc, argv);
}

