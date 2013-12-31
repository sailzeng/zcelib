/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_inotify_directory_reactor.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年4月1日
* @brief      监控目录下的文件变化的封装，
* 
* @details    Linux下使用的INotify+SELECT机制，而Windows下使用ReadDirectoryChangesW的OverLapped
*             由于Windows和LINUX在监控文件变化上差别实在有点大。
*             Linux下使用的INotify+SELECT机制，而Windows下使用ReadDirectoryChangesW的OverLapped
*             1.Windows是一个异步（OverLapped）调用过程，而Linux是一个IO复用过程，很难完全一致。
*             2.Windows 下只能监听MAXIMUM_WAIT_OBJECTS个目录，而Linux 没有限制
*             3.Windows 只能监听目录，而不能监听文件
*             4.Windows 可以监控子目录，而Linux不可以。
*             5.Windows 可以监控的类型很少……，……
*             6.Linux监控的是一个设备句柄，而Windows是对每个目录的句柄进行处理
*             目前只能考虑用一个更高的层次进行封装了，不能在OS层搞掂了。
* 
*             最后吐槽一下，令人发指的Windows API，周末想把inotify在windows下封装一个
*             可以用的是ReadDirectoryChangesW + FindFirstChangeNotification ,
*             FindNextChangeNotification这一系列函数。
*             1.如果配合上Windows IO机制，使用方式就超级TMD的多了。如果你不想使用阻塞的模式，你就要和
*             Overlapped这个孩子打交道。好吧Windows IO就是这样的，这个也忍了。但
*             FindFirstChangeNotification又被折腾成是句柄+WaitForSingleObject模式，这你让人如
*             何搞。
*             2.ReadDirectoryChangesW 函数，监控的参数dwNotifyFilter 参数和返回的Action完全是两
*             回事。所以当你得到Action时，你能判定的东东其实很少。比起Inotify的的监控细分，简直不值一提。
*             FindFirstChangeNotification 函数就更加不值一提了。你知道事情发生了，你完全不知道事情是
*             什么…………
*             3.Windows 可以监控子文件夹，这个大概是Windows API可以唯一得瑟一下的东东
*             和inotify的函数一比，这组Windows API的设计者应该掩面跳楼。
* 
*             ZCE_INotify_Dir_Reactor为了兼容多个平台，有点别扭。如果只在Linux下使用，
*             而且想和Select 和Epoll Reactor一起使用，ZCE_Event_INotify是更好的选择。
* 
* 纪念张国荣先生去世10周年，
* 仍然记得那天，2003年的4月1日下班，非典仍在肆掠，百业萧条，
* 我和Linhai,Zhangke 在科技园的大冲闲逛，Linhai淘了一张
* 哥哥的碟。结果第二天就听到了这个噩耗。
* 
* 《我》
* 演唱：张国荣  歌词：林夕
* i am what i am
* 我永远都爱这样的我
* 快乐是　快乐的方式不只一种
* 最荣幸是　谁都是造物者的光荣
* 不用闪躲　为我喜欢的生活而活
* 不用粉墨　就站在光明的角落
* #我就是我　是颜色不一样的烟火
* 天空海阔　要做最坚强的泡沫
* 我喜欢我　让蔷薇开出一种结果
* 孤独的沙漠里　一样盛放的赤裸裸
* 多么高兴　在琉璃屋中快乐生活
* 对世界说　什么是光明和磊落
* 
* 
*/


#ifndef ZCE_LIB_INOTIFY_DIRECTORY_REACTOR_H_
#define ZCE_LIB_INOTIFY_DIRECTORY_REACTOR_H_


#include "zce_os_adapt_predefine.h"

class ZCE_INotify_Event_Base;



/*!
* @brief      Inotify的监控文件夹文件变化的反应器
*
*/
class ZCE_INotify_Dir_Reactor
{
public:

    ///构造函数
    ZCE_INotify_Dir_Reactor(size_t max_watch_dir = DEFUALT_MAX_WATCH_DIR_SIZE );

    ///析构函数
    ~ZCE_INotify_Dir_Reactor();

    ///打开反应器，监控各种反映
    int open();

    ///关闭文件监控反应器,释放相关资源，
    int close();


