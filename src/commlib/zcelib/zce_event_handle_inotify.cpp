#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_error.h"
#include "zce_event_handle_inotify.h"
#include "zce_trace_debugging.h"
#include "zce_event_reactor_base.h"



//构造函数和析构函数
ZCE_Event_INotify::ZCE_Event_INotify():
    ZCE_Event_Handler(),

    read_buffer_(NULL)
{
    read_buffer_ = new char [READ_BUFFER_LEN + 1];
    read_buffer_[READ_BUFFER_LEN] = '\0';
#if defined ZCE_OS_LINUX
    inotify_handle_ = ZCE_INVALID_HANDLE;

#elif defined ZCE_OS_WINDOWS
    watch_handle_ = ZCE_INVALID_HANDLE;
    watch_path_[0] = '\0';
    watch_mask_ = 0;
    watch_sub_dir_ = FALSE;
    memset((void *)&over_lapped_, 0, sizeof(OVERLAPPED));
#endif
}

ZCE_Event_INotify::~ZCE_Event_INotify()
{
    if (read_buffer_)
    {
        delete []read_buffer_;
        read_buffer_ = NULL;
    }
}

//打开监控句柄等，绑定reactor等
int ZCE_Event_INotify::open(ZCE_Reactor *reactor_base)
{

#if defined (ZCE_OS_LINUX)
    int ret = 0;
    //如果已经初始化过了
    if (ZCE_INVALID_HANDLE != inotify_handle_)
    {
        return -1;
    }
    //在LINUX下使用INOTIFY的机制
    inotify_handle_ = ::inotify_init();
    if (ZCE_INVALID_HANDLE == inotify_handle_ )
    {
        ZCE_LOG(RS_ERROR, "[%s] invoke ::inotify_init fail,error [%u].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error());
        return -1;
    }

    ret = reactor_base->register_handler(this, INOTIFY_MASK);
    if (ret != 0)
    {
        ::close(inotify_handle_);
        return ret;
    }

#endif //#if defined (ZCE_OS_LINUX)

    reactor(reactor_base);
    return 0;
}


//关闭监控句柄等，解除绑定reactor等
int ZCE_Event_INotify::close()
{

#if defined (ZCE_OS_LINUX)

    //由于是HASH MAP速度有点慢
    HDL_TO_EIN_MAP::iterator iter_temp =  watch_event_map_.begin();
    for (; iter_temp != watch_event_map_.end();)
    {
        rm_watch(iter_temp->second.watch_handle_);
        //让迭代器继续从最开始干起
        iter_temp = watch_event_map_.begin();
    }


    //关闭监控句柄
    if (inotify_handle_ != ZCE_INVALID_HANDLE)
    {
        //从反应器移除
        reactor()->remove_handler(this, false);

        ::close(inotify_handle_);
        inotify_handle_ = ZCE_INVALID_HANDLE;
    }

#elif defined (ZCE_OS_WINDOWS)

    rm_watch(watch_handle_);

#endif

    return 0;
}


//添加监控
int ZCE_Event_INotify::add_watch(const char *pathname,
                                 uint32_t mask,
                                 ZCE_HANDLE *watch_handle,
                                 bool watch_sub_dir)
{

    //检查参数是否有效，
    ZCE_ASSERT( pathname &&  mask);
    if ( pathname == NULL || mask == 0)
    {
        errno = EINVAL;
        return -1;
    }

#if defined (ZCE_OS_LINUX)

    //Linux的Inotify没法监控子目录
    ZCE_ASSERT(watch_sub_dir == false);

    *watch_handle = ZCE_INVALID_HANDLE;
    EVENT_INOTIFY_NODE watch_note;
    ZCE_HANDLE hdl_dir = ZCE_INVALID_HANDLE;

    hdl_dir =  ::inotify_add_watch(inotify_handle_, pathname, mask);
    if (hdl_dir == ZCE_INVALID_HANDLE )
    {
        ZCE_LOG(RS_ERROR, "[%s] invoke ::inotify_add_watch fail,error [%u].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error());
        return -1;
    }

    watch_note.watch_handle_ = hdl_dir;
    watch_note.watch_mask_ = mask;
    strncpy(watch_note.watch_path_, pathname, MAX_PATH);

    std::pair<HDL_TO_EIN_MAP::iterator, bool>
    ins_ret = watch_event_map_.insert(HDL_TO_EIN_MAP::value_type(hdl_dir, watch_note));

    //如果插入不成功，进行各种难过清理工作
    if (ins_ret.second == false)
    {

        //下面这段代码屏蔽的原因是，而LInux下，如果inotify_add_watch 同一个目录，handle是一样的。
        //::inotify_rm_watch(inotify_handle_, hdl_dir);

        ZCE_LOG(RS_ERROR, "[%s] insert code node to map fail. code error or map already haved one equal HANDLE[%u].",
                __ZCE_FUNC__,
                hdl_dir);
        return -1;
    }
    *watch_handle = hdl_dir;
    return 0;


#elif defined (ZCE_OS_WINDOWS)

    int ret = 0;
    *watch_handle = ZCE_INVALID_HANDLE;

    //已经监控了一个目录，Windows的一个ZCE_Event_INotify不能同时监控两个目录
    if (watch_handle_ != ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s]add_watch fail handle[%lu]. Windows one ZCE_Event_INotify only watch one dir.",
                __ZCE_FUNC__,
                watch_handle_);
        return -1;
    }

    //采用OVERLAPPED的方式打开文件，注意FILE_LIST_DIRECTORY和FILE_FLAG_OVERLAPPED
    watch_handle_ = ::CreateFileA(pathname, // pointer to the file name
                                  FILE_LIST_DIRECTORY,                // access (read/write) mode
                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
                                  NULL,                               // security descriptor
                                  OPEN_EXISTING,                      // how to create
                                  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,       // file attributes
                                  NULL                                // file with attributes to copy
                                 );

    if (watch_handle_ == ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s] invoke ::CreateFile [%s] inotify fail,error [%u].",
                __ZCE_FUNC__,
                pathname,
                ZCE_LIB::last_error());
        return -1;
    }

    read_buffer_ = new char[READ_BUFFER_LEN + 1];
    read_buffer_[READ_BUFFER_LEN] = '\0';
    watch_sub_dir_ = watch_sub_dir;

    watch_mask_ = mask;
    strncpy(watch_path_, pathname, MAX_PATH);


    DWORD bytes_returned = 0;
    BOOL bret = ::ReadDirectoryChangesW(
                    watch_handle_,                                  // handle to directory
                    read_buffer_,                                    // read results buffer
                    READ_BUFFER_LEN,                               // length of buffer
                    watch_sub_dir_,                                 // monitoring option
                    FILE_NOTIFY_CHANGE_SECURITY |
                    FILE_NOTIFY_CHANGE_CREATION |
                    FILE_NOTIFY_CHANGE_LAST_ACCESS |
                    FILE_NOTIFY_CHANGE_LAST_WRITE |
                    FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_ATTRIBUTES |
                    FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_FILE_NAME,          // filter conditions
                    &bytes_returned,                       // bytes returned
                    &over_lapped_,   // overlapped buffer
                    NULL // completion routine
                );

    //如果读取失败，一般而言，这是这段代码有问题
    if (bret == FALSE)
    {
        ZCE_LOG(RS_ERROR, "[%s] ::ReadDirectoryChangesW fail,error [%u|%s].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()));

        ::CloseHandle(watch_handle_);

        return -1;
    }
    ret = reactor()->register_handler(this, INOTIFY_MASK);
    if (ret != 0)
    {
        ::CloseHandle(watch_handle_);
        return ret;
    }

    *watch_handle = watch_handle_;
    return 0;

#endif
}


