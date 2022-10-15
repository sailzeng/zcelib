#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_log_logging.h"
#include "zce_event_reactor_select.h"

//WINDOWS的代码自己会在FD_SET这儿有一个告警
#if defined (ZCE_OS_WINDOWS)
#pragma warning(disable : 4127)
#endif

//
ZCE_Select_Reactor::ZCE_Select_Reactor():
    read_fd_set_{0},
    write_fd_set_{0},
    exception_fd_set_{0}
{
    initialize(FD_SETSIZE);
}

ZCE_Select_Reactor::ZCE_Select_Reactor(size_t max_event_number):
    read_fd_set_{0},
    write_fd_set_{0},
    exception_fd_set_{0}
{
    initialize(max_event_number);
}

ZCE_Select_Reactor::~ZCE_Select_Reactor()
{
}

//初始化
int ZCE_Select_Reactor::initialize(size_t max_event_number)
{

    //清0
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    FD_ZERO(&exception_fd_set_);

    return ZCE_Reactor::initialize(max_event_number);
}

//打开某些mask标志，
int ZCE_Select_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET) (event_handler->get_handle());
    ZCE_Event_Handler *tmp_handler = NULL;

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
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK) )
    {
        FD_SET(socket_hd, &read_fd_set_);
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        FD_SET(socket_hd, &write_fd_set_);
    }

    //在WINDOWS下，如果是非阻塞连接，如果连接失败返回的是事件是超时
#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK))
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

    return ZCE_Reactor::schedule_wakeup(event_handler, event_mask);
}

//取消某些mask标志，，
int ZCE_Select_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET)event_handler->get_handle();
    ZCE_Event_Handler *tmp_handler = NULL;

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
    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        FD_CLR(socket_hd, &read_fd_set_);
    }

    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::WRITE_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        FD_CLR(socket_hd, &write_fd_set_);
    }

#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::EXCEPT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::EXCEPT_MASK))
#endif
    {
        FD_CLR(socket_hd, &exception_fd_set_);
    }

    ret = ZCE_Reactor::cancel_wakeup(event_handler, cancel_mask);

    if (0 != ret )
    {
        return ret;
    }

#if defined ZCE_OS_LINUX

    //检查最新的mask值，如果为0，就更新max_fd_plus_one_
    int new_event_mask =  event_handler->get_mask();

    //这个地方其实是比较耗时，另外BOOST好像在取消的时候甚至没有减少，
    //我不想维护一个MAP，也不像ACE那样自己包装一个fd_set类，所以只有出此下策，下面的代码如此的晦涩，我也很讨厌
    //看你妹的select的。又是+1，又是-1的，要了老命了

    //如果需要删除的是最大的文件句柄值，那么就调整最大值
    if (new_event_mask == 0 &&  max_fd_plus_one_ == socket_hd + 1)
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
int ZCE_Select_Reactor::handle_events(ZCE_Time_Value *max_wait_time,
                                      size_t *size_event)
{
    //
    *size_event = 0;

    //保留句柄，因为select函数是输入输出参数，所以必须保留了，费时的麻烦呀
    para_read_fd_set_ = read_fd_set_;
    para_write_fd_set_ = write_fd_set_;
    para_exception_fd_set_ = exception_fd_set_;

    //
    int const nfds = zce::select (max_fd_plus_one_,
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
    process_ready(&para_read_fd_set_, ZCE_Event_Handler::READ_MASK);

    //处理写事件
    process_ready(&para_write_fd_set_, ZCE_Event_Handler::WRITE_MASK);

    //处理异常事件
    process_ready(&para_exception_fd_set_, ZCE_Event_Handler::EXCEPT_MASK);

    *size_event = nfds;

    return 0;
}

//处理ready的FD，调用相应的虚函数
void ZCE_Select_Reactor::process_ready(const fd_set *out_fds,
                                       ZCE_Event_Handler::EVENT_MASK proc_mask)
{
    int ret = 0, hdl_ret = 0;
    //
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

        ZCE_Event_Handler *event_hdl = NULL;
        ret =  find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);

        //到这个地方，可能是代码有问题(比如你用了多线程？)，也可能不是，因为一个事件处理后，可能就被关闭了？
        if (0 != ret)
        {
            return;
        }

        //根据不同的事件进行调动，触发

        //READ和ACCEPT事件都调用handle_input，ACCEPT_MASK,INOTIFY_MASK,不写的原因是，这个函数是我内部调用的，我只用了3个参数
        if (proc_mask == ZCE_Event_Handler::READ_MASK)
        {
            hdl_ret = event_hdl->handle_input();
        }
        //WRITE和CONNECT事件都调用handle_output,CONNECT_MASK,不写的原因是，这个函数是我内部调用的，我只用了3个参数
        else if (proc_mask == ZCE_Event_Handler::WRITE_MASK)
        {
            hdl_ret = event_hdl->handle_output();
        }
        //异常事件，其实我也不知道，什么算异常
        else if (proc_mask == ZCE_Event_Handler::EXCEPT_MASK)
        {
#if defined ZCE_OS_WINDOWS
            //如果是非阻塞连接，连接失败后会触发异常事件，为了和LINUX环境统一，我们触发handle_input
            int register_mask = event_hdl->get_mask();

            if (ZCE_BIT_IS_SET(register_mask, ZCE_Event_Handler::CONNECT_MASK) )
            {
                hdl_ret = event_hdl->handle_input();
            }
            else
            {
                hdl_ret = event_hdl->handle_exception();
            }

#elif defined ZCE_OS_LINUX
            hdl_ret = event_hdl->handle_exception();
#endif

        }

        else
        {
            ZCE_ASSERT(false);
        }

        //返回-1表示 handle_xxxxx希望调用handle_close退出
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }
}


#if defined (ZCE_OS_WINDOWS)
#pragma warning(default : 4127)
#endif
