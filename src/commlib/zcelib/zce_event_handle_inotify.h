/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_handle_inotify.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��4��1��
*
* @brief      һ��������Linux��Windows��Inotify���¼�������࣬���Ŀ¼�µ��ļ��仯�ķ�װ��
*             ���Լ������Ŀ¼�ķ�ӳ�����ڼ���ļ�ϵͳ�ı仯��
*             ������Ŀ���Ǻ�Reactor����ݣ����Ҹ�����Ȼ
*             Linux ����ʹ��EPOLL Reactor��Select Reactor
*             Windows ��ֻ��ʹ��WFMO Reactor
*
* @details    Linux��ʹ�õ�INotify+SELECT���ƣ���Windows��ʹ��ReadDirectoryChangesW��OverLapped
*             ����Windows��LINUX�ڼ���ļ��仯�ϲ��ʵ���е��
*             Linux��ʹ�õ�INotify+SELECT���ƣ���Windows��ʹ��ReadDirectoryChangesW��OverLapped
*             1.Windows��һ���첽��OverLapped�����ù��̣���Linux��һ��IO���ù��̣�������ȫһ�¡�
*             2.Windows ��ֻ�ܼ���MAXIMUM_WAIT_OBJECTS��Ŀ¼����Linux û������
*             3.Windows ֻ�ܼ���Ŀ¼�������ܼ����ļ�
*             4.Windows ���Լ����Ŀ¼����Linux�����ԡ�
*             5.Windows ���Լ�ص����ͺ��١���������
*             6.Linux��ص���һ���豸�������Windows�Ƕ�ÿ��Ŀ¼�ľ�����д���
*             Ŀǰֻ�ܿ�����һ�����ߵĲ�ν��з�װ�ˣ�������OS�����ˡ�
*
*             ����²�һ�£����˷�ָ��Windows API����ĩ���inotify��windows�·�װһ��
*             �����õ���ReadDirectoryChangesW + FindFirstChangeNotification ,
*             FindNextChangeNotification��һϵ�к�����
*             1.��������Windows IO���ƣ�ʹ�÷�ʽ�ͳ���TMD�Ķ��ˡ�����㲻��ʹ��������ģʽ�����Ҫ��
*             Overlapped������Ӵ򽻵����ð�Windows IO���������ģ����Ҳ���ˡ���
*             FindFirstChangeNotification�ֱ����ڳ��Ǿ��+WaitForSingleObjectģʽ������������
*             �θ㡣
*             2.ReadDirectoryChangesW ��������صĲ���dwNotifyFilter �����ͷ��ص�Action��ȫ����
*             ���¡����Ե���õ�Actionʱ�������ж��Ķ�����ʵ���١�����Inotify�ĵļ��ϸ�֣���ֱ��ֵһ�ᡣ
*             FindFirstChangeNotification �����͸��Ӳ�ֵһ���ˡ���֪�����鷢���ˣ�����ȫ��֪��������
*             ʲô��������
*             3.Windows ���Լ�����ļ��У���������Windows API����Ψһ��ɪһ�µĶ���
*             ��inotify�ĺ���һ�ȣ�����Windows API�������Ӧ��������¥��
*
*
*             �������Select ��Epoll Reactorһ��ʹ�ã�ZCE_Event_INotify�Ǹ��õ�ѡ��
*             Kliu���ѣ�EpollҲ�������ڴ���Inotify��ʱ�䷴Ӧ�����ش���������ʾ��л��
*
*
* @note      ԭ��ΪInotify����д��һ����Ӧ����event ���룬����Kliu��Ȱ���¿������������ǿ���
*            ��Linux�����÷�Ӧ��ʹ�������event handle����������ʵ����WFMO ��Reactor��
*            �Ͱ�Windows ���ֵĴ���Ҳͳһ�����ˣ�
*
*
* �����Ź�������ȥ��10���꣬
* ��Ȼ�ǵ����죬2003���4��1���°࣬�ǵ��������ӣ���ҵ������
* �Һ�Linhai,Zhangke �ڿƼ�԰�Ĵ���й䣬Linhai����һ��
* ���ĵ�������ڶ�������������ج�ġ���Щ�¾��������ɡ�
*
* ���ҡ�
* �ݳ����Ź���  ��ʣ���Ϧ
* i am what i am
* ����Զ������������
* �����ǡ����ֵķ�ʽ��ֻһ��
* �������ǡ�˭���������ߵĹ���
* �������㡡Ϊ��ϲ�����������
* ���÷�ī����վ�ڹ����Ľ���
* #�Ҿ����ҡ�����ɫ��һ�����̻�
* ��պ�����Ҫ�����ǿ����ĭ
* ��ϲ���ҡ���Ǿޱ����һ�ֽ��
* �¶���ɳĮ�һ��ʢ�ŵĳ�����
* ��ô���ˡ����������п�������
* ������˵��ʲô�ǹ���������
*
*/

