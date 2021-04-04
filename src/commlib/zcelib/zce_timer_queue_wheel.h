/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_wheel.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008��10��6��
* @brief      ʱ���ֵĶ�ʱ�����У��ٶȷɷɷɷɷɷɷɿ죬�����о������ƵĶ�ʱ������
*             ��һ��ѭ�������¼ʱ�䣬
*             ���ķ��ã�ȡ����ʱ����ʱ�����ٶ���O(1)���𣬿����ɣ�
*
* @details    ���WHEEL��ACE��WHEEL����һ���������ҵ�����˼·�ǽ�һ��ʱ������һ��
*             ���ӣ�WHELL,��������N��ʱ��ۣ������ϵ�ÿ���۷���һ��ʱ��ĳ�ʱ��ʱ����
*             ����㶼�Ǵ���һ���̶����ȣ�һ����0.1s�����Ҿ�����Է�����һ���㹻�ˣ�
*
*             ����һ����ʱʱ���TimerҪ����������ʱ������ݵ�ǰ��ʱ�䣬������ʱ��ʱ��
*             ����һ���۵��ϣ�����ж����ʱ�������γ�һ������
*             �����ϵĳ�ʱ���ݲ��������κ�������
*             ��ʱ��ǰ������ԽN����λ�ǣ����N����λ�����еĶ�ʱ������������
*
*             ����������ʱ��ʱ���Ҳο���LINUX�ں˵�ʵ�֣���ACE��4��ʵ�֣�
*             �����ʵ���ϣ���ǿ�����ٶȣ�����������ȫ��׼ȷ�ԣ�ͬһ����λ�Ķ�ʱ������
*             ���ȼ����򣬺����Ķ�ʱ�����ȴ�����
*
*             �ҹ���10��Ķ�ʱ����10s���������ʵ�֣�����CPU��1%�����á�
*
*/

#ifndef ZCE_TIMER_QUEUE_WHEEL_H_
#define ZCE_TIMER_QUEUE_WHEEL_H_

#include "zce_timer_queue_base.h"
#include "zce_time_value.h"

/*!
* @brief      ʱ���ֶ�ʱ��������Ҳ��Ϊʮ������ѭ�����У���
*             Timer������,��Ч�ʣ������ܵĴ����ƣ�
*             ��Ҫ���˼·��ֻ�ܱ���һ��ʱ���Ĵ�����ʱ������һ��WHEEL(ѭ���Ķ��б�ʾʱ���
*             ���ʱ�����Ҫ�����Ķ�ʱ���ͷ����������棬��˫��������ʱ��NODE��ÿ��NODE
*             �ڲ�Ҳָ����WHEEL
*             ���ö�ʱ����ȡ����ʱ�����ַ���ʱ�������ļ�����O(1)
*/
class ZCE_Timer_Wheel : public ZCE_Timer_Queue_Base
{

protected:

    ///���ֻ��װ�������ݽṹ�������ں�������һ����ͬʱ�����м�����ʱ��
    ///��������һ��˫������ͬʱҲ��¼ͷҲ����WHEEL�ĵ㣬������Ƶ���Ҫ��Ϊ�˷���ɾ��
    struct ZCE_WHEEL_TIMER_NODE
    {

        ///���캯��
        ZCE_WHEEL_TIMER_NODE():
            list_prev_(ZCE_Timer_Queue_Base::INVALID_TIMER_ID),
            list_next_(ZCE_Timer_Queue_Base::INVALID_TIMER_ID),
            wheel_point_id_(ZCE_Timer_Queue_Base::INVALID_TIMER_ID)
        {
        }
        ///��������
        ~ZCE_WHEEL_TIMER_NODE()
        {
        }

        ///˫�������ǰ��
        int    list_prev_;
        ///˫������ĺ��ã�
        int    list_next_;

        ///��ʾ��ǰTIMER NODE����WHEEL�ϵ��Ǹ���ı�ʾ,���������size_t����һ�㣬����Ϊ�˿ռ�ʹ����㣬����int�洢
        int    wheel_point_id_;
    };

    ///�������NODE�����飬���ڳ�ʼ��ʱ����ÿռ䣬���������䣬
    typedef   std::vector<ZCE_WHEEL_TIMER_NODE>  ARRAY_WHEEL_TIMER_NODE;

protected:

    ///Ĭ�϶�ʱ��֧�ֵ�ʱ�䳤��MESC,(��������Ҫ��ô�����λ���ڲ����ȿ����Լ�����)
    static const unsigned int DEFAULT_TIMER_LENGTH_MESC   = 72 * 3600 * 1000;

    ///��С���������ӳ���
    static const unsigned int MIN_TIMER_LENGTH_MESC       = 1 * 3600 * 1000;

    ///δ��������ʱ��ID������ĳ�ʼ����С��
    static const size_t ARRAY_FUTURE_TRIGGER_TIMERID_SIZE = 512;

public:

    /*!
    * @brief      ���캯��
    * @param[in]  num_timer_node        ��ʱ��NODE����������ʼ����ʱ������ã�
    * @param[in]  timer_length_mesc     ���ӱ�ʶ��ʱ�䳤�ȣ�msec�������ϵĲ�λ����= timer_length_mesc/timer_precision_mesc
    * @param[in]  timer_precision_mesc  ��ʱ���Ľ��ȣ�����msec��������ù�����Щ��ʱ���ᳬʱ��Ŵ��������̫С����Ҫ������ֵĲ�λ�ռ䣬
    * @param[in]  trigger_mode          ����ģʽ����ǽ��ʱ�ӻ���CPU TICK���ο� @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   �����ʼ����NODE�ڵ������������Ƿ��Զ���չ
    */
    ZCE_Timer_Wheel(size_t num_timer_node,
                    unsigned int timer_length_mesc = DEFAULT_TIMER_LENGTH_MESC,
                    unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                    TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                    bool dynamic_expand_node = true );
    ///���캯��
    ZCE_Timer_Wheel();

    ///��������
    virtual ~ZCE_Timer_Wheel();
public:

    /*!
    * @brief      ��ʼ��
    * @return     int                   ����0��ʾ�ɹ��������ʾʧ��
    * @param[in]  num_timer_node        ��ʱ��NODE����������ʼ����ʱ������ã�
    * @param[in]  timer_length_mesc     ���ӱ�ʶ��ʱ�䳤�ȣ�msec�������ϵĲ�λ����= timer_length_mesc/timer_precision_mesc
    * @param[in]  timer_precision_mesc  ��ʱ���Ľ��ȣ�����msec��������ù�����Щ��ʱ���ᳬʱ��Ŵ��������̫С����Ҫ������ֵĲ�λ�ռ䣬
    * @param[in]  trigger_mode          ����ģʽ����ǽ��ʱ�ӻ���CPU TICK���ο� @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   �����ʼ����NODE�ڵ������������Ƿ��Զ���չ
    */
    int initialize(size_t num_timer_node,
                   unsigned int timer_length_mesc = DEFAULT_TIMER_LENGTH_MESC,
                   unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );

    /*!
    * @brief      ���õ�һ����ʱ�����ӿڲο���ACE����ƣ���������ʵ���������
    * @return     int           ���ض�ʱ��ID��>=0��ʶ�ɹ���-1��ʶʧ��
    * @param      timer_hdl     ����Ķ�ʱ�������������ص��Ķ���
    * @param[in]  action        һ��ָ�룬�ڶ�ʱ���������ò�������㣬
    * @param[in]  delay_time    ��һ�δ�����ʱ�䣬ΪʲôҪ������Ʋ��������Լ�����һ�£����������10:00���ŵ�6�׹㲥���
    * @param[in]  interval_time ��һ�δ����󣬺������ @a interval_time ��ʱ�����һ�δ���
    *                           �����������ZCE_Time_Value::ZERO_TIME_VALUE����ʶ����Ҫ����������
    */
    virtual int schedule_timer(ZCE_Timer_Handler *timer_hdl,
                               const void *action,
                               const ZCE_Time_Value &delay_time,
                               const ZCE_Time_Value &interval_time = ZCE_Time_Value::ZERO_TIME_VALUE) override;

    /*!
    * @brief      ȡ����ʱ��
    * @return     int      0��ʶ�ɹ�������ʧ��
    * @param      timer_id ��ʱ��ID
    */
    virtual int cancel_timer(int timer_id) override;

    /*!
    * @brief      ������ض�ʱ����NODE��������Ҳ���õײ��extend_node����
    * @return     int ����0��ʶ�ɹ�
    * @param[in]  num_timer_node Ҫ���õĶ�ʱ��NODE����
    * @param[out] old_num_node   ����ԭ���Ķ�ʱ��NODE����
    */
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node) override;

protected:

    ///�ڴ���һ�κ�Ҫ�Զ�ʱ���������¼���
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec) override;

    ///ȡ�õ�һ��Ԫ�أ�Ҳ���ǣ���С��ʱ��
    virtual int get_frist_nodeid(int &first_node_id);

    ///�ַ���ʱ��
    size_t dispatch_timer(const ZCE_Time_Value &now_time,
                          uint64_t now_trigger_msec);

    ///��Queue��TimerNode��
    void bind_wheel_listnode(int time_node_id);

    ///��Queue��TimerNode�����
    void unbind_wheel_listnode(int time_node_id);

protected:
    ///��ʱ���ܴ���ĺ�����
    unsigned int            timer_length_mesc_;

    ///��ʱ����������������������ö೤ʱ��Ķ�ʱ��
    size_t                  num_wheel_point_;
    ///
    std::vector<int>        timer_wheel_point_;

    ///��ǰ�����ʱ��㣬��Ϊ��һ�����ӣ������ʾ���ӿ�ʼ�ĵط���ÿ�δ�����ǰ��һ��
    size_t                  proc_wheel_start_;

    ///ʱ���������
    ARRAY_WHEEL_TIMER_NODE  wheel_node_list_;

    ///δ��������ʱ��ID��������dispatch_timer �����м�¼�����β�������timer id��
    ///��Ϊ��һ���������ӣ���������ܼ�¼��ʱ����ǳ�������һ�ܱ�ʾ��ʱ��ģ�
    ///��������Ϊ�Ƽ�Ȧ����ȥ��ģ�������������������������ڴ���ʱ���ַ�ʱ��Ҫ����һ��
    std::vector<int>        future_trigger_tid_;
};

#endif //#ifndef ZCE_LIB_TIMER_QUEUE_WHEEL_H_

