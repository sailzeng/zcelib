/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008��10��6��
* @brief      Timer���д���Ļ���
*
* @details
*
* @note
*
*/

#ifndef ZCE_TIMER_QUEUE_BASE_H_
#define ZCE_TIMER_QUEUE_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_time_value.h"

class ZCE_Timer_Handler;
/******************************************************************************************
ZCE_Timer_Queue
******************************************************************************************/
class ZCE_Timer_Queue_Base : public ZCE_NON_Copyable
{

public:
    //��Ч��ʱ��ID
    static const int  INVALID_TIMER_ID = -1;

    ///����ģʽ��
    enum TRIGGER_MODE
    {

        ///����ϵͳʱ�ӽ��д�����ȱ���ǣ���ϵͳʱ�䱻����������£��ᶪʧ��ʱ��
        TRIGGER_MODE_SYSTEM_CLOCK = 1,

        ///CPU��TICK���ô�������ʧ���������泬ʱʱ�䲻��ǽ��ʱ�ӣ�
        TRIGGER_MODE_CPU_TICK     = 2,
    };

    //Ĭ�ϵ�ʱ�侫��
    static const unsigned int DEFAULT_TIMER_PRECISION_MSEC = 100;

    //gethrtime���ص�ԭ�������룬��ֻ��Ҫ����
    static const uint64_t MSEC_PER_NSEC = 1000000ll;

    //����Զ����ţ�һ�����ŵ�NODE����
    static const size_t     ONCE_EXTEND_NODE_NUMBER = 1024 * 8;

protected:
    /******************************************************************************************
    ZCE_TIMER_NODE ��ʱ���Ľڵ㣬���ڶ�ʱ���ķ�Ӧ���ڲ����涨ʱ������Ϣ
    ******************************************************************************************/
    struct ZCE_TIMER_NODE
    {
    public:

        ///��ʱ����ID,������˼���Ҳ�����֧��>22�ڸ���ʱ�����ҵ��ۣ�
        int                         time_id_;

        ///�Ժ�ÿ�μ���Ĵ����ȴ�ʱ��
        ZCE_Time_Value              interval_time_;

        ///�ص���ʱ�򷵻ص�ָ�룬��ֻ�Ǳ������������õġ����Լ����պ�
        const void                 *action_;

        ///��Ӧ��ʱ�����ĵ�ָ��
        ZCE_Timer_Handler          *timer_handle_;

        ///��һ�δ����㣬������һ������ʱ�䣬Ҳ������һ��CPU TICK�ļ���,������һ������ֵ
        uint64_t                    next_trigger_point_;

        ///�Ƿ��Ѿ��������ˣ�
        ///yunfeiyang��æ������һ��bug��Ϊ�˽�����bug����������������ֶΡ���������������ģ�
        ///1. dispatch_timer�ڵ���timer_timeoutʱ�����timer_timeout��ɾ�����Լ��Ķ�ʱ�������������Լ��Ķ�ʱ������ʱ���ӵĶ�ʱ����
        ///   time_node_id�뵱ǰ����dispatch_timer�д����time_node_id��һ����,(��Ϊ������е�ԭ��)
        ///2. dispatch������timer_timeout�󣬻�reschedule_timer, reschedule_timer�������timer��һ���Եģ�ɾ�������time_node_id
        ///   �ڶ�ʱ���ڲ��Ѿ�ʧЧ�������ⲿ����֪����
        ///3. ���������У��ⲿ���ͷ�ʱ���仹�����time_node_id��Ч�������ֻ����cancel_timer����������ʱ���time_node_id�ڶ�ʱ����
        ///   ������Ϊ�Ѿ��ͷ��ˣ����³���
        bool                        already_trigger_;

    public:
        //���캯������������
        ZCE_TIMER_NODE():
            time_id_(INVALID_TIMER_ID),
            interval_time_(ZCE_Time_Value::ZERO_TIME_VALUE),
            action_(NULL),
            timer_handle_(NULL),
            next_trigger_point_(0),
            already_trigger_(false)
        {
        };

        ~ZCE_TIMER_NODE()
        {
        };

        void clear()
        {
            interval_time_.set(0, 0);

            action_ = NULL;
            timer_handle_ = NULL;
            next_trigger_point_ = 0;
            already_trigger_ = false;
        }
    };

protected:
    //���캯��
    ZCE_Timer_Queue_Base(size_t num_timer_node,
                         unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                         TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                         bool dynamic_expand_node = true);
    ZCE_Timer_Queue_Base();
public:
    virtual ~ZCE_Timer_Queue_Base();

public:

    /*!
    @brief      ���õ�һ����ʱ�����ӿڲο���ACE����ƣ���������ʵ��������ˣ�����չ���࣬����ʵ������ӿ�
    @return     int           ���ض�ʱ��ID��>=0��ʶ�ɹ���-1��ʶʧ��
    @param      timer_hdl     ����Ķ�ʱ�������������ص��Ķ���
    @param[in]  action        һ��ָ�룬�ڶ�ʱ���������ò��������㣬
    @param[in]  delay_time    ��һ�δ�����ʱ�䣬ΪʲôҪ������Ʋ��������Լ�����һ�£����������10:00���ŵ�6�׹㲥���
    @param[in]  interval_time ��һ�δ����󣬺������ @a interval_time ��ʱ�����һ�δ���
                              �����������ZCE_Time_Value::ZERO_TIME_VALUE����ʶ����Ҫ����������
    */
    virtual int schedule_timer(ZCE_Timer_Handler *timer_hdl,
                               const void *action,
                               const ZCE_Time_Value &delay_time,
                               const ZCE_Time_Value &interval_time = ZCE_Time_Value::ZERO_TIME_VALUE) = 0;

    /*!
    @brief      ȡ����ʱ������̳к����ʵ������ӿ�
    @return     int      0��ʶ�ɹ�������ʧ��
    @param[in]  timer_id ��ʱ��ID
    */
    virtual int cancel_timer(int timer_id) = 0;

protected:

    /*!
    @brief      ȡ�õ�һ��Ҫ�����Ķ�ʱ��NODE��Ҳ���ǣ�����Ĵ�����ʱ������̳к����ʵ������ӿ�
    @return     int   0��ʶ�ɹ�������ʧ��
    @param[out] timer_node_id
    */
    virtual int get_frist_nodeid(int &timer_node_id) = 0;

    /*!
    @brief      �ַ���ʱ��
    @return     size_t            ���طַ��Ĵ����Ķ�ʱ��������
    @param      now_time          ��ǰ��ʱ�䣬ǽ��ʱ��
    @param      now_trigger_msec  ��ǰ�����ĵ�ĺ����������ݴ���ģʽ��������岻һ��
                                  ����ģʽ�� @a TRIGGER_MODE_SYSTEM_CLOCK ��ô��ʾǽ��ʱ�ӵ�mesc����
                                  ����ģʽ�� @a TRIGGER_MODE_CPU_TICK ��ô��ʶCPU Tick�ĺ�������
    */
    virtual size_t dispatch_timer(const ZCE_Time_Value &now_time,
                                  uint64_t now_trigger_msec) = 0;

    /*!
    @brief      �ڴ���һ�κ�Ҫ�Զ�ʱ���������¼��㣬���ţ������ʵ�ֵ��麯��
    @return     int              0��ʶ�ɹ�������ʧ��
    @param      timer_id         ��ʱ��ID
    @param      now_trigger_msec ��ǰ�Ĵ�����ĺ���������ͬģʽ�±�ﲻ̫һ��
    @note
    */
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec) = 0;

    //---------------------------------------------------------------------------------------
public:

    //������Щ�Ѿ�ʵ�֣���������أ�����ֱ��ʹ�ö���

    /*!
    @brief      ���г�ʱ��������Ҫ���õĺ������ַ�,ֱ�ӵ��ú���ȡ��ʱ��Ȼ����зַ�
    @return     size_t ���طַ��Ĵ����Ķ�ʱ��������
    */
    virtual size_t expire();

    /*!
    @brief      ʹ�� ZCE_Timer_Handler��ָ�� @a timer_hdl ȡ����ʱ����ʽ������
                �������������ĺ�����(��Ȼʹ���������ܱȽϷ���)������Լ̳�,
                һ������£��Ƽ���time id ȡ����ʱ��
    @return     int       ����0��ʾ�ɹ�������ʧ��
    @param      timer_hdl ��ʱ�������ָ��
    */
    virtual int cancel_timer(const ZCE_Timer_Handler *timer_hdl);

    /*!
    @brief      ������ض�ʱ����NODE��������
                �������໹�к�NOE��ص����ݽṹ������չ��ҲҪ��չ��������visual
    @return     int ����0��ʶ�ɹ�
    @param[in]  num_timer_node Ҫ���õĶ�ʱ��NODE����
    @param[out] old_num_node   ����ԭ���Ķ�ʱ��NODE����
    */
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node) = 0;

    /*!
    @brief      �رն�ʱ������
    @return     int 0��ʾ�ɣ�����ʧ��
    */
    virtual int close();