#ifndef ZCE_LIB_EVENT_HANDLE_INOTIFY_H_
#define ZCE_LIB_EVENT_HANDLE_INOTIFY_H_



#include "zce_event_handle_base.h"

class ZCE_Reactor;

/*!
@brief      INotify �¼�����ľ������Linux��ʹ�ã�����ʹ��ZCE_Select_Reactor��ZCE_Epoll_Reactor
            ��Ϊ��Ӧ����������
            ��Windows �£�Ҫ��ZCE_WFMO_Reactor��
*/
class ZCE_Event_INotify : public ZCE_Event_Handler
{


public:


    /*!
    @brief      ���캯����ͬʱ������۵ķ�Ӧ��ָ��
    @param      reactor �����صķ�Ӧ��ָ��
    */
    ZCE_Event_INotify(void);
    /*!
    @brief      ��������
    */
    virtual ~ZCE_Event_INotify();

public:

    /*!
    @brief      �򿪼�ؾ���ȣ���reactor��
    @param      reactor_base �����صķ�Ӧ��ָ��,
    @return     ����0��ʾ�ɹ�������ʧ��
    */
    int open(ZCE_Reactor *reactor_base);

    /*!
    @brief      �رռ�ؾ���ȣ�ȡ����reactor��
    @return     ����0��ʾ�ɹ�������ʧ��
    */
    int close();



    /*!
    @brief      ȡ�ض�Ӧ��ZCE_SOCKET ���
    @return     int ZCE_Event_INotify ��Ӧ�ľ����ע��LINUX�¾����ZCE_SOCKET����int
    */
    virtual ZCE_HANDLE get_handle (void) const
    {
#if defined ZCE_OS_LINUX
        return inotify_handle_;
#elif defined ZCE_OS_WINDOWS
        return watch_handle_;
#endif
    }


    /*!
    @brief      ����һ��Ҫ���м�ص��ļ�����
    @return     int           ����0��ʾ�ɹ�������-1��ʾʧ��
    @param[in]  pathname      ��ص�·��
    @param[in]  mask          ��ص�ѡ��
    @param[out] watch_handle  ���صļ�ض�Ӧ�ľ��
    @param[in]  watch_sub_dir �Ƿ�����Ŀ¼���˲���ֻ��Windows�����ã�
    */
    int add_watch(const char *pathname,
                  uint32_t mask,
                  ZCE_HANDLE *watch_handle,
                  bool watch_sub_dir = false);


    /*!
    @brief      ͨ���ļ�������Ƴ�һ��Ҫ��ص���Ŀ��
    @return     int          ����0��ʾ�ɹ�������-1��ʾʧ��
    @param[in]  watch_handle ���Ŀ¼���ļ����,Windwos �����������Ч
    */
    int rm_watch(ZCE_HANDLE watch_handle);


    /*!
    @brief      ��ȡ�¼��������ú��������ڶ�ȡ���ݣ�����ʱ�䷢��ʱ������������ص���
                �����ڲ��������巢�����¼���
    @return     int ����0��ʾ�������������return -1�󣬷�Ӧ��������handle_close�������������
    */
    virtual int handle_input ();

    /*!
    @brief
    @return     int
    */
    virtual int handle_close ();

    ///��Ҫ��̳�ʹ�õ��麯�������עʲô�¼���������ʲô����
protected:

