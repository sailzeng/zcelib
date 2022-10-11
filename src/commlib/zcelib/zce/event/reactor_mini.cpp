#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/reactor_mini.h"

namespace zce
{
//
reactor_mini* reactor_mini::instance_ = NULL;

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
int reactor_mini::initialize(size_t max_event_number,
                             size_t once_max_events,
                             bool after_trigger_close)
{
    max_event_number_ = max_event_number;
    once_max_events_ = once_max_events;
    after_trigger_close_ = after_trigger_close;
    event_set_.rehash(max_event_number_ + 16);
    //这个只有LINUX下才有
#if defined (ZCE_OS_LINUX)
    epoll_fd_ = ::epoll_create(max_event_number_ + 64);

    if (epoll_fd_ < 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Epoll reactor ::epoll_create fail.please check code. "
                "error = [%u|%u]",
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    once_events_ary_ = new epoll_event[once_max_events_];
#endif
    return 0;
}

//关闭反应器，将所有注册的EVENT HANDLER 注销掉
int reactor_mini::close()
{
    //由于是HASH MAP速度有点慢
    event_call_set_t::iterator iter_temp = event_set_.begin();
    //
    for (; iter_temp != event_set_.end();)
    {
        //先close_socket,
        zce::close_socket((ZCE_SOCKET)(iter_temp->handle_));
        event_set_.erase(iter_temp);
        //让迭代器继续从最开始干起
        iter_temp = event_set_.begin();
    }

    event_set_.clear();
#if defined (ZCE_OS_LINUX)
    ::close(epoll_fd_);

    //释放内存
    if (once_events_ary_)
    {
        delete[] once_events_ary_;
    }
#endif
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
    event_call_set_t::iterator find_iter;
    size_t hdl_event_num = 0;
    if (find_event(handle, event_todo, find_iter, hdl_event_num))
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] find eaqul handle [%lu] event[%u]. "
                "please check you code .",
                __ZCE_FUNC__,
                handle,
                event_todo);
        return -1;
    }
    //不检测了，失败了就是命不好
    EVENT_CALL ec(handle, event_todo, call_back);
    auto ins_iter = event_set_.insert(ec);

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
    struct epoll_event ep_event;
    make_epoll_event(&ep_event, event_handler);

    //EPOLL 在LINUX才有，WINDOWS也不可能模拟出来，难道让我用个SELECT模拟？
    int op = EPOLL_CTL_ADD;
    if (hdl_event_num > 0)
    {
        op = EPOLL_CTL_MOD;
    }
    int ret = ::epoll_ctl(epoll_fd_,
                          op,
                          handle,
                          &ep_event);

    if (0 != ret)
    {
        //回滚
        event_set_.erase(ins_iter);
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] Epoll reactor ::epoll_ctl fail.please check code."
                " ret =%d error = [%u|%s]",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

#endif
    return 0;
}

//从反应器注销一个zce::Event_Handler，同事取消他所有的mask
//event_mask其实只判断里面的DONT_CALL
int reactor_mini::remove_event(ZCE_HANDLE handle,
                               EVENT_MASK event_todo)
{
    //remove_handler可能会出现两次调用的情况，我推荐你直接调用event_close
    event_call_set_t::iterator find_iter;
    size_t hdl_event_num = 0;
    if (!find_event(handle, event_todo, find_iter, hdl_event_num))
    {
        // 未找到
        ZCE_LOG(RS_INFO, "[zcelib][%s] find handle [%lu] event[%u] fail. my be reclose ?",
                __ZCE_FUNC__,
                handle,
                event_todo);
        return -1;
    }

    //不检测了，失败了就是命不好
    event_set_.erase(find_iter);
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
    struct epoll_event ep_event;
    make_epoll_event(&ep_event, event_handler);
    int op = EPOLL_CTL_DEL;
    if (hdl_event_num > 1)
    {
        op = EPOLL_CTL_MOD;
    }
    //EPOLL_CTL_MOD用于修改
    ret = ::epoll_ctl(epoll_fd_,
                      op,
                      handle,
                      &ep_event);

    if (0 != ret)
    {
        //回滚不改动标志位
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code."
                " ret =%d error = [%u|%s]",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }
#endif
    return 0;
}

