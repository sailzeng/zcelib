#include "ogre/predefine.h"
#include "ogre/application.h"
#include "ogre/app_timer.h"
#include "ogre/configure.h"

int main(int argc, const char* argv[])
{
    //利用模板开始启动
    return soar::svrd_main < Ogre_Service_App,
        Ogre_Server_Config,
        Ogre_App_Timer_Handler >(argc, argv);
}