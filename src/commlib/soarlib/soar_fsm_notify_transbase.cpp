#include "soar_predefine.h"

#include "soar_fsm_notify_transbase.h"

/******************************************************************************************
class Notify_Trans_Base
******************************************************************************************/

/******************************************************************************************
class Notify_Trans_Abnormal_Base 在某些特殊时候(避免多次拷贝),时使用，除非对性能有强烈的爱好，否则不要用
******************************************************************************************/
Notify_Trans_Abnormal_Base::Notify_Trans_Abnormal_Base(ZCE_Timer_Queue *timer_queue, Transaction_Manager *trans_notify_mgr)
    : Notify_Trans_Base<ZCE_MT_SYNCH>(timer_queue, trans_notify_mgr)
    , abnormal_frame_(NULL)
{
    //必须要求trans_notify_mgr至少是trans_notify_mgr的子类，
}

Notify_Trans_Abnormal_Base::~Notify_Trans_Abnormal_Base()
{
};

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2008年9月22日
Function        : MallocSendQueueFrame 请在使用这个函数之前明确你要干什么，
Return          : void
Parameter List  :
Param1: size_t frame_len 你要分配的FRAME的长度
Description     :  分配一个想发送队列发送FRAME
Calls           :
Called By       :
Other           : 这个函数仅仅提供给希望避免大量FRAME拷贝的地方,不要改变FRAME的头部数据,
Modify Record   : 和GetSendQueueFrame,pushbak_mgr_sendqueue一起使用
******************************************************************************************/
void Notify_Trans_Abnormal_Base::malloc_abnormalframe(size_t frame_len)
{
    //不能重复调用这个函数
    ZCE_ASSERT (abnormal_frame_ == NULL);
    abnormal_frame_ = trans_notify_mgr_->alloc_appframe(frame_len);
    return ;
}

//取得FRAME进行操作，不要改变长度等FRAME HEAD的数据，其实推荐使用下面的函数
Zerg_App_Frame *Notify_Trans_Abnormal_Base::get_abnormal_frame()
{
    ZCE_ASSERT (abnormal_frame_ != NULL);
    return abnormal_frame_;
}

//取得FRAME的数据区用于操作
char *Notify_Trans_Abnormal_Base::get_abnormal_framedata()
{
    ZCE_ASSERT (abnormal_frame_ != NULL);
    return abnormal_frame_->frame_appdata_;
}

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2008年12月16日
Function        : Notify_Trans_Abnormal_Base::pushbak_mgr_sendqueue
Return          : int
Parameter List  :
  Param1: unsigned int cmd     命令字
  Param2: unsigned int option  选项
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Notify_Trans_Abnormal_Base::pushbak_mgr_sendqueue(unsigned int cmd,
                                                      unsigned int option)
{
    int ret = 0;
    //
    unsigned int frame_len = abnormal_frame_->frame_length_;
    abnormal_frame_->init_framehead(frame_len, option, cmd);

    abnormal_frame_->frame_uin_ = req_qq_uin_;
    //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
    abnormal_frame_->transaction_id_ = transaction_id_;
    abnormal_frame_->backfill_trans_id_ = req_session_id_;

    SERVICES_ID proxysvc(0, 0);
    SERVICES_ID selfsvc = *(trans_notify_mgr_->self_svc_info());
    abnormal_frame_->recv_service_ = selfsvc;
    abnormal_frame_->proxy_service_ = proxysvc;
    abnormal_frame_->send_service_ = selfsvc;

    abnormal_frame_->app_id_ = req_game_app_id_;


    ret = trans_notify_mgr_->enqueue_sendqueue(abnormal_frame_, true);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    //发送成功后，这个数据已经交割Send Queue，
    abnormal_frame_ = NULL;

    return SOAR_RET::SOAR_RET_SUCC;
};

//回收后的处理，用于资源的释放，等等，尽量保证基类的这个函数最后调用，类似析构函数。
void Notify_Trans_Abnormal_Base::finish()
{
    //表示sndqueue_frame_没有被正常使用,回收
    if (abnormal_frame_)
    {
        trans_notify_mgr_->free_appframe(abnormal_frame_);
        abnormal_frame_ = NULL;
    }

    Transaction_Base::finish();
}

