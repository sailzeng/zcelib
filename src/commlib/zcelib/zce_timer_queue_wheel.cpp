#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_timer_handler_base.h"
#include "zce_timer_queue_wheel.h"

//���캯��
ZCE_Timer_Wheel::ZCE_Timer_Wheel(size_t num_timer_node,
                                 unsigned int timer_length_mesc,
                                 unsigned int timer_precision_mesc,
                                 TRIGGER_MODE trigger_mode,
                                 bool dynamic_expand_node
                                ):
    timer_length_mesc_(0),
    num_wheel_point_(0),
    proc_wheel_start_(0)
{
    int ret = 0;
    ret = initialize(num_timer_node,
                     timer_length_mesc,
                     timer_precision_mesc,
                     trigger_mode,
                     dynamic_expand_node);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Timer_Wheel::initialize fail.");
    }
}

//���캯��
ZCE_Timer_Wheel::ZCE_Timer_Wheel():
    timer_length_mesc_(0),
    num_wheel_point_(0),
    proc_wheel_start_(0)
{
}

//��������
ZCE_Timer_Wheel::~ZCE_Timer_Wheel()
{
}

int ZCE_Timer_Wheel::initialize(size_t num_timer_node,
                                unsigned int timer_length_mesc,
                                unsigned int timer_precision_mesc,
                                TRIGGER_MODE trigger_mode,
                                bool dynamic_expand_node)
{

    int ret = 0;
    assert(timer_length_mesc > 0 );

    if (timer_length_mesc < MIN_TIMER_LENGTH_MESC)
    {
        timer_length_mesc = MIN_TIMER_LENGTH_MESC;
    }

    //��¼����ܽ��ܵĺ�������
    timer_length_mesc_ = timer_length_mesc;

    ret = ZCE_Timer_Queue_Base::initialize(num_timer_node,
                                           timer_precision_mesc,
                                           trigger_mode,
                                           dynamic_expand_node);

    if (ret != 0)
    {
        return ret;
    }

    //��Ҫ������ٸ�ʱ���
    num_wheel_point_ = static_cast<size_t>(timer_length_mesc) / timer_precision_mesc + 1;
    timer_wheel_point_.resize(num_wheel_point_);

    //��QUEUE�ϵ����㴦�����Ч����ʾû�й�NODE
    for (size_t i = 0; i < num_wheel_point_; ++i)
    {
        timer_wheel_point_[i] = INVALID_TIMER_ID;
    }

    proc_wheel_start_ = 0;
    future_trigger_tid_.reserve(ARRAY_FUTURE_TRIGGER_TIMERID_SIZE);
    return 0;
}

//�������ʮ�������NODE��������Ҳ���õײ��extend_node����
int ZCE_Timer_Wheel::extend_node(size_t num_timer_node,
                                 size_t &old_num_node)
{
    int ret = 0;
    ret = ZCE_Timer_Queue_Base::extend_node(num_timer_node, old_num_node);

    if (ret != 0)
    {
        return ret;
    }

    //
    wheel_node_list_.resize(num_timer_node_);

    //���캯����ʵ��д��
    //�����еĽڵ㶼ΪNULL
    //for (size_t i=old_num_node;i<num_timer_node_;++i)
    //{
    //    wheel_node_list_[i].wheel_point_id_ = INVALID_TIMER_ID;
    //    wheel_node_list_[i].list_prev_ = INVALID_TIMER_ID;
    //    wheel_node_list_[i].list_next_ = INVALID_TIMER_ID;
    //}

    return 0;
}

//��Queue��TimerNode��
void ZCE_Timer_Wheel::bind_wheel_listnode(int time_node_id)
{

    //ǰ���˶���ʱ���
    size_t front_num = static_cast<size_t>
                       ((time_node_ary_[time_node_id].next_trigger_point_
                         - prev_trigger_msec_) / timer_precision_mesc_ );

    //���Դ��룬��ʱ����
    //ZCE_LOG(RS_DEBUG,"[zcelib] ZCE_Timer_Wheel::bind_wheel_listnode next_trigger_point_[%llu] prev_trigger_msec_ [%llu] front_num [%lu]",
    //  time_node_ary_[time_node_id].next_trigger_point_,
    //  prev_trigger_msec_,
    //  front_num);

    //�ҵ����ӵ�λ��
    size_t wheel_point_id = (proc_wheel_start_ + front_num) % num_wheel_point_;

    //�Ƚ��ѿ���˫������û����std�������ڣ�ֻ����������һ��ͷָ����ѿ���д��
    int old_node_id = timer_wheel_point_[wheel_point_id];
    timer_wheel_point_[wheel_point_id] = time_node_id;

    //Ҫ�����¼��������ɾ��ʱ��ѯ��
    wheel_node_list_[time_node_id].wheel_point_id_ = static_cast<int>(wheel_point_id);

    //���ԭ��ͷָ�����һ�����ݣ�Ҫ���룬����˫������
    if (INVALID_TIMER_ID != old_node_id)
    {
        wheel_node_list_[old_node_id].list_prev_ = time_node_id;
        wheel_node_list_[time_node_id].list_next_ = old_node_id;
    }
}