int ZCE_Event_INotify::rm_watch(ZCE_HANDLE watch_handle)
{
#if defined (ZCE_OS_LINUX)
    //先用句柄查询
    HDL_TO_EIN_MAP::iterator iter_del = watch_event_map_.find(watch_handle);
    if (iter_del == watch_event_map_.end())
    {
        return -1;
    }

    int ret =  ::inotify_rm_watch(inotify_handle_, iter_del->second.watch_handle_);
    if (ret != 0)
    {
        return -1;
    }

    //从MAP中删除这个NODe
    watch_event_map_.erase(iter_del);
    return 0;
#elif defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(watch_handle);
    if (watch_handle_ != ZCE_INVALID_HANDLE)
    {
        //从反应器移除
        reactor()->remove_handler(this, false);

        ::CloseHandle(watch_handle_);
        watch_handle_ = ZCE_INVALID_HANDLE;
        watch_path_[0] = '\0';
    }
    return 0;
#endif
}


//读取事件触发调用函数
int ZCE_Event_INotify::handle_input ()
{

#if defined (ZCE_OS_LINUX)

    ZCE_LOG(RS_DEBUG, "ZCE_Event_INotify::handle_input");

    int detect_ret = 0;
    size_t watch_event_num = 0;

    //读取
    ssize_t read_ret = ZCE_LIB::read(inotify_handle_, read_buffer_, READ_BUFFER_LEN);
    if (read_ret <= 0)
    {
        return -1;
    }

    uint32_t read_len = static_cast<uint32_t>(read_ret);
    uint32_t next_entry_offset = 0;

    //可能一次读取出来多个inotify_event数据，所以要循环处理
    do
    {
        detect_ret = 0;

        ::inotify_event *ne_ptr = (::inotify_event *) (read_buffer_ + next_entry_offset);

        //检查读取的数据是否还有一个，
        read_len -= (sizeof(::inotify_event) + ne_ptr->len);
        next_entry_offset += sizeof(::inotify_event) + ne_ptr->len;

        HDL_TO_EIN_MAP::iterator active_iter = watch_event_map_.find(ne_ptr->wd);
        if (active_iter == watch_event_map_.end())
        {
            //某个FD在MAP中间无法找到，最大的可能是
            ZCE_LOG(RS_DEBUG,
                    "You code error or a handle not in map (delete in this do while), please check you code. handle[%u]",
                    ne_ptr->wd);
            continue;
        }
        EVENT_INOTIFY_NODE *node_ptr = &(active_iter->second);
        const char *active_path = ne_ptr->name;
        //根据返回的mask决定如何处理

        //注意下面的代码分支用的if else if ,而不是if if，我的初步看法是这些事件不会一起触发，但也许不对。
        //下面5个是和Windows 共有的，
        uint32_t event_mask = ne_ptr->mask;
        if (event_mask & IN_CREATE )
        {
            detect_ret = inotify_create(node_ptr->watch_handle_,
                                        event_mask,
                                        node_ptr->watch_path_,
                                        active_path);
        }
        else if (event_mask & IN_DELETE  )
        {
            detect_ret = inotify_delete(node_ptr->watch_handle_,
                                        event_mask,
                                        node_ptr->watch_path_,
                                        active_path);
        }
        else if ( event_mask & IN_MODIFY )
        {
            detect_ret = inotify_modify(node_ptr->watch_handle_,
                                        event_mask,
                                        node_ptr->watch_path_,
                                        active_path);
        }
        else if ( event_mask & IN_MOVED_FROM)
        {
            detect_ret = inotify_moved_from(node_ptr->watch_handle_,
                                            event_mask,
                                            node_ptr->watch_path_,
                                            active_path);
        }
        else if ( event_mask & IN_MOVED_TO)
        {
            detect_ret = inotify_moved_to(node_ptr->watch_handle_,
                                          event_mask,
                                          node_ptr->watch_path_,
                                          active_path);
        }
        //下面这些是LINUX自己特有的
        else if ( event_mask & IN_ACCESS)
        {
            detect_ret = inotify_access(node_ptr->watch_handle_,
                                        event_mask,
                                        node_ptr->watch_path_,
                                        active_path);
        }
        else if (event_mask & IN_OPEN)
        {
            detect_ret = inotify_open(node_ptr->watch_handle_,
                                      event_mask,
                                      node_ptr->watch_path_,
                                      active_path);
        }
        else if (event_mask & IN_CLOSE_WRITE || event_mask | IN_CLOSE_NOWRITE)
        {
            detect_ret = inotify_close(node_ptr->watch_handle_,
                                       event_mask,
                                       node_ptr->watch_path_,
                                       active_path);
        }
        else if (event_mask & IN_ATTRIB)
        {
            detect_ret = inotify_attrib(node_ptr->watch_handle_,
                                        event_mask,
                                        node_ptr->watch_path_,
                                        active_path);
        }
        else if (event_mask & IN_MOVE_SELF)
        {
            detect_ret = inotify_move_slef(node_ptr->watch_handle_,
                                           event_mask,
                                           node_ptr->watch_path_,
                                           active_path);
        }
        else if (event_mask & IN_DELETE_SELF)
        {
            detect_ret = inotify_delete_slef(node_ptr->watch_handle_,
                                             event_mask,
                                             node_ptr->watch_path_,
                                             active_path);
        }

        //返回-1，关闭之,
        if (detect_ret == -1)
        {
            rm_watch(node_ptr->watch_handle_);
        }

        //累计发现事件计数
        ++(watch_event_num);

    }
    while (read_len > 0);

    return 0;

#elif defined (ZCE_OS_WINDOWS)

    int detect_ret = 0;
    DWORD num_read = 0;
    BOOL bret = ::GetOverlappedResult(watch_handle_,
                                      &over_lapped_,
                                      &num_read,
                                      FALSE);

    //读取结果失败
    if (FALSE == bret)
    {
        ZCE_LOG(RS_ERROR, "[%s] ::GetOverlappedResult fail,error [%u].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error());
        return -1;
    }


    //记录当前处理的句柄，


    FILE_NOTIFY_INFORMATION *read_ptr = NULL;
    DWORD next_entry_offset = 0;
    do
    {
        detect_ret = 0;
        read_ptr = (FILE_NOTIFY_INFORMATION *)(read_buffer_ + next_entry_offset);

        //文件名称进行转换,从宽字节转换为多字节,
        //天杀的Windows在这些地方又埋了陷阱FILE_NOTIFY_INFORMATION里面的长度FileNameLength是字节长度
        //而WideCharToMultiByte函数需要的长度是字长度，你能TMD统一一点吗？
        int length_of_ws = ::WideCharToMultiByte(CP_ACP,
                                                 0,
                                                 read_ptr->FileName,
                                                 read_ptr->FileNameLength / sizeof(wchar_t),
                                                 NULL,
                                                 0,
                                                 NULL,
                                                 NULL);
        //Windows 的目录名称最大长度可以到3K，我没有兴趣去搞一套这个玩
        if (length_of_ws >= MAX_PATH)
        {
            ZCE_LOG(RS_ALERT, "My God ,your path length [%u] more than MAX_PATH [%u],I don't process this.",
                    length_of_ws,
                    MAX_PATH);
            continue;
        }
        char active_path[MAX_PATH + 1];
        ::WideCharToMultiByte(CP_ACP,
                              0,
                              read_ptr->FileName,
                              read_ptr->FileNameLength / sizeof(wchar_t),
                              active_path,
                              length_of_ws,
                              NULL,
                              NULL);
        active_path[length_of_ws] = '\0';

        //根据Action和mask确定调用函数
        switch (read_ptr->Action)
        {
            case FILE_ACTION_ADDED:
                if (watch_mask_ | IN_CREATE)
                {
                    detect_ret = inotify_create(watch_handle_,
                                                watch_mask_,
                                                watch_path_,
                                                active_path);
                }
                break;
            case FILE_ACTION_REMOVED:
                if (watch_mask_ | IN_DELETE)
                {
                    detect_ret = inotify_delete(watch_handle_,
                                                watch_mask_,
                                                watch_path_,
                                                active_path);
                }
                break;
                //注意Windows 下的这个类型，包括了属性更改
            case FILE_ACTION_MODIFIED:
                if (watch_mask_ | IN_MODIFY)
                {
                    detect_ret = inotify_modify(watch_handle_,
                                                watch_mask_,
                                                watch_path_,
                                                active_path);
                }
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
                if (watch_mask_ | IN_MOVED_FROM)
                {
                    detect_ret = inotify_moved_from(watch_handle_,
                                                    watch_mask_,
                                                    watch_path_,
                                                    active_path);
                }
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                if (watch_mask_ | IN_MOVED_TO)
                {
                    detect_ret = inotify_moved_to(watch_handle_,
                                                  watch_mask_,
                                                  watch_path_,
                                                  active_path);
                }
                break;
        }

        //累计偏移长度
        next_entry_offset += read_ptr->NextEntryOffset;

        //返回-1，关闭之
        if (detect_ret == -1)
        {
            handle_close();
        }

        //为什么要这样做，因为上面的处理过程，可能有人已经调用了rm_watch，或者handle_close，
        if (watch_handle_ == ZCE_INVALID_HANDLE)
        {
            return 0;
        }

    }
    while (read_ptr->NextEntryOffset != 0);

    DWORD bytes_returned = 0;

    //继续进行监控处理
    bret = ::ReadDirectoryChangesW(
               watch_handle_,            // handle to directory
               read_buffer_, // read results buffer
               READ_BUFFER_LEN,                               // length of buffer
               watch_sub_dir_,                                 // monitoring option
               FILE_NOTIFY_CHANGE_SECURITY |
               FILE_NOTIFY_CHANGE_CREATION |
               FILE_NOTIFY_CHANGE_LAST_ACCESS |
               FILE_NOTIFY_CHANGE_LAST_WRITE |
               FILE_NOTIFY_CHANGE_SIZE |
               FILE_NOTIFY_CHANGE_ATTRIBUTES |
               FILE_NOTIFY_CHANGE_DIR_NAME |
               FILE_NOTIFY_CHANGE_FILE_NAME,          // filter conditions
               &bytes_returned,                       // bytes returned
               & (over_lapped_), // overlapped buffer
               NULL                                   // completion routine
           );
    if (FALSE == bret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib][%s] ::ReadDirectoryChangesW fail,error [%u].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error());
    }

    return 0;

#endif
}


//关闭监控句柄
int ZCE_Event_INotify::handle_close ()
{
    return close();
}







