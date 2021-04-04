/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_svrd_timer_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Monday, September 08, 2014
* @brief
*
*
* @details    �����㰴����Ϊ��µ�,�����㰴�޺��������
*
*
*
* @note
*      ��
*      �ݳ����վ�Ө
*      ����ɽ�º��в�������
*      �г�����С��
*      �з�ר�����
*      �����Ź�̷���ϵ�������
*      �����������Ϊ��п�
*      ������򿨺����㾫��
*      ������Ů��
*      �м��˵�����Щ������˵
*      ��������Ȼ���J
*      �м���Ϲ���е�û�Ķ�̬
*      �����㰴����Ϊ��µ�
*      �����㰴�޺��������
*      �����Ұ���
*
*      ����ɹ��������ˣ����
*      ��С���д��
*      ��ǰ����ǰŮ��
*      �м���û���й�����Ƭ��
*      �����������Ϊ��п�
*      ������򿨺����㾫��
*      ��Ǳˮ͵���д�С��
*      �к����й���
*      �����������ƶ�
*
*      �����㰴����Ϊ��µ�
*      �����㰴�޺��������
*      �����Ұ���
*
*      ����԰�û�����λԴ
*      �������Ļǰȡ����ȫ����
*      ˢ����ˢ����Ҫ�Ľ���
*      ���������ʵ�����ж���
*      �����㰴����Ϊ��µ�
*      �����㰴�޺��������
*      �����Ұ���
*
*/

#ifndef SOARING_LIB_TIMER_HANDLER_H_
#define SOARING_LIB_TIMER_HANDLER_H_


class Soar_Stat_Monitor;
class Server_Config_Base;


/*!
* @brief      ��������ܵĶ�ʱ��������
*             ���Դ���õ���ǰ��ʱ��
* @note
*/
class Server_Timer_Base : public ZCE_Timer_Handler
{

    friend class Soar_Svrd_Appliction;
public:

    ///���캯��,��Ϊ��ܵ���ƹ����ʱ�򲻳�ʼ��timer queue��
    Server_Timer_Base();
    ///��������
    ~Server_Timer_Base();

protected:

    // ���������
    void check_monitor(const ZCE_Time_Value &now_time);

    // ϵͳ������״̬����
    void report_status();

    ///ȡ�õ�ǰ��ʱ�䣬���ڶ�ʱ�侫��Ҫ�󲻸ߵĳ���
    ZCE_Time_Value gettimeofday();


protected:


    /*!
    * @brief      ��ʼ�������ϣ������APP�Ķ�ʱ�����ߵ����������ȣ����ڵ����������ǰ���
    * @return     virtual int
    * @param      queue
    */
    virtual int initialize(ZCE_Timer_Queue_Base *queue);


    /// ��ʱ����������
    virtual int timer_timeout(const ZCE_Time_Value &now_time,
                              const void *act = 0);

    ///��ʱ���ر�
    virtual int timer_close();

    ///����������ʱ���Ľ��ȣ�Ĭ����0.5sһ�Σ�������ò�������initializeǰ��������
    ///����USEC
    void set_heart_precision(const ZCE_Time_Value &precision);


    /*!
    * @brief      ����һ��APP�Ķ�ʱ��
    * @param[in]  interval ���ӵĶ�ʱ���ļ��
    * @param[in]  act      ���ӵĶ�ʱ���ı�ʾ
    */
    void add_app_timer(const ZCE_Time_Value &interval, const void *act);


protected:

    ///��������ʱ��ID,
    static const  int  SERVER_TIMER_ID[];

    ///APP Timer�����������
    static const size_t MAX_APP_TIMER_NUMBER = 6;

    ///Ĭ�������ľ���
    static const time_t DEF_TIMER_INTERVAL_USEC = 500000;

protected:

    ///�����ľ��ȣ�
    ZCE_Time_Value heart_precision_ = ZCE_Time_Value(0, DEF_TIMER_INTERVAL_USEC);

    ///������������heartbeat_counter_����0��ʼ�����Ǳ����һ��ģ����ʱ��ͷ�������
    uint64_t  heartbeat_counter_ = 1;

    ///�ӿ�ʼ���������ڵĺ������������ֵ��ͨ��heartbeat_counter_�;��ȵõ��ģ�
    ///����׼ȷ�����ڸ��ֳ��Լ���
    uint64_t  heart_total_mesc_ = 0;

    time_t last_check_ = 0;

    // ��ص�ʵ��
    Soar_Stat_Monitor *stat_monitor_ = NULL;


    ///���������⣬����������N��APP��ʱ�������Լ�����,��Ϊ������
    size_t zan_timer_num_ = 0;
    ///
    ZCE_Time_Value zan_timer_internal_[MAX_APP_TIMER_NUMBER];
    ///
    const void *zan_timer_act_[MAX_APP_TIMER_NUMBER] = {0};


public:
    ///��ǰʱ��
    static ZCE_Time_Value now_time_;

};

#endif // SOARING_LIB_TIMER_HANDLER_H_
