#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/event/handle_base.h"
#include "zce/os_adapt/socket.h"
#include "zce/logger/logging.h"
#include "zce/event/reactor_select.h"

namespace zce
{
//
select_reactor::select_reactor() :
    read_fd_set_{ 0 },
    write_fd_set_{ 0 },
    exception_fd_set_{ 0 }
{
    initialize(FD_SETSIZE);
}

select_reactor::select_reactor(size_t max_event_number) :
    read_fd_set_{ 0 },
    write_fd_set_{ 0 },
    exception_fd_set_{ 0 }
{
    initialize(max_event_number);
}

select_reactor::~select_reactor()
{
}

//初始化
int select_reactor::initialize(size_t max_event_number)
{
    //清0
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    FD_ZERO(&exception_fd_set_);

    return zce::reactor::initialize(max_event_number);
}

//打开某些mask标志，
int select_reactor::schedule_wakeup(zce::event_handler* event_handler,
                                    int event_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET)(event_handler->get_handle());
    zce::event_handler* tmp_handler = nullptr;

    //如果已经存在，不能继续注册
    ret = find_event_handler((ZCE_HANDLE)socket_hd, tmp_handler);
    if (ret != 0)
    {
        ZCE_LOG(RS_INFO, "[zcelib] [%s] fail find handle [%lu],maybe one handle is close previous.",
                __ZCE_FUNC__,
                socket_hd);
        return ret;
    }

    //因为这些标志可以一起注册，所以下面的判断是并列的，但是我这儿统一化的处理序列是读，写，异常

    //注意connect的失败，会触发读写事件，需要注意,我记得好像自己都错过两次了。
    if ((event_mask & zce::READ_MASK)
        || (event_mask & zce::ACCEPT_MASK)
        || (event_mask & zce::CONNECT_MASK)
        || (event_mask & zce::INOTIFY_MASK))
    {
        FD_SET(socket_hd, &read_fd_set_);
    }

    if ((event_mask & zce::WRITE_MASK)
        || (event_mask & zce::CONNECT_MASK))
    {
        FD_SET(socket_hd, &write_fd_set_);
    }

    //在WINDOWS下，如果是非阻塞连接，如果连接失败返回的是事件是超时
#if defined (ZCE_OS_WINDOWS)

