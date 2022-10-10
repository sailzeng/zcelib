#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/reactor_mini.h"

namespace zce
{
//
reactor_mini* reactor_mini::instance_ = NULL;

reactor_mini::reactor_mini() :
    max_event_number_(FD_SETSIZE)
{
    initialize(max_event_number_);
}

reactor_mini::reactor_mini(size_t max_event_number) :
    max_event_number_(max_event_number)
{
    initialize(max_event_number_);
}

reactor_mini::~reactor_mini()
{
}

//当前反应器容器的句柄数量
size_t reactor_mini::size()
{
    return event_set_.size();
}

//当前反应器保留的最大句柄数量，容量
size_t reactor_mini::max_size()
{
    return max_event_number_;
}

//
int reactor_mini::initialize(size_t max_event_number)
{
    max_event_number_ = max_event_number;
    event_set_.rehash(max_event_number_ + 16);

    return 0;
}

//关闭反应器，将所有注册的EVENT HANDLER 注销掉
int reactor_mini::close()
{
    //由于是HASH MAP速度有点慢
    EVENT_CALL_SET::iterator iter_temp = event_set_.begin();
    //
    for (; iter_temp != event_set_.end();)
    {
        //先close_socket,
        //zce::close_socket(iter_temp->handle_);
        event_set_.erase(iter_temp);
        //让迭代器继续从最开始干起
        iter_temp = event_set_.begin();
    }

    event_set_.clear();

    return 0;
}

//注册一个zce::Event_Handler到反应器
int reactor_mini::register_event(ZCE_HANDLE handle,
                                 EVENT_MASK event_todo,
                                 event_callback_t call_back)
{
    ZCE_SOCKET socket_hd = (ZCE_SOCKET)(handle);
    //如果已经大于最大数量，返回错误
    if (event_set_.size() >= max_event_number_)
    {
        return -1;
    }

    //如果已经存在，不能继续注册
    EVENT_CALL ec(handle, event_todo, call_back);
    auto iter = event_set_.find(ec);
    if (iter != event_set_.end())
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] find eaqul handle [%lu] event[%u]. "
                "please check you code .",
                __ZCE_FUNC__,
                handle,
                event_todo);
        return -1;
    }

    //不检测了，失败了就是命不好
    event_set_.insert(ec);

#if defined (ZCE_OS_WINDOWS)
    //注意connect的失败，会触发读写事件，需要注意,我记得好像自己都错过两次了。
    if ((event_todo == zce::READ_MASK)
        || (event_todo == zce::ACCEPT_MASK)
        || (event_todo == zce::CONNECT_MASK)
        || (event_todo == zce::INOTIFY_MASK))
    {
        FD_SET(socket_hd, &read_fd_set_);
    }

    if ((event_todo == zce::WRITE_MASK)
        || (event_todo == zce::CONNECT_MASK))
    {
        FD_SET(socket_hd, &write_fd_set_);
    }

    //在WINDOWS下，如果是非阻塞连接，如果连接失败返回的是事件是超时
    if ((event_todo == zce::EXCEPTION_MASK)
        || (event_todo == zce::CONNECT_MASK))
    {
        FD_SET(socket_hd, &exception_fd_set_);
    }
#elif defined (ZCE_OS_LINUX)

#endif
    return 0;
}

//从反应器注销一个zce::Event_Handler，同事取消他所有的mask
//event_mask其实只判断里面的DONT_CALL
int reactor_mini::remove_event(ZCE_HANDLE handle,
                               EVENT_MASK event_todo)
{
    //remove_handler可能会出现两次调用的情况，我推荐你直接调用event_close
    EVENT_CALL ec(handle, event_todo);
    auto iter = event_set_.find(ec);
    if (iter == event_set_.end())
    {
        // 未找到
        ZCE_LOG(RS_INFO, "[zcelib][%s] find handle [%lu] event[%u] fail. my be reclose ?",
                __ZCE_FUNC__,
                handle,
                event_todo);
        return -1;
    }

    //不检测了，失败了就是命不好
    event_set_.erase(ec);
    ZCE_SOCKET socket_hd = (ZCE_SOCKET)(handle);
#if defined (ZCE_OS_WINDOWS)
    //因为这些标志可以一起注册，所以下面的判断是并列的
    if ((event_todo == zce::READ_MASK)
        || (event_todo == zce::ACCEPT_MASK)
        || (event_todo == zce::CONNECT_MASK)
        || (event_todo == zce::INOTIFY_MASK))
    {
        FD_CLR(socket_hd, &read_fd_set_);
    }

    if ((event_todo & zce::WRITE_MASK)
        || (event_todo & zce::CONNECT_MASK))
    {
        FD_CLR(socket_hd, &write_fd_set_);
    }
    if ((event_todo & zce::EXCEPTION_MASK)
        || (event_todo & zce::CONNECT_MASK))
    {
        FD_CLR(socket_hd, &exception_fd_set_);
    }
#elif defined (ZCE_OS_LINUX)

#endif
    return 0;
}

//通过句柄查询event handler，如果存在返回0
bool reactor_mini::find_event(ZCE_HANDLE handle,
                              EVENT_MASK event_todo)
{
    EVENT_CALL ec(handle, event_todo);
    auto iter_temp = event_set_.find(ec);

    //已经有一个HANDLE了
    if (iter_temp == event_set_.end())
    {
        return false;
    }

    return true;
}

//
int reactor_mini::handle_events(zce::time_value* time_out,
                                size_t* size_event)
{
#if defined (ZCE_OS_WINDOWS)

#elif defined (ZCE_OS_LINUX)

#endif
    return 0;
}

//得到唯一的单子实例
reactor_mini* reactor_mini::instance()
{
    //这个地方和其他单子函数不同，要先赋值
    return instance_;
}

//赋值唯一的单子实例
void reactor_mini::instance(reactor_mini* inst)
{
    clear_inst();
    instance_ = inst;
    return;
}

//清除单子实例
void reactor_mini::clear_inst()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}
}