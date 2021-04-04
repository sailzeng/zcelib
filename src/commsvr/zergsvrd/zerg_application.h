
#ifndef ZERG_SERVER_APPLICATION_H_
#define ZERG_SERVER_APPLICATION_H_

#include "zerg_tcp_ctrl_handler.h"


class Zerg_Comm_Manager;



/*!
* @brief
*
* @note
*/
class Zerg_Service_App : public Soar_Svrd_Appliction
{


public:
    //����Ҫ͵͵����
    Zerg_Service_App();
    virtual ~Zerg_Service_App();

public:

    ///��ʼ��,����������������
    virtual int app_start(int argc, const char *argv[]);

    ///�����˳���������
    virtual int app_exit();

    ///����,���к���,�����򲻵���,�����˳�,Ϊ�˼ӿ췢�͵��ٶȣ��Զ����������˲�ͬ��΢����
    virtual int app_run();

public:

    ///ͨ�Ź�����
    Zerg_Comm_Manager       *zerg_comm_mgr_;

    ///OP KEY�ļ���ʱ���
    time_t                   conf_timestamp_;

};

//

#endif //_ZERG_SERVER_APPLICATION_H_

