

#ifndef OGRE_SERVER_APPLICATION_H_
#define OGRE_SERVER_APPLICATION_H_

/****************************************************************************************************
class  Ogre_Service_App
****************************************************************************************************/
class Ogre_Service_App : public Soar_Svrd_Appliction
{

public:

    //����Ҫ͵͵����
    Ogre_Service_App();
    virtual ~Ogre_Service_App();

public:

    ///��ʼ��,����������������
    virtual int app_start(int argc, const char *argv[]);

    ///�����˳���������
    virtual int app_exit();

    ///����,���к���,�����򲻵���,�����˳�,Ϊ�˼ӿ췢�͵��ٶȣ��Զ����������˲�ͬ��΢����
    virtual int app_run();




};

#endif //OGRE_SERVER_APPLICATION_H_