    /*!
    * @brief      增加一个要进行监控的文件对象
    * @return     int           返回0表示成功，返回-1表示失败
    * @param[in]  event_base    监控事件发生后，对应的回调处理类
    * @param[in]  pathname      监控的路径
    * @param[in]  mask          监控的选项
    * @param[out] watch_handle  返回的监控对应的句柄
    * @param[in]  watch_sub_dir 是否监控子目录，这个参数只对Windows下有效，LINUX下没效果
    */
    int add_watch(ZCE_INotify_Event_Base *event_base,
                  const char *pathname,
                  uint32_t mask,
                  ZCE_HANDLE *watch_handle,
                  bool watch_sub_dir = false);


    /*!
    * @brief      通过文件句柄，移除一个要监控的项目，
    * @return     int          返回0表示成功，返回-1表示失败
    * @param[in]  watch_handle 监控目录的文件句柄
    */
    int rm_watch(ZCE_HANDLE watch_handle);

    /*!
    * @brief      通过事件对象，移除所有相关的要监控的项目，此函数用迭代器在Hash内部遍历，速度会比较慢，
    * @return     int        返回0表示成功，返回-1表示失败
    * @param[in]  event_base 要取消监控注册的事件指针，
    */
    int rm_watch(const ZCE_INotify_Event_Base *event_base);


    /*!
    * @brief      触发反应器反应的函数，如果有事件发生，这个函数会回调相应的ZCE_INotify_Event_Base
    * @return     int               返回0表示正常
    * @param[in]  time_out          反应等待的超时时间
    * @param[in]  watch_event_num   监控的事件数量
    */
    int watch_event(ZCE_Time_Value *time_out, size_t *watch_event_num);


protected:


    ///BUFFER的长度
    static const size_t     READ_BUFFER_LEN = 16 * 1024 - 1;

#if defined (ZCE_OS_WINDOWS)
    ///默认的监控的目录最大个数，Windows 最大只能有MAXIMUM_WAIT_OBJECTS个
    static  const size_t    DEFUALT_MAX_WATCH_DIR_SIZE = MAXIMUM_WAIT_OBJECTS;
#elif defined (ZCE_OS_LINUX)
    ///默认的监控的目录最大个数
    static  const size_t    DEFUALT_MAX_WATCH_DIR_SIZE = 256;
#endif

    ///进行文件监控的节点
    struct ZCE_INOTIFY_NODE
    {
        ZCE_INOTIFY_NODE():
            watch_handle_(ZCE_INVALID_HANDLE),
            watch_mask_(0),
            watch_event_(NULL)
        {

#if defined (ZCE_OS_WINDOWS)
            memset((void *)&over_lapped_, 0, sizeof(OVERLAPPED));
            read_buffer_ = NULL;
            watch_sub_dir_ = FALSE;
#endif
        }
        ~ZCE_INOTIFY_NODE()
        {
        }

        ///监控的句柄
        ZCE_HANDLE              watch_handle_;
        ///监视的文件路径
        char                    watch_path_[MAX_PATH];
        ///监控项的掩码
        uint32_t                watch_mask_;

        ///监控事件的回调事件类
        ZCE_INotify_Event_Base *watch_event_;

#if defined (ZCE_OS_WINDOWS)
        ///读取的Buffer，
        char                   *read_buffer_;
        ///over lapped 使用的对象
        OVERLAPPED              over_lapped_;
        ///是否监控子目录
        BOOL                    watch_sub_dir_;
#endif

    };

    typedef unordered_map<ZCE_HANDLE, ZCE_INOTIFY_NODE >  HANDLE_TO_EVENT_MAP;

    ///反应器管理的目录节点信息的MAP,
    HANDLE_TO_EVENT_MAP watch_event_map_;


#if defined (ZCE_OS_WINDOWS)
    ///WaitForMultipleObjects等待的目录句柄
    ///为什么要有这个重复的结构，主要是为了方便使用WaitForMultipleObjects的速度
    ZCE_HANDLE          watch_handle_ary_[MAXIMUM_WAIT_OBJECTS];

#elif defined (ZCE_OS_LINUX)
    ///inotify_init 初始化得到的句柄
    ZCE_HANDLE          inotify_handle_;

    char               *read_buffer_;
#endif


};



#endif //ZCE_LIB_INOTIFY_DIRECTORY_REACTOR_H_