//通过句柄查询event handler，如果存在返回0
bool reactor_mini::find_event(ZCE_HANDLE handle,
                              EVENT_MASK event_todo,
                              event_call_set_t::iterator &find_iter,
                              size_t &hdl_event_num) const
{
    auto iter_end = event_set_.cend();
    find_iter = iter_end;
    hdl_event_num = 0;
    EVENT_CALL ec(handle);
    auto iter_temp = event_set_.find(ec);

    //已经有一个HANDLE了
    if (iter_temp == event_set_.end())
    {
        return false;
    }
    //因为使用的是mutliset，还必须继续找找看看。
    bool found = false;
    for (; iter_temp->handle_ == handle && iter_temp != iter_end; ++iter_temp)
    {
        ++hdl_event_num;
        //必须找到相同的event_todo_
        if (iter_temp->event_todo_ == event_todo)
        {
            find_iter = iter_temp;
            found = true;
            //不能直接返回要循环计数
        }
    }
    return found;
}

//
int reactor_mini::handle_events(zce::time_value* time_out,
                                size_t* size_event)
{
#if defined (ZCE_OS_WINDOWS)

    *size_event = 0;
    //保留句柄，因为select函数是输入输出参数，所以必须保留了，费时的麻烦呀
    para_read_fd_set_ = read_fd_set_;
    para_write_fd_set_ = write_fd_set_;
    para_exception_fd_set_ = exception_fd_set_;
    int const nfds = zce::select(max_fd_plus_one_,
                                 &para_read_fd_set_,
                                 &para_write_fd_set_,
                                 &para_exception_fd_set_,
                                 time_out);

    if (nfds == 0)
    {
        return 0;
    }
    if (nfds == -1)
    {
        // TODO: 出错处理
        return 0;
    }

    //严格遵守调用顺序，读取，写，异常处理3个步骤完成，

    //处理读事件
    process_ready(&para_read_fd_set_, SELECT_EVENT::SE_READ);
    //处理写事件
    process_ready(&para_write_fd_set_, SELECT_EVENT::SE_WRITE);
    //处理异常事件
    process_ready(&para_exception_fd_set_, SELECT_EVENT::SE_EXCEPTION);
    *size_event = nfds;
#elif defined (ZCE_OS_LINUX)
    //默认一直阻塞
    int msec_timeout = -1;

    if (time_out)
    {
        //根据bluehu 提醒修改了下面这段，（不过小伙子们，你们改代码要认真一点喔）
        //由于select的超时参数可以精确到微秒，而epoll_wait的参数只精确到毫秒
        //当超时时间小于1000微秒时，比如20微秒，将时间转换成毫秒会变成0毫秒
        //所以如果用epoll_wait的话，超时时间大于0并且小于1毫秒的情况下统一改为1毫秒

        msec_timeout = static_cast<int>(time_out->total_msec_round());
    }

    int event_happen = 0;

    //EPOLL等待事件触发，
    event_happen = ::epoll_wait(epoll_fd_,
                                once_events_ary_,
                                once_max_events_,
                                msec_timeout);

    if (event_happen <= 0)
    {
        return event_happen;
    }

    *size_event = event_happen;
    for (int i = 0; i < event_happen; ++i)
    {
        process_ready_event(once_events_ary_ + i);
    }

    return 0;
#endif
    return 0;
}

