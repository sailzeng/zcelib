
#ifndef SOARING_LIB_SVRD_APP_MAIN_H_
#define SOARING_LIB_SVRD_APP_MAIN_H_

//�����ú�ʵ���ˣ�����д���ˣ���΢��ļ��У��ͷ��־���ʹ�ࡣ������ģ��ѡ�

namespace SOAR_LIB
{

template <class application_class, class  config_class, class timer_class >
int soar_svrd_main(int argc, const char *argv[])
{
    //�������쳣����Ϊ�����˲��õ���,�ر�����Win32�µ��ԡ�

    application_class::set_instance(new application_class());

    application_class::instance()->initialize(new config_class(),
                                              new timer_class );


    //��ʼ��־���޷������
    //ZCE_LOG(RS_INFO,"[framework] App init_instance start");

    //��ʼ��
    int ret = application_class::instance()->app_start(argc, argv);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] init_instance fail:%d|%s",
                application_class::instance()->get_app_runname(),
                typeid(*application_class::instance()).name(),
                ret,
                SOAR_RET::error_string(ret));
        return ret;
    }

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] init_instance sucess.",
            application_class::instance()->get_app_runname(),
            typeid(*application_class::instance()).name());


    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] run_instance start.",
            application_class::instance()->get_app_runname(),
            typeid(*application_class::instance()).name());
    //����
    ret = application_class::instance()->app_run();

    //��ʾ����ʧ��
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] run_instance fail:%d|%s .",
                application_class::instance()->get_app_runname(),
                typeid(*application_class::instance()).name(),
                ret,
                SOAR_RET::error_string(ret));
        //������˳�,���ǽ���exit_instance,����һ�����⣬����
        //return ret;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] run_instance sucess.",
                application_class::instance()->get_app_runname(),
                typeid(*application_class::instance()).name());
    }

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] exit_instance start.",
            application_class::instance()->get_app_runname(),
            typeid(*application_class::instance()).name());
    //�˳�����
    application_class::instance()->app_exit();

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] exit_instance start.",
            application_class::instance()->get_app_runname(),
            typeid(*application_class::instance()).name());

    //
    application_class::clean_instance();
    //�ر���־
    ZCE_Trace_LogMsg::clean_instance();

    return 0;

}

};

#endif //#ifndef SOARING_LIB_SVRD_APP_NONCTRL_H_

