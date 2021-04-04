/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_heap.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��10��3��
* @brief      �ѵĶ�ʱ�����У����еĳ�ʱʱ���öѽ������������ж��Ƿ��ж�ʱ��Ҫ�����ͺܼ򵥣�
*             ֻ���ڶѶ��жϾ�OK��
*             ��˵�ŵ㣬Ч��Ҳ���У�һ��Ⱥ�ڵ������������㣬���Ҽ�ʹ���1ms���Ѷ�ʱ�������ܱ�֤
*             ������˳��
*             ���⣬���Ŀռ��WHEELҪС��
*             ȱ���ǣ�ÿ�η�����ʱ����ʵ��ÿ��NODE������ʱ�������ö�ʱ����ȡ����ʱ������Ҫ����
*             ���������������ļ�����O(log2N),NΪ��ǰ�ж��ٶ�ʱ����
*             �ַ���ʱ�������������ڷַ���ĵ�����
*
* @details    ���µ��������ݣ�������ACE�У���HEAP�Ķ�ʱ��ģ�ͣ�����10W����ʱ����ÿ����ʱ������10s
*             ������5���ӣ�������ѹ��������CPU.5%����
*             ���ǵ�ʵ�ֿ϶�С��ACE�����ģ�
*
* @note
*
*/
#ifndef ZCE_LIB_TIMER_QUEUE_HEAP_H_
#define ZCE_LIB_TIMER_QUEUE_HEAP_H_

#include "zce_timer_queue_base.h"
#include "zce_time_value.h"

//

/*!
*@brief     �ѵĶ�ʱ�����У����еĳ�ʱʱ���öѽ������������ж��Ƿ��ж�ʱ��Ҫ�����ͺܼ򵥣�
*           ֻ���ڶѶ��жϾ�OK��
*           ����ֲ�����ʱ�临�Ӷȶ�������O(log(2N)) �������
*
*/
class ZCE_Timer_Heap : public ZCE_Timer_Queue_Base
{

public:
    ///���캯��
    ZCE_Timer_Heap(size_t num_timer_node,
                   unsigned int timer_length_mesc = 1000,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );
    ///Ĭ�Ϲ��캯��
    ZCE_Timer_Heap();

    ///��������
    virtual ~ZCE_Timer_Heap();
public:

    /*!
    * @brief      ��ʼ��
    * @return     int                   ����0��ʶ��ʼ���ɹ��𣬷���ʧ��
    * @param[in]  num_timer_node        ��ʱ��NODE����������ʼ����ʱ������ã�
    * @param[in]  timer_precision_mesc  ��ʱ���Ľ��ȣ�����msec��������ù�����Щ��ʱ���ᳬʱ��Ŵ��������̫С����Ҫ������ֵĲ�λ�ռ䣬
    * @param[in]  trigger_mode          ����ģʽ����ǽ��ʱ�ӻ���CPU TICK���ο� @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   �����ʼ����NODE�ڵ������������Ƿ��Զ���չ
    */
    int initialize(size_t num_timer_node,
                   unsigned int timer_precision_mesc = 1000,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );

    /*!
    * @brief      ���õ�һ����ʱ�����ӿڲο���ACE����ƣ���������ʵ��������ˣ�����չ���࣬����ʵ������ӿ�
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
    * @brief      ȡ����ʱ������̳к����ʵ������ӿ�
    * @return     int      0��ʶ�ɹ�������ʧ��
    * @param[in]  timer_id ��ʱ��ID
    */
    virtual int cancel_timer(int timer_id) override;

    //dump����
    void dump();

    //�������ʮ�������NODE��������Ҳ���õײ��extend_node����
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node) override;

protected:

    /*!
    * @brief      �ڴ���һ�κ�Ҫ�Զ�ʱ���������¼���,�����ʵ�ֵ��麯��
    * @return     int              0��ʶ�ɹ�������ʧ��
    * @param      timer_id         ��ʱ��ID
    * @param      now_trigger_msec ��ǰ�Ĵ�����ĺ���������ͬģʽ�±�ﲻ̫һ��
    */
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec) override;

    /*!
    * @brief      �ַ���ʱ��
    * @return     size_t            ���طַ��Ĵ����Ķ�ʱ��������
    * @param      now_time          ��ǰ��ʱ�䣬ǽ��ʱ��
    * @param      now_trigger_msec  ��ǰ�����ĵ�ĺ����������ݴ���ģʽ��������岻һ��
    */
    virtual size_t dispatch_timer(const ZCE_Time_Value &now_time,
                                  uint64_t now_trigger_msec) override;

    /*!
    * @brief      ȡ�õ�һ��Ҫ�����Ķ�ʱ��NODE��Ҳ���ǣ�����Ĵ�����ʱ������̳к����ʵ������ӿ�
    * @return     int   0��ʶ�ɹ�������ʧ��
    * @param[out] timer_node_id
    */
    virtual int get_frist_nodeid(int &timer_node_id) override;

    //�����ǶѴ���ĺ���----------------------------------------------------------

    /*!
    * @brief      ����Ѱ�Һ��ʵ�λ��
    * @return     bool    �Ƿ����˵���
    * @param      heap_id ������Ԫ���ڶ��ϵ�ID
    */
    bool reheap_up(size_t heap_id);

    /*!
    * @brief      ����Ѱ�Һ��ʵ�λ��
    * @return     bool     �Ƿ����˵���
    * @param      heap_id  ������Ԫ���ڶ��ϵ�ID
    */
    bool reheap_down(size_t heap_id);

    /*!
    * @brief      ����һ��Timer NodeԪ�ص�����
    * @return     int           0��ʾ�ɹ�������ʧ��
    * @param      timer_node_id ��ӵĶ�ʱ��Timer Node ID
    */
    int add_nodeid(int timer_node_id);
    //
    /*!
    * @brief      �Ӷ���ɾ�����Timer NodeԪ�أ�
    * @return     int           0��ʾ�ɹ�������ʧ��
    * @param      timer_node_id ɾ���Ķ�ʱ��Timer Node ID
    * @note
    */
    int remove_nodeid(int timer_node_id);

protected:

    ///�ѵĴ�С����TIMER NODE�Ĵ�СӦ��һ��
    size_t               size_heap_ = 0;

    ///������һ����С�ѣ�������Timer NODE��ID
    std::vector <int>    timer_node_heap_;

    ///TIMER NODE��HEAPID�Ķ�Ӧ������Ƴɺ�TIMER NODEһ���࣬
    ///TIMER NODE��ID��Ҳ����TIMER ID���������±궨λ�ҵ��ѵ�λ��
    std::vector <int>    note_to_heapid_;

};

#endif //#ifndef ZCE_LIB_TIMER_QUEUE_HEAP_H_

