#include "zerg/predefine.h"
#include "zerg/application.h"
#include "zerg/configure.h"
#include "zerg/app_timer.h"

int main(int argc, const char* argv[])
{
    //利用模板开始启动
    return soar::svrd_main < zerg::zerg_app,
        zerg::zerg_config,
        zerg::app_timer >(argc, argv);
}