//��Queue��TimerNode�����
void ZCE_Timer_Wheel::unbind_wheel_listnode(int time_node_id)
{
    int wheel_point_id =  wheel_node_list_[time_node_id].wheel_point_id_;

    //���WHEEL�ϵ�һ��Ԫ�ؼ�¼��Ҳ�����NODE
    if ( timer_wheel_point_[wheel_point_id] == time_node_id )
    {
        timer_wheel_point_[wheel_point_id] = wheel_node_list_[time_node_id].list_next_;
    }
    else
    {
        int prev = wheel_node_list_[time_node_id].list_prev_ ;
        int next =  wheel_node_list_[time_node_id].list_next_;

        //�����ǰ,ǰ��ͺ��������
        if (prev != INVALID_TIMER_ID)
        {
            wheel_node_list_[prev].list_next_ = next;
        }

        //����к󣬺����ҵ�ǰ�����
        if (next != INVALID_TIMER_ID)
        {
            //���ݵĺ�ָ����ں�ָ��
            wheel_node_list_[next].list_prev_ = prev;
        }
    }

    //����ΪNULL�������鷳��
    wheel_node_list_[time_node_id].list_prev_ = INVALID_TIMER_ID;
    wheel_node_list_[time_node_id].list_next_ = INVALID_TIMER_ID;
    wheel_node_list_[time_node_id].wheel_point_id_ = INVALID_TIMER_ID;
}

//���ö�ʱ��
int ZCE_Timer_Wheel::schedule_timer(ZCE_Timer_Handler *timer_hdl,
                                    const void *action,
                                    const ZCE_Time_Value &delay_time,
                                    const ZCE_Time_Value &interval_time)
{
    int ret = 0;
    int time_node_id = INVALID_TIMER_ID;

#if defined(ZCE_HAS_DEBUG)

#endif //

    ZCE_TIMER_NODE *alloc_time_node = NULL;
    ret = alloc_timernode(timer_hdl,
                          action,
                          delay_time,
                          interval_time,
                          time_node_id,
                          alloc_time_node);

    //ע�⣬����ط�����INVALID_TIMER_ID��ʾ��Ч����ʵҲ�������Ӧ��������ƣ���Ϊ�˼���ACE�Ĵ���
    if (ret != 0)
    {
        return INVALID_TIMER_ID;
    }

    bind_wheel_listnode(time_node_id);

    return time_node_id;
}

