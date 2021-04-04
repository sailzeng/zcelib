/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_handler_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008��10��6��
* @brief      Timer����Ļ���
*
*
* @details
*
*
*
* @note       ΪʲôҪ������NODE����һ���ļ��м䣬��Ϊ�ҿ���д�����Ӧ��
*
*/

#ifndef ZCE_LIB_TIMER_HANDLER_BASE_H_
#define ZCE_LIB_TIMER_HANDLER_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_time_value.h"

class ZCE_Timer_Queue_Base;

/******************************************************************************************
class ZCE_Timer_Handler
******************************************************************************************/
class ZCE_Timer_Handler
{

protected:

    //���캯������������
    ZCE_Timer_Handler(ZCE_Timer_Queue_Base *timer_queue);
    ZCE_Timer_Handler();
    virtual ~ZCE_Timer_Handler();

protected:
    //
    ZCE_Timer_Queue_Base    *timer_queue_ = nullptr;
    //

public:

    //��ʱ����
    virtual int timer_timeout(const ZCE_Time_Value &now_timenow_time,
                              const void *act = 0)  = 0;
    //��ʱ���ر�
    virtual int timer_close();

    //
    ZCE_Timer_Queue_Base *timer_queue();

    //
    void timer_queue(ZCE_Timer_Queue_Base *set_timer_queue);

    //���ǵİ��죬�Ҿ��ö���Timer�Ĵ����ǲ��ÿ���ʵ��һ��handle_close�ˡ�
    //handle_close
};

#endif //# ZCE_LIB_TIMER_HANDLER_BASE_H_
