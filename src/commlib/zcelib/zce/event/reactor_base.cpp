#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/logger/logging.h"
#include "zce/event/handle_base.h"
#include "zce/event/reactor_base.h"

namespace zce
{
//
reactor* reactor::instance_ = nullptr;

reactor::reactor() :
    max_event_number_(FD_SETSIZE)
{
    initialize(max_event_number_);
}

reactor::reactor(size_t max_event_number) :
    max_event_number_(max_event_number)
{
    initialize(max_event_number_);
}

reactor::~reactor()
{
}

//当前反应器容器的句柄数量
size_t reactor::size()
{
    return handler_map_.size();
}

//当前反应器保留的最大句柄数量，容量
size_t reactor::max_size()
{
    return max_event_number_;
}

//
int reactor::initialize(size_t max_event_number)
{
    max_event_number_ = max_event_number;
    handler_map_.rehash(max_event_number_ + 16);

    return 0;
}

//关闭反应器，将所有注册的EVENT HANDLER 注销掉
int reactor::close()
{
    //由于是HASH MAP速度有点慢
    MAP_OF_HANDLER_TO_EVENT::iterator iter_temp = handler_map_.begin();

    //
    for (; iter_temp != handler_map_.end();)
    {
        //关闭之
        zce::event_handler* event_handler = (iter_temp->second);

        //先close_handle,
        event_handler->close_handle();

        //让迭代器继续从最开始干起
        iter_temp = handler_map_.begin();
    }

    handler_map_.clear();

    return 0;
}

//注册一个zce::Event_Handler到反应器
int reactor::register_handler(zce::event_handler* event_handler,
                              int event_mask)
{
    int ret = 0;

    //如果已经大于最大数量，返回错误
    if (handler_map_.size() >= max_event_number_)
    {
        return -1;
    }

    ZCE_HANDLE socket_hd = event_handler->get_handle();
    zce::event_handler* tmp_handler = nullptr;

    //如果已经存在，不能继续注册
    ret = find_event_handler(socket_hd, tmp_handler);
    if (ret == 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] find_event_handler eaqul handle [%lu]. "
                "please check you code .",
                __ZCE_FUNC__,
                tmp_handler);
        return -1;
    }

    //不检测了，失败了就是命不好
    handler_map_.insert(std::make_pair(socket_hd, event_handler));

    if (event_mask != 0)
    {
        schedule_wakeup(event_handler, event_mask);
    }

    return 0;
}

//从反应器注销一个zce::Event_Handler，同事取消他所有的mask
//event_mask其实只判断里面的DONT_CALL
int reactor::remove_handler(zce::event_handler* event_handler,
                            bool call_close_event)
{
    int ret = 0;

    ZCE_HANDLE ev_hd = event_handler->get_handle();
    zce::event_handler* tmp_handler = nullptr;

    //remove_handler可能会出现两次调用的情况，我推荐你直接调用event_close
    ret = find_event_handler(ev_hd, tmp_handler);
    if (ret != 0)
    {
        // 未找到
        ZCE_LOG(RS_INFO, "[zcelib][%s] find handle [%lu] fail. my be reclose ?",
                __ZCE_FUNC__,
                ev_hd);
        return -1;
    }

    //不好用这个断言保护，因为可能出现一些重复调用等问题
    //ZCE_ASSERT_DGB(tmp_handler == event_handler);

    int event_mask = event_handler->get_mask();

    //如果有mask，取消掉
    if (event_mask != 0)
    {
        cancel_wakeup(event_handler, event_mask);
    }

    //不检测了，失败了就是命不好
    handler_map_.erase(event_handler->get_handle());

    //
    if (call_close_event)
    {
        //调用event_close
        event_handler->close_handle();
    }

    return 0;
}

//
int reactor::cancel_wakeup(zce::event_handler* event_handler, int cancel_mask)
{
    event_handler->disable_mask(cancel_mask);
    return 0;
}

//
int reactor::schedule_wakeup(zce::event_handler* event_handler, int event_mask)
{
    event_handler->enable_mask(event_mask);
    return 0;
}

//查询一个event handler是否注册了，如果存在返回0
int reactor::exist_event_handler(zce::event_handler* event_handler)
{
    ZCE_HANDLE socket_hd = event_handler->get_handle();

    auto iter_temp = handler_map_.find(socket_hd);

    //已经有一个HANDLE了
    if (iter_temp == handler_map_.end())
    {
        return -1;
    }
    return 0;
}

//通过句柄查询event handler，如果存在返回0
int reactor::find_event_handler(ZCE_HANDLE handle,
                                zce::event_handler*& event_handler)
{
    auto iter_temp = handler_map_.find(handle);

    //已经有一个HANDLE了
    if (iter_temp == handler_map_.end())
    {
        event_handler = nullptr;
        return -1;
    }

    event_handler = iter_temp->second;

    return 0;
}

//得到唯一的单子实例
reactor* reactor::instance()
{
    //这个地方和其他单子函数不同，要先赋值
    return instance_;
}

//赋值唯一的单子实例
void reactor::instance(reactor* pinstatnce)
{
    clear_inst();
    instance_ = pinstatnce;
    return;
}

//清除单子实例
void reactor::clear_inst()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = nullptr;
    return;
}
}