//ȡ����ʱ��
int ZCE_Timer_Wheel::cancel_timer(int timer_id)
{
    //
    int ret = 0;

    //����
    unbind_wheel_listnode(timer_id);

    //�������TIMER NODE
    ret = ZCE_Timer_Queue_Base::cancel_timer(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //
    return 0;
}

//�ڴ���һ�κ�Ҫ�Զ�ʱ���������¼���
int ZCE_Timer_Wheel::reschedule_timer(int timer_id,
                                      uint64_t now_trigger_msec)
{
    bool contiue_trigger = false;

    //������һ�δ����ĵ�
    calc_next_trigger(timer_id,
                      now_trigger_msec,
                      contiue_trigger);

    //�������Ҫ����������ʱ���ˣ�ȡ��
    if (!contiue_trigger)
    {
        return cancel_timer(timer_id);
    }

    //���TIMER NODE����Ҫ�������´����Ĺ�������������
    //�ȴ����Ͻ����
    unbind_wheel_listnode(timer_id);

    //�����°���ȥ
    bind_wheel_listnode(timer_id);

    return 0;
}

//ȡ�õ�һ��Ԫ�أ�Ҳ���ǣ���С��ʱ��,Timer_Wheel�������ĵ�NODE���٣������е���,
int ZCE_Timer_Wheel::get_frist_nodeid(int &first_node_id)
{

    first_node_id = INVALID_TIMER_ID;

    int timer_node_id = INVALID_TIMER_ID;

    //���û�ж�ʱ�����ͱ���ˣ�WHEEL��first����
    if (num_use_node_ == 0)
    {
        return -1;
    }

    for (size_t i = 0; i < num_wheel_point_; ++i)
    {
        //������WHEEL NODE�����Ƿ���TIMER NODE������
        size_t wheel_node_id = (proc_wheel_start_ + i)  % num_wheel_point_;
        timer_node_id = timer_wheel_point_[wheel_node_id];

        if ( INVALID_TIMER_ID != timer_node_id)
        {
            break;
        }
    }

    if ( INVALID_TIMER_ID != timer_node_id)
    {
        first_node_id = timer_node_id;
        return 0;
    }

    //Never goto here.?
    return -1;
}

//�ַ���ʱ�������طַ�������
size_t ZCE_Timer_Wheel::dispatch_timer(const ZCE_Time_Value &now_time,
                                       uint64_t now_trigger_msec)
{
    //�����˶��ٸ���ʱ������
    size_t num_dispatch = 0;

    //�ȵ�����ʱ�Ĳο��㣬��Ϊ�ڴ���ʱ���Ĺ��̣��ص�timer_timeoutʱ�����˻���timer_timeout�������ö�ʱ����
    //���������timer_refer_pointer_����ô�������õ�ʱ����ܳ������⣬
    //����м����һ���ܳ�ʱ�䣬���timer_refer_pointer_û���ȵ������ص�timer_timeoutʱ���õĶ�ʱ���ʹ����ˡ������ܻ������ѭ����
    timer_refer_pointer_ = now_trigger_msec;

    //���������������if����ʾʱ��ͬ������������ʱ�����䵽��ȥ��ĳ��ʱ����(pasttime)ȥ��,
    if (now_trigger_msec < prev_trigger_msec_)
    {
        //����ô�������Ϊ�б�Ҫ����һ���㣬��ʱ���������ˣ�
        ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Timer_Wheel error. now_trigger_msec[%llu] < prev_trigger_msec_[%llu],may be you adjust systime time to past time.",
                now_trigger_msec,
                prev_trigger_msec_);
        prev_trigger_msec_ = now_trigger_msec;
        return 0;
    }

    uint64_t elapsed_msec = now_trigger_msec - prev_trigger_msec_;

    //�������������Ҫ��������һֱû�е���Dispatch��Ҫ���������ʱ������ƵĹ�С,����ʱ����δ�������˺ܶ�
    if (elapsed_msec > timer_length_mesc_)
    {
        //ע�͵����ԣ�yunfei˵����һ����������ʱ����ǰ���䳬����ѭ������.��Ȼ����Ϊ��������ʱ���ᷢ�������ǽ���ʱ��������δ����ĳ��ʱ�䣬
        //assert (elapsed_msec < timer_length_mesc_);

        ZCE_LOG(RS_ALERT, "[zcelib] ZCE_Timer_Wheel alert. now_trigger_msec[%llu], prev_trigger_msec_[%llu],elapsed_msec[%llu],timer_length_mesc_[%llu],"
                "may be you adjust systime time to future time or you ,or wheel is too little ,or dispatch_timer or expire invoke too little.",
                now_trigger_msec,
                prev_trigger_msec_,
                elapsed_msec,
                timer_length_mesc_);

        //���ð�ȫ�ķ�ʽ?
        prev_trigger_msec_ = now_trigger_msec;
        return 0;

        //���ʱ����ǰ���䳬����ѭ������, �����һ�������б�
        //��yunfei�������������ʱ����Ȼ��ǰ�������������еĶ�ʱ��������������ǲ����и��ߵķ��գ���������ʵ������ҲӦ���ö��ԣ�
        //elapsed_msec = timer_length_mesc_;
    }

    size_t passing_wheel_node = static_cast<size_t>( elapsed_msec / timer_precision_mesc_);

    //���Դ��룬��ʱ�ر�
    //ZCE_LOG(RS_DEBUG,"[zcelib] ZCE_Timer_Wheel . now_trigger_msec[%llu], prev_trigger_msec_[%llu],elapsed_msec[%llu],timer_length_mesc_[%u],passing_wheel_node[%lu]",
    //  now_trigger_msec,
    //  prev_trigger_msec_,
    //  elapsed_msec,
    //  timer_length_mesc_,
    //  passing_wheel_node);

    //����N�����ӽڵ㣬��0��1,<=��<�����ַ���2�κ�Ϳ,0�Ǳ��룬��Ϊ����ϣ�����ϴ�����=Ҳ�Ǳ���ģ���Ϊ����Ҫ������ǰʱ�䡣
    for (size_t i = 0; i <= passing_wheel_node; ++i)
    {
        //������WHEEL NODE�����Ƿ���TIMER NODE������
        size_t wheel_node_id = (proc_wheel_start_ + i  )  % num_wheel_point_;
        int timer_node_id = timer_wheel_point_[wheel_node_id];

        if ( INVALID_TIMER_ID == timer_node_id)
        {
            continue;
        }

        //�ж��ľ���,ֱ��ʹ��timer_precision_mesc_ҲӦ�ÿ��ԣ�����Ϊ������ʵ�ܴ����� * 10�о���ȫһ��
        const unsigned int JUDGE_PRECISION_MESC = 10 * timer_precision_mesc_;

        //
        while ( INVALID_TIMER_ID != timer_node_id  )
        {
            //����ط�����һ�£���Ϊ����ԭ����д�����������Ķ�ʱ���Ĵ���ʱ�䳬�����ӳ��ȣ���ô�ᱻ��ǰ������
            //��Ϊ�������⣬���Ա��� + JUDGE_PRECISION_MESC,
            if (time_node_ary_[timer_node_id].next_trigger_point_ <= now_trigger_msec + JUDGE_PRECISION_MESC )
            {
                //��������ʱ���Ѿ�����������ϸ��already_trigger_�Ľ���
                time_node_ary_[timer_node_id].already_trigger_ = true;
                //ʱ�Ӵ���
                time_node_ary_[timer_node_id].timer_handle_->timer_timeout(now_time,
                                                                           time_node_ary_[timer_node_id].action_);

                ++num_dispatch;

                //��Ϊtimer_timeout��ʵ����ȡ���������ʱ���������ڵ���֮��Ҫ����һ�¼��
                if (time_node_ary_[timer_node_id].timer_handle_ && time_node_ary_[timer_node_id].already_trigger_ == true )
                {
                    //���¹滮�����ʱ���Ƿ�Ҫ���ţ���������ڲ���ȡ���������۲�����ʧ�ܣ���ʱ���Ӹ���
                    reschedule_timer(timer_node_id, now_trigger_msec);
                }
            }
            //��else ��2�����ܣ�
            //һ����ԭ���Ķ�ʱ����ʱ����һ��������ʱ�䣬���������ӵ�����
            //һ����ʱ�����ڱ�����������������������
            else
            {
                //�ȱ�����
                future_trigger_tid_.push_back(timer_node_id);
                //�ȴ����Ͻ����
                unbind_wheel_listnode(timer_node_id);
            }

            //������ͷ����ʼ��
            timer_node_id = timer_wheel_point_[wheel_node_id];
        }

        //�������λ��δ��������Timer ID��
        if (future_trigger_tid_.size() > 0)
        {
            for (size_t x = 0; x < future_trigger_tid_.size(); ++x )
            {
                //���¼�����ص�δ���Ĵ����㣬Ȼ���������
                bind_wheel_listnode(future_trigger_tid_[x]);
            }
            //clearӦ�ò�����տռ�
            future_trigger_tid_.clear();
        }

    }

    //���������NODE��㣬��ǰʱ�䣬��ǰ�������򲻶���
    if (passing_wheel_node)
    {
        //����ע���д����Ǵ���ģ���Ϊ���ÿ�ζ�����������ÿ��ǰ���Ĳ������ᶪ�����Ⱥ��沿�֣����ٳɶ࣬���ö�ʱ���Ĵ����ӳ�
        //prev_trigger_msec_ = now_trigger_msec;

        //��������Ϊ��ȷ��ÿ��ֻǰ��N�����������Ҳ��ᶪʧ���Ȳ���
        prev_trigger_msec_ += (passing_wheel_node * timer_precision_mesc_);

        //��ʼ����ĵ��Ի��ζ��еķ�ʽǰ��
        proc_wheel_start_ = (proc_wheel_start_ + passing_wheel_node) % num_wheel_point_;
    }

    //��������
    return num_dispatch;
}