    /*!
    @brief      ��⵽���ļ���Ŀ¼�������Ļص��������������Ҫ����������Ϊ����̳����أ�
                ��Ӧ����IN_CREATE�����溯���Ĳ��������ƣ���ο�inotify_create��
    @return     int          ����0��ʾ�ɹ�������-1��ʾ
    @param[in]  watch_handle ����ļ������ע���Ǽ�صľ�������ǲ����ļ��ľ��
    @param[in]  watch_mask   ��ط�������Ϊ�����룬����ͨ�������ж����ļ�����Ŀ¼
    @param[in]  watch_path   ��ص�·��
    @param[in]  active_path  ������������Ϊ���ļ�����Ŀ¼��·��
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

    ///��⵽��ɾ���ļ�����Ŀ¼,��Ӧ����IN_DELETE������˵���ο�@fun inotify_create
    virtual int inotify_delete(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///��⵽���ļ����޸�,��Ӧ����IN_MODIFY������˵���ο�@fun inotify_create
    virtual int inotify_modify(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///����ļ���ĳ��Ŀ¼�ƶ���ȥ��IN_MOVED_FROM,����˵���ο�@fun inotify_create
    virtual int inotify_moved_from(ZCE_HANDLE /*watch_handle*/,
                                   uint32_t /*watch_mask*/,
                                   const char * /*watch_path*/,
                                   const char * /*active_path*/)
    {
        return 0;
    }

    ///����ļ��ƶ���ĳ��Ŀ¼��IN_MOVED_TO,(���Լ�����ֻ���ڼ��Ŀ¼���ƶ��Żᷢ������¼�),
    ///����˵���ο�@fun inotify_create
    virtual int inotify_moved_to(ZCE_HANDLE /*watch_handle*/,
                                 uint32_t /*watch_mask*/,
                                 const char * /*watch_path*/,
                                 const char * /*active_path*/)
    {
        return 0;
    }

    ///�������Ŀ¼���ļ�������ʱ���ص���IN_ACCESS,����˵���ο�@fun inotify_create
    virtual int inotify_access(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///�������Ŀ¼���ļ�����ʱ���ص���IN_OPEN,����˵���ο�@fun inotify_create
    virtual int inotify_open(ZCE_HANDLE /*watch_handle*/,
                             uint32_t /*watch_mask*/,
                             const char * /*watch_path*/,
                             const char * /*active_path*/)
    {
        return 0;
    }

    ///�������Ŀ¼���ļ����ر��¼�ʱ���ص���IN_CLOSE_WRITE,IN_CLOSE_NOWRITE,
    ///����˵���ο�@fun inotify_create
    virtual int inotify_close(ZCE_HANDLE /*watch_handle*/,
                              uint32_t /*watch_mask*/,
                              const char * /*watch_path*/,
                              const char * /*active_path*/)
    {
        return 0;
    }

    ///����Ŀ¼�����ļ�����Ŀ¼���Ա��޸��¼�ʱ���ص���IN_ATTRIB�� permissions, timestamps,
    ///extended attributes, link count (since Linux 2.6.25), UID, GID,
    ///����˵���ο�@fun inotify_create
    virtual int inotify_attrib(ZCE_HANDLE /*watch_handle*/,
                               uint32_t /*watch_mask*/,
                               const char * /*watch_path*/,
                               const char * /*active_path*/)
    {
        return 0;
    }

    ///������ص�Ŀ¼���ƶ�ʱ���ص���IN_MOVE_SELF,����˵���ο�@fun inotify_create
    virtual int inotify_move_slef(ZCE_HANDLE /*watch_handle*/,
                                  uint32_t /*watch_mask*/,
                                  const char * /*watch_path*/,
                                  const char * /*active_path*/)
    {
        return 0;
    }

    ///������ص�Ŀ¼��ɾ��ʱ���ص���IN_DELETE_SELF,����˵���ο�@fun inotify_create
    virtual int inotify_delete_slef(ZCE_HANDLE /*watch_handle*/,
                                    uint32_t /*watch_mask*/,
                                    const char * /*watch_path*/,
                                    const char * /*active_path*/)
    {
        return 0;
    }


protected:

    ///�����ļ���صĽڵ�
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

        ///��صľ��
        ZCE_HANDLE              watch_handle_;
        ///���ӵ��ļ�·��
        char                    watch_path_[MAX_PATH];
        ///����������
        uint32_t                watch_mask_;

    };

protected:

    ///BUFFER�ĳ���
    static const size_t     READ_BUFFER_LEN = 16 * 1024 - 1;

protected:

#if defined ZCE_OS_LINUX
    ///EINN��Event��Inotify Node����д
    typedef std::unordered_map<ZCE_HANDLE, EVENT_INOTIFY_NODE >  HDL_TO_EIN_MAP;
    ///��Ӧ�������Ŀ¼�ڵ���Ϣ��MAP,
    HDL_TO_EIN_MAP     watch_event_map_;

    ///inotify_init ��ʼ���õ��ľ��
    int                inotify_handle_;


#elif defined ZCE_OS_WINDOWS

    ///��صľ��
    ZCE_HANDLE        watch_handle_;
    ///���ӵ��ļ�·��
    char              watch_path_[MAX_PATH];
    ///����������
    uint32_t          watch_mask_;
    ///over lapped ʹ�õĶ���
    OVERLAPPED        over_lapped_;
    ///�Ƿ�����Ŀ¼
    BOOL              watch_sub_dir_;
#endif

    ///��ȡ��Buffer��
    char              *read_buffer_;

};


#endif //ZCE_LIB_EVENT_HANDLE_INOTIFY_H_