    if ((event_mask & zce::EXCEPTION_MASK)
        || (event_mask & zce::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if ((event_mask & zce::EXCEPTION_MASK))
#endif
    {
        FD_SET(socket_hd, &exception_fd_set_);
    }

    //Windows的select对于这个值没有要求,而且啊CE反馈Windows 64位的版本select函数第一个参数必须传递0
#if defined ZCE_OS_LINUX

    //注意，需要+1，
    if (max_fd_plus_one_ < socket_hd + 1)
    {
        max_fd_plus_one_ = socket_hd + 1;
    }

#endif

    return zce::reactor::schedule_wakeup(event_handler, event_mask);
}

//取消某些mask标志，，
int select_reactor::cancel_wakeup(zce::event_handler* event_handler, int cancel_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET)event_handler->get_handle();
    zce::event_handler* tmp_handler = nullptr;

    //如果已经存在，不能继续注册
    ret = find_event_handler((ZCE_HANDLE)socket_hd, tmp_handler);
    if (ret != 0)
    {
        ZCE_LOG(RS_INFO, "[zcelib] [%s] fail find handle [%lu],maybe one handle is close previous.",
                __ZCE_FUNC__,
                socket_hd);
        return ret;
    }

    //因为这些标志可以一起注册，所以下面的判断是并列的
    if ((cancel_mask & zce::READ_MASK)
        || (cancel_mask & zce::ACCEPT_MASK)
        || (cancel_mask & zce::CONNECT_MASK)
        || (cancel_mask & zce::INOTIFY_MASK))
    {
        FD_CLR(socket_hd, &read_fd_set_);
    }

    if ((cancel_mask & zce::WRITE_MASK)
        || (cancel_mask & zce::CONNECT_MASK))
    {
        FD_CLR(socket_hd, &write_fd_set_);
    }

#if defined (ZCE_OS_WINDOWS)

    if ((cancel_mask & zce::EXCEPTION_MASK)
        || (cancel_mask & zce::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if (cancel_mask & zce::EXCEPTION_MASK)
#endif
    {
        FD_CLR(socket_hd, &exception_fd_set_);
    }

    ret = zce::reactor::cancel_wakeup(event_handler, cancel_mask);

    if (0 != ret)
    {
        return ret;
    }

#if defined ZCE_OS_LINUX

    //检查最新的mask值，如果为0，就更新max_fd_plus_one_
    int new_event_mask = event_handler->get_mask();

    //这个地方其实是比较耗时，另外BOOST好像在取消的时候甚至没有减少，
    //我不想维护一个MAP，也不像ACE那样自己包装一个fd_set类，所以只有出此下策，下面的代码如此的晦涩，我也很讨厌
    //看你妹的select的。又是+1，又是-1的，要了老命了

    //如果需要删除的是最大的文件句柄值，那么就调整最大值
    if (new_event_mask == 0 && max_fd_plus_one_ == socket_hd + 1)
    {
        //因为最大值已经删除，先--
        --max_fd_plus_one_;

        while (max_fd_plus_one_ > 0)
        {
            //寻找是否有这个FD，注意这儿要max_fd_plus_one_-1
            if (FD_ISSET(max_fd_plus_one_ - 1, &read_fd_set_))
            {
                break;
            }

            if (FD_ISSET(max_fd_plus_one_ - 1, &write_fd_set_))
            {
                break;
            }

            if (FD_ISSET(max_fd_plus_one_ - 1, &exception_fd_set_))
            {
                break;
            }

            //
            --max_fd_plus_one_;
        }
    }

#endif

    return 0;
}

//事件触发
int select_reactor::handle_events(zce::time_value* max_wait_time,
                                  size_t* size_event)
{
    //
    *size_event = 0;

    //保留句柄，因为select函数是输入输出参数，所以必须保留了，费时的麻烦呀
    para_read_fd_set_ = read_fd_set_;
    para_write_fd_set_ = write_fd_set_;
    para_exception_fd_set_ = exception_fd_set_;

    int const nfds = zce::select(max_fd_plus_one_,
                                 &para_read_fd_set_,
                                 &para_write_fd_set_,
                                 &para_exception_fd_set_,
                                 max_wait_time);

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

    return 0;
}

//处理ready的FD，调用相应的虚函数
void select_reactor::process_ready(const fd_set* out_fds,
                                   SELECT_EVENT proc_event)
{
    int ret = 0;
    int max_process = max_fd_plus_one_;

    //下面三个代码段都非常类似，也许封装几个函数更好看一点，但是...
    //处理读事件

#if defined ZCE_OS_WINDOWS
    //WINDOWS的实现机理略有不同，用这个加快速度
    max_process = out_fds->fd_count;
#endif

    for (int i = 0; i < max_process; i++)
    {
        ZCE_SOCKET socket_handle;
        bool hd_ready = zce::is_ready_fds(i, out_fds, &socket_handle);
        if (!hd_ready)
        {
            continue;
        }

        zce::event_handler* event_hdl = nullptr;
        ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);

        //到这个地方，可能是代码有问题(比如你用了多线程？)，也可能不是，因为一个事件处理后，可能就被关闭了？
        if (0 != ret)
        {
            return;
        }

        int register_mask = event_hdl->get_mask();
        //根据不同的事件进行调动，触发
        //READ和ACCEPT事件都调用read_event，ACCEPT_MASK,INOTIFY_MASK,不写的原因是，这个函数是我内部调用的，我只用了3个参数
        if (proc_event == SELECT_EVENT::SE_READ)
        {
            if (register_mask & zce::CONNECT_MASK)
            {
                event_hdl->connect_event(false);
            }
            else if (register_mask & zce::ACCEPT_MASK)
            {
                event_hdl->accept_event();
            }
            else if (register_mask & zce::INOTIFY_MASK)
            {
                event_hdl->inotify_event();
            }
            else
            {
                event_hdl->read_event();
            }
        }
        //WRITE和CONNECT事件都调用write_event,CONNECT_MASK,不写的原因是，这个函数是我内部调用的，我只用了3个参数
        else if (proc_event == SELECT_EVENT::SE_WRITE)
        {
            if (register_mask & zce::CONNECT_MASK)
            {
                event_hdl->connect_event(true);
            }
            else
            {
                event_hdl->write_event();
            }
        }
        //异常事件，其实我也不知道，什么算异常
        else if (proc_event == SELECT_EVENT::SE_EXCEPTION)
        {
#if defined ZCE_OS_WINDOWS
            //如果是非阻塞连接，连接失败后会触发异常事件，
            //（曾经，曾经为了和LINUX环境统一，我们触发read_event)
            if (register_mask & zce::CONNECT_MASK)
            {
                event_hdl->connect_event(false);
            }
            else
            {
                event_hdl->exception_event();
            }

#elif defined ZCE_OS_LINUX
            event_hdl->exception_event();
#endif
        }
        else
        {
            ZCE_ASSERT(false);
        }
    }
}
}