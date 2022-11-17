#ifndef ZCE_LIB_THREAD_WAIT_MANAGER_H_
#define ZCE_LIB_THREAD_WAIT_MANAGER_H_

#include "zce/util/non_copyable.h"

namespace zce
{
/*!
* @brief      线程的等待管理器
*
* @note       本来是在线程ZCE_Thread_Base 内部处理的，但是嵌入过多，而且用大量的static 变量，
*             也影响ZCE_Thread_Base的性能
*/
class thread_wait_manager : public zce::non_copyable
{
protected:

    //记录需要等待的线程信息，通过开关使用
    struct MANAGE_WAIT_INFO
    {
    public:

        MANAGE_WAIT_INFO(ZCE_THREAD_ID wait_thr_id, int wait_group_id) :
            wait_thr_id_(wait_thr_id),
            wait_group_id_(wait_group_id)
        {
        }
        ~MANAGE_WAIT_INFO()
        {
        }
    public:
        //线程的ID
        ZCE_THREAD_ID     wait_thr_id_;
        //分组ID
        int               wait_group_id_;
    };

protected:
    //用list管理，性能不是特别好，但考虑到要中间删除因素等等，忍了
    typedef std::list <MANAGE_WAIT_INFO>   MANAGE_WAIT_THREAD_LIST;

    //单子实例
    static thread_wait_manager* instance_;

protected:

    //所有希望等待的线程记录
    MANAGE_WAIT_THREAD_LIST   wait_thread_list_;

public:
    //构造函数，允许你拥有实例，但推荐你用单件处理
    thread_wait_manager();
    ~thread_wait_manager();

    thread_wait_manager(const thread_wait_manager &) = delete;
    thread_wait_manager& operator=(const thread_wait_manager&) = delete;

    //如果需要管理处理，要自己登记，
    void record_wait_thread(ZCE_THREAD_ID wait_thr_id, int wait_group_id = 0);
    //登记一个要进行等待处理等待线程
    void record_wait_thread(const zce::thread_task* wait_thr_task);

    //等所有的线程退出
    void wait_all();

    //
    void wait_group(int group_id);

public:

    //单子函数
    static thread_wait_manager* instance();
    //清理单子的函数
    static void clear_inst();
};
}
#endif //#ifndef ZCE_LIB_THREAD_WAIT_MANAGER_H_
