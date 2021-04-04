
#ifndef ZERG_APPLICATION_TIMER_H_
#define ZERG_APPLICATION_TIMER_H_



/*!
* @brief      ZERG����Ķ�ʱ������ʱ�����ڸ��ִ���
*
* @note
*/
class  Zerg_App_Timer_Handler : public  Server_Timer_Base
{

public:
    //
    Zerg_App_Timer_Handler();
protected:
    //�Լ������Լ�������
    ~Zerg_App_Timer_Handler();

    ///��ʱ������
    virtual int timer_timeout(const ZCE_Time_Value &time, const void *arg);


public:

    ///ZERG��������ʱ��ID,
    static const  int  ZERG_TIMER_ID[];

protected:



    //��ʱ�����������ƫ��, �������з�����ͬʱ����
    unsigned int random_reload_point_;


};

#endif //ZERG_APPLICATION_TIMER_H_

