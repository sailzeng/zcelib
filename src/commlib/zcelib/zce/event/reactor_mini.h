#pragma once

#include "zce/os_adapt/common.h"

namespace zce
{
/*!
* @brief      MINI反应器
*
*/
typedef std::function <int(ZCE_HANDLE socket,
    EVENT_MASK event)> event_callback_t;

class reactor_mini
{
protected:

    struct EVENT_CALL
    {
        EVENT_CALL(ZCE_HANDLE handle,
                   EVENT_MASK event_todo,
                   event_callback_t call_back) :
            handle_(handle),
            event_todo_(event_todo),
            call_back_(call_back)
        {
        }
        EVENT_CALL(ZCE_HANDLE handle,
                   EVENT_MASK event_todo) :
            handle_(handle),
            event_todo_(event_todo)
        {
        }
        ~EVENT_CALL() = default;
        //
        ZCE_HANDLE handle_ = ZCE_INVALID_HANDLE;
        //
        EVENT_MASK event_todo_ = NULL_MASK;
        //回调函数
        event_callback_t call_back_;
    };

    struct hash_event_call
    {
        size_t operator()(const EVENT_CALL &obj) const
        {
            return (size_t)(obj.handle_) + obj.event_todo_;
        }
    };

    struct equal_to_event_call
    {
        bool operator()(const EVENT_CALL &obj1, const EVENT_CALL &obj2) const
        {
            if (obj1.handle_ == obj2.handle_ &&
                obj1.event_todo_ == obj2.event_todo_)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    typedef std::unordered_set<EVENT_CALL,
        hash_event_call,
        equal_to_event_call> EVENT_CALL_SET;

protected:

    /*!
    * @brief      构造函数
    */
    reactor_mini();
    /*!
    * @brief      构造函数
    * @param[in]  max_event_number 最大的容量，
    */
    reactor_mini(size_t max_event_number);
    /*!
    * @brief      析构函数，virtual的喔
    */
    virtual ~reactor_mini();

public:

    /*!
    * @brief      当前反应器容器的句柄数量
    * @return     size_t 返回的当前在反应器的句柄数量
    */
    size_t size();

    /*!
    * @brief      反应器的最大尺寸，容量，
    * @return     size_t 返回容量
    */
    size_t max_size();

    //
    int initialize(size_t max_event_number);

    //
    virtual int close();

    //
    int register_event(ZCE_HANDLE handle,
                       EVENT_MASK event_todo,
                       event_callback_t call_back);

    //
    int remove_event(ZCE_HANDLE handle,
                     EVENT_MASK event_todo);

    //
    bool find_event(ZCE_HANDLE handle,
                    EVENT_MASK event_todo);

    //
    int handle_events(zce::time_value* time_out,
                      size_t* size_event);

public:

    /*!
    * @brief      获取单子函数
    * @return     reactor* 反应器的指针
    */
    static reactor_mini* instance();
    ///清理单子函数
    static void clear_inst();
    ///设置单子的函数
    static void instance(reactor_mini* inst);

protected:
    ///单子实例指针
    static reactor_mini* instance_;

protected:

    ///存放ZCE_SOCKET对应zce::event_handler *的MAP,方便事件触发的时候，调用zce::event_handler *的函数
    EVENT_CALL_SET    event_set_;

    ///最大的处理句柄大小，用于一些容器的resize
    size_t            max_event_number_;

    //! Windows 下用select 进行事件处理
#if defined (ZCE_OS_WINDOWS)
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
#elif defined (ZCE_OS_LINUX)

#endif
};
}
