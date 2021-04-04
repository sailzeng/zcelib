

#include "ogre_predefine.h"
#include "ogre_app_timer.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_comm_manager.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_ip_restrict.h"
#include "ogre_application.h"

/****************************************************************************************************
class  Ogre_Service_App
****************************************************************************************************/

//����Ҫ͵͵����
Ogre_Service_App::Ogre_Service_App()
{
}

Ogre_Service_App::~Ogre_Service_App()
{
}

//
int Ogre_Service_App::app_start(int argc, const char *argv[])
{
    int ret = 0;
    ZCE_TRACE_FUNC_RETURN(RS_INFO, &ret);
    //
    ret = Soar_Svrd_Appliction::app_start(argc, argv);

    if (ret != 0)
    {
        return ret;
    }

    Ogre_Server_Config *config = reinterpret_cast<Ogre_Server_Config *>(config_base_);

    size_t max_accept = 0, max_connect = 0, max_peer = 0;
    Ogre_TCP_Svc_Handler::get_maxpeer_num(max_accept, max_connect);
    ZCE_LOG(RS_INFO, "Ogre max accept number :%u,max connect number:%u.\n",
            max_accept, max_connect);
    max_peer = max_accept + max_connect + 16;

    //�������ļ�û�ж�ȡ������ʱ��,ֻ��ʾ������Ϣ

    //ʹ����־����,�õ������ļ�,���ܵõ���־�ļ�����

    //�ȱ����ʼ��Buffer Storage,����Ϊ�����������1/5,����512��
    size_t size_list = (max_peer / 32 < 512 ) ? 512 : max_peer / 32;
    Ogre_Buffer_Storage::instance()->init_buffer_list(size_list);

    //Ogre_Comm_Manger ��ʼ��
    ret = Ogre_Comm_Manger::instance()->get_config(config);
    if (ret != 0 )
    {
        ZCE_LOG(RS_ERROR, "Ogre_Comm_Manger::instance()->init_comm_manger() fail !\n");
        return SOAR_RET::ERR_OGRE_INIT_COMM_MANAGER;
    }

    ZCE_LOG(RS_INFO, "%s success.Have fun, my brother!!!\n", __ZCE_FUNC__);
    return 0;
}

//�����˳���������
int Ogre_Service_App::app_exit()
{
    //
    Ogre_Comm_Manger::instance()->uninit_comm_manger();
    Ogre_Comm_Manger::clean_instance();

    Ogre_Buffer_Storage::instance()->uninit_buffer_list();

    //������˳�
    Soar_Svrd_Appliction::app_exit();

    //
    ZCE_LOG(RS_INFO, "%s Succ.Have Fun.!!!\n", __ZCE_FUNC__);

    return 0;
}


//���к���,�����򲻵���,�����˳�.
//����Ҫ�ĺ���, ����Ҳ���
int Ogre_Service_App::app_run()
{
    //����N�κ�,SLEEP��ʱ����
    const unsigned int  IDLE_SLEEP_INTERVAL = 512;

    size_t numevent = 0;
    unsigned int idle = 0;
    size_t procframe = 0;
    ZCE_LOG(RS_INFO, "Ogre_Service_App::Run Start.\n");
    //microsecond
    const int INTERVAL_MACRO_SECOND = 10000;

    ZCE_Time_Value sleeptime(0, INTERVAL_MACRO_SECOND);
    ZCE_Time_Value interval(0, INTERVAL_MACRO_SECOND);

    for (; app_run_;)
    {
        //ÿ�ζ��������ʼ��ZCE_Time_Value����,��Ҫ����.
        interval.usec(INTERVAL_MACRO_SECOND);
        ZCE_Reactor::instance()->handle_events(&interval, &numevent);

        //ȡ�÷�����������
        Ogre_Comm_Manger::instance()->get_all_senddata_to_write(procframe);
        //û�д����κ��¼�

        //������¼������˾ͼ���
        if (numevent > 0 || procframe > 0)
        {
            idle = 0;
            continue;
        }
        else
        {
            ++idle;
        }

        //����Ը��ݿ���״̬����һЩ�Լ�������,
        if (idle > IDLE_SLEEP_INTERVAL )
        {
            zce::sleep(sleeptime);
        }
    }

    return 0;
}



