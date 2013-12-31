


#include "zealot_predefine.h"
#include "zealot_test_function.h"


class My_INotify_Event : public ZCE_INotify_Event_Base
{
public:
    My_INotify_Event()
    {

    }
    ~My_INotify_Event()
    {

    }
protected:
    virtual int watch_close()
    {
        return 0;
    }

    virtual int inotify_create(ZCE_HANDLE watch_handle,
        uint32_t watch_mask,
        const char *watch_path, 
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        std::cout << "Event add .watch path[" << watch_path << "]ative path[" <<
                  active_path << "]." << std::endl;
        return 0;
    }

    virtual int inotify_delete(ZCE_HANDLE watch_handle,
        uint32_t watch_mask,
        const char *watch_path, 
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        std::cout << "Event delete .watch path[" << watch_path << "]ative path[" <<
                  active_path << "]." << std::endl;
        return 0;
    }

    virtual int inotify_modify(ZCE_HANDLE watch_handle,
        uint32_t watch_mask,
        const char *watch_path, 
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        std::cout << "Event modify .watch path[" << watch_path << "]ative path[" <<
                  active_path << "]." << std::endl;
        return 0;
    }


    virtual int inotify_moved_from(ZCE_HANDLE watch_handle,
        uint32_t watch_mask,
        const char *watch_path, 
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        std::cout << "Event move from .watch path[" << watch_path << "]ative path[" <<
                  active_path << "]." << std::endl;
        return 0;
    }

    virtual int inotify_moved_to(ZCE_HANDLE watch_handle,
        uint32_t watch_mask,
        const char *watch_path, 
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        std::cout << "Event move to .watch path[" << watch_path << "]ative path[" <<
                  active_path << "]." << std::endl;
        return 0;
    }
};


#if defined ZCE_OS_WINDOWS
#define  TEST_PATH_1  "D:\\testdir\\1"
#define  TEST_PATH_2  "D:\\testdir\\1"
#elif defined ZCE_OS_LINUX
#define  TEST_PATH_1  "/data/testdir/1"
#define  TEST_PATH_2  "/data/testdir/2"
#endif


#if defined ZCE_OS_WINDOWS
#pragma warning ( push )
#pragma warning ( disable : 4702)
#endif

//独立的Inotify reactor，
int test_inotify_reactor(int /*argc*/ , char * /*argv*/ [])
{
    My_INotify_Event inotify_event;
    ZCE_INotify_Dir_Reactor *reactor = new ZCE_INotify_Dir_Reactor();
    ZCE_HANDLE watch_handle = ZCE_INVALID_HANDLE;
    reactor->open();
    reactor->add_watch(&inotify_event,
                       TEST_PATH_1,
                       IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
                       &watch_handle);

    reactor->add_watch(&inotify_event,
                       TEST_PATH_2,
                       IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
                       &watch_handle);

    for (;;)
    {
        ZCE_Time_Value time_out(5, 0);
        size_t num_event;
        reactor->watch_event(&time_out, &num_event);
    }

    return 0;

}


#if defined ZCE_OS_LINUX

class My_INotify_EvtHandle : public ZCE_Event_INotify
{
public:
    My_INotify_EvtHandle()
    {

    }
    ~My_INotify_EvtHandle()
    {

    }
protected:

    virtual int inotify_create(int watch_handle,
        uint32_t watch_mask,
        const char *watch_path,
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event add .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" <<"watch mask" << watch_mask << std::endl;

        return 0;
    }

    ///监测到有删除文件或者目录,对应掩码IN_DELETE，参数说明参考@fun inotify_create
    virtual int inotify_delete(int watch_handle,
        uint32_t watch_mask,
        const char *watch_path,
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event delete .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" <<"watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监测到有文件被修改,对应掩码IN_MODIFY，参数说明参考@fun inotify_create
    virtual int inotify_modify(int watch_handle,
        uint32_t watch_mask,
        const char *watch_path,
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event modify .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" <<"watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监控文件从某个目录移动出去，IN_MOVED_FROM,参数说明参考@fun inotify_create
    virtual int inotify_moved_from(int watch_handle,
        uint32_t watch_mask,
        const char *watch_path,
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event moved from .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" <<"watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监控文件移动到某个目录，IN_MOVED_TO,(我自己测试只有在监控目录下移动才会发生这个事件),
    ///参数说明参考@fun inotify_create
    virtual int inotify_moved_to(int watch_handle,
        uint32_t watch_mask,
        const char *watch_path,
        const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event moved to .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" <<"watch mask" << watch_mask << std::endl;
        return 0;
    }
};




//独立的Event reactor，
int test_inotify_eventreactor(int /*argc*/ , char * /*argv*/ [])
{
    int ret = 0;
    My_INotify_EvtHandle *inotify_event = new My_INotify_EvtHandle();
    ZCE_Select_Reactor *reactor = new ZCE_Select_Reactor();
    ret = reactor->initialize(1024);
    if (ret != 0)
    {
        ZLOG_ERROR("reactor reactor fial? ret =%d",ret);
        return ret;
    }

    ret = inotify_event->open(reactor);
    if (ret != 0)
    {
        ZLOG_ERROR("Open fial? ret =%d",ret);
        return ret;
    }

    ZCE_HANDLE watch_handle = ZCE_INVALID_HANDLE;
    

    inotify_event->add_watch(TEST_PATH_1,
        IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
        &watch_handle);

    inotify_event->add_watch(TEST_PATH_2,
        IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
        &watch_handle);

    for (;;)
    {
        ZCE_Time_Value time_out(5, 0);
        size_t num_event;
        ret = reactor->handle_events(&time_out, &num_event);
        ZLOG_INFO("handle_events? ret =[%d] number of event[%u]",ret,num_event);
        std::cout<<"handle_events ret ="<<ret << " number of event="<<num_event<<std::endl;
    }
    
    return 0;

}



#if defined ZCE_OS_WINDOWS
#pragma warning ( pop )
#endif

#endif



