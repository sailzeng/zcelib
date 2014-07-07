/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_inotify.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年3月28日
* @brief      监控目录变化的事件处理类
*
*
* @details
*
*
*
* @note       我在是否使用ZCE_INotify_Event_Base是可以同时监控多个目录，还是
*             只能监控一个目录上犹豫了半天。
*             我考虑了半天，觉得还是简单让event可以监控多个目录算了，因为这样这样
*             还简单。关键是这个event只是一个被触发的角色，其实他不需要关心目录，
*             也就不需要知道watch_hadnle这类东东。
*
*
*/


#ifndef ZCE_LIB_INOTIFY_EVENT_BASE_H_
#define ZCE_LIB_INOTIFY_EVENT_BASE_H_


#include "zce_os_adapt_predefine.h"




/*!
* @brief      监控目录变化的事件处理类，你来填充各种回调函数。
*             ZCE_INotify_Dir_Reactor 负责等待触发，
*
*/

class ZCE_INotify_Dir_Reactor;

class ZCE_INotify_Event_Base
{
    //只有反应器可以调用
    friend class ZCE_INotify_Dir_Reactor;

    //
public:

    ///构造函数
    ZCE_INotify_Event_Base();
    ///析构函数
    virtual ~ZCE_INotify_Event_Base();

    ///设置反应器
    void set_reactor(ZCE_INotify_Dir_Reactor *reactor);
    ///取得反应器
    ZCE_INotify_Dir_Reactor *get_reactor();

    //反应器的回调函数，当方式这个事件时，反应器进行回调
protected:


    /*!
    @brief      关闭监视器的回调函数，
    @return     int
    */
    virtual int watch_close();

    //===================================================================================================
    //下面这组（5个）是Windows 下可以支持的监控项目（LINUX也有）
    //如果希望通用，最好只用这5个，（甚至是前3个）

    /*!
    * @brief      监测到有文件，目录创建，的回调函数，如果你需要处理这种行为，清继承重载，
    *             对应掩码IN_CREATE，下面函数的参数都类似，请参考inotify_create，
    * @return     int          返回0表示成功，返回-1表示
    * @param[in]  watch_handle 监控文件句柄，注意是监控的句柄，不是操作文件的句柄
    * @param[in]  watch_mask   监控发生的行为的掩码，可以通过掩码判断是文件还是目录
    * @param[in]  watch_path   监控的路径
    * @param[in]  active_path  发生动作，行为的文件或者目录的路径
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

    //===================================================================================================
    //下面这些是LINUX特有的一些监控项目，加这个宏到不是为了编译，是避免你误使用

#if defined (ZCE_OS_LINUX)

    ///发生监控目录下文件被访问时被回调，IN_ACCESS,参数说明参考@fun inotify_create
    virtual int inotify_access(int /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控目录下文件被打开时被回调，IN_OPEN,参数说明参考@fun inotify_create
    virtual int inotify_open(int /*watch_handle*/,
                             uint32_t /*watch_mask*/,
                             const char * /*watch_path*/,
                             const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控目录下文件被关闭事件时被回调，IN_CLOSE_WRITE,IN_CLOSE_NOWRITE,
    ///参数说明参考@fun inotify_create
    virtual int inotify_close(int /*watch_handle*/,
                              uint32_t /*watch_mask*/,
                              const char * /*watch_path*/,
                              const char * /*active_path*/)
    {
        return 0;
    }

    ///发生目录下有文件或者目录属性被修改事件时被回调，IN_ATTRIB， permissions, timestamps,
    ///extended attributes, link count (since Linux 2.6.25), UID, GID,
    ///参数说明参考@fun inotify_create
    virtual int inotify_attrib(int /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控的目录被移动时被回调，IN_MOVE_SELF,参数说明参考@fun inotify_create
    virtual int inotify_move_slef(int /*watch_handle*/,
                                  uint32_t /*watch_mask*/,
                                  const char * /*watch_path*/,
                                  const char * /*active_path*/)
    {
        return 0;
    }

    ///发生监控的目录被删除时被回调，IN_DELETE_SELF,参数说明参考@fun inotify_create
    virtual int inotify_delete_slef(int /*watch_handle*/,
                                    uint32_t /*watch_mask*/,
                                    const char * /*watch_path*/,
                                    const char * /*active_path*/)
    {
        return 0;
    }

#endif


protected:

    //
    ZCE_INotify_Dir_Reactor  *inotify_reactor_;
};






#endif //ZCE_LIB_INOTIFY_EVENT_BASE_H_


