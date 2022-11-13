#include "predefine.h"
#include "application.h"
#include "configture.h"

int main(int argc, const char* argv[])
{
    //利用模板开始启动
    return soar::svrd_main < wormhole::application,
        wormhole::configure,
        soar::server_timer >(argc, argv);
}