protected:

    //--------------------------------------------------------------------------------------

    /*!
    @brief      ��ʼ��
    @return     int                   ����0��ʶ��ʼ���ɹ��𣬷���ʧ��
    @param[in]  num_timer_node        ��ʱ��NODE����������ʼ����ʱ������ã�
    @param[in]  timer_precision_mesc  ��ʱ���Ľ��ȣ�����msec��������ù�����Щ��ʱ���ᳬʱ��Ŵ��������̫С����Ҫ������ֵĲ�λ�ռ䣬
    @param[in]  trigger_mode          ����ģʽ����ǽ��ʱ�ӻ���CPU TICK���ο� @ref ZCE_Timer_Queue::TRIGGER_MODE
    @param[in]  dynamic_expand_node   �����ʼ����NODE�ڵ������������Ƿ��Զ���չ
    */
    int initialize(size_t num_timer_node,
                   unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true);

    /*!
    @brief      ����һ��ո�µ�Timer Node
    @return     int             ����0��ʶ����ɹ�
    @param[in]  timer_hdl       Timer Handler��ָ�룬TIMER NODE����Ҫ��¼�ģ�
    @param[in]  action          schedule_timer�Ĳ������ڻص�timer_timeout��ʱ�������ȥ
    @param[in]  delay_time_     ��һ�δ�����ʱ��
    @param[in]  interval_time_  �������������ļ��ʱ��
    @param[out] time_node_id    ���صķ����ID
    @param[out] alloc_time_node ���صķ����TIMER NODE��ָ��
    */
    int alloc_timernode(ZCE_Timer_Handler *timer_hdl,
                        const void *action,
                        const ZCE_Time_Value &delay_time_,
                        const ZCE_Time_Value  &interval_time_,
                        int &time_node_id,
                        ZCE_TIMER_NODE *&alloc_time_node);

    /*!
    @brief      ������һ�������㣬��һ����ʱ�����ڴ�������Ҫ��������һ�εĴ����㣬
    @param[in]  time_node_id     TIME ID,������д��������޸Ķ�Ӧ��TIMER NODE��һ�δ���ʱ���
    @param[in]  now_trigger_msec ��ǰ�Ĵ���ʱ��㣬
    @param[out] continue_trigger ����д����㷵��0�����û�з���
    */
    void calc_next_trigger(int time_node_id,
                           uint64_t now_trigger_msec,
                           bool &continue_trigger);

    /*!
    @brief      �ͷ�Timer Node
    @return     int          0��ʶ�ɹ��ͷţ������ʾ���������⣬�����Ĳ�����ȷ������ɹ��ͷ�
    @param[in]  time_node_id ���ص�TIMER NODEָ��
    */
    int free_timernode(int time_node_id);

    /*!
    @brief      �õ���콫�ڶ���ʱ���ע����ʱ�䳤�ȣ�����
    @return     int           0��ʾ�ɣ�����ʧ��
    @param      first_timeout ����ʱ�䳤�ȣ�������ʱ��㣩
    */
    int get_first_timeout(ZCE_Time_Value *first_timeout);

public:
    //����ط��ĵ���ʹ�ã��������ط����в�ͬ��Ҫ�ȵ��ø�ֵ�ĺ�����������ָ�븶���������

    //
    static ZCE_Timer_Queue_Base *instance();
    //
    static void instance(ZCE_Timer_Queue_Base *pinstatnce);
    //
    static void clean_instance();

    //���ݳ�Ա
protected:

    ///TIMER NODE(Ҳ����HANDLER)��������
    size_t                      num_timer_node_ = 0;

    ///ʱ�侫��,�Ժ���Ϊ��λ����Ҫ΢��ö�ʱ�������������أ�Ҳ����ʵ
    ///�����ʱ����Ⱥ󣬱�������������ڵ���dispatch_timer ����
    ///��ͨ�������ҽ�������s�ļ���Ҳ����1000ms
    ///Ҫ�󾫶ȱȽϸߵķ��������ҽ�������100ms
    ///Ҫ�󲻸ߵķ�������������������N s��NС��10s
    unsigned int                timer_precision_mesc_ = DEFAULT_TIMER_PRECISION_MSEC;

    ///�Ѿ�ʹ�õĽڵ��������Ҳ���Ƿ����˶��ٸ���ʱ��
    size_t                      num_use_node_ = 0;

    ///����ģʽ
    TRIGGER_MODE                trigger_mode_ = TRIGGER_MODE_SYSTEM_CLOCK;

    ///�Ƿ�֧�ֶ�̬����NODE
    bool                        dynamic_expand_node_ = true;

    ///TIMER NODE�Ķ���,������ָ���Լ�����ģ����ǿ������������ŵĴ�����Ҫд�ò�����resize
    ///���ˣ���Ҫ�Լ�����������
    std::vector<ZCE_TIMER_NODE> time_node_ary_;

    ///����TIMER NODE���е�ͷ�±꣬��������
    int                         free_node_id_head_ = INVALID_TIMER_ID;
    ///���ж��е��±��б������
    std::vector<int>            free_node_id_list_;
    ///�����뵰�۵�дһ��ʹ���б������������Ҫ�ã��ͱ�����˫���б��ҵ��ۣ�������

    ///��һ�ε�CPU TICK�Ĵ����㣬
    uint64_t                    prev_trigger_msec_ = 0;

    ///��ʱ������Ĳο�ʱ��㣬ÿ�ζ�ʱ��������ʱ��ı�
    uint64_t                    timer_refer_pointer_ = 0;

protected:

    ///����ʵ��ָ��
    static ZCE_Timer_Queue_Base     *instance_;
};

#endif //# ZCE_LIB_TIMER_QUEUE_BASE_H_

