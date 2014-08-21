/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_handle_inotify.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年9月22日
*
* @brief      一个用于在Linux下处理Inotify的事件句柄基类，
*             可以监听多个目录的反映，用于监控文件系统的变化。
*             这个类的目的是和Reactor类兼容，而且更加自然
*             如果你希望跨平台，ZCE_INotify_Dir_Reactor 也许是更好的选择
*             但ZCE_INotify_Dir_Reactor为了兼容多个平台，有点别扭。
*
* @details
*
* @note       Kliu提醒，Epoll也可以用于处理Inotify的时间反应器，
*             特此修正，表示感谢。
*
*/

#ifndef ZCE_LIB_EVENT_HANDLE_INOTIFY_H_
#define ZCE_LIB_EVENT_HANDLE_INOTIFY_H_

//此代码只能在LINUX环境下跑


#include "zce_event_handle_base.h"

class ZCE_Reactor;

/*!
@brief      INotify 事件处理的句柄，只能在Linux下使用，可以使用ZCE_Select_Reactor，ZCE_Epoll_Reactor
作为反应器，
被调用
*/
class ZCELIB_EXPORT ZCE_Event_INotify : public ZCE_Event_Handler
{


public:


    /*!
    @brief      构造函数，同时设置香港的反应器指针
    @param      reactor 句柄相关的反应器指针
    */
    ZCE_Event_INotify(void);
    /*!
    @brief      析构函数
    */
    virtual ~ZCE_Event_INotify();

public:

    /*!
    @brief      打开监控句柄等，绑定reactor等
    @param      reactor_base 句柄相关的反应器指针,
    @return     返回0表示成功，否则失败
    */
    int open(ZCE_Reactor *reactor_base);

    /*!
    @brief      关闭监控句柄等，取消绑定reactor等
    @return     返回0表示成功，否则失败
    */
    int close();



    /*!
    @brief      取回对应的ZCE_SOCKET 句柄
    @return     int ZCE_Event_INotify 对应的句柄，注意LINUX下句柄和ZCE_SOCKET都是int
    */
    virtual ZCE_HANDLE get_handle (void) const
    {
#if defined ZCE_OS_LINUX
        return inotify_handle_;
#else if defined ZCE_OS_WINDOWS
        return watch_handle_;
#endif
    }


    /*!
    @brief      增加一个要进行监控的文件对象
    @return     int           返回0表示成功，返回-1表示失败
    @param[in]  pathname      监控的路径
    @param[in]  mask          监控的选项
    @param[out] watch_handle  返回的监控对应的句柄
    @param[in]  watch_sub_dir 是否监控子目录，此参数只在Windows下有用，
    */
    int add_watch(const char *pathname,
        uint32_t mask,
        ZCE_HANDLE *watch_handle,
        bool watch_sub_dir = false);


    /*!
    @brief      通过文件句柄，移除一个要监控的项目，
    @return     int          返回0表示成功，返回-1表示失败
    @param[in]  watch_handle 监控目录的文件句柄,Windwos 下这个参数无效
    */
    int rm_watch(ZCE_HANDLE watch_handle);


    /*!
    @brief      读取事件触发调用函数，用于读取数据，当有时间发生时，这个函数被回调，
                函数内部分析具体发生的事件，
    @return     int 返回0表示句柄处理正常，return -1后，反应器会主动handle_close，帮助结束句柄
    */
    virtual int handle_input ();

    /*!
    @brief
    @return     int
    */
    virtual int handle_close ();

    ///需要你继承使用的虚函数，你关注什么事件，就重载什么函数
protected:

    /*!
    @brief      监测到有文件，目录创建，的回调函数，如果你需要处理这种行为，清继承重载，
                对应掩码IN_CREATE，下面函数的参数都类似，请参考inotify_create，
    @return     int          返回0表示成功，返回-1表示
    @param[in]  watch_handle 监控文件句柄，注意是监控的句柄，不是操作文件的句柄
    @param[in]  watch_mask   监控发生的行为的掩码，可以通过掩码判断是文件还是目录
    @param[in]  watch_path   监控的路径
    @param[in]  active_path  发生动作，行为的文件或者目录的路径
    */
    virtual int inotify_create(ZCE_HANDLE watch_handle,
                               uint32_t watch_mask,
                               const char *watch_path,
                               const char *active_path)
    {
        ZCE_UNUSED_ARG(watch_handle);
        ZCE_UNUSED_ARG(watch_mask);
        ZCE_UNUSED_ARG(watch_path);
        ZCE_UNUSED_ARG(active_path);
        return 0;
    }