#if defined ZCE_OS_WINDOWS
//处理ready的FD，调用相应的虚函数
void reactor_mini::process_ready(const fd_set* out_fds,
                                 SELECT_EVENT proc_event)
{
    int max_process = max_fd_plus_one_;

    //下面三个代码段都非常类似，也许封装几个函数更好看一点，但是...
    //WINDOWS的实现机理略有不同，用这个加快速度
    max_process = out_fds->fd_count;

    auto iter_end = event_set_.cend();
    for (int i = 0; i < max_process; i++)
    {
        ZCE_SOCKET socket_handle;
        bool hd_ready = zce::is_ready_fds(i, out_fds, &socket_handle);
        if (!hd_ready)
        {
            continue;
        }
        ZCE_HANDLE handle = (ZCE_HANDLE)socket_handle;
        EVENT_CALL ec((ZCE_HANDLE)socket_handle);
        auto iter_temp = event_set_.find(ec);

        //已经有一个HANDLE了
        if (iter_temp == event_set_.end())
        {
            //???
            return;
        }
        //因为使用的是mutliset，还必须继续找找看看。
        EVENT_MASK event_todo = NULL_MASK;
        bool if_trigger = false;
        for (; iter_temp->handle_ == handle && iter_temp != iter_end; ++iter_temp)
        {
            bool connect_succ = true;
            //必须找到相同的event_todo_
            event_todo = iter_temp->event_todo_;
            if_trigger = false;
            if (proc_event == SELECT_EVENT::SE_READ)
            {
                if (event_todo == zce::CONNECT_MASK ||
                    event_todo == zce::ACCEPT_MASK ||
                    event_todo == zce::INOTIFY_MASK ||
                    event_todo == zce::READ_MASK)
                {
                    //必须把链接失败区分出来
                    if (event_todo == zce::CONNECT_MASK)
                    {
                        connect_succ = false;
                    }
                    iter_temp->call_back_(handle,
                                          event_todo,
                                          connect_succ);
                    if_trigger = true;
                    break;
                }
            }
            //WRITE和CONNECT事件都调用write_event,CONNECT_MASK,不写的原因是，这个函数是我内部调用的，我只用了3个参数
            else if (proc_event == SELECT_EVENT::SE_WRITE)
            {
                if (event_todo == zce::CONNECT_MASK ||
                    event_todo == zce::WRITE_MASK)
                {
                    iter_temp->call_back_(handle,
                                          event_todo,
                                          connect_succ);
                    if_trigger = true;
                    break;
                }
            }
            //异常事件，其实我也不知道，什么算异常
            else if (proc_event == SELECT_EVENT::SE_EXCEPTION)
            {
                //如果是非阻塞连接，连接失败后会触发异常事件，
                //（曾经，曾经为了和LINUX环境统一，我们触发read_event)
                if (event_todo == zce::CONNECT_MASK ||
                    event_todo == zce::EXCEPTION_MASK)
                {
                    if (event_todo == zce::CONNECT_MASK)
                    {
                        connect_succ = false;
                    }
                    iter_temp->call_back_(handle,
                                          event_todo,
                                          connect_succ);
                    if_trigger = true;
                    break;
                }
            }
            else
            {
                ZCE_ASSERT(false);
            }
        }
        if (if_trigger && after_trigger_close_)
        {
            remove_event(handle, event_todo);
        }
    }
}
#elif defined (ZCE_OS_LINUX)

//将mask转换为epoll_event结构
void reactor_mini::make_epoll_event(struct epoll_event* ep_event,
                                    ZCE_HANDLE handle,
                                    EVENT_MASK event_todo) const
{
    ep_event->events = 0;
    ep_event->data.fd = handle;

    int event_mask = event_handler->get_mask();
    if (event_todo == zce::READ_MASK)
    {
        ep_event->events |= EPOLLIN;
    }
    else if (event_todo == zce::WRITE_MASK)
    {
        ep_event->events |= EPOLLOUT;
    }
    else if (event_todo == zce::EXCEPTION_MASK)
    {
        ep_event->events |= EPOLLERR;
    }
    //Connect有成功和失败两种情况
    else if ((event_todo == zce::CONNECT_MASK))
    {
        ep_event->events |= EPOLLOUT;
        ep_event->events |= EPOLLIN;
    }
    else if ((event_todo == zce::ACCEPT_MASK))
    {
        ep_event->events |= EPOLLIN;
    }
    else if (event_todo == zce::INOTIFY_MASK)
    {
        ep_event->events |= EPOLLIN;
    }
}

