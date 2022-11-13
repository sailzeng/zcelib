/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/event/reactor_select.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月1日
* @brief      SELECT 的IO反应器，IO多路复用模型
*             更多的突出性能第一的前提，你从zce::Event_Handler上继承得到处理的句柄
*             封装，各种事件的处理都已经封装了，而且整体是和LINUX平台一致。
*
*             他的优点是在Windows平台也可以用，缺点就是SELECT函数所有的缺点，
*             当然我们通过封装，已经保证了在个个平台都有不错的性能。
*             不容易。
*
* @details    这个和ACE的ZCE_Reactor有点像，但我们简化了很多地方，我们不融入定时器，
*             我们也不融入消息队列，他简简单单就是为了IO触发。
*
*             请参考：
*             http://www.cnblogs.com/fullsail/archive/2012/08/12/2634336.html
*             http://www.cnblogs.com/fullsail/archive/2011/11/06/2238464.html
* @note
*
*/
#pragma once

#include "zce/event/reactor_base.h"

namespace zce
{
/*!
* @brief      SELECT 的IO反应器，IO多路复用模型
*
*/
class select_reactor : public zce::reactor
{
public:

    /*!
    * @brief    构造函数
    */
    select_reactor();

    /*!
    * @brief      构造函数和析构函数
    * @param      max_event_number 最大的句柄数量，用于初始化一些容器的大小，加快处理
    */
    select_reactor(size_t max_event_number);

    /*!
    * @brief      和析构函数 virtual的喔。
    */
    virtual ~select_reactor();

public:

    /*!
    * @brief      初始化
    * @return     int              返回0表示成功，否则失败
    * @param      max_event_number 最大的句柄数量，
    */
    int initialize(size_t max_event_number);

    /*!
    * @brief      取消某些mask标志，
    * @return     int            返回0表示成功，否则失败
    * @param      event_handler  操作的句柄
    * @param      cancel_mask    要取消的MASK值
    */
    virtual int cancel_wakeup(zce::event_handler* event_handler,
                              int cancel_mask) override;

    /*!
    * @brief      打开某些mask标志，
    * @return     virtual int    返回0表示成功，否则失败
    * @param      event_handler  操作的句柄
    * @param      event_mask     要增加的MASK值
    */
    virtual int schedule_wakeup(zce::event_handler* event_handler,
                                int event_mask) override;

    /*!
    * @brief      事件触发
    * @return     int           返回0表示成功，否则失败
    * @param      time_out      超时时长
    * @param      size_event    触发的句柄数量
    */
    virtual int handle_events(zce::time_value* time_out,
                              size_t* size_event) override;

protected:

    /*!
    * @brief      处理ready的FD，调用相应的虚函数
    * @param      out_fds    句柄的fd set
    * @param      proc_mask  要处理的MASK值，内部会按照，读，写，异常的顺序进行处理，
    */
    void process_ready(const fd_set* out_fds,
                       SELECT_EVENT proc_event);

protected:

    ///最大文件句柄+1的数值,倒霉的SELECT，非要搞呀。
    int          max_fd_plus_one_ = 0;

    ///保存使用的Read FD SET
    fd_set       read_fd_set_;
    ///保存使用的Write FD SET
    fd_set       write_fd_set_;
    ///保存使用的Exception FD SET
    fd_set       exception_fd_set_;

    ///每次做作为SELECT 函数的参数
    ///
    fd_set       para_read_fd_set_;
    ///
    fd_set       para_write_fd_set_;
    ///
    fd_set       para_exception_fd_set_;
};
}
