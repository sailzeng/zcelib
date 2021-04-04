#include "soar_predefine.h"
#include "soar_fsm_trans_mgr.h"
#include "soar_svrd_app_fsm.h"
#include "soar_mmap_buspipe.h"
#include "soar_svrd_cfg_fsm.h"


//
Comm_SvrdApp_FSM::Comm_SvrdApp_FSM() :
    Soar_Svrd_Appliction()
{
}

//
Comm_SvrdApp_FSM::~Comm_SvrdApp_FSM()
{
}

//���ӵ���register_func_cmd
int Comm_SvrdApp_FSM::app_start(int argc, const char *argv[])
{
    int ret = 0;
    ret = Soar_Svrd_Appliction::app_start(argc, argv);

    if (0 != ret)
    {
        return ret;
    }

    Server_Config_FSM *svd_config = dynamic_cast<Server_Config_FSM *>(config_base_);

    //����������ĳ�ʼ��, �Զ�����ʹ��notify
    Transaction_Manager *p_trans_mgr_ = new Transaction_Manager();
    p_trans_mgr_->initialize(ZCE_Timer_Queue_Base::instance(),
                             svd_config->framework_config_.trans_info_.trans_cmd_num_,
                             svd_config->framework_config_.trans_info_.trans_num_,
                             self_svc_id_,

                             Soar_MMAP_BusPipe::instance());
    Transaction_Manager::instance(p_trans_mgr_);

    ret = register_trans_cmd();

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] register trans cmd fail. ret=%d", ret);
        return ret;
    }

    return 0;
}

//���д���,
int Comm_SvrdApp_FSM::app_run()
{
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance start.",
            get_app_basename(),
            typeid(*this).name());

    //����N�κ�,����SELECT�ĵȴ�ʱ����
    const unsigned int LIGHT_IDLE_SELECT_INTERVAL = 128;
    //����N�κ�,SLEEP��ʱ����
    const unsigned int HEAVY_IDLE_SLEEP_INTERVAL = 10240;

    //microsecond
    // 64λtlinux��idle��ʱ�����̫�̻ᵼ��cpu����
    const int LIGHT_IDLE_INTERVAL_MICROSECOND = 10000;
    const int HEAVY_IDLE_INTERVAL_MICROSECOND = 100000;

    Transaction_Manager *trans_mgr = Transaction_Manager::instance();

    size_t size_io_event = 0, num_timer_expire  = 0;

    size_t proc_frame = 0, gen_trans = 0, proc_data_num = 0;
    unsigned int idle = 0;

    ZCE_Time_Value select_interval(0, 0);

    ZCE_Timer_Queue_Base *time_queue = ZCE_Timer_Queue_Base::instance();
    ZCE_Reactor *reactor = ZCE_Reactor::instance();

    for (; app_run_;)
    {
        //�����յ�������
        trans_mgr->process_pipe_frame(proc_frame, gen_trans);

        //��ʱ
        num_timer_expire = time_queue->expire();

        // IO�¼�
        size_io_event = 0;
        reactor->handle_events(&select_interval, &size_io_event);

        //���û�д����κ�֡
        if ((proc_frame + num_timer_expire + proc_data_num + size_io_event) <= 0)
        {
            ++idle;
        }
        else
        {
            idle = 0;
        }

        //���æ�������ɻ�
        if (idle < LIGHT_IDLE_SELECT_INTERVAL)
        {
            select_interval.usec(0);
            continue;
        }
        //������кܶ�,��Ϣһ��,�����ȽϿ��У������SELECT�൱��Sleep��
        else if (idle >= HEAVY_IDLE_SLEEP_INTERVAL)
        {
            select_interval.usec(HEAVY_IDLE_INTERVAL_MICROSECOND);
        }
        //else �൱�� else if (idle >= LIGHT_IDLE_SELECT_INTERVAL)
        else
        {
            select_interval.usec(LIGHT_IDLE_INTERVAL_MICROSECOND);
        }
    }

    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance end.",
            get_app_basename(),
            typeid(*this).name());
    ZCE_LOG(RS_INFO, "======================================================================================================");
    return 0;
}

//�˳�����
int Comm_SvrdApp_FSM::app_exit()
{
    int ret = 0;
    Transaction_Manager::clean_instance();

    ret = Soar_Svrd_Appliction::app_exit();

    if ( 0 != ret )
    {
        return ret;
    }

    return 0;
}