void reactor_mini::process_ready_event(struct epoll_event* ep_event)
{
    int ret = 0;
    bool event_in_happen = false, event_out_happen = false;

    ZCE_HANDLE handle = (ZCE_HANDLE)ep_event->data.fd;
    EVENT_CALL ec((ZCE_HANDLE)handle);
    auto iter_temp = event_set_.find(ec);

    //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
    if (iter_temp == event_set_.end())
    {
        //???
        return;
    }
    //因为使用的是mutliset，还必须继续找找看看。
    for (; iter_temp->handle_ == handle && iter_temp != iter_end; ++iter_temp)
    {
    }

    //根据不同的事件进行调动，触发
    int hdl_ret = 0;

    //【注意】所有的reactor 都要保证各种触发的顺序，必须保证，否则就有不兼容的问题，正确的顺序应该是读,写,异常处理

    //修正connect发生连接错误，内部希望停止处理，但epoll会返回3个事件的修正,
    //先说明一下这个问题，当内部处理的事件发生错误后，上层可能就会删除掉对应的zce::event_handler，但底层可能还有事件要触发。
    //由于该代码的时候面临抉择，我必须写一段说明，我在改这段代码的时候有几种种方式,
    //1.整体写成do { break} while()的方式，每个IO事件都作为触发方式，这个好处是简单，但对于epoll,如果是水平触发，那么你可能丢失事件
    //2.继续使用return -1作为一个判断，ACE就是这样的，原来曾经觉得ACE这个设计有点冗余，但自己设计看来，还是有好处的，
    //3.每次调用后，都检查一下event_hdl是否还存在，
    //最后我采用了兼容2，3的方法，2是为了和ACE兼容，3是为了保证就是你TMD天翻地覆，我也能应付,

    //代码有点怪，部分目的是加快速度,避免每个调用都要检查，
    int register_mask = event_hdl->get_mask();
    //READ和CONNECT事件都调用read_event
    if (ep_event->events & EPOLLIN)
    {
        event_in_happen = true;
        if (register_mask & zce::CONNECT_MASK)
        {
            hdl_ret = event_hdl->connect_event(false);
        }
        else if (register_mask & zce::ACCEPT_MASK)
        {
            hdl_ret = event_hdl->accept_event();
        }
        else if (register_mask & zce::INOTIFY_MASK)
        {
            hdl_ret = event_hdl->inotify_event();
        }
        else
        {
            hdl_ret = event_hdl->read_event();
        }
        //返回-1表示 handle_xxxxx希望调用event_close退出
        if (hdl_ret == -1)
        {
            event_hdl->close_event();
        }
    }

    //READ和ACCEPT事件都调用read_event
    if (ep_event->events & EPOLLOUT)
    {
        //如果写事件触发了，那么里面可能调用handle_close,再查询一次，double check.
        if (event_in_happen)
        {
            ret = find_event_handler(ep_event->data.fd,
                                     event_hdl);
            //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
            if (0 != ret)
            {
                return;
            }
        }

        event_out_happen = true;
        if (register_mask & zce::CONNECT_MASK)
        {
            hdl_ret = event_hdl->connect_event(true);
        }
        else
        {
            hdl_ret = event_hdl->write_event();
        }

        //返回-1表示 handle_xxxxx希望调用event_close退出
        if (hdl_ret == -1)
        {
            event_hdl->close_event();
        }
    }

    //异常事件，其实我也不知道，什么算异常
    if (ep_event->events & EPOLLERR)
    {
        //如果读取或者写事件触发了，那么里面可能调用handle_close,再查询一次，double check.
        if (event_out_happen || event_in_happen)
        {
            ret = find_event_handler(ep_event->data.fd,
                                     event_hdl);
            //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
            if (0 != ret)
            {
                return;
            }
        }

        hdl_ret = event_hdl->exception_event();
        //返回-1表示 handle_xxxxx希望调用event_close退出
        if (hdl_ret == -1)
        {
            event_hdl->close_event();
        }
    }
}

#endif

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