#include "zce_predefine.h"


#if defined (ZCE_OS_LINUX)

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_error.h"
#include "zce_event_handle_inotify.h"
#include "zce_trace_log_debug.h"
#include "zce_event_reactor_base.h"



//构造函数和析构函数
ZCE_Event_INotify::ZCE_Event_INotify():
    ZCE_Event_Handler(),
    inotify_handle_(ZCE_INVALID_SOCKET),
    read_buffer_(NULL)
{
    //如果不是LINUX环境下使用，死给你看，
#if !defined ZCE_OS_LINUX
    ZCE_ASSERT_ALL(false);
#endif
     
    read_buffer_ = new char [READ_BUFFER_LEN + 1];
    read_buffer_[READ_BUFFER_LEN] = '\0';
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
    int ret = 0;
    
    //如果已经初始化过了
    if (ZCE_INVALID_HANDLE != inotify_handle_ )
    {
        return -1;
    }

    //在LINUX下使用INOTIFY的机制
    inotify_handle_ = ::inotify_init();
    if (ZCE_INVALID_HANDLE == inotify_handle_ )
    {
        ZLOG_ERROR("[%s] invoke ::inotify_init fail,error [%u].",
            __ZCE_FUNCTION__,
            ZCE_OS::last_error());
        return -1;
    }

    reactor(reactor_base);
    ret = reactor()->register_handler(this,READ_MASK);
    if (ret != 0)
    {
        ::close(inotify_handle_);
        return ret;
    }

    return 0;
}


//关闭监控句柄等，解除绑定reactor等
int ZCE_Event_INotify::close()
{
    int ret = 0;

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
        ret = reactor()->remove_handler(this,false);
        if (ret != 0)
        {
            return ret;
        }

        ::close(inotify_handle_);
        inotify_handle_ = ZCE_INVALID_HANDLE;
    }

    return 0;
}



//添加监控                                 
int ZCE_Event_INotify::add_watch(const char *pathname,
                                 uint32_t mask,
                                 ZCE_HANDLE *watch_handle)
{

    //检查参数是否有效，检查句柄是否已经初始化
    ZCE_ASSERT( pathname
        &&  mask 
        && inotify_handle_ != ZCE_INVALID_HANDLE );

    if ( pathname == NULL
        || mask == 0
        ||  inotify_handle_ == ZCE_INVALID_HANDLE )
    {
        errno = EINVAL;
        return -1;
    }

    *watch_handle = ZCE_INVALID_HANDLE;
    EVENT_INOTIFY_NODE watch_note;
    ZCE_HANDLE hdl_dir = ZCE_INVALID_HANDLE;

    hdl_dir =  ::inotify_add_watch(inotify_handle_, pathname, mask);
    if (hdl_dir == ZCE_INVALID_HANDLE )
    {
        ZLOG_ERROR("[%s] invoke ::inotify_add_watch fail,error [%u].",
            __ZCE_FUNCTION__,
            ZCE_OS::last_error());
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

        ZLOG_ERROR("[%s] insert code node to map fail. code error or map already haved one equal HANDLE[%u].",
            __ZCE_FUNCTION__,
            hdl_dir);
        return -1;
    }

    return 0;
}


int ZCE_Event_INotify::rm_watch(ZCE_HANDLE watch_handle)
{

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
}


//读取事件触发调用函数
int ZCE_Event_INotify::handle_input ()
{
    ZCE_LOGMSG(RS_DEBUG, "ZCE_Event_INotify::handle_input");
    int detect_ret = 0;
    size_t watch_event_num = 0;

    //读取
    ssize_t read_ret = ZCE_OS::read(inotify_handle_, read_buffer_, READ_BUFFER_LEN);
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
            ZCE_LOGMSG(RS_DEBUG, 
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
        else if (event_mask | IN_OPEN)
        {
            detect_ret = inotify_open(node_ptr->watch_handle_,
                event_mask,
                node_ptr->watch_path_, 
                active_path);
        }
        else if (event_mask | IN_CLOSE_WRITE || event_mask | IN_CLOSE_NOWRITE)
        {
            detect_ret = inotify_close(node_ptr->watch_handle_,
                event_mask,
                node_ptr->watch_path_, 
                active_path);
        }
        else if (event_mask | IN_ATTRIB)
        {
            detect_ret = inotify_attrib(node_ptr->watch_handle_,
                event_mask,
                node_ptr->watch_path_, 
                active_path);
        }
        else if (event_mask | IN_MOVE_SELF)
        {
            detect_ret = inotify_move_slef(node_ptr->watch_handle_,
                event_mask,
                node_ptr->watch_path_, 
                active_path);
        }
        else if (event_mask | IN_DELETE_SELF)
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
}


//关闭监控句柄
int ZCE_Event_INotify::handle_close ()
{
    return close();
}




#endif //#if defined (ZCE_OS_LINUX)


