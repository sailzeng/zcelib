

#include "ogre_predefine.h"
#include "ogre_application.h"
#include "ogre_app_timer.h"
#include "ogre_configure.h"



int main(int argc, const char *argv[])
{
    //利用模板开始启动
    return SOAR_LIB::soar_svrd_main < Ogre_Service_App,
           Ogre_Server_Config,
           Ogre_App_Timer_Handler > (argc, argv);
}