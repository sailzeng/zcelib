#include "zce_predefine.h"
#include "zce_inotify_event_base.h"

ZCE_INotify_Event_Base::ZCE_INotify_Event_Base():
    inotify_reactor_(NULL)
{
}

ZCE_INotify_Event_Base::~ZCE_INotify_Event_Base()
{
}

//设置反应器
void ZCE_INotify_Event_Base::set_reactor(ZCE_INotify_Dir_Reactor *reactor)
{
    inotify_reactor_ = reactor;
}
//得到反应器
ZCE_INotify_Dir_Reactor *ZCE_INotify_Event_Base::get_reactor()
{
    return inotify_reactor_;
}

int ZCE_INotify_Event_Base::watch_close()
{
    return 0;
}

