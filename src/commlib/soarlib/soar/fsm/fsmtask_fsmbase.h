#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_
#define SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_

#include "soar/fsm/fsm_base.h"
#include "soar/fsm/fsmtask_mgr.h"

class Notify_Trans_Base: public FSM_Base
{
public:

    //这而要求传递的Transaction_Manager *是子类NotifyTrans_Manger的，
    //必须如此。
    //这儿是一个典型的设计两难，如果使用向下朔型DOWNCAST，无意是丑陋的。
    Notify_Trans_Base(FSM_Manager *trans_notify_mgr,unsigned int create_cmd)
        : FSM_Base(trans_notify_mgr,create_cmd)
        ,trans_notify_mgr_(NULL)
    {
        //必须要求trans_notify_mgr至少是trans_notify_mgr的子类，
        trans_notify_mgr_ = dynamic_cast<FSMTask_Manger *> (trans_notify_mgr);
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
    int pushbak_mgr_sendqueue(uint32_t cmd,
                              const T &msg,
                              uint32_t option = 0)
    {
        soar::SERVICES_ID proxy_svc(0,0);
        return trans_notify_mgr_->enqueue_sendqueue(cmd,
                                                    this->req_zerg_head_.user_id_,
                                                    this->transaction_id_,
                                                    0,
                                                    info,
                                                    option);
    };

protected:

    //管理器
    FSMTask_Manger *trans_notify_mgr_ = nullptr;
};

#endif //#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_
