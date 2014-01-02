
#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_
#define SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_

#include "soar_fsm_trans_base.h"
#include "soar_fsm_notify_trans_mgr.h"

//有点怀疑我是不是模板变态了。TMD。
/******************************************************************************************
class Notify_Trans_Base
******************************************************************************************/
template <class _ZCE_SYNCH >
class Notify_Trans_Base : public Transaction_Base
{
protected:

    //管理器
    NotifyTrans_Manger<_ZCE_SYNCH>    *trans_notify_mgr_;

public:

    //这而要求传递的Transaction_Manager *是子类NotifyTrans_Manger的，
    //必须如此。
    //这儿是一个典型的设计两难，如果使用向下朔型DOWNCAST，无意是丑陋的。
    //但是我觉得那部分功能还是在NotifyTrans_Manger比较好。用virual函数也不是太好,甚至不可能，我用了模板函数。我。。。。
    Notify_Trans_Base(ZCE_Timer_Queue *timer_queue, Transaction_Manager *trans_notify_mgr)
        : Transaction_Base(timer_queue, trans_notify_mgr)
        , trans_notify_mgr_(NULL)
    {
        //必须要求trans_notify_mgr至少是trans_notify_mgr的子类，
        trans_notify_mgr_ = dynamic_cast<NotifyTrans_Manger<_ZCE_SYNCH>* > (trans_notify_mgr);
        ZCE_ASSERT(trans_notify_mgr_);
    }

protected:
    //
    virtual ~Notify_Trans_Base()
    {
    };

protected:

    //
    template <class T>
    int pushbak_mgr_sendqueue(unsigned int cmd,
                              const T &info,
                              unsigned int option = 0)
    {
        SERVICES_ID proxy_svc(0, 0);
        return trans_notify_mgr_->enqueue_sendqueue(cmd,
                                                    this->req_qq_uin_,
                                                    this->transaction_id_,
                                                    0,
                                                    info,
                                                    this->req_game_app_id_,
                                                    option);
    };

};

//两个typedef

//用线程锁的类Notify Trans 类
typedef Notify_Trans_Base<ZCE_MT_SYNCH> MT_NOTIFY_TRANS_BASE ;
//无锁的类Notify Trans 类
typedef Notify_Trans_Base<ZCE_NULL_SYNCH> NULL_NOTIFY_TRANS_BASE ;

/******************************************************************************************
class Notify_Trans_Abnormal_Base 在某些特殊时候(避免多次拷贝),时使用，除非对性能有强烈的爱好，否则不要用
他使用的前提是：
1.你是强烈的性能爱好者，希望避免一次拷贝
2.你知道你要使用的Frame的长度,可以自己进行管理分配
3.你知道自己在做什么，并且愿意承担后果

使用Notify_Trans_Abnormal_Base的过程必须是
malloc_abnormalframe
使用get_abnormal_frame，
******************************************************************************************/
class Notify_Trans_Abnormal_Base : public Notify_Trans_Base<ZCE_MT_SYNCH>
{

    //设置成private是有目的的，就是不让你用。
private:
    //在某些特殊时候(避免多次拷贝)，要自己管理向SEND QUEUE发送的FRAME
    Zerg_App_Frame                        *abnormal_frame_;

public:
    //构造函数
    Notify_Trans_Abnormal_Base(ZCE_Timer_Queue *timer_queue, Transaction_Manager *trans_notify_mgr);
protected:
    //
    virtual ~Notify_Trans_Abnormal_Base();
public:

    //分配得到一个某个长度的APP Frame
    void malloc_abnormalframe(size_t frame_len);

    //取得FRAME进行操作，不要改变长度等FRAME HEAD的数据，数
    Zerg_App_Frame *get_abnormal_frame();
    //取得FRAME的数据区用于操作
    char *get_abnormal_framedata();

    //投送
    int pushbak_mgr_sendqueue(unsigned int cmd,
                              unsigned int option);

    //增加了回收sndqueue_frame_的功能
    virtual void finish();
};

#endif //#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_