    ///监测到有删除文件或者目录,对应掩码IN_DELETE，参数说明参考@fun inotify_create
    virtual int inotify_delete(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///监测到有文件被修改,对应掩码IN_MODIFY，参数说明参考@fun inotify_create
    virtual int inotify_modify(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///监控文件从某个目录移动出去，IN_MOVED_FROM,参数说明参考@fun inotify_create
    virtual int inotify_moved_from(ZCE_HANDLE /*watch_handle*/,
                                   uint32_t /*watch_mask*/,
                                   const char * /*watch_path*/,
                                   const char * /*active_path*/)
    {
        return 0;
    }

    ///监控文件移动到某个目录，IN_MOVED_TO,(我自己测试只有在监控目录下移动才会发生这个事件),
    ///参数说明参考@fun inotify_create
    virtual int inotify_moved_to(ZCE_HANDLE /*watch_handle*/,
                                 uint32_t /*watch_mask*/,
                                 const char * /*watch_path*/,
                                 const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控目录下文件被访问时被回调，IN_ACCESS,参数说明参考@fun inotify_create
    virtual int inotify_access(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控目录下文件被打开时被回调，IN_OPEN,参数说明参考@fun inotify_create
    virtual int inotify_open(ZCE_HANDLE /*watch_handle*/,
                             uint32_t /*watch_mask*/,
                             const char * /*watch_path*/,
                             const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控目录下文件被关闭事件时被回调，IN_CLOSE_WRITE,IN_CLOSE_NOWRITE,
    ///参数说明参考@fun inotify_create
    virtual int inotify_close(ZCE_HANDLE /*watch_handle*/,
                              uint32_t /*watch_mask*/,
                              const char * /*watch_path*/,
                              const char * /*active_path*/)
    {
        return 0;
    }

    ///发生目录下有文件或者目录属性被修改事件时被回调，IN_ATTRIB， permissions, timestamps,
    ///extended attributes, link count (since Linux 2.6.25), UID, GID,
    ///参数说明参考@fun inotify_create
    virtual int inotify_attrib(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控的目录被移动时被回调，IN_MOVE_SELF,参数说明参考@fun inotify_create
    virtual int inotify_move_slef(ZCE_HANDLE /*watch_handle*/,
                                  uint32_t /*watch_mask*/,
                                  const char * /*watch_path*/,
                                  const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控的目录被删除时被回调，IN_DELETE_SELF,参数说明参考@fun inotify_create
    virtual int inotify_delete_slef(ZCE_HANDLE /*watch_handle*/,
                                    uint32_t /*watch_mask*/,
                                    const char * /*watch_path*/,
                                    const char * /*active_path*/)
    {
        return 0;
    }


protected:

    ///进行文件监控的节点
    struct EVENT_INOTIFY_NODE
    {
        EVENT_INOTIFY_NODE():
            watch_handle_(ZCE_INVALID_HANDLE),
            watch_mask_(0)
        {
            watch_path_[0] = '\0';
        }
        ~EVENT_INOTIFY_NODE()
        {
        }

        ///监控的句柄
        ZCE_HANDLE              watch_handle_;
        ///监视的文件路径
        char                    watch_path_[MAX_PATH];
        ///监控项的掩码
        uint32_t                watch_mask_;

    };

protected:

    ///BUFFER的长度
    static const size_t     READ_BUFFER_LEN = 16 * 1024 - 1;

protected:

#if defined ZCE_OS_LINUX
    ///EINN是Event，Inotify Node的缩写
    typedef unordered_map<ZCE_HANDLE, EVENT_INOTIFY_NODE >  HDL_TO_EIN_MAP;
    ///反应器管理的目录节点信息的MAP,
    HDL_TO_EIN_MAP     watch_event_map_;

    ///inotify_init 初始化得到的句柄
    int                inotify_handle_;
    

#elif defined ZCE_OS_WINDOWS

    ///监控的句柄
    ZCE_HANDLE        watch_handle_;
    ///监视的文件路径
    char              watch_path_[MAX_PATH];
    ///监控项的掩码
    uint32_t          watch_mask_;
    ///over lapped 使用的对象
    OVERLAPPED        over_lapped_;
    ///是否监控子目录
    BOOL              watch_sub_dir_;
#endif

    ///读取的Buffer，
    char              *read_buffer_;




};


#endif //ZCE_LIB_EVENT_HANDLE_INOTIFY_H_

