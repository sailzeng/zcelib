#include "zealot_predefine.h"
#include "zealot_test_function.h"

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

class My_INotify_Event: public zce::Event_INotify
{
public:
    My_INotify_Event()
    {
    }
    ~My_INotify_Event()
    {
    }
protected:

    virtual int inotify_create(ZCE_HANDLE watch_handle,
                               uint32_t watch_mask,
                               const char* watch_path,
                               const char* active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event add .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" << "watch mask" << watch_mask << std::endl;

        return 0;
    }

    ///监测到有删除文件或者目录,对应掩码IN_DELETE，参数说明参考@fun inotify_create
    virtual int inotify_delete(ZCE_HANDLE watch_handle,
                               uint32_t watch_mask,
                               const char* watch_path,
                               const char* active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event delete .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" << "watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监测到有文件被修改,对应掩码IN_MODIFY，参数说明参考@fun inotify_create
    virtual int inotify_modify(ZCE_HANDLE watch_handle,
                               uint32_t watch_mask,
                               const char* watch_path,
                               const char* active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event modify .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" << "watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监控文件从某个目录移动出去，IN_MOVED_FROM,参数说明参考@fun inotify_create
    virtual int inotify_moved_from(ZCE_HANDLE watch_handle,
                                   uint32_t watch_mask,
                                   const char* watch_path,
                                   const char* active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event moved from .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" << "watch mask" << watch_mask << std::endl;
        return 0;
    }

    ///监控文件移动到某个目录，IN_MOVED_TO,(我自己测试只有在监控目录下移动才会发生这个事件),
    ///参数说明参考@fun inotify_create
    virtual int inotify_moved_to(ZCE_HANDLE watch_handle,
                                 uint32_t watch_mask,
                                 const char* watch_path,
                                 const char* active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        std::cout << "Event moved to .watch path[" << watch_path << "]ative path[" <<
            active_path << "]" << "watch mask" << watch_mask << std::endl;
        return 0;
    }
};

//独立的Event reactor，
int test_inotify_reactor(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    My_INotify_Event* inotify_event = new My_INotify_Event();

#if defined ZCE_OS_WINDOWS
    zce::WFMO_Reactor* reactor_ptr = new zce::WFMO_Reactor();
    ret = reactor_ptr->initialize();
#else
    zce::Select_Reactor* reactor_ptr = new zce::Select_Reactor();
    ret = reactor_ptr->initialize(1024);

#endif

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "reactor initialize fial? ret =%d", ret);
        delete reactor_ptr;
        return ret;
    }
    zce::ZCE_Reactor::instance(reactor_ptr);

    ret = inotify_event->open(zce::ZCE_Reactor::instance());
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "Open fial? ret =%d", ret);
        return ret;
    }

    ZCE_HANDLE watch_handle = ZCE_INVALID_HANDLE;

    ret = inotify_event->add_watch(TEST_PATH_1,
                                   IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
                                   &watch_handle);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "inotify_event add_watch fail.dir[%s]? ret =%d", TEST_PATH_1, ret);
        return ret;
    }

    //Linux下，一个event handle可以监控多个目录
    ret = inotify_event->add_watch(TEST_PATH_2,
                                   IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
                                   &watch_handle);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "inotify_event add_watch fail.dir[%s]? ret =%d,This is ok in windows.", TEST_PATH_2, ret);
        //Windows下，是比如俺出错的。Windows下一个event handle，只能监控一个目录
#if !defined ZCE_OS_WINDOWS
        return ret;
#endif
    }

#if defined ZCE_OS_WINDOWS
    //Windows下，可以用一个新的event handle监控目录，
    //当然Windows有一个功能，使用监控子目录的功能
    My_INotify_Event* inotify_event2 = new My_INotify_Event();
    ret = inotify_event2->open(zce::ZCE_Reactor::instance());
    ret = inotify_event2->add_watch(TEST_PATH_2,
                                    IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO,
                                    &watch_handle);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "inotify_event2 add_watch fail.dir[%s]? ret =%d", ret, TEST_PATH_2);
        return ret;
    }
#endif

    for (;;)
    {
        zce::Time_Value time_out(60, 0);
        size_t num_event;
        ret = zce::ZCE_Reactor::instance()->handle_events(&time_out, &num_event);
        //ZCE_LOG(RS_INFO,"handle_events? ret =[%d] number of event[%u]",ret,num_event);
        std::cout << "handle_events ret =" << ret << " number of event=" << num_event << std::endl;
    }

    return 0;
}

#if defined ZCE_OS_WINDOWS
#pragma warning ( pop )
#endif