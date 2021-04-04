#ifndef WORMHOLE_APPLICATION_H_
#define WORMHOLE_APPLICATION_H_

#include "wormhole_proxyprocess.h"



/*!
* @brief
*
* @note
*/
class Wormhole_Proxy_App : public Soar_SvrdApp_ZergBus
{

public:
    // �Լ��ĵ���͵͵���ţ�
    Wormhole_Proxy_App();
    ~Wormhole_Proxy_App();

public:


    /*!
    * @brief      �����յ���APPFRAME,
    * @return     int
    * @param      recv_frame  �����Frame����ʹ��const��ԭ������ΪΪ�˼ӿ��ٶȣ��ܶ�ط���ֱ�ӽ�recv_frame�޸�
    * @note       app_run������Comm_SvrdApp_BUS�����Ѿ�ʵ���ˣ�
    */
    virtual int process_recv_frame(Zerg_App_Frame *recv_frame);

    ///��ʼ��,����������������
    virtual int app_start(int argc, const char *argv[]);

    ///�����˳���������
    virtual int app_exit();

protected:
    ///���¼�������
    virtual int reload_config();

protected:

    /// �����PROXY�Ľӿ�
    Interface_WH_Proxy *interface_proxy_ = NULL;



};

#endif //WORMHOLE_APPLICATION_H_

