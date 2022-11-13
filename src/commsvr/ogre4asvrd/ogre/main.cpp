#include "ogre/predefine.h"
#include "ogre/application.h"
#include "ogre/app_timer.h"
#include "ogre/configure.h"

int main(int argc, const char* argv[])
{
    //利用模板开始启动
    return soar::svrd_main < ogre::application,
        ogre::configure,
        ogre::app_timer >(argc, argv